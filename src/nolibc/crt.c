#include "nolibc/sys.h"
#include "nolibc/types.h"

extern void before_main();
extern int main();
extern void after_main();

void _start() {
  before_main();
  int ret = main();
  after_main();
  SYSCALL1(SYS_EXIT, ret);
}

void *memset(void *s, int c, size_t len) {
  unsigned char *p = s;
  while (len--) {
    *p++ = (unsigned char)c;
  }
  return s;
}
