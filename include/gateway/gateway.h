#ifndef _GATEWAY_H_
#define _GATEWAY_H_

#include <memory>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
#include <muduo/net/EventLoop.h>

#include "common/noncopyable.h"
#include "common/nonmoveable.h"
#include "net/rpc_server.h"

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
    private:
        muduo::net::EventLoop* m_eventLoop;
        muduo::net::InetAddress m_listenAddr;
        net::RpcServer* m_rpcServer;
        bool m_isRunning = false;
        bool m_isInitialized = false;
    };
}
#endif // _GATEWAY_H_