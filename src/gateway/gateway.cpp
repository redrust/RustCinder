#include <muduo/base/Logging.h>

#include "gateway/gateway.h"
#include "gateway/gateway_service.h"
#include "common/common_service.h"

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
        
        for(auto& servicePair : m_services)
        {
            if (servicePair.second)
            {
                delete servicePair.second; // Clean up each GatewayService
            }
            servicePair.second = nullptr;
        }
        m_services.clear(); // Clean up GatewayService

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
        m_rpcServer = new muduo::net::RpcServer(m_eventLoop, m_listenAddr);

        registerService("GatewayService", new GatewayService());
        registerService("CommonService", new CommonService());

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

    void Gateway::registerService(std::string serviceName, google::protobuf::Service* service)
    {
        if (m_services.find(serviceName) != m_services.end())
        {
            LOG_ERROR << "Service " << serviceName << " is already registered.";
            return; // Service already registered
        }
        if (service == nullptr)
        {
            LOG_ERROR << "Cannot register a null service.";
            return; // Cannot register a null service
        }
        m_services[serviceName] = service;
        m_rpcServer->registerService(service);
        LOG_INFO << "Service " << serviceName << " registered successfully.";
    }
}