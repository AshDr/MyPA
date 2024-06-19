#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  if(s == NULL) return 0;
  int len = 0;
  for(;s[len] != '\0';++len) ;
  return len;
}

char *strcpy(char *dst, const char *src) {
  if(dst == NULL || src == NULL) return NULL;
  int i = 0;
  for(;src[i] != '\0';++i) {
    dst[i] = src[i];
  }
  dst[i] = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  if(dst == NULL || src == NULL) return NULL;
  int i = 0;
  for(;src[i] != '\0' && i < n;++i) {
    dst[i] = src[i];
  }
  dst[i] = '\0';
  return dst;
}

char *strcat(char *dst, const char *src) {
  if(dst == NULL) return NULL;
  size_t idx = strlen(dst);
  size_t i;
  for(i = 0; src[i] != '\0'; i++) {
    dst[idx + i] = src[i];
  }
  dst[idx + i] = '\0';
  return dst;

}

int strcmp(const char *s1, const char *s2) {
  if(s1 == NULL && s2 == NULL) return 0;
  size_t i;
  for(i = 0; s1[i] != '\0' && s2[i] != '\0'; ++i) {
    if(s1[i] != s2[i]) return s1[i] - s2[i];
  }
  return s1[i] - s2[i];
}

int strncmp(const char *s1, const char *s2, size_t n) {
  if(s1 == NULL && s2 == NULL) return 0;
  size_t i;
  for(i = 0; i < n && s1[i] != '\0' && s2[i] != '\0'; ++i) {
    if(s1[i] != s2[i]) return s1[i] - s2[i];
  }
  return s1[i] - s2[i];
}

void *memset(void *s, int c, size_t n) {
  for(size_t i = 0; i < n; ++i) {
    ((char *)s)[i] = c;
  }
  return s;
}
void *memmove(void *dst, const void *src, size_t n) {
  if(src == NULL) return dst;
  if(dst < src) {
    for(size_t i = 0; i < n; ++i) {
      ((char *)dst)[i] = ((char *)src)[i];
    }
  } else {
    for(size_t i = n - 1; i >= 0; --i) {
      ((char *)dst)[i] = ((char *)src)[i];
    }
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  if(in == NULL) return out;
  for(size_t i = 0; i < n; ++i) {
    ((char *)out)[i] = ((char *)in)[i];
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  if((s1 == NULL && s2 == NULL) || n == 0) return 0;
  for(size_t i = 0; i < n; ++i) {
    if(((char *)s1)[i] != ((char *)s2)[i]) return ((char *)s1)[i] - ((char *)s2)[i];
  }
  return 0;

}

#endif
