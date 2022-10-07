#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

int main()
{
    printf("size  of time_t = %lu\n", sizeof(time_t));
    
    struct stat BuffSize = {};
    time_t = stat();
}