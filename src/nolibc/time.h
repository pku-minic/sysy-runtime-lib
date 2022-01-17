#ifndef LIBSYSY_TIME_H_
#define LIBSYSY_TIME_H_

#include "nolibc/sys.h"
#include "nolibc/types.h"

struct timeval {
  long tv_sec;
  long tv_usec;
};

static inline int gettimeofday(struct timeval *tv, void *tz) {
  return SYSCALL2(SYS_GETTIMEOFDAY, tv, tz);
}

#endif  // LIBSYSY_TIME_H_
