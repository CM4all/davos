#ifndef CHRONO_HXX
#define CHRONO_HXX

#include "util/Compiler.h"

#include <chrono>

#include <sys/time.h>

gcc_const
static inline std::chrono::system_clock::time_point
ToSystemTime(const struct timespec ts)
{
    return std::chrono::system_clock::from_time_t(ts.tv_sec)
        + std::chrono::nanoseconds(ts.tv_nsec);
}

gcc_const
static inline struct timeval
ToTimeval(std::chrono::system_clock::time_point p)
{
    struct timeval tv;
    tv.tv_sec = std::chrono::system_clock::to_time_t(p);

    const auto remainder = p - std::chrono::system_clock::from_time_t(tv.tv_sec);
    tv.tv_usec = std::chrono::duration_cast<std::chrono::microseconds>(remainder).count();
    return tv;
}

#endif
