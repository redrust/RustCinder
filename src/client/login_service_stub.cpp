#include <muduo/base/Logging.h>

#include "client/login_service_stub.h"

namespace RustCinder
{
    LoginServiceStub::LoginServiceStub(muduo::net::RpcChannelPtr channel)
        : m_stub(new gateway_service::LoginService::Stub(channel.get()))
    {}

    void LoginServiceStub::login(const std::string& account, const std::string& password, 
        google::protobuf::Closure* done)
    {
        gateway_service::LoginRequest request;
        request.set_account(account);
        request.set_password(password);

        gateway_service::LoginResponse* response = new gateway_service::LoginResponse;
        google::protobuf::RpcController* controller = nullptr; // Use default controller

        if(done == nullptr)
        {
            done = google::protobuf::NewCallback(this, &LoginServiceStub::loginCallback, response);
        }

        m_stub->login(controller, &request, response, done);
    }

    void LoginServiceStub::loginCallback(gateway_service::LoginResponse* response)
    {
        if(response->status() == gateway_service::LoginResponse::SUCCESS)
        {
            LOG_INFO << "Login successful: ";
        }
    }
}