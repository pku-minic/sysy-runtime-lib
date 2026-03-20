#ifndef LIBSYSY_TIME_H_
#define LIBSYSY_TIME_H_

#include "nolibc/sys.h"
#include "nolibc/types.h"

struct timeval {
  long tv_sec;
  long tv_usec;
};

int gettimeofday(struct timeval *tv, void *tz);

#ifndef SYS_clock_gettime64
#define SYS_clock_gettime64 403
#endif

struct timespec {
  int tv_sec;
  int tv_nsec;
};

// CLOCK_REALTIME: 0
// CLOCK_MONOTONIC: 1
int clock_gettime(int clk_id, struct timespec *tp);

#endif  // LIBSYSY_TIME_H_
