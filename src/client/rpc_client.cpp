#include <muduo/base/Logging.h>
#include <google/protobuf/stubs/callback.h>

#include "client/rpc_client.h"

namespace RustCinder {

    TcpService::~TcpService()
    {
        if(m_tcpClient)
        {
            m_tcpClient->disconnect(); // Disconnect the TcpClient
            m_tcpClient->stop(); // Stop the TcpClient
            delete m_tcpClient; // Clean up TcpClient
        }
        m_eventLoop = nullptr;
        m_channel.reset();
        m_isConnected = false;
        LOG_INFO << getServiceName() << " destroyed.";
    }

    void TcpService::init(muduo::net::EventLoop* loop, const std::string& serverAddr, uint16_t serverPort, const std::string& serviceName)
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
            std::bind(&TcpService::handleConnection, this, std::placeholders::_1));
        m_tcpClient->setMessageCallback(
            std::bind(&muduo::net::RpcChannel::onMessage, m_channel.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        
        m_tcpClient->connect();
    }

    void TcpService::handleConnection(const muduo::net::TcpConnectionPtr& conn)
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

    GatewayServiceStub::~GatewayServiceStub()
    {
        if (m_loginServiceStub)
        {
            delete m_loginServiceStub; // Clean up LoginServiceStub
        }
        if (m_tcpService)
        {
            delete m_tcpService; // Clean up TcpService
        }
    }

    void GatewayServiceStub::init(muduo::net::EventLoop* loop, const std::string& serverAddr, uint16_t serverPort)
    {
        if (m_tcpService)
        {
            LOG_ERROR << "GatewayServiceStub is already initialized.";
            return; // Already initialized
        }
        m_tcpService = new TcpService();
        m_tcpService->init(loop, serverAddr, serverPort, "GatewayServiceStub");
        m_loginServiceStub = new gateway_service::LoginService::Stub(m_tcpService->getChannel().get());
    }

    RpcClient::~RpcClient()
    {
        if (m_eventLoop)
        {
            stop();
            delete m_eventLoop; // Clean up EventLoop
        }
        m_eventLoop = nullptr;
    }

    void RpcClient::init()
    {
        if (m_eventLoop)
        {
            LOG_ERROR << "[RpcClient::init] RpcClient is already initialized.";
            return; // Already initialized
        }
        m_eventLoop = new muduo::net::EventLoop();
        m_gatewayServiceStub = new GatewayServiceStub();
        std::string serverAddr = "127.0.0.1"; // Default server address
        uint16_t serverPort = 9981; // Default port for the RPC server
        m_gatewayServiceStub->init(m_eventLoop, serverAddr, serverPort);
    }

    void RpcClient::start()
    {
        LOG_INFO << "Starting RPC client...";
        m_eventLoop->loop();
    }

    void RpcClient::stop()
    {
        LOG_INFO << "Stopping RPC client...";
        if(m_eventLoop)
        {
            m_eventLoop->quit();
        }
    }

    void RpcClient::Login(const std::string& account, const std::string& password, 
        google::protobuf::Closure* done)
    {
        if (!m_eventLoop)
        {
            LOG_ERROR << "Event loop is not initialized.";
            return;
        }

        gateway_service::LoginRequest request;
        request.set_account(account);
        request.set_password(password);

        gateway_service::LoginResponse* response = new gateway_service::LoginResponse;
        google::protobuf::RpcController* controller = nullptr; // Use default controller

        if(done == nullptr)
        {
            done = google::protobuf::NewCallback(this, &RpcClient::LoginCallback, response);
        }

        m_gatewayServiceStub->loginService()->Login(controller, &request, response, done);
    }

    void RpcClient::LoginCallback(gateway_service::LoginResponse* response)
    {
        if(response->status() == gateway_service::LoginResponse::SUCCESS)
        {
            LOG_INFO << "Login successful: ";
        }
    }
}