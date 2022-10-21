#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#define RED    "\e[1;31m"
#define CANCEL "\e[0m"
#define GREEN  "\e[0;32m"

#include "read_write.h"
#include "stack.h"

const   double DELTA = 0.000001;
typedef double stack_el;

const unsigned mask01 = 31;


const int REG_NUM = 8;

#define DEF_CMD(name, number, code)     \
        CMD_##name = number,
#define DEF_JMP_CMD(name, number, code) \
        CMD_##name = number,
enum CMD
{
    #include "cmd.h"
    CMD_NUM_ARG      = 1 << 4 ,
    CMD_REG_ARG      = 1 << 5 ,
    CMD_MEM_ARG      = 1 << 6
};
#undef DEF_CMD
#undef DEF_JMP_CMD

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

struct exe_store
{
    machine execution;
    size_t  execution_size;

    char version;

int main(int argc, const cahr *argv[])
{
    exe_store progress = {};
    progress.execution;
}

/**
*   @brief Checks if signature is correct. Stops disasm if it is not correct.
*
*   @param progress [in] - "exe_store" contains all information about program
*
*   @return true if signature is correct and false else
*/

bool check_signature(exe_store *progress)
{
    assert(progress != nullptr);

    header signature = *(header *) progress->execution.machine_code;

    if (signature.fst_let != 'G' || signature.sec_let != 'D')
    {
        fprintf(stderr, "./DisAsm2: Signature check falls\n");
        return false;
    }
    if ((progress->version = signature.version) < 1 || progress->version > 2)
    {
        fprintf(stderr, "./DisAsm2: doesn't support the version %d\n", signature.version);
        return false;
    }

    progress->execution.machine_pos = sizeof(header);
    
    