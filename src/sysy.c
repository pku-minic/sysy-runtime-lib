#include "sysy.h"

#ifdef NO_LIBC
#include "nolibc/crt.h"
#include "nolibc/io.h"
#include "nolibc/time.h"
#else
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#endif

// ============================================================
// Internal implementations of IO operations.
// ============================================================

#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define IS_SPACE(c)                                                          \
  ((c) == ' ' || (c) == '\f' || (c) == '\n' || (c) == '\r' || (c) == '\t' || \
   (c) == '\v')

static int last_char;
static int last_char_valid = 0;

#define OUTPUT_BUFFER_SIZE 1024
static char output_buffer[OUTPUT_BUFFER_SIZE];
static size_t output_buffer_index = 0;

static void flush_buffer(int fd) {
  if (output_buffer_index > 0) {
    write(fd, output_buffer, output_buffer_index);
    output_buffer_index = 0;
  }
}

static void write_buffer(int fd, const void *buffer, size_t size) {
  if (fd == STDERR_FILENO) {
    // do not buffer stderr
    write(fd, buffer, size);
  } else {
    if (output_buffer_index + size > OUTPUT_BUFFER_SIZE) flush_buffer(fd);
    for (size_t i = 0; i < size; i++) {
      output_buffer[output_buffer_index++] = ((char *)buffer)[i];
    }
  }
}

static void put_char_buffered(int fd, char c) {
  write_buffer(fd, &c, 1);
  if (c == '\n') flush_buffer(fd);
}

static void put_string_buffered(int fd, const char *str) {
  for (int i = 0; str[i]; ++i) put_char_buffered(fd, str[i]);
}

static void put_int_buffered(int fd, int num) {
  // check if is a negative integer
  if (num < 0) {
    putch('-');
    num = -num;
  }
  // convert integer to string
  char digits[21];
  int i = 20;
  if (!num) {
    i = 19;
    digits[19] = '0';
  } else {
    while (num) {
      i -= 1;
      digits[i] = (num % 10) + '0';
      num /= 10;
    }
  }
  // write string to stdout
  write_buffer(fd, digits + i, 20 - i);
}

// ============================================================
// Implementations of IO functions.
// ============================================================

int getint() {
  // skip spaces
  int c = getch();
  while (IS_SPACE(c)) c = getch();
  // check if is a negative integer
  int is_neg = 0;
  if (c == '-') {
    is_neg = 1;
    c = getch();
  }
  // read digits
  int num = 0;
  for (; IS_DIGIT(c); c = getch()) {
    num = num * 10 + c - '0';
  }
  // unget char
  last_char = c;
  last_char_valid = 1;
  return is_neg ? -num : num;
}

int getch() {
  if (last_char_valid) {
    // char buffer is valid, consume the char in it
    last_char_valid = 0;
    return last_char;
  } else {
    // char buffer is not valid, read char from stdin
    char c;
    return read(STDIN_FILENO, &c, 1) == 1 ? c : -1;
  }
}

int getarray(int a[]) {
  int n = getint();
  for (int i = 0; i < n; i++) a[i] = getint();
  return n;
}

void putint(int num) { put_int_buffered(STDOUT_FILENO, num); }

void putch(int ch) { put_char_buffered(STDOUT_FILENO, ch); }

void putarray(int n, int a[]) {
  putint(n);
  putch(':');
  for (int i = 0; i < n; i++) {
    putch(' ');
    putint(a[i]);
  }
  putch('\n');
}

// ============================================================
// Implementations of timing functions.
// ============================================================

static struct timeval timer_start, timer_end;

typedef struct {
  int h, m, s, us;
} timer_t;

#define TIMER_COUNT_MAX 1024
static timer_t timer[TIMER_COUNT_MAX] = {};
static int timer_idx = 0;

static void put_timer(const char *name, const timer_t *t) {
  put_string_buffered(STDERR_FILENO, name);
  put_string_buffered(STDERR_FILENO, ": ");
  put_int_buffered(STDERR_FILENO, t->h);
  put_string_buffered(STDERR_FILENO, "H-");
  put_int_buffered(STDERR_FILENO, t->m);
  put_string_buffered(STDERR_FILENO, "M-");
  put_int_buffered(STDERR_FILENO, t->s);
  put_string_buffered(STDERR_FILENO, "S-");
  put_int_buffered(STDERR_FILENO, t->us);
  put_string_buffered(STDERR_FILENO, "us\n");
}

void starttime() { gettimeofday(&timer_start, NULL); }

void stoptime() {
  gettimeofday(&timer_end, NULL);

  // abort if timer_idx is invalid
  if (timer_idx >= TIMER_COUNT_MAX) abort();

  timer_t *t = &timer[timer_idx];
  t->us += 1000000 * (timer_end.tv_sec - timer_start.tv_sec) +
           timer_end.tv_usec - timer_start.tv_usec;
  t->s += t->us / 1000000;
  t->us %= 1000000;
  t->m += t->s / 60;
  t->s %= 60;
  t->h += t->m / 60;
  t->m %= 60;

  timer_idx++;
}

void __attribute((destructor)) after_main() {
  // clear output buffer
  flush_buffer(STDOUT_FILENO);

  // print timing results
  if (timer_idx <= 0) return;
  timer_t total = {};
  for (int i = 0; i < timer_idx; i++) {
    timer_t *t = &timer[i];
    put_timer("Timer", t);
    total.us += t->us;
    total.s += t->s;
    total.m += t->m;
    total.h += t->h;
  }
  put_timer("TOTAL", &total);
}
