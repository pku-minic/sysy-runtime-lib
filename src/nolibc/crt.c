#include "nolibc/sys.h"
#include "nolibc/types.h"

#ifndef SYS_MACOS  // macOS always provides a C runtime
extern int main();
extern void after_main();

void __attribute__((noreturn)) _start() {
#ifdef SYS_X86_64
  asm volatile(
      "xor %%rbp, %%rbp\n"  // clear rbp to indicate the end of stack frames
      "and $-16, %%rsp\n"   // align the stack to 16 bytes
      :
      :
      : "rbp", "rsp");
#endif  // SYS_X86_64
  int ret = main();
  after_main();
  SYSCALL1(SYS_EXIT, ret);
  __builtin_unreachable();
}

void *memset(void *dest, int c, size_t n) {
  unsigned char *p = dest;
  while (n--) *p++ = (unsigned char)c;
  return dest;
}

void *memcpy(void *dest, const void *src, size_t n) {
  char *d = dest;
  const char *s = src;
  while (n--) *d++ = *s++;
  return dest;
}
#endif  // SYS_MACOS

#define SIGABRT 6

void abort() {
  int pid = SYSCALL0(SYS_GETPID);
  SYSCALL2(SYS_KILL, pid, SIGABRT);
  // if kill returns, exit with error
  SYSCALL1(SYS_EXIT, 1);
  __builtin_unreachable();
}
