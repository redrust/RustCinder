#include <muduo/base/Logging.h>

#include "stub/common_service_stub.h"
#include "utils/time_util.h"

namespace RustCinder
{
    CommonServiceStub::CommonServiceStub(muduo::net::RpcChannelPtr channel, muduo::net::EventLoop* eventLoop)
        : m_stub(new common_service::CommonService::Stub(channel.get())), m_eventLoop(eventLoop) 
    {
    }

    CommonServiceStub::~CommonServiceStub()
    {
        stopAll();
        if (m_stub)
        {
            delete m_stub; // Clean up the stub
            m_stub = nullptr;
        }
    }

    void CommonServiceStub::startAll()
    {
        startSyncServerTime();
        startPing();
        LOG_INFO << "CommonServiceStub started.";
    }

    void CommonServiceStub::stopAll()
    {
        LOG_INFO << "Stopping CommonServiceStub...";
        stopSyncServerTime();
        stopPing();
        LOG_INFO << "CommonServiceStub stopped.";
    }

    void CommonServiceStub::startSyncServerTime()
    {
        m_syncServerTimeTimerId = m_eventLoop->runEvery(5, std::bind(&CommonServiceStub::syncServerTime, this));
    }

    void CommonServiceStub::stopSyncServerTime()
    {
        m_eventLoop->cancel(m_syncServerTimeTimerId);
    }

    void CommonServiceStub::startPing()
    {
        m_pingTimerId = m_eventLoop->runEvery(1, std::bind(&CommonServiceStub::ping, this));
    }

    void CommonServiceStub::stopPing()
    {
        m_eventLoop->cancel(m_pingTimerId);
    }

    void CommonServiceStub::syncServerTime()
    {
        common_service::SyncTimeRequest request;
        auto clientSendTs = TimeUtil::getNowMs();
        common_service::SyncTimeResponse* response = new common_service::SyncTimeResponse;
        auto done = google::protobuf::NewCallback(this, &CommonServiceStub::syncServerTimeCallback, clientSendTs, response);
        m_stub->syncServerTime(nullptr, &request, response, done);
    }

    void CommonServiceStub::syncServerTimeCallback(uint64_t clientSendTs, common_service::SyncTimeResponse* response)
    {
        auto clientRecvTs = TimeUtil::getNowMs();
        auto offset = (response->server_recv_ts() - clientSendTs) - 
                      (response->server_send_ts() - clientRecvTs);
        TimeUtil::lastSyncTs = response->server_send_ts() + offset;
        TimeUtil::lastRecvTs = clientRecvTs;
        LOG_INFO << "syncServerTime response received: "
                 << ", Client Send Time: " << clientSendTs
                 << ", Server Receive Time: " << response->server_recv_ts()
                 << ", Server Send Time: " << response->server_send_ts()
                 << ", Client Receive Time: " << clientRecvTs
                 << ", Offset: " << offset
                 << ", Last Sync Time: " << TimeUtil::lastSyncTs
                 << ", Last Receive Time: " << TimeUtil::lastRecvTs;
    }

    void CommonServiceStub::ping()
    {
        common_service::PingRequest request;
        request.set_timestamp(TimeUtil::getNowMs());
        common_service::PongResponse* response = new common_service::PongResponse;
        auto done = google::protobuf::NewCallback(this, &CommonServiceStub::pingCallback, response);
        m_stub->ping(nullptr, &request, response, done);
        LOG_INFO << "Ping request sent with timestamp: " << request.timestamp();
    }

    void CommonServiceStub::pingCallback(common_service::PongResponse* response)
    {
        auto currentTime = TimeUtil::getNowMs();
        LOG_INFO << "Ping response received: "
                 << ", Timestamp: " << response->timestamp()
                 << ", Current Time: " << currentTime
                 << ", Round Trip Time: " << (currentTime - response->timestamp());
        TimeUtil::lastPingTs = response->timestamp();
    }
}