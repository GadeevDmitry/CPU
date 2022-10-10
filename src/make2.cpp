#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define RED    "\e[1;31m"
#define CANCEL "\e[0m"

int main(int argc, const char *argv[])
{
    if (argc == 2)
    {
        if (!strcmp("--help", argv[1]))
        {
            fprintf(stderr, "usage: ./make SRC_FILE             - compile SRC_FILE using \"./Asm2\" in \"machine2.cpu\" if it has not been compiled yet\n"
                            "                                     or if last compilation was before last changes of this file.\n"
                            "                                     Execute \"machine2.cpu\".\n"
                            "   or: ./make SRC_FILE -o EXE_FILE - compile SRC_FILE using \"./Asm\" in EXE_FILE. Execute EXE_FILE.\n"
                            "   or: ./make -o EXE_FILE          - Execute EXE_FILE.\n"
                            "   or: ./make --help               - Show this manual and exit.\n");
            return 0;
        }

        struct stat src_stat = {};
        if (stat(argv[1], &src_stat) == -1)
        {
            fprintf(stderr, RED "ERROR: " CANCEL "Can't stat th file %s\n", argv[1]);
            return 1;
        }

        struct stat exe_stat = {};
        if (stat("machine2.cpu", &exe_stat) == -1 || exe_stat.st_mtime < src_stat.st_mtime)
        {
            char cmd[101] = "";
            sprintf(cmd, "./Asm2 %s machine2.cpu", argv[1]);

            if (cmd[100] == '\0') system(cmd);
            else
            {
                fprintf(stderr, RED "ERROR: " CANCEL "Can't open the file %s\n", argv[1]);
                return 1;
            }
        }
        system("./CPU machine2.cpu");
    }
    else if (argc == 3 && !strcmp("-o", argv[1]))
    {
        char cmd[101] = "";
        sprintf(cmd, "./CPU %s", argv[2]);

        if (cmd[100] == '\0') system(cmd);
    }
    else if (argc == 4 && !strcmp("-o", argv[2]))
    {
        char cmd[201] = "";
        sprintf(cmd, "./Asm2 %s %s", argv[1], argv[3]);

        if (cmd[200] == '\0') system(cmd);

        sprintf(cmd, "./CPU %s", argv[3]);
        system(cmd);
    }
    else
    {
        fprintf(stderr, "wrong format\n"
                        "print \"./make --help\" to get manual\n");
    }
}