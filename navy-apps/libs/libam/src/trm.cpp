#include <am.h>
#include <unistd.h>
#include <stdio.h>

Area heap;

void putch(char ch) {
  putchar(ch);
}

void halt(int code) {
  _exit(code);
}
