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
    CMD_HLT          ,
    CMD_PUSH         ,
    CMD_ADD          ,
    CMD_SUB          ,
    CMD_MUL          ,
    CMD_DIV          ,
    CMD_OUT          ,
    CMD_NOT_EXICTING
};

#endif //CPU_H