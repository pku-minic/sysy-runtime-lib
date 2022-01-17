#ifndef LIBSYSY_IO_H_
#define LIBSYSY_IO_H_

#include "nolibc/sys.h"
#include "nolibc/types.h"

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

ssize_t read(int fd, void *buf, size_t nbytes);
ssize_t write(int fd, const void *buf, size_t nbytes);

#endif  // LIBSYSY_IO_H_
