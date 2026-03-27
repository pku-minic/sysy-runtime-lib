#include "nolibc/time.h"

#include "nolibc/sys.h"

#ifdef SYS_MACOS

// macOS does not support clock_gettime via syscall,
// so we use gettimeofday to emulate it.

struct timeval {
  long tv_sec;
  long tv_usec;
};

int clock_gettime(clockid_t clk_id, struct timespec *tp) {
  // `clk_id` should always be CLOCK_MONOTONIC
  if (clk_id != CLOCK_MONOTONIC) return -1;

  struct timeval tv;
  int ret = SYSCALL2(SYS_GETTIMEOFDAY, &tv, 0);
  if (ret == 0 && tp) {
    tp->tv_sec = tv.tv_sec;
    tp->tv_nsec = tv.tv_usec * 1000;
  }
  return ret;
}

#elif defined(SYS_RISCV) && defined(SYS_32)

// qemu-riscv32 does not support clock_gettime via syscall,
// so we use clock_gettime64 instead.

#define SYS_CLOCK_GETTIME64 403

struct timespec64 {
  long long tv_sec;
  long long tv_nsec;
};

int clock_gettime(clockid_t clk_id, struct timespec *tp) {
  struct timespec64 ts64;
  int ret = SYSCALL2(SYS_CLOCK_GETTIME64, clk_id, &ts64);
  if (ret == 0 && tp) {
    tp->tv_sec = (long)ts64.tv_sec;
    tp->tv_nsec = (long)ts64.tv_nsec;
  }
  return ret;
}

#else

// Other Linux platforms (x86_64, AArch64, RISC-V 64-bit)

int clock_gettime(clockid_t clk_id, struct timespec *tp) {
  return SYSCALL2(SYS_CLOCK_GETTIME, clk_id, tp);
}

#endif  // SYS_MACOS
