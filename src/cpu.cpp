#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#define RED    "\e[1;31m"
#define CANCEL "\e[0m"
#define GREEN  "\e[0;32m"

#include "cpu.h"
#include "read_write.h"
#include "stack.h"

const   double DELTA = 0.000001;
typedef double stack_el;

struct cpu_store
{
    struct machine execution;
    size_t execution_size;

    stack stk;
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

bool   check_signature(cpu_store *progress);
bool   execution      (cpu_store *progress);
bool   approx_equal   (double a, double b);

ERRORS cmd_arithmetic(cpu_store *progress, char mode);
ERRORS cmd_out       (cpu_store *progress);
ERRORS cmd_push      (cpu_store *progress);

void   output_error  (ERRORS status);

/*------------------------------------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    cpu_store progress = {};
    stack_ctor(&progress.stk, sizeof(stack_el));

    progress.execution.machine_code = read_file(argv[1], &progress.execution_size);

    if (progress.execution.machine_code == nullptr)
    {
        fprintf(stderr, RED "ERROR: " CANCEL "Can't execute the file \"%s\"\n", argv[1]);
        return 1;
    }

    if (!check_signature(&progress)) return 1;

    bool execution_status = execution(&progress);
    if (!execution_status) return 1;

    output_error(OK);
}

bool execution(cpu_store *progress)
{
    assert(progress != nullptr);

    progress->execution.machine_pos = sizeof(header);

    while (progress->execution.machine_pos < progress->execution_size)
    {
        char cmd = *((char *) progress->execution.machine_code + progress->execution.machine_pos);
        progress->execution.machine_pos += sizeof(char);

        /*-----------
        fprintf(stderr, "cmd = %d\n", cmd);
        //-----------*/

        ERRORS status = OK;
        switch (cmd)
        {
            case CMD_HLT : return true;

            case CMD_PUSH:
                
                cmd_push(progress);
                break;

            case CMD_ADD: case CMD_SUB: case CMD_MUL: case CMD_DIV:
                
                status = cmd_arithmetic(progress, cmd);
                if (status != OK) 
                {
                    output_error(status);
                    return false;
                }
                break;

            case CMD_OUT:
                
                status = cmd_out(progress);
                if (status != OK)
                {
                    output_error(status);
                    return false;
                }
                break;

            default:
                
                output_error(UNDEFINED_CMD);
                return false;
        }
    }

    return true;
}

ERRORS cmd_arithmetic(cpu_store *progress, char mode)
{
    assert(progress != nullptr);

    stack_el var_1 = 0;
    stack_el var_2 = 0;

    if (stack_empty(&progress->stk)) return EMPTY_STACK;
    var_1 = *(stack_el *) stack_pop(&progress->stk);

    if (stack_empty(&progress->stk)) return EMPTY_STACK;
    var_2 = *(stack_el *) stack_pop(&progress->stk);
    
    stack_el ans = 0;
    switch (mode)
    {
        case CMD_ADD:
            ans = var_1 + var_2;
            break;
        case CMD_SUB:
            ans = var_2 - var_1;
            break;
        case CMD_MUL:
            ans = var_1 * var_2;
            break;
        case CMD_DIV:
            if (approx_equal(var_1, 0)) return ZERO_DIVISION;
            ans = var_2 / var_1;
            break;
        default:
            return UNDEFINED_CMD;
    }


    stack_push(&progress->stk, &ans);
    return OK;
}

ERRORS cmd_out(cpu_store *progress)
{
    assert(progress != nullptr);

    if (stack_empty(&progress->stk)) return EMPTY_STACK;

    stack_el var = *(stack_el *) stack_front(&progress->stk);
    printf("%lg\n", var);

    return OK;
}

ERRORS cmd_push(cpu_store *progress)
{
    assert(progress != nullptr);

    stack_el push_val = *(stack_el *) ((char *) progress->execution.machine_code + progress->execution.machine_pos);
    progress->execution.machine_pos += sizeof(stack_el);

    //-------------
    //fprintf(stderr, "push_val = %lg\n", push_val);
    //-------------

    stack_push(&progress->stk, &push_val);

    /*-------------
    fprintf(stderr, "stk.size     = %lu\n"
                    "stk.capacity = %lu\n", progress->stk.size, progress->stk.capacity);

    for (int i = 0; i < progress->stk.size; ++i)
    {
        fprintf(stderr, "%lg ", *(double *) ((char *) progress->stk.data + sizeof(double) * i));
    }
    fprintf(stderr, "\n");
    //-------------*/

    return OK;
}

bool check_signature(cpu_store *progress)
{
    assert(progress != nullptr);

    header signature = *(header *) progress->execution.machine_code;

    if (signature.fst_let != 'G' || signature.sec_let != 'D')
    {
        fprintf(stderr, "./CPU: Signature check falls\n");
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

void output_error(ERRORS status)
{
    if (status == OK)
    {
        fprintf(stderr, GREEN "%s\n" CANCEL, error_messages[status]);
        return;
    }
    fprintf(stderr, RED "ERROR: " CANCEL "%s\n", error_messages[status]);
}

bool approx_equal(double a, double b)
{
    return fabs(a - b) < DELTA;
}