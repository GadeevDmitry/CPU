#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <assert.h>

#define RED    "\e[1;31m"
#define CANCEL "\e[0m"

const int CMD_SIZE = 101;

/*---------------------------------------------------------*/

bool is_valid_system_cmd(const char *cmd, const char *file_name);

/*---------------------------------------------------------*/

int main(int argc, const char *argv[])
{
    if (argc == 2)
    {
        if (!strcmp("--help", argv[1]))
        {
            fprintf(stderr, "usage: ./make2 SRC_FILE             - compile SRC_FILE using \"./Asm2\" in \"machine2.cpu\"\n"
                            "                                      Execute \"machine2.cpu\".\n"
                            "   or: ./make2 SRC_FILE -o EXE_FILE - compile SRC_FILE using \"./Asm2\" in EXE_FILE if it has not been compiled\n"
                            "                                      yet or if last compilation was before last changes of this file.\n"
                            "                                      Execute EXE_FILE\n"
                            "   or: ./make2 --help               - Show this manual and exit.\n");
            return 0;
        }

        char cmd[CMD_SIZE] = "";
        sprintf(cmd, "./Asm2 %s machine2.cpu", argv[1]);

        if (!is_valid_system_cmd(cmd, argv[1])) return 0;
        system(cmd);

        system("./CPU machine2.cpu");
    }
    else if (argc == 4 && !strcmp("-o", argv[2]))
    {
        struct stat src_stat = {};
        struct stat exe_stat = {};
        char cmd[CMD_SIZE] = "";

        if (stat(argv[1], &src_stat) == -1)
        {
            fprintf(stderr, RED "ERROR: " CANCEL "Can't stat the source file %s\n", argv[1]);
            return 0;
        }

        if (stat(argv[3], &exe_stat) == -1 || exe_stat.st_mtime < src_stat.st_mtime)
        {
            sprintf(cmd, "./Asm2 %s %s", argv[1], argv[3]);

            if (!is_valid_system_cmd(cmd, argv[1])) return 0;
            system(cmd); 
        }

        sprintf(cmd, "./CPU %s\n", argv[3]);

        if (!is_valid_system_cmd(cmd, argv[3])) return 0;
        system(cmd);
    }
    else
    {
        fprintf(stderr, "wrong format\n"
                        "print \"./make2 --help\"\n");
    }
}

bool is_valid_system_cmd(const char *cmd, const char *file_name)
{
    assert(cmd       != nullptr);
    assert(file_name != nullptr);

    if (cmd[CMD_SIZE - 1] != '\0')
    {
        fprintf(stderr, RED "ERROR: " CANCEL "Can't open the file \"%s\"\n", file_name);
        return false;
    }

    return true;
}