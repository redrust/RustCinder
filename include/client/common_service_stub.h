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
        CommonServiceStub(muduo::net::RpcChannelPtr channel, muduo::net::EventLoop* eventLoop)
            : m_stub(new common_service::CommonService::Stub(channel.get())), m_eventLoop(eventLoop) 
            {
                m_eventLoop->runEvery(5, std::bind(&CommonServiceStub::syncServerTime, this));
            }
        ~CommonServiceStub() { delete m_stub; }

        void syncServerTime();

        void syncServerTimeCallback(uint64_t clientSendTs, common_service::SyncTimeResponse* response);
    private:
        common_service::CommonService::Stub* m_stub = nullptr;
        muduo::net::EventLoop* m_eventLoop = nullptr;

    };
}

#endif // _COMMON_SERVICE_STUB_H_