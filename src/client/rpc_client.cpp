#include <muduo/base/Logging.h>
#include <google/protobuf/stubs/callback.h>

#include "client/rpc_client.h"
#include "service/common_service.h"

namespace RustCinder 
{
    RpcClient::~RpcClient()
    {
        if (m_eventLoop)
        {
            stop();
            delete m_eventLoop; // Clean up EventLoop
            m_eventLoop = nullptr;
        }
        if(m_loginServiceStub)
        {
            delete m_loginServiceStub; // Clean up LoginServiceStub
            m_loginServiceStub = nullptr;
        }
        if(m_commonServiceStub)
        {
            delete m_commonServiceStub; // Clean up CommonServiceStub
            m_commonServiceStub = nullptr;
        }
        if(m_tcpClient)
        {
            delete m_tcpClient; // Clean up TcpClient
            m_tcpClient = nullptr;
        }
        LOG_INFO << "RpcClient destroyed.";
    }

    void RpcClient::init()
    {
        if (m_eventLoop)
        {
            LOG_ERROR << "[RpcClient::init] RpcClient is already initialized.";
            return; // Already initialized
        }
        m_eventLoop = new muduo::net::EventLoop();

        std::string serverAddr = "127.0.0.1"; // Default server address
        uint16_t serverPort = 9981; // Default port for the RPC server
        m_tcpClient = new net::TcpClient();
        m_tcpClient->init(m_eventLoop, serverAddr, serverPort, "TcpClient");

        auto channel = m_tcpClient->getChannel();
        m_loginServiceStub = new LoginServiceStub(channel);
        m_commonServiceStub = new CommonServiceStub(channel, m_eventLoop);

        m_serviceManager = new Service::ServiceManager();
        m_serviceManager->registerService(new Service::CommonService());

        channel->setServices(&m_serviceManager->getServices());
    }

    void RpcClient::start()
    {
        LOG_INFO << "Starting RPC client...";
        m_tcpClient->connect(); // Connect the TcpClient
        m_commonServiceStub->startAll(); // Start the CommonServiceStub
        m_eventLoop->loop();
    }

    void RpcClient::stop()
    {
        LOG_INFO << "Stopping RPC client...";
        m_commonServiceStub->stopAll(); // Stop the CommonServiceStub
        if(m_eventLoop)
        {
            m_eventLoop->quit();
        }
    }
}