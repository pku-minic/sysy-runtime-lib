#include "sysy.h"

#ifdef NO_LIBC
#include "nolibc/io.h"
#include "nolibc/time.h"
#else
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

#define TIMER_COUNT_MAX 1024
static struct timeval timer_start, timer_end;
static int timer_h[TIMER_COUNT_MAX], timer_m[TIMER_COUNT_MAX],
    timer_s[TIMER_COUNT_MAX], timer_us[TIMER_COUNT_MAX];
static int timer_idx = 1;

void __attribute((destructor)) after_main() {
  // clear output buffer
  write(STDOUT_FILENO, output_buffer, output_buffer_index);
  // print timing results
  if (timer_idx <= 1) return;
  for (int i = 1; i < timer_idx; i++) {
    put_string_buffered(STDERR_FILENO, "Timer: ");
    put_int_buffered(STDERR_FILENO, timer_h[i]);
    put_string_buffered(STDERR_FILENO, "H-");
    put_int_buffered(STDERR_FILENO, timer_m[i]);
    put_string_buffered(STDERR_FILENO, "M-");
    put_int_buffered(STDERR_FILENO, timer_s[i]);
    put_string_buffered(STDERR_FILENO, "S-");
    put_int_buffered(STDERR_FILENO, timer_us[i]);
    put_string_buffered(STDERR_FILENO, "us\n");
    timer_us[0] += timer_us[i];
    timer_s[0] += timer_s[i];
    timer_us[0] %= 1000000;
    timer_m[0] += timer_m[i];
    timer_s[0] %= 60;
    timer_h[0] += timer_h[i];
    timer_m[0] %= 60;
  }
  put_string_buffered(STDERR_FILENO, "TOTAL: ");
  put_int_buffered(STDERR_FILENO, timer_h[0]);
  put_string_buffered(STDERR_FILENO, "H-");
  put_int_buffered(STDERR_FILENO, timer_m[0]);
  put_string_buffered(STDERR_FILENO, "M-");
  put_int_buffered(STDERR_FILENO, timer_s[0]);
  put_string_buffered(STDERR_FILENO, "S-");
  put_int_buffered(STDERR_FILENO, timer_us[0]);
  put_string_buffered(STDERR_FILENO, "us\n");
}

void starttime() { gettimeofday(&timer_start, NULL); }

void stoptime() {
  gettimeofday(&timer_end, NULL);
  timer_us[timer_idx] += 1000000 * (timer_end.tv_sec - timer_start.tv_sec) +
                         timer_end.tv_usec - timer_start.tv_usec;
  timer_s[timer_idx] += timer_us[timer_idx] / 1000000;
  timer_us[timer_idx] %= 1000000;
  timer_m[timer_idx] += timer_s[timer_idx] / 60;
  timer_s[timer_idx] %= 60;
  timer_h[timer_idx] += timer_m[timer_idx] / 60;
  timer_m[timer_idx] %= 60;
  timer_idx++;
}
