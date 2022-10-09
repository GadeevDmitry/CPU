#ifndef CPU_H
#define CPU_H

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

enum CMD
{
    CMD_HLT                   ,
    CMD_PUSH                  ,
    CMD_ADD                   ,
    CMD_SUB                   ,
    CMD_MUL                   ,
    CMD_DIV                   ,
    CMD_OUT                   ,
    CMD_NOT_EXICTING          ,
    CMD_NUM_ARG      = 1 << 4 ,
    CMD_REG_ARG      = 1 << 5 ,
    CMD_MEM_ARG      = 1 << 6
};

#endif //CPU_H