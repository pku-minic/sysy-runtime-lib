#include "sysy.h"

int main() {
  for (int i = 0; i < 5000; ++i) {
    starttime();
    stoptime();
  }
  return 0;
}
