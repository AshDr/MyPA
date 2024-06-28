#ifndef __CPU_IRINGBUF_H__
#define __CPU_IRINGBUF_H__
struct IRingBuf {
  int size;
  int p;
  char *buf;
};
extern struct IRingBuf ringbuf;
void init_IRingBuf();
void write_IRingBuf(const char *);
void print_IRingBuf();
#endif