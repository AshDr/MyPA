#include <common.h>
#include "syscall.h"
#define CONFIG_STRACE


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
      if(fd == 1 || fd == 2) {
        for(int i = 0; i < count; i++) {
          putch(((char *)buf)[i]);
        }
        c->GPRx = count;
      } else {
        c->GPRx = -1;
      }
      printf("count: %u\n", count);
      break;
    }
    case SYS_brk: {
      #ifdef CONFIG_STRACE
      printf("Syscall: SYS_brk\n");
      #endif
      c->GPRx = 0;
      break;
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
