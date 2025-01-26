#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
int main() {
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return 0;
}