#ifndef _RPC_SERVER_H_
#define _RPC_SERVER_H_

#include <google/protobuf/service.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include "common/noncopyable.h"
#include "common/nonmoveable.h"
#include "net/client_mailbox.h"
#include "service/service_manager.h"

namespace RustCinder::net
{
    class RpcServer : public NonCopyable, NonMoveable
    {
    public:
        RpcServer() = default;
        ~RpcServer();

        void init(muduo::net::EventLoop* loop, const muduo::net::InetAddress& listenAddr);
        void start();
        void stop();
        void registerService(google::protobuf::Service* service);

    private:
        void onNewConnection(const muduo::net::TcpConnectionPtr& conn);

    private:
        muduo::net::EventLoop* m_eventLoop = nullptr;
        muduo::net::TcpServer* m_tcpServer = nullptr;
        muduo::net::InetAddress m_listenAddr;
        std::unordered_map<muduo::net::RpcChannelPtr, ClientMailBox::ClientMailBoxPtr> m_clientMailboxes;
        Service::ServiceManager* m_serviceManager;

        bool m_isRunning = false;
        bool m_isInitialized = false;
    };
}

#endif // _RPC_SERVER_H_