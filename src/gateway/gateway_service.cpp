#include <muduo/base/Logging.h>

#include "gateway/gateway_service.h"

namespace RustCinder
{
    void GatewayService::Login(google::protobuf::RpcController* controller,
                                   const gateway_service::LoginRequest* request,
                                   gateway_service::LoginResponse* response,
                                   google::protobuf::Closure* done)
    {
        LOG_INFO << "Login request received: "
                  << "Account: " << request->account()
                  << ", Password: " << request->password();
        done->Run();
    }

    void GatewayService::Logout(google::protobuf::RpcController* controller,
                                    const gateway_service::LogoutRequest* request,
                                    gateway_service::LogoutResponse* response,
                                    google::protobuf::Closure* done)
    {
        LOG_INFO << "Logout request received: "
                  << "Account: " << request->account();
        done->Run();
    }
}