#include <stdio.h>
#include <utils.h>

static int32_t call_depth = 0;
static FILE *ftrace_log = NULL;

#define ftrace_write(...) \
    do { \
        if (ftrace_log == NULL) { \
            ftrace_log = fopen("~/nemu_trace.log", "w"); \
        } \
        fprintf(ftrace_log, __VA_ARGS__); \
        fflush(ftrace_log); \
    } while (0)

void trace_func_call(paddr_t pc, paddr_t target) {
    assert(call_depth >= 0);
    ftrace_write(FMT_PADDR ": %*scall[%s@" FMT_PADDR "]\n", pc, call_depth, "", "funcname",target);
    ++call_depth;
}

void trace_func_ret(paddr_t pc) {
    ftrace_write(FMT_PADDR ": %*sret[%s]\n", pc, call_depth, "", "funcname");
    --call_depth;
}