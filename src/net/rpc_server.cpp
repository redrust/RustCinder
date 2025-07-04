
#include "muduo/net/protorpc/RpcServer.h"

#include "muduo/base/Logging.h"
#include "muduo/net/protorpc/RpcChannel.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>

#include "net/rpc_server.h"

namespace RustCinder::net
{
    RpcServer::~RpcServer()
    {
        if(m_tcpServer)
        {
            delete m_tcpServer; // Clean up TcpServer
            m_tcpServer = nullptr;
        }

        if(m_serviceManager)
        {
            delete m_serviceManager; // Clean up ServiceManager
            m_serviceManager = nullptr;
        }

        m_eventLoop = nullptr;
        m_isRunning = false;
        m_isInitialized = false;
    }

    void RpcServer::init(muduo::net::EventLoop* loop, const muduo::net::InetAddress& listenAddr)
    {
        if(m_isInitialized)
        {
            return;
        }
        m_eventLoop = loop;
        m_listenAddr = listenAddr;
        m_tcpServer = new muduo::net::TcpServer(m_eventLoop, m_listenAddr, "RpcServer");
        m_serviceManager = new Service::ServiceManager();

        m_tcpServer->setConnectionCallback(
            std::bind(&RpcServer::onNewConnection, this, std::placeholders::_1));
        
        m_isInitialized = true;
    }

    void RpcServer::registerService(google::protobuf::Service* service)
    {
        if(!service)
        {
            LOG_ERROR << "[RpcServer::registerService] Service is null.";
            return;
        }

        m_serviceManager->registerService(service);
        LOG_INFO << "[RpcServer::registerService] Registered service: " 
                 << Service::ServiceManager::getServiceName(service);
    }

    void RpcServer::start()
    {
        if(m_isRunning || !m_isInitialized)
        {
            return;
        }
        m_tcpServer->start();
    }
    
    void RpcServer::onNewConnection(const muduo::net::TcpConnectionPtr& conn)
    {
        LOG_INFO << "RpcServer - " << conn->peerAddress().toIpPort() << " -> "
            << conn->localAddress().toIpPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");
        if (conn->connected())
        {
            muduo::net::RpcChannelPtr channel(new muduo::net::RpcChannel(conn));
            channel->setServices(&m_serviceManager->getServices());
            conn->setMessageCallback(
                std::bind(&muduo::net::RpcChannel::onMessage, muduo::get_pointer(channel), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            conn->setContext(channel);

            ClientMailBox::ClientMailBoxPtr clientMailbox(new ClientMailBox());
            clientMailbox->init(channel, m_eventLoop);
            clientMailbox->start();
            m_clientMailboxes[channel] = clientMailbox;
        }
        else
        {
            auto channel = boost::any_cast<muduo::net::RpcChannelPtr>(conn->getContext());
            auto it = m_clientMailboxes.find(channel);
            if (it != m_clientMailboxes.end())
            {
                it->second->stop();
                m_clientMailboxes.erase(it);
            }

            conn->setContext(muduo::net::RpcChannelPtr());
        }
    }

}