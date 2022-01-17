#include "sysy.h"

int main() {
  // sum of array
  int arr[20], n, sum = 0;
  n = getarray(arr);
  putarray(n, arr);
  for (int i = 0; i < n; ++i) sum += arr[i];
  putint(sum);
  putch(10);

  // integer
  n = getint();
  putint(-10 + n);
  putch(10);

  // read characters
  n = getint();
  for (int i = 0; i < n; ++i) putch(getch());
  putch(10);

  return 0;
}
