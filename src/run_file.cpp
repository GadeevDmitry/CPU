#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int main(int argc, const char *argv[])
{
    if (argc == 2)
    {
        if (strcmp("--help", argv[1]) == 0)
        {
            fprintf(stderr, "Usage:\n"
                            "./run FILE\n         - compile the FILE using ./Asm if it is not compiled yet or\n"
                            "                       if the FILE has been changed since last compilation. Execute\n"
                            "                       it using ./CPU\n"
                            "./run --help         - show this manual and exit\n");
            return 0;
        }

        ;
    }
}