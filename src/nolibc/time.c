#include "nolibc/time.h"

#include "nolibc/sys.h"

int gettimeofday(struct timeval *tv, void *tz) {
  return SYSCALL2(SYS_GETTIMEOFDAY, tv, tz);
}
