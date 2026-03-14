#include "nolibc/time.h"

#include "nolibc/sys.h"

int gettimeofday(struct timeval *tv, void *tz) {
  return SYSCALL2(SYS_GETTIMEOFDAY, tv, tz);
}

// compat struct, only for syscall
struct timespec64 {
  long long tv_sec;
  long long tv_nsec;
};

int do_clock_gettime64(int clk_id, struct timespec64 *tp) {
    return SYSCALL2(SYS_clock_gettime64, clk_id, tp);
}

int clock_gettime(int clk_id, struct timespec *tp) {
    struct timespec64 ts64;
    int ret;

    if (!tp) {
        return -1; 
    }

    ret = do_clock_gettime64(clk_id, &ts64);

    if (ret == 0) {
        tp->tv_sec = (long)ts64.tv_sec;
        tp->tv_nsec = (long)ts64.tv_nsec;
    }

    return ret;
}