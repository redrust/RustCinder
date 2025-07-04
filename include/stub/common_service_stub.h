#ifndef _COMMON_SERVICE_STUB_H_
#define _COMMON_SERVICE_STUB_H_

#include <muduo/net/EventLoop.h>
#include <muduo/net/protorpc/RpcChannel.h>

#include "common_service.pb.h"

namespace RustCinder
{
    class CommonServiceStub
    {
    public:
        CommonServiceStub(muduo::net::RpcChannelPtr channel, muduo::net::EventLoop* eventLoop);
        ~CommonServiceStub();

        void startAll();
        void stopAll();

        void startSyncServerTime();
        void stopSyncServerTime();
        void startPing();
        void stopPing();

        void syncServerTime();
        void syncServerTimeCallback(uint64_t clientSendTs, common_service::SyncTimeResponse* response);

        void ping();
        void pingCallback(common_service::PongResponse* response);
    private:
        common_service::CommonService::Stub* m_stub = nullptr;
        muduo::net::EventLoop* m_eventLoop = nullptr;
        muduo::net::TimerId m_syncServerTimeTimerId;
        muduo::net::TimerId m_pingTimerId;
    };
}

#endif // _COMMON_SERVICE_STUB_H_