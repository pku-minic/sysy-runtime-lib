#include "sysy.h"

#include <stdio.h>
#include <sys/time.h>

#define TIMER_COUNT_MAX 1024
struct timeval timer_start, timer_end;
int timer_h[TIMER_COUNT_MAX], timer_m[TIMER_COUNT_MAX],
    timer_s[TIMER_COUNT_MAX], timer_us[TIMER_COUNT_MAX];
int timer_idx;

int getint() {
  int t;
  scanf("%d", &t);
  return t;
}

int getch() {
  char c;
  scanf("%c", &c);
  return (int)c;
}

int getarray(int a[]) {
  int n;
  scanf("%d", &n);
  for (int i = 0; i < n; i++) scanf("%d", &a[i]);
  return n;
}

void putint(int a) { printf("%d", a); }
void putch(int a) { printf("%c", a); }

void putarray(int n, int a[]) {
  printf("%d:", n);
  for (int i = 0; i < n; i++) printf(" %d", a[i]);
  printf("\n");
}

void __attribute((constructor)) before_main() {
  for (int i = 0; i < TIMER_COUNT_MAX; i++) {
    timer_h[i] = timer_m[i] = timer_s[i] = timer_us[i] = 0;
  }
  timer_idx = 1;
}

void __attribute((destructor)) after_main() {
  for (int i = 1; i < timer_idx; i++) {
    fprintf(stderr, "Timer: %dH-%dM-%dS-%dus\n", timer_h[i], timer_m[i],
            timer_s[i], timer_us[i]);
    timer_us[0] += timer_us[i];
    timer_s[0] += timer_s[i];
    timer_us[0] %= 1000000;
    timer_m[0] += timer_m[i];
    timer_s[0] %= 60;
    timer_h[0] += timer_h[i];
    timer_m[0] %= 60;
  }
  fprintf(stderr, "TOTAL: %dH-%dM-%dS-%dus\n", timer_h[0], timer_m[0],
          timer_s[0], timer_us[0]);
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
