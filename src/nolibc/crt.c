#include "nolibc/sys.h"
#include "nolibc/types.h"

#define SIGABRT 6

extern int main();
extern void after_main();

void _start() {
  int ret = main();
  after_main();
  SYSCALL1(SYS_EXIT, ret);
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

void abort() {
  int pid = SYSCALL0(SYS_GETPID);
  SYSCALL2(SYS_KILL, pid, SIGABRT);
  // if kill returns, exit with error
  SYSCALL1(SYS_EXIT, 1);
  __builtin_unreachable();
}
