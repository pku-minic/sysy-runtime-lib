#ifndef LIBSYSY_NOLIBC_TIME_H_
#define LIBSYSY_NOLIBC_TIME_H_

#include "nolibc/types.h"

#ifdef SYS_MACOS
#define CLOCK_MONOTONIC 6
#else  // SYS_LINUX
#define CLOCK_MONOTONIC 1
#endif

struct timespec {
  time_t tv_sec;
  long tv_nsec;
};

int clock_gettime(clockid_t clk_id, struct timespec *tp);

#endif  // LIBSYSY_NOLIBC_TIME_H_
