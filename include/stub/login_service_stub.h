#ifndef _LOGIN_SERVICE_STUB_H_
#define _LOGIN_SERVICE_STUB_H_

#include <muduo/net/protorpc/RpcChannel.h>

#include "gateway_service.pb.h"

namespace RustCinder
{
    class LoginServiceStub
    {
    public:
        LoginServiceStub(muduo::net::RpcChannelPtr channel);
        ~LoginServiceStub() = default;

        void login(const std::string& account, const std::string& password, 
                   google::protobuf::Closure* done = nullptr);
        void loginCallback(gateway_service::LoginResponse* response);
    private:
        gateway_service::LoginService::Stub* m_stub = nullptr;
    };
}
#endif // _LOGIN_SERVICE_STUB_H_