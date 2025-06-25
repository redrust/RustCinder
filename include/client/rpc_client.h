#ifndef _RPC_CLIENT_H_
#define _RPC_CLIENT_H_

#include <memory>

#include <muduo/net/EventLoop.h>
#include <muduo/net/protorpc/RpcServer.h>
#include <muduo/net/protorpc/RpcChannel.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/InetAddress.h>

#include "gateway_service.pb.h"

#include "common/noncopyable.h"

namespace RustCinder {
    class TcpService : public NonCopyable
    {
    public:
        TcpService() = default;
        virtual ~TcpService();
        void handleConnection(const muduo::net::TcpConnectionPtr& conn);
        bool isConnected() const { return m_isConnected; }

        void init(muduo::net::EventLoop* loop, const std::string& serverAddr, uint16_t serverPort, const std::string& serviceName);

        const std::string& getServiceName() const { return m_serviceName; }
        muduo::net::RpcChannelPtr getChannel() const { return m_channel; }
    protected:
        muduo::net::EventLoop* m_eventLoop = nullptr;
        muduo::net::TcpClient* m_tcpClient = nullptr;
        muduo::net::RpcChannelPtr m_channel;
        muduo::net::InetAddress m_serverAddr;
        bool m_isConnected = false;
        std::string m_serviceName = "TcpService";
    };

    class GatewayServiceStub : public NonCopyable
    {
    public:
        GatewayServiceStub() = default;
        ~GatewayServiceStub();

        void init(muduo::net::EventLoop* loop, const std::string& serverAddr, uint16_t serverPort);
        gateway_service::LoginService::Stub* loginService() const { return m_loginServiceStub; }
        const TcpService* tcpService() const { return m_tcpService; }
    private:
        gateway_service::LoginService::Stub* m_loginServiceStub = nullptr;
        TcpService* m_tcpService = nullptr;
    };


    class RpcClient : public NonCopyable
    {
    public:
        RpcClient() = default;
        ~RpcClient();

        void init();
        void start();
        void stop();

        muduo::net::EventLoop* getEventLoop() const { return m_eventLoop; }
        const GatewayServiceStub* getGatewayServiceStub() const { return m_gatewayServiceStub; }

        void Login(const std::string& account, const std::string& password, 
                   google::protobuf::Closure* done = nullptr);
        void LoginCallback(gateway_service::LoginResponse* response);

    private:
        muduo::net::EventLoop* m_eventLoop = nullptr;
        GatewayServiceStub* m_gatewayServiceStub = nullptr;
    };
}

#endif // _RPC_CLIENT_H_