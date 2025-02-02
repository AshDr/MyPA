#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <NDL.h>
int main() {
    struct timeval tv;
    struct timezone tz;
    NDL_Init(0);
    uint32_t start = NDL_GetTicks();
    NDL_Quit();
    return 0;
}