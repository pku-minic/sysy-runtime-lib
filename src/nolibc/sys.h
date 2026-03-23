#ifndef LIBSYSY_SYS_H_
#define LIBSYSY_SYS_H_

#if defined(__linux__)
#define SYS_LINUX
#elif defined(__APPLE__)
#define SYS_MACOS
#else
#error "supports only Linux/macOS"
#endif

#if defined(__x86_64__)
#define SYS_X86_64
#define SYS_64
#elif defined(__aarch64__)
#define SYS_AARCH64
#define SYS_64
#elif defined(__riscv)
#define SYS_RISCV
#else
#error "supports only x86-64, AArch64 or RISC-V targets"
#endif

#ifdef SYS_RISCV
#if __riscv_xlen == 32
#define SYS_32
#elif __riscv_xlen == 64
#define SYS_64
#else
#error "supports only 32-bit or 64-bit RISC-V targets"
#endif
#endif

// Reference: printf SYS_[a-z]* | gcc -include sys/syscall.h -E -
#ifdef SYS_LINUX
#ifdef SYS_X86_64
#define SYS_READ 0
#define SYS_WRITE 1
#define SYS_EXIT 60
#define SYS_GETTIMEOFDAY 96
#else  // AArch64 and RISC-V.
#define SYS_READ 63
#define SYS_WRITE 64
#define SYS_EXIT 93
#define SYS_GETTIMEOFDAY 169
#endif
#else  // macOS.
#ifdef SYS_X86_64
#define SYS_READ (0x2000000 | 3)
#define SYS_WRITE (0x2000000 | 4)
#define SYS_EXIT (0x2000000 | 1)
#define SYS_GETTIMEOFDAY (0x2000000 | 116)
#else  // AArch64.
#define SYS_READ 3
#define SYS_WRITE 4
#define SYS_EXIT 1
#define SYS_GETTIMEOFDAY 116
#endif
#endif

// Reference: libc/sysdeps/unix/sysv/linux/*/sysdep.h
#if defined(SYS_X86_64)
#define SYSCALL1(number, arg0)                            \
  ({                                                      \
    unsigned long int resultvar;                          \
    unsigned long int __arg0 = (unsigned long int)(arg0); \
    register unsigned long int _a0 asm("rdi") = __arg0;   \
    asm volatile("syscall\n\t"                            \
                 : "=a"(resultvar)                        \
                 : "0"(number), "r"(_a0)                  \
                 : "memory", "cc", "r11", "cx");          \
    (long int)resultvar;                                  \
  })
#define SYSCALL2(number, arg0, arg1)                      \
  ({                                                      \
    unsigned long int resultvar;                          \
    unsigned long int __arg1 = (unsigned long int)(arg1); \
    unsigned long int __arg0 = (unsigned long int)(arg0); \
    register unsigned long int _a1 asm("rsi") = __arg1;   \
    register unsigned long int _a0 asm("rdi") = __arg0;   \
    asm volatile("syscall\n\t"                            \
                 : "=a"(resultvar)                        \
                 : "0"(number), "r"(_a0), "r"(_a1)        \
                 : "memory", "cc", "r11", "cx");          \
    (long int)resultvar;                                  \
  })
#define SYSCALL3(number, arg0, arg1, arg2)                   \
  ({                                                         \
    unsigned long int resultvar;                             \
    unsigned long int __arg2 = (unsigned long int)(arg2);    \
    unsigned long int __arg1 = (unsigned long int)(arg1);    \
    unsigned long int __arg0 = (unsigned long int)(arg0);    \
    register unsigned long int _a2 asm("rdx") = __arg2;      \
    register unsigned long int _a1 asm("rsi") = __arg1;      \
    register unsigned long int _a0 asm("rdi") = __arg0;      \
    asm volatile("syscall\n\t"                               \
                 : "=a"(resultvar)                           \
                 : "0"(number), "r"(_a0), "r"(_a1), "r"(_a2) \
                 : "memory", "cc", "r11", "cx");             \
    (long int)resultvar;                                     \
  })
#elif defined(SYS_AARCH64)
#ifdef SYS_LINUX
#define SYSCALL1(number, arg0)                                             \
  ({                                                                       \
    long _sys_result;                                                      \
    long _x0tmp = (long)(arg0);                                            \
    register long _x0 asm("x0");                                           \
    _x0 = _x0tmp;                                                          \
    register long _x8 asm("x8") = (number);                                \
    asm volatile("svc 0\n\t" : "=r"(_x0) : "r"(_x8), "r"(_x0) : "memory"); \
    _sys_result = _x0;                                                     \
    _sys_result;                                                           \
  })
#define SYSCALL2(number, arg0, arg1)            \
  ({                                            \
    long _sys_result;                           \
    long _x1tmp = (long)(arg1);                 \
    long _x0tmp = (long)(arg0);                 \
    register long _x0 asm("x0");                \
    _x0 = _x0tmp;                               \
    register long _x1 asm("x1") = _x1tmp;       \
    register long _x8 asm("x8") = (number);     \
    asm volatile("svc 0\n\t"                    \
                 : "=r"(_x0)                    \
                 : "r"(_x8), "r"(_x0), "r"(_x1) \
                 : "memory");                   \
    _sys_result = _x0;                          \
    _sys_result;                                \
  })
#define SYSCALL3(number, arg0, arg1, arg2)                \
  ({                                                      \
    long _sys_result;                                     \
    long _x2tmp = (long)(arg2);                           \
    long _x1tmp = (long)(arg1);                           \
    long _x0tmp = (long)(arg0);                           \
    register long _x0 asm("x0");                          \
    _x0 = _x0tmp;                                         \
    register long _x1 asm("x1") = _x1tmp;                 \
    register long _x2 asm("x2") = _x2tmp;                 \
    register long _x8 asm("x8") = (number);               \
    asm volatile("svc 0\n\t"                              \
                 : "=r"(_x0)                              \
                 : "r"(_x8), "r"(_x0), "r"(_x1), "r"(_x2) \
                 : "memory");                             \
    _sys_result = _x0;                                    \
    _sys_result;                                          \
  })
#else  // macOS.
#define SYSCALL1(number, arg0)                                                \
  ({                                                                          \
    long _sys_result;                                                         \
    long _x0tmp = (long)(arg0);                                               \
    register long _x0 asm("x0");                                              \
    _x0 = _x0tmp;                                                             \
    register long _x8 asm("x16") = (number);                                  \
    asm volatile("svc 0x80\n\t" : "=r"(_x0) : "r"(_x8), "r"(_x0) : "memory"); \
    _sys_result = _x0;                                                        \
    _sys_result;                                                              \
  })
#define SYSCALL2(number, arg0, arg1)            \
  ({                                            \
    long _sys_result;                           \
    long _x1tmp = (long)(arg1);                 \
    long _x0tmp = (long)(arg0);                 \
    register long _x0 asm("x0");                \
    _x0 = _x0tmp;                               \
    register long _x1 asm("x1") = _x1tmp;       \
    register long _x8 asm("x16") = (number);    \
    asm volatile("svc 0x80\n\t"                 \
                 : "=r"(_x0)                    \
                 : "r"(_x8), "r"(_x0), "r"(_x1) \
                 : "memory");                   \
    _sys_result = _x0;                          \
    _sys_result;                                \
  })
#define SYSCALL3(number, arg0, arg1, arg2)                \
  ({                                                      \
    long _sys_result;                                     \
    long _x2tmp = (long)(arg2);                           \
    long _x1tmp = (long)(arg1);                           \
    long _x0tmp = (long)(arg0);                           \
    register long _x0 asm("x0");                          \
    _x0 = _x0tmp;                                         \
    register long _x1 asm("x1") = _x1tmp;                 \
    register long _x2 asm("x2") = _x2tmp;                 \
    register long _x8 asm("x16") = (number);              \
    asm volatile("svc 0x80\n\t"                           \
                 : "=r"(_x0)                              \
                 : "r"(_x8), "r"(_x0), "r"(_x1), "r"(_x2) \
                 : "memory");                             \
    _sys_result = _x0;                                    \
    _sys_result;                                          \
  })
#endif
#else  // RISC-V.
#define SYSCALL1(number, arg0)                                     \
  ({                                                               \
    long int _sys_result;                                          \
    long int _arg0 = (long int)(arg0);                             \
    register long int __a7 asm("a7") = number;                     \
    register long int __a0 asm("a0") = _arg0;                      \
    asm volatile("ecall\n\t" : "+r"(__a0) : "r"(__a7) : "memory"); \
    _sys_result = __a0;                                            \
    _sys_result;                                                   \
  })
#define SYSCALL2(number, arg0, arg1)                                          \
  ({                                                                          \
    long int _sys_result;                                                     \
    long int _arg0 = (long int)(arg0);                                        \
    long int _arg1 = (long int)(arg1);                                        \
    register long int __a7 asm("a7") = number;                                \
    register long int __a0 asm("a0") = _arg0;                                 \
    register long int __a1 asm("a1") = _arg1;                                 \
    asm volatile("ecall\n\t" : "+r"(__a0) : "r"(__a7), "r"(__a1) : "memory"); \
    _sys_result = __a0;                                                       \
    _sys_result;                                                              \
  })
#define SYSCALL3(number, arg0, arg1, arg2)         \
  ({                                               \
    long int _sys_result;                          \
    long int _arg0 = (long int)(arg0);             \
    long int _arg1 = (long int)(arg1);             \
    long int _arg2 = (long int)(arg2);             \
    register long int __a7 asm("a7") = number;     \
    register long int __a0 asm("a0") = _arg0;      \
    register long int __a1 asm("a1") = _arg1;      \
    register long int __a2 asm("a2") = _arg2;      \
    asm volatile("ecall\n\t"                       \
                 : "+r"(__a0)                      \
                 : "r"(__a7), "r"(__a1), "r"(__a2) \
                 : "memory");                      \
    _sys_result = __a0;                            \
    _sys_result;                                   \
  })
#endif

#endif  // LIBSYSY_SYS_H_
