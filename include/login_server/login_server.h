#ifndef _LOGIN_SERVER_H_
#define _LOGIN_SERVER_H_

#include "common/noncopyable.h"
#include "common/nonmoveable.h"

namespace RustCinder 
{
    class LoginServer : public NonCopyable, public NonMoveable
    {
    public:
        LoginServer() = default;
        ~LoginServer() = default;
    };
}

#endif // _LOGIN_SERVER_H_