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
#elif defined(__riscv)
#define SYS_RISCV
#else
#error "supports only x86-64 or RISC-V targets"
#endif

#ifdef SYS_RISCV
#if __riscv_xlen == 32
#define SYS_32
#elif __risv_xlen == 64
#define SYS_64
#else
#error "supports only 32-bit or 64-bit RISC-V targets"
#endif
#endif

#ifdef SYS_LINUX
#define SYS_READ 0
#define SYS_WRITE 1
#define SYS_GETTIMEOFDAY 96
#else
#define SYS_READ (0x20000000 | 3)
#define SYS_WRITE (0x20000000 | 4)
#define SYS_GETTIMEOFDAY (0x20000000 | 116)
#endif

#ifdef SYS_X86_64
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
#else
#define SYSCALL2(number, arg0, arg1)           \
  ({                                           \
    long int _sys_result;                      \
    long int _arg0 = (long int)(arg0);         \
    long int _arg1 = (long int)(arg1);         \
                                               \
    register long int __a7 asm("a7") = number; \
    register long int __a0 asm("a0") = _arg0;  \
    register long int __a1 asm("a1") = _arg1;  \
    __asm__ volatile("scall\n\t"               \
                     : "+r"(__a0)              \
                     : "r"(__a7), "r"(__a1)    \
                     : "memory");              \
    _sys_result = __a0;                        \
    _sys_result;                               \
  })
#define SYSCALL3(number, arg0, arg1, arg2)             \
  ({                                                   \
    long int _sys_result;                              \
    long int _arg0 = (long int)(arg0);                 \
    long int _arg1 = (long int)(arg1);                 \
    long int _arg2 = (long int)(arg2);                 \
                                                       \
    register long int __a7 asm("a7") = number;         \
    register long int __a0 asm("a0") = _arg0;          \
    register long int __a1 asm("a1") = _arg1;          \
    register long int __a2 asm("a2") = _arg2;          \
    __asm__ volatile("scall\n\t"                       \
                     : "+r"(__a0)                      \
                     : "r"(__a7), "r"(__a1), "r"(__a2) \
                     : "memory");                      \
    _sys_result = __a0;                                \
    _sys_result;                                       \
  })
#endif

#endif  // LIBSYSY_SYS_H_
