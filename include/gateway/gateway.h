#ifndef _GATEWAY_H_
#define _GATEWAY_H_

#include <memory>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/protorpc/RpcServer.h>

#include "common/noncopyable.h"
#include "common/nonmoveable.h"

namespace RustCinder 
{
    class Gateway : public NonCopyable, public NonMoveable
    {
    public:
        Gateway() = default;
        virtual ~Gateway();
        
        void init();
        void start();
        void stop();
        void registerService(std::string serviceName, google::protobuf::Service* service);
    private:
        muduo::net::EventLoop* m_eventLoop;
        muduo::net::InetAddress m_listenAddr;
        muduo::net::RpcServer* m_rpcServer;
        std::unordered_map<std::string, google::protobuf::Service*> m_services;
        bool m_isRunning = false;
        bool m_isInitialized = false;
    };
}
#endif // _GATEWAY_H_