#include <muduo/base/Logging.h>

#include "gateway/gateway.h"
#include "gateway/gateway_service.h"
#include "service/common_service.h"

namespace RustCinder
{
    Gateway::~Gateway()
    {
        if (m_isRunning)
        {
            stop();
        }
        if(m_eventLoop)
        {
            delete m_eventLoop; // Clean up EventLoop
        }
        m_eventLoop = nullptr;

        if(m_rpcServer)
        {
            delete m_rpcServer; // Clean up RpcServer
        }
        m_rpcServer = nullptr;
        
        m_isInitialized = false;
        m_isRunning = false;
    }

    void Gateway::init()
    {
        if (m_isInitialized)
        {
            return;
        }
        m_eventLoop = new muduo::net::EventLoop();
        m_listenAddr = muduo::net::InetAddress(9981);
        m_rpcServer = new net::RpcServer();

        m_rpcServer->init(m_eventLoop, m_listenAddr);

        m_rpcServer->registerService(new GatewayService());
        m_rpcServer->registerService(new Service::CommonService());

        m_isInitialized = true;
    }

    void Gateway::start()
    {
        if (m_isInitialized == false || m_isRunning == true)
        {
            return;
        }

        m_isRunning = true;
        m_rpcServer->start();
        m_eventLoop->loop();
    }

    void Gateway::stop()
    {
        if (m_isRunning == false)
        {
            return;
        }
        m_isRunning = false;
        m_eventLoop->quit();
    }
}