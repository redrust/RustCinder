#include <muduo/base/Logging.h>

#include "net/client_mailbox.h"

namespace RustCinder::net
{
    ClientMailBox::~ClientMailBox()
    {
        stop();
        if(m_commonServiceStub)
        {
            delete m_commonServiceStub; // Clean up the CommonServiceStub
            m_commonServiceStub = nullptr;
        }
    }

    void ClientMailBox::init(muduo::net::RpcChannelPtr channel, muduo::net::EventLoop* eventLoop)
    {
        if(m_is_initialized)
        {
            return;
        }
        m_channel = channel;
        m_eventLoop = eventLoop;
        m_commonServiceStub = new CommonServiceStub(m_channel, m_eventLoop);
        m_is_initialized = true;
    }

    void ClientMailBox::start()
    {
        if(!m_is_initialized)
        {
            LOG_ERROR << "[ClientMailBox::start] ClientMailBox is not initialized.";
            return;
        }
        m_commonServiceStub->startPing();
    }

    void ClientMailBox::stop()
    {
        if(!m_is_initialized)
        {
            LOG_ERROR << "[ClientMailBox::stop] ClientMailBox is not initialized.";
            return;
        }
        m_commonServiceStub->stopPing();
    }
}