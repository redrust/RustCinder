#include <muduo/base/Logging.h>

#include "service/common_service.h"
#include "utils/time_util.h"

namespace RustCinder::Service
{
    void CommonService::syncServerTime(google::protobuf::RpcController* controller,
                                    const common_service::SyncTimeRequest* request,
                                    common_service::SyncTimeResponse* response,
                                    google::protobuf::Closure* done)
    {
        LOG_INFO << "syncServerTime request received: ";
        response->set_server_recv_ts(TimeUtil::getNowMs());
        response->set_server_send_ts(TimeUtil::getNowMs());
        done->Run();
    }

    void CommonService::ping(google::protobuf::RpcController* controller,
                             const common_service::PingRequest* request,
                             common_service::PongResponse* response,
                             google::protobuf::Closure* done)
    {
        auto currentTime = TimeUtil::getNowMs();
        LOG_INFO << "ping request received: " << request->timestamp()
                 << ", current time: " << currentTime 
                 << ", round trip time: " << (currentTime - request->timestamp());
        response->set_timestamp(currentTime);
        done->Run();
    }
}