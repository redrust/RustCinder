#include <muduo/base/Logging.h>

#include "net/tcp_client.h"

namespace RustCinder::net
{
    TcpClient::~TcpClient()
    {
        m_eventLoop = nullptr;
        m_channel.reset();
        m_isConnected = false;
        if(m_tcpClient)
        {
            m_tcpClient->disconnect(); // Disconnect the TcpClient
            // m_tcpClient->stop(); // Stop the TcpClient
            delete m_tcpClient; // Clean up TcpClient
        }
        LOG_INFO << getServiceName() << " destroyed.";
    }

    void TcpClient::init(muduo::net::EventLoop* loop, const std::string& serverAddr, uint16_t serverPort, const std::string& serviceName)
    {
        if (m_eventLoop)
        {
            LOG_ERROR << serviceName <<" is already initialized.";
            return; // Already initialized
        }
        m_eventLoop = loop;
        m_serverAddr = muduo::net::InetAddress(serverAddr, serverPort);
        m_tcpClient = new muduo::net::TcpClient(loop, m_serverAddr, serviceName);
        m_channel = std::make_shared<muduo::net::RpcChannel>();
        m_serviceName = serviceName;

        m_tcpClient->setConnectionCallback(
            std::bind(&TcpClient::handleConnection, this, std::placeholders::_1));
        m_tcpClient->setMessageCallback(
            std::bind(&muduo::net::RpcChannel::onMessage, m_channel.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    void TcpClient::connect()
    {
        if (!m_eventLoop)
        {
            LOG_ERROR << getServiceName() << " EventLoop is not initialized.";
            return; // EventLoop is not initialized
        }
        if (m_tcpClient && !m_isConnected)
        {
            LOG_INFO << getServiceName() << " connecting to server: " << m_serverAddr.toIpPort();
            m_tcpClient->connect(); // Connect the TcpClient
        }
    }
    
    void TcpClient::handleConnection(const muduo::net::TcpConnectionPtr& conn)
    {
        if (conn->connected())
        {
            LOG_INFO << getServiceName() <<" connected to server: " << conn->peerAddress().toIpPort();
            m_isConnected = true;
            m_channel->setConnection(conn);
        }
        else
        {
            m_channel->setConnection(nullptr);
            LOG_INFO << getServiceName() << " disconnected from server.";
        }
    }
}