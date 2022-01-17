#ifndef LIBSYSY_IO_H_
#define LIBSYSY_IO_H_

#include "nolibc/sys.h"
#include "nolibc/types.h"

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

static inline ssize_t read(int fd, const void *buf, size_t nbytes) {
  return SYSCALL3(SYS_READ, fd, buf, nbytes);
}

static inline ssize_t write(int fd, const void *buf, size_t nbytes) {
  return SYSCALL3(SYS_WRITE, fd, buf, nbytes);
}

#endif  // LIBSYSY_IO_H_
