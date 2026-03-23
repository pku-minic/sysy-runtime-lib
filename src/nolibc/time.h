#ifndef LIBSYSY_TIME_H_
#define LIBSYSY_TIME_H_

#include "nolibc/sys.h"
#include "nolibc/types.h"

struct timeval {
  long tv_sec;
  long tv_usec;
};

int gettimeofday(struct timeval *tv, void *tz);

#if defined(SYS_LINUX)
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1
#endif

#endif  // LIBSYSY_TIME_H_
