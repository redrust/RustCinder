#include <muduo/base/Logging.h>

#include "client/common_service_stub.h"
#include "utils/time_util.h"

namespace RustCinder
{
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

}