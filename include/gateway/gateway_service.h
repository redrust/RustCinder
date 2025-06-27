#ifndef _GATEWAY_SERVICE_H_
#define _GATEWAY_SERVICE_H_

#include <google/protobuf/service.h>

#include "gateway_service.pb.h"
#include "common/noncopyable.h"
#include "common/nonmoveable.h"


namespace RustCinder
{
    class GatewayService :  public NonCopyable, public NonMoveable, \
                            public gateway_service::LoginService
    {
    public:
        GatewayService() = default;
        virtual ~GatewayService() = default;
        
        /* rpc接口 */
        virtual void login(google::protobuf::RpcController* controller,
                           const gateway_service::LoginRequest* request,
                           gateway_service::LoginResponse* response,
                           google::protobuf::Closure* done) override;
        virtual void logout(google::protobuf::RpcController* controller,
                            const gateway_service::LogoutRequest* request,
                            gateway_service::LogoutResponse* response,
                            google::protobuf::Closure* done) override;
    };
}
#endif // _GATEWAY_SERVICE_H_