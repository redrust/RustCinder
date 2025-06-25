#ifndef _NONMOVEABLE_H_
#define _NONMOVEABLE_H_

namespace RustCinder
{
    class NonMoveable
    {
    public:
        NonMoveable(NonMoveable&&) = delete;
        NonMoveable& operator=(NonMoveable&&) = delete;
    protected:
        NonMoveable() = default;
        ~NonMoveable() = default;
    };
}
#endif // _NONMOVEABLE_H_
