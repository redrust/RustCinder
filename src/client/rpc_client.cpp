#include <muduo/base/Logging.h>
#include <google/protobuf/stubs/callback.h>

#include "client/rpc_client.h"

namespace RustCinder 
{
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

        std::string serverAddr = "127.0.0.1"; // Default server address
        uint16_t serverPort = 9981; // Default port for the RPC server
        m_tcpService = new TcpService();
        m_tcpService->init(m_eventLoop, serverAddr, serverPort, "TcpClient");

        m_loginServiceStub = new LoginServiceStub(m_tcpService->getChannel());
        m_commonServiceStub = new CommonServiceStub(m_tcpService->getChannel(), m_eventLoop);
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
        if(m_tcpService)
        {
            delete m_tcpService; // Clean up TcpService
            m_tcpService = nullptr;
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
    }
}