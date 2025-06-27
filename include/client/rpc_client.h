#ifndef _RPC_CLIENT_H_
#define _RPC_CLIENT_H_

#include <muduo/net/EventLoop.h>

#include "common_service.pb.h"

#include "common/noncopyable.h"
#include "client/tcp_service.h"
#include "client/login_service_stub.h"
#include "client/common_service_stub.h"

namespace RustCinder 
{
    class RpcClient : public NonCopyable
    {
    public:
        RpcClient() = default;
        ~RpcClient();

        void init();
        void start();
        void stop();

        muduo::net::EventLoop* getEventLoop() const { return m_eventLoop; }
        LoginServiceStub* getLoginServiceStub() const { return m_loginServiceStub; }
        CommonServiceStub* getCommonServiceStub() const { return m_commonServiceStub; }

    private:
        muduo::net::EventLoop* m_eventLoop = nullptr;
        TcpService* m_tcpService = nullptr;
        LoginServiceStub* m_loginServiceStub = nullptr;
        CommonServiceStub* m_commonServiceStub = nullptr;
    };
}

#endif // _RPC_CLIENT_H_