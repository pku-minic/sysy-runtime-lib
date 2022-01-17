#include "nolibc/sys.h"

extern int main();

void _start() {
  int ret = main();
  SYSCALL1(SYS_EXIT, ret);
}
