#include <common.h>
#include "syscall.h"
#include <sys/time.h>
#define CONFIG_STRACE

extern int fs_open(const char *pathname, int flags, int mode);
extern size_t fs_read(int fd, void *buf, size_t len);
extern size_t fs_write(int fd, const void *buf, size_t len);
extern size_t fs_lseek(int fd, size_t offset, int whence);
extern int fs_close(int fd);

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1; // a7
  // a[1] = c->GPR2; // a0
  // a[2] = c->GPR3; // a1
  // a[3] = c->GPR4; // a2
  switch (a[0]) {
    case SYS_exit: {
      #ifdef CONFIG_STRACE
      printf("Syscall: SYS_exit\n");
      #endif
      halt(0); // need change ?
      break;
    }
    case SYS_yield: {
      #ifdef CONFIG_STRACE
      printf("Syscall: SYS_yield\n");
      #endif
      yield();
      c->GPRx = 0; 
      break;
    }
    case SYS_write: {
      #ifdef CONFIG_STRACE
      printf("Syscall: SYS_write\n");
      #endif
      int fd = (int)c->GPR2;
      void *buf = (void *)c->GPR3;
      size_t count = (size_t)c->GPR4;
      c->GPRx = fs_write(fd, buf, count);
      break;
    }
    case SYS_open: {
      #ifdef CONFIG_STRACE
      printf("Syscall: SYS_open\n");
      #endif
      const char *path = (const char *)c->GPR2;
      int flags = (int)c->GPR3;
      int mode = (int)c->GPR4;
      c->GPRx = fs_open(path, flags, mode);
      break;
    }
    case SYS_read: {
      #ifdef CONFIG_STRACE
      printf("Syscall: SYS_read\n");
      #endif
      int fd = (int)c->GPR2;
      void *buf = (void *)c->GPR3;
      size_t count = (size_t)c->GPR4;
      c->GPRx = fs_read(fd, buf, count);
      break;
    }
    case SYS_lseek: {
      #ifdef CONFIG_STRACE
      printf("Syscall: SYS_lseek\n");
      #endif
      int fd = (int)c->GPR2;
      size_t offset = (size_t)c->GPR3;
      int whence = (int)c->GPR4;
      c->GPRx = fs_lseek(fd, offset, whence);
      break;
    }
    case SYS_close: {
      #ifdef CONFIG_STRACE
      printf("Syscall: SYS_close\n");
      #endif
      int fd = (int)c->GPR2;
      c->GPRx = fs_close(fd);
      break;
    }
    case SYS_brk: {
      #ifdef CONFIG_STRACE
      printf("Syscall: SYS_brk\n");
      #endif
      c->GPRx = 0;
      break;
    }
    case SYS_gettimeofday: {
      #ifdef CONFIG_STRACE
      printf("Syscall: SYS_gettimeofday\n");
      #endif
      uint64_t us = io_read(AM_TIMER_UPTIME).us;
      struct timeval *tv = (struct timeval *)c->GPR2;
      tv->tv_sec = us / 1000000;
      tv->tv_usec = us % 1000000;
      c->GPRx = 0;
      break;
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
