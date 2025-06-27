#include <muduo/base/Logging.h>

#include "common/heartbeat_service.h"

namespace RustCinder
{
    void HeartbeatService::ping(google::protobuf::RpcController* controller,
                            const common_service::PingRequest* request,
                            common_service::PongResponse* response,
                                google::protobuf::Closure* done)
    {
        LOG_INFO << "Ping request received: "
                << ", Timestamp: " << request->timestamp();
        response->set_timestamp(123456);
        done->Run();
    }
}
