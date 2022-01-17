#ifndef SYSY_H_
#define SYSY_H_

// SysY runtime library.
// Reference: https://bit.ly/3tzTFks
// Modified by MaxXing.

// Input & output functions
int getint(), getch(), getarray(int a[]);
void putint(int a), putch(int a), putarray(int n, int a[]);

// Timing function implementation
void starttime();
void stoptime();

#endif  // SYSY_H_
