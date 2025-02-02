#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int putchar (int ch) {
  putch(ch);
  return ch;
}
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

int unsiged_to_str(unsigned val, char *str) {
  int idx = 0;
  if (val == 0) {
    str[idx++] = '0';
    str[idx] = '\0';
    return 1;
  }
  while (val > 0) {
    str[idx++] = val % 10 + '0';
    val /= 10;
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

int unsigned_to_hex(unsigned val, char *str) {
  int idx = 0;
  if (val == 0) {
    str[idx++] = '0';
    str[idx] = '\0';
    return 1;
  }
  while (val > 0) {
    int digit = val % 16;
    if (digit < 10) {
      str[idx++] = digit + '0';
    } else {
      str[idx++] = digit - 10 + 'a';
    }
    val /= 16;
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
      case 'c': {
        char ch = va_arg(ap, int);
        putch(ch);
        ++length;
        break;
      }
      case 's': {
        char *s = va_arg(ap, char *);
        while (*s != '\0') {
          putch(*s++);
          ++length;
        }
        break;
      }
      case 'p':{
        unsigned val = va_arg(ap, unsigned);
        putch('0');
        putch('x');
        int len = unsigned_to_hex(val, buf);
        if (len >= 50)
          panic("length of int > 50");
        for (int i = 0; i < len; ++i) {
          putch(buf[i]);
        }
        length += len;
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
      case 'u':{
        unsigned val = va_arg(ap, unsigned);
        int len = unsiged_to_str(val, buf);
        if (len >= 50)
          panic("length of int > 50");
        for (int i = 0; i < len; ++i) {
          putch(buf[i]);
        }
        length += len;
        break;
      }
      case 'x':{
        unsigned val = va_arg(ap, unsigned);
        int len = unsigned_to_hex(val, buf);
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
      case 'c': {
          char ch = va_arg(ap, int);
          *out++ = ch;
          break;
        }
      case 's': {
          char *s = va_arg(ap, char *);
          while (*s != '\0') {
            *out++ = *s++;
          }
          break;
        }
      case 'p':{
          unsigned val = va_arg(ap, unsigned);
          *out++ = '0';
          *out++ = 'x';
          int len = unsigned_to_hex(val, buf);
          if (len >= 50)
            panic("length of int > 50");
          for (int i = 0; i < len; ++i) {
            *out++ = buf[i];
          }
          break;
        }
      case 'd': {
          int val = va_arg(ap, int);
          int len = int_to_str(val, buf);
          if (len >= 50)
            panic("length of int > 50");
          for (int i = 0; i < len; ++i) {
            *out++ = buf[i];
          }
          break;
        }
      case 'u':{
          unsigned val = va_arg(ap, unsigned);
          int len = unsiged_to_str(val, buf);
          if (len >= 50)
            panic("length of int > 50");
          for (int i = 0; i < len; ++i) {
            *out++ = buf[i];
          }
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
  va_list ap;
  va_start(ap, fmt);
  const char *p = fmt;
  char *start = out;
  char buf[50];
  while (*p != '\0') {
    if (*p == '%') {
      ++p;
      switch (*p) {
      case 'c': {
          char ch = va_arg(ap, int);
          *out++ = ch;
          break;
        }
      case 's': {
          char *s = va_arg(ap, char *);
          while (*s != '\0') {
            *out++ = *s++;
          }
          break;
        }
      case 'p':{
          unsigned val = va_arg(ap, unsigned);
          *out++ = '0';
          *out++ = 'x';
          int len = unsigned_to_hex(val, buf);
          if (len >= 50)
            panic("length of int > 50");
          for (int i = 0; i < len; ++i) {
            *out++ = buf[i];
          }
          break;
        }
      case 'd': {
          int val = va_arg(ap, int);
          int len = int_to_str(val, buf);
          if (len >= 50)
            panic("length of int > 50");
          for (int i = 0; i < len; ++i) {
            *out++ = buf[i];
          }
          break;
        }
      case 'u':{
          unsigned val = va_arg(ap, unsigned);
          int len = unsiged_to_str(val, buf);
          if (len >= 50)
            panic("length of int > 50");
          for (int i = 0; i < len; ++i) {
            *out++ = buf[i];
          }
          break;
        }
      }
    } else {
      *out++ = *p;
    }
    ++p;
  }
  if(out - start >= n) {
    out -= (out - start - n);
  }
  *out = '\0';
  va_end(ap);
  return out - start;
}
int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sscanf(const char *in, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  const char *p = fmt;
  const char *start = in;
  while (*p != '\0') {
    if (*p == '%') {
      ++p;
      switch (*p) {
      case 'c': {
          char *ch = va_arg(ap, char *);
          *ch = *in++;
          break;
        }
      case 's': {
          char *s = va_arg(ap, char *);
          while (*in != ' ' && *in != '\0') {
            *s++ = *in++;
          }
          *s = '\0';
          break;
        }
      case 'd': {
          int *val = va_arg(ap, int *);
          int is_neg = 0;
          if (*in == '-') {
            is_neg = 1;
            ++in;
          }
          *val = 0;
          while (*in != ' ' && *in != '\0') {
            *val = *val * 10 + *in - '0';
            ++in;
          }
          if (is_neg) {
            *val = -*val;
          }
          break;
        }
      case 'u': {
          unsigned *val = va_arg(ap, unsigned *);
          *val = 0;
          while (*in != ' ' && *in != '\0') {
            *val = *val * 10 + *in - '0';
            ++in;
          }
          break;
        }
      }
    } else {
      ++in;
    }
    ++p;
  }
  va_end(ap);
  return in - start;
}
#endif
