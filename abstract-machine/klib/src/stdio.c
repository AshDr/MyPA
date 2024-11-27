#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
int int_to_str(int val, char *str) {
  int idx = 0, is_neg = 0;
  if (val == 0) {
    str[idx++] = '0';
    str[idx] = '\0';
    return 1;
  }
  if (val < 0) {
    is_neg = 1;
    val = -val;
  }
  while (val > 0) {
    str[idx++] = val % 10 + '0';
    val /= 10;
  }
  if (is_neg) {
    str[idx++] = '-';
  }
  str[idx] = '\0';
  int l = 0, r = idx - 1;
  while (l < r) {
    char tmp = str[l];
    str[l] = str[r];
    str[r] = tmp;
    l++;
    r--;
  }
  return idx;
}

int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  const char *p = fmt;
  char buf[50];
  int f = 1;
  int length = 0;
  while (*p != '\0') {
    if (*p == '%') {
      ++p;
      switch (*p) {
      case 's': {
        char *s = va_arg(ap, char *);
        while (*s != '\0') {
          putch(*s++);
          ++length;
        }
        break;
      }
      case 'd': {
        int val = va_arg(ap, int);
        int len = int_to_str(val, buf);
        if (len >= 50)
          panic("length of int > 50");
        for (int i = 0; i < len; ++i) {
          putch(buf[i]);
        }
        length += len;
        break;
      }
      default:
        f = 0;
      }
    } else {
      putch(*p);
      ++length;
    }
    ++p;
  }
  va_end(ap);
  if (f == 0)
    return 0;
  return length;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  const char *p = fmt;
  char *start = out;
  char buf[50];
  while (*p != '\0') {
    if (*p == '%') {
      ++p;
      switch (*p) {
      case 's': {
        char *s = va_arg(ap, char *);
        strcpy(out, s);
        out += strlen(out);
        break;
      }
      case 'd': {
        int val = va_arg(ap, int);
        int len = int_to_str(val, buf);
        if (len >= 50)
          panic("length of int > 50");
        strcpy(out, buf);
        out += len;
        break;
      }
      }
    } else {
      *out++ = *p;
    }
    ++p;
  }
  *out = '\0';
  va_end(ap);
  return out - start;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
