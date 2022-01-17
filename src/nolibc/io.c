#include "nolibc/io.h"

#include "nolibc/sys.h"

ssize_t read(int fd, void *buf, size_t nbytes) {
  return SYSCALL3(SYS_READ, fd, buf, nbytes);
}

ssize_t write(int fd, const void *buf, size_t nbytes) {
  return SYSCALL3(SYS_WRITE, fd, buf, nbytes);
}
