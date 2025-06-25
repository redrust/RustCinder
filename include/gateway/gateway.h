#ifndef _GATEWAY_H_
#define _GATEWAY_H_

#include <memory>

#include <muduo/net/EventLoop.h>
#include <muduo/net/protorpc/RpcServer.h>

#include "common/noncopyable.h"
#include "common/nonmoveable.h"

namespace RustCinder 
{
    class GatewayService;
    class Gateway : public NonCopyable, public NonMoveable
    {
    public:
        Gateway() = default;
        virtual ~Gateway();
        
        void init();
        void start();
        void stop();
    private:
        std::shared_ptr<muduo::net::EventLoop> m_eventLoop;
        std::shared_ptr<muduo::net::InetAddress> m_listenAddr;
        std::shared_ptr<muduo::net::RpcServer> m_rpcServer;
        std::shared_ptr<GatewayService> m_service;
        bool m_isRunning = false;
        bool m_isInitialized = false;
    };
}
#endif // _GATEWAY_H_