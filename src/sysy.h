#ifndef LIBSYSY_SYSY_H_
#define LIBSYSY_SYSY_H_

// SysY runtime library.
// Reference: https://bit.ly/3tzTFks
// Modified by MaxXing.

// Input & output functions
int getint(), getch(), getarray(int a[]);
void putint(int num), putch(int ch), putarray(int n, int a[]);

// Timing functions
void starttime();
void stoptime();

#endif  // LIBSYSY_SYSY_H_
