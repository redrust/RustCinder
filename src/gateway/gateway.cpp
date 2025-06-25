#include "gateway/gateway.h"
#include "gateway/gateway_service.h"

namespace RustCinder
{
    Gateway::~Gateway()
    {
        if (m_isRunning)
        {
            stop();
        }
        m_eventLoop.reset();
        m_rpcServer.reset();
        m_service.reset();
        m_isInitialized = false;
        m_isRunning = false;
    }

    void Gateway::init()
    {
        if (m_isInitialized)
        {
            return;
        }
        m_eventLoop = std::make_shared<muduo::net::EventLoop>();
        m_listenAddr = std::make_shared<muduo::net::InetAddress>(9981);
        m_rpcServer = std::make_shared<muduo::net::RpcServer>(m_eventLoop.get(), *m_listenAddr);
        m_service = std::make_shared<GatewayService>();

        // 注册RPC服务
        m_rpcServer->registerService(m_service.get());

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