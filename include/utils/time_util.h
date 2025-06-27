#ifndef _TIME_UTIL_H_
#define _TIME_UTIL_H_

#include <chrono>
#include <atomic>

namespace RustCinder
{
    class TimeUtil
    {
    public:
        static decltype(auto) getDurationNow()
        {
            return std::chrono::steady_clock::now().time_since_epoch();
        }
        static uint64_t getNowMs()
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                getDurationNow()).count();
        }
        // 获取当前时间戳（秒）
        static uint64_t getNow()
        {
            if(lastSyncTs == 0)
            {
                return std::chrono::duration_cast<std::chrono::seconds>(
                    getDurationNow()).count();
            }
            else
            {
                return (lastSyncTs + getNowMs() - lastRecvTs) / 1000;
            }
        }
        static std::atomic<uint64_t> lastSyncTs;
        static std::atomic<uint64_t> lastRecvTs;
    };
}

#endif // _TIME_UTIL_H_
