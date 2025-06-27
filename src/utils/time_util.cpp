#include "utils/time_util.h"

namespace RustCinder
{
    std::atomic<uint64_t> TimeUtil::lastSyncTs = 0;
    std::atomic<uint64_t> TimeUtil::lastRecvTs = 0;
}