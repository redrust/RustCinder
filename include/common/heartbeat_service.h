#ifndef _HEARTBEAT_SERVICE_H_
#define _HEARTBEAT_SERVICE_H_

#include <google/protobuf/service.h>

#include "common_service.pb.h"
#include "common/noncopyable.h"
#include "common/nonmoveable.h"

namespace RustCinder
{
    class HeartbeatService : public NonCopyable, public NonMoveable,\
                             public common_service::HeartbeatService
    {
    public:
        HeartbeatService() = default;
        virtual ~HeartbeatService() = default;

    public:
        virtual void ping(google::protobuf::RpcController* controller,
                          const common_service::PingRequest* request,
                          common_service::PongResponse* response,
                          google::protobuf::Closure* done) override;
    };
}

#endif // _HEARTBEAT_SERVICE_H_