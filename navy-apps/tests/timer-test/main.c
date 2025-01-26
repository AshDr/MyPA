#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <NDL.h>
int main() {
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    NDL_Init();
    uint32_t start = NDL_GetTicks();
    NDL_Quit();
    return 0;
}