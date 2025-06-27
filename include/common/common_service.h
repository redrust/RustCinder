#ifndef _COMMON_SERVICE_H_
#define _COMMON_SERVICE_H_

#include <google/protobuf/service.h>

#include "common_service.pb.h"
#include "common/noncopyable.h"
#include "common/nonmoveable.h"

namespace RustCinder
{
    class CommonService : public NonCopyable, public NonMoveable, \
                          public common_service::CommonService
    {
    public:
        CommonService() = default;
        virtual ~CommonService() = default;
    
    public:
        virtual void syncServerTime(google::protobuf::RpcController* controller,
                                    const common_service::SyncTimeRequest* request,
                                    common_service::SyncTimeResponse* response,
                                    google::protobuf::Closure* done) override;
    };
}
#endif // _COMMON_SERVICE_H_