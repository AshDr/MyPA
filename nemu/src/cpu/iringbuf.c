#include <cpu/iringbuf.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
struct IRingBuf ringbuf;
void init_IRingBuf() {
    ringbuf.size = 1024;
    ringbuf.p = 0;
    ringbuf.buf = malloc(ringbuf.size);
}
void write_IRingBuf(const char *str) {
    if(strlen(str) + ringbuf.p < ringbuf.size) {
        ringbuf.p += snprintf(ringbuf.buf + ringbuf.p, ringbuf.size - ringbuf.p, "%s\n", str);
    }
    else {
        ringbuf.p = 0;
        ringbuf.p += snprintf(ringbuf.buf + ringbuf.p, ringbuf.size - ringbuf.p, "%s\n", str);
    }
}
void print_IRingBuf() {
    printf("%s\n", ringbuf.buf);
}