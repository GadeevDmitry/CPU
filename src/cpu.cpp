#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#define RED    "\e[1;31m"
#define CANCEL "\e[0m"
#define GREEN  "\e[0;32m"

#include "cpu.h"
#include "read_write.h"

typedef double Stack_elem;
#include "stack.h"

const double DELTA = 0.000001;

struct cpu_store
{
    struct machine execution;
    size_t execution_size;

    Stack stack;
};

enum ERRORS
{
    OK            ,
    ZERO_DIVISION ,
    EMPTY_STACK   ,
    UNDEFINED_CMD
};

const char *error_messages[] = 
{
    "./CPU IS OK"       ,
    "DIVISION BY ZERO"  ,
    "STACK IS EMPTY"    ,
    "UNDEFINED COMMAND"

};

/*-----------------------------------------FUNCTION_DECLARATION-----------------------------------------*/

bool check_signature(cpu_store *progress);
bool execution      (cpu_store *progress);
bool approx_equal   (double a, double b);

ERRORS cmd_arithmetic(cpu_store *progress, CMD mode);
ERRORS cmd_out       (cpu_store *progress);

/*------------------------------------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    cpu_store progress = {};
    StackCtor(&progress.stack, 0);
    progress.execution.machine_code = read_file(argv[1], &progress.execution_size);

    if (progress.execution.machine_code == nullptr)
    {
        fprintf(stderr, RED "ERROR: " CANCEL "Can't execute the file \"%s\"\n", argv[1]);
        return 1;
    }

    if (!check_signature(&progress)) return 1;
}

bool execution(cpu_store *progress)
{
    assert(progress != nullptr);

    while (progress->execution.machine_pos < progress->execution_size)
    {
        char cmd = *((char *) progress->execution.machine_code + progress->execution.machine_pos);
        progress->execution.machine_pos++;

        switch (cmd)
        {
        }
    }
    return true;
}

ERRORS cmd_arithmetic(cpu_store *progress, CMD mode)
{
    assert(progress != nullptr);

    double var_1 = 0, var_2 = 0;

    bool is_empty = false;
    Stack_IsEmpty(&progress->stack, &is_empty);

    if (is_empty) return EMPTY_STACK;

    StackPop     (&progress->stack, &var_1);
    Stack_IsEmpty(&progress->stack, &is_empty);

    if (is_empty) return EMPTY_STACK;

    StackPop(&progress->stack, &var_2);

    switch (mode)
    {
        case CMD_ADD:
            StackPush(&progress->stack, var_1 + var_2);
            return OK;
        
        case CMD_SUB:
            StackPush(&progress->stack, var_1 - var_2);
            return OK;

        case CMD_MUL:
            StackPush(&progress->stack, var_1 * var_2);
            return OK;
        
        case CMD_DIV:
            if (approx_equal(var_2, 0)) return ZERO_DIVISION;

            StackPush(&progress->stack, var_1 / var_2);
            return OK;
    }

    return UNDEFINED_CMD;
}

ERRORS cmd_out(cpu_store *progress)
{
    assert(progress != nullptr);

    bool is_empty = false;
    Stack_IsEmpty(&progress->stack, &is_empty);

    if (is_empty) return EMPTY_STACK;

    double var = 0;
    StackPush(&progress->stack, var);
    printf("%lg", var);

    return OK;
}

bool check_signature(cpu_store *progress)
{
    assert(progress != nullptr);

    header signature = *(header *) progress->execution.machine_code;

    if (signature.fst_let != 'G' || signature.sec_let != 'D')
    {
        fprintf(stderr, "./CPU: Access denied\n");
        return false;
    }
    if (signature.version != 1)
    {
        fprintf(stderr, "./CPU doesn't support the version %d\n", signature.version);
        return false;
    }

    progress->execution.machine_pos = sizeof(header);
    
    return true;
}

void output_error(ERRORS err)
{
    if (err == OK)
    {
        fprintf(stderr, GREEN "%s\n" CANCEL, error_messages[err]);
        return;
    }
    fprintf(stderr, RED "ERROR: " CANCEL "%s\n", error_messages[err]);
}

bool approx_equal(double a, double b)
{
    return fabs(a - b) < DELTA;
}