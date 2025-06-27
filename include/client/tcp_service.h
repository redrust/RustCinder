#ifndef _TCP_SERVICE_H_
#define _TCP_SERVICE_H_

#include <muduo/net/EventLoop.h>
#include <muduo/net/protorpc/RpcServer.h>
#include <muduo/net/protorpc/RpcChannel.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/InetAddress.h>

#include "common/noncopyable.h"

namespace RustCinder
{
    
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
}

#endif // _TCP_SERVICE_H_