#ifndef LIBSYSY_TYPE_H_
#define LIBSYSY_TYPE_H_

#include "nolibc/sys.h"

#ifdef SYS_32
typedef int ssize_t;
typedef unsigned int size_t;
#else
typedef long ssize_t;
typedef unsigned long size_t;
#endif

#define NULL ((void *)0)

#endif  // LIBSYSY_TYPE_H_
