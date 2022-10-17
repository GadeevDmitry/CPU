#ifndef MACHINE_H
#define MACHINE_H

const   double DELTA = 0.000001;
typedef double stack_el;

const unsigned mask01 = 31;

struct header
{
    char fst_let;
    char sec_let;
    char version;

    size_t cmd_num;
};

struct machine
{
    void *machine_code;
    int   machine_pos;
};

#define DEF_CMD(name, number, ...)     \
        CMD_##name = number,

#define DEF_JMP_CMD(name, number, ...) \
        CMD_##name = number,
enum CMD
{
    #include "cmd.h"
    CMD_NUM_ARG      = 1 << 5 ,
    CMD_REG_ARG      = 1 << 6 ,
    CMD_MEM_ARG      = 1 << 7 ,
};
#undef DEF_CMD
#undef DEF_JMP_CMD

#endif //MACHINE_H