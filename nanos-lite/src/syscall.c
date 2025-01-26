#include <common.h>
#include "syscall.h"

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1; // a7
  a[1] = c->GPR2; // a0
  a[2] = c->GPR3; // a1
  a[3] = c->GPR4; // a2
  switch (a[0]) {
    case SYS_exit: {
      #ifdef CONFIG_STRACE
      Log("Syscall: SYS_exit");
      #endif
      halt(0); // need change ?
      break;
    }
    case SYS_yield: {
      #ifdef CONFIG_STRACE
      Log("Syscall: SYS_yield");
      #endif
      yield();
      c->GPRx = 0; 
      break;
    }
    case SYS_write: {
      #ifdef CONFIG_STRACE
      Log("Syscall: SYS_write");
      #endif
      // int fd = a[1];
      const void *buf = (const void *)a[2];
      size_t count = a[3];
      for(size_t i = 0; i < count; i++) {
        putch(((char *)buf)[i]);
      }
      c->GPRx = count;
      break;
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
