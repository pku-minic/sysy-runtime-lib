#include "sysy.h"

int main() {
  // sum of array
  int arr[20], n, sum = 0;
  n = getarray(arr);
  putarray(n, arr);
  int count = getint();
  putint(count);
  putch(10);
  starttime();
  for (int j = 0; j < count; ++j) {
    for (int k = 0; k < n; ++k) {
      for (int i = 0; i < n; ++i) sum += arr[i] * arr[k];
    }
  }
  stoptime();
  putint(sum);
  putch(10);

  // read characters
  n = getint();
  getch();
  for (int i = 0; i < n; ++i) putch(getch());
  putch(10);

  return 0;
}
