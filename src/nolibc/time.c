#include "nolibc/time.h"

#include "nolibc/sys.h"
#include "nolibc/types.h"

#ifdef SYS_MACOS

// On macOS, direct raw syscalls are not a reliable path for this API on
// Apple Silicon. Use libSystem's `clock_gettime` instead.

#elif defined(SYS_RISCV) && defined(SYS_32)

// qemu-riscv32 does not support clock_gettime via syscall,
// so we use `clock_gettime64` instead.

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
