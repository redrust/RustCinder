#include <muduo/base/Logging.h>

#include "common/common_service.h"
#include "utils/time_util.h"

namespace RustCinder
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
}