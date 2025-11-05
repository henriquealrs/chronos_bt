#include "thread_utils.hpp"

#include <algorithm>
#include <cerrno>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace chronos::thread_utils {

void AdjustCurrentThreadNice(int delta) noexcept {
    const pid_t tid = static_cast<pid_t>(::syscall(SYS_gettid));
    errno = 0;
    const int current = ::getpriority(PRIO_PROCESS, tid);
    if (current == -1 && errno != 0) {
        return;
    }
    const int target = std::clamp(current + delta, PRIO_MIN, PRIO_MAX);
    if (target != current) {
        ::setpriority(PRIO_PROCESS, tid, target);
    }
}

} // namespace chronos::thread_utils
