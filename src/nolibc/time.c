#include "nolibc/time.h"

#include "nolibc/sys.h"

#if defined(SYS_LINUX) && defined(SYS_32)

struct timespec64 {
  long long tv_sec;
  long long tv_nsec;
};

int do_clock_gettime64(int clk_id, struct timespec64 *tp) {
  return SYSCALL2(SYS_CLOCK_GETTIME64, clk_id, tp);
}

#endif

int gettimeofday(struct timeval *tv, void *tz) {
// gettimeofday does not work on qemu's riscv32
// use clock_gettime64 instead
#if defined(SYS_LINUX) && defined(SYS_32)
  struct timespec64 ts64;

  if (do_clock_gettime64(CLOCK_MONOTONIC, &ts64) == 0) {
    if (tv) {
      tv->tv_sec = (long)ts64.tv_sec;
      tv->tv_usec = (long)ts64.tv_nsec / 1000;
    }
    return 0;
  }
  return -1;
#else
  return SYSCALL2(SYS_GETTIMEOFDAY, tv, tz);
#endif
}
