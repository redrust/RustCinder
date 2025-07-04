#ifndef _REMOTE_CLIENT_H_
#define _REMOTE_CLIENT_H_
#include <memory>

#include <muduo/net/EventLoop.h>
#include <muduo/net/protorpc/RpcChannel.h>
#include <muduo/base/Logging.h>

#include "stub/common_service_stub.h"

namespace RustCinder::net
{
    class ClientMailBox
    {
    public:
        typedef std::shared_ptr<ClientMailBox> ClientMailBoxPtr;

        ClientMailBox() = default;
        ~ClientMailBox();
    
        void init(muduo::net::RpcChannelPtr channel, muduo::net::EventLoop* eventLoop);
        void start();
        void stop();

        template <typename StubType>
        StubType* getStub()
        {
            if (!m_is_initialized)
            {
                LOG_ERROR << "[ClientMailBox::getStub] ClientMailBox is not initialized.";
                return nullptr;
            }
            if (m_commonServiceStub && std::is_same<StubType, CommonServiceStub>::value)
            {
                return m_commonServiceStub;
            }
            return nullptr; // Add other stubs as needed
        }
    private:
        muduo::net::RpcChannelPtr m_channel;
        CommonServiceStub* m_commonServiceStub = nullptr;
        muduo::net::EventLoop* m_eventLoop = nullptr;
        bool m_is_initialized = false;
    };
}

#endif // _REMOTE_CLIENT_H_