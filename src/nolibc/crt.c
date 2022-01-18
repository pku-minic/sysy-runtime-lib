#include "nolibc/sys.h"
#include "nolibc/types.h"

extern int main();

void _start() {
  int ret = main();
  SYSCALL1(SYS_EXIT, ret);
}

void *memset(void *s, int c, size_t len) {
  unsigned char *p = s;
  while (len--) {
    *p++ = (unsigned char)c;
  }
  return s;
}
