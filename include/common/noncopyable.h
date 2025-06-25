#ifndef _NONCOPYABLE_H_
#define _NONCOPYABLE_H_

namespace RustCinder
{
    class NonCopyable
    {
    public:
        NonCopyable(const NonCopyable&) = delete;
        NonCopyable& operator=(const NonCopyable&) = delete;
    protected:
        NonCopyable() = default;
        ~NonCopyable() = default;
    };
}
#endif // _NONCOPYABLE_H_
