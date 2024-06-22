#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int int_to_str(int val, char *str) {
  int idx = 0, is_neg = 0;
  if(val == 0) {
    str[idx++] = '0';
    str[idx] = '\0';
    return 1;
  }
  if(val < 0) {
    is_neg = 1;
    val = -val;
  }
  while(val > 0) {
    str[idx++] = val % 10 + '0';
    val /= 10;
  }
  if(is_neg) {
    str[idx++] = '-';
  }
  str[idx] = '\0';
  int l = 0, r = idx - 1;
  while(l < r) {
    char tmp = str[l];
    str[l] = str[r];
    str[r] = tmp;
    l++;
    r--;
  }
  return idx;
}
int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  const char *p = fmt;
  char buf[50];
  while(*p != '\0') {
    if(*p == '%') {
      ++p;
      switch (*p) {
        case 's': {
          char *s = va_arg(ap, char *);
          while(*s != '\0') {
            *out++ = *s++;
          }
          break;
        }
        case 'd': {
          int val = va_arg(ap, int);
          int len = int_to_str(val, buf);
          if(len >= 50) panic("length of int > 50");
          strcpy(out, buf);
          out += len;
          break;
        }
      }
    }else {
      *out++ = *p;
    }
    ++p;
  }
  *out = '\0';
  va_end(ap);
  return 0; // 如果成功，则返回写入的字符总数，不包括字符串追加在字符串末尾的空字符。如果失败，则返回一个负数
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
