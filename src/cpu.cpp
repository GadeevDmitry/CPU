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

const unsigned mask01 =   (1 << 4) - 1;
const unsigned mask10 = ~((1 << 4) - 1);

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

const int REG_NUM = 8;

enum CMD
{
    CMD_HLT                   , // 0
    CMD_PUSH                  , // 1
    CMD_ADD                   , // 2
    CMD_SUB                   , // 3
    CMD_MUL                   , // 4
    CMD_DIV                   , // 5
    CMD_OUT                   , // 6
    CMD_NOT_EXICTING          , // 7
    CMD_POP                   , // 8
    CMD_JMP                   , // 9
    CMD_NUM_ARG      = 1 << 5 ,
    CMD_REG_ARG      = 1 << 6 ,
    CMD_MEM_ARG      = 1 << 7
};

struct cpu_store
{
    machine execution;
    size_t  execution_size;

    char version;
    stack    stk;
    stack_el regs[REG_NUM + 1]; //zero register is invalid
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

bool     check_signature (cpu_store *progress);
bool     execution       (cpu_store *progress);
bool     approx_equal    (double a, double b);

ERRORS   cmd_arithmetic  (cpu_store *progress, unsigned char mode);
ERRORS   cmd_out         (cpu_store *progress);
ERRORS   cmd_push        (cpu_store *progress);
ERRORS   cmd_pop         (cpu_store *progress);
ERRORS   cmd_jmp         (cpu_store *progress);

void    *get_machine_cmd (cpu_store *const progress, const size_t val_size);
void     output_error    (ERRORS status);

stack_el get_push_val    (cpu_store *const progress, const unsigned char cmd);

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

#define err_check(status)                       \
        if (status != OK)                       \
        {                                       \
            output_error(status);               \
            return false;                       \
        }

/**
*   @brief Manages of program executing by reading commands from "progress->execution.machine_code" and calling functions to execute them.
*   @brief Prints messages about errors in stderr.
*
*   @param progress [in] - "cpu_store" contains all information about program
*
*   @return true if there are not any errors and false else
*/

bool execution(cpu_store *progress)
{
    assert(progress != nullptr);

    progress->execution.machine_pos = sizeof(header);
    while (progress->execution.machine_pos < progress->execution_size)
    {
        unsigned char cmd = *(unsigned char *) get_machine_cmd(progress, sizeof(char));

        ERRORS status = OK;
        switch ((cmd & mask01))
        {
            case CMD_HLT : /*fprintf(stderr, "HLT\n");*/ return true;

            case CMD_PUSH:
                cmd_push(progress);
                break;
            
            case CMD_POP:
                status = cmd_pop(progress);
                err_check(status);
                break;

            case CMD_JMP:
                status = cmd_jmp(progress);
                break;

            case CMD_ADD: case CMD_SUB: case CMD_MUL: case CMD_DIV:
                status = cmd_arithmetic(progress, cmd);
                err_check(status);
                break;

            case CMD_OUT:
                status = cmd_out(progress);
                err_check(status);
                break;

            default:
                output_error(UNDEFINED_CMD);
                return false;
        }
    }

    return true;
}

/**
*   @brief Reads another command or argument from "progress->execution.machine_code".
*
*   @param progress [in] - "cpu_store" contains all information about program
*   @param val_size [in] - size (in bytes) of value to read
*
*   @return pointer to the value
*/

void *get_machine_cmd(cpu_store *const progress, const size_t val_size)
{
    assert(progress != nullptr);

    void *cmd = (char *) progress->execution.machine_code + progress->execution.machine_pos;
    progress->execution.machine_pos += val_size;

    return cmd;
}

/**
*   @brief Executes arithmetic commands.
*
*   @param progress [in] - "cpu_store" contains all information about program
*   @param mode     [in] - mode encodes the arithmetic operation
*
*   @return enum "ERRORS" error value
*/

ERRORS cmd_arithmetic(cpu_store *progress, unsigned char mode)
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
            //fprintf(stderr, "ADD\n");
            ans = var_1 + var_2;
            break;
        case CMD_SUB:
            //fprintf(stderr, "SUB\n");
            ans = var_2 - var_1;
            break;
        case CMD_MUL:
            //fprintf(stderr, "MUL\n");
            ans = var_1 * var_2;
            break;
        case CMD_DIV:
            //fprintf(stderr, "DIV\n");
            if (approx_equal(var_1, 0)) return ZERO_DIVISION;
            ans = var_2 / var_1;
            break;
        default:
            return UNDEFINED_CMD;
    }


    stack_push(&progress->stk, &ans);
    return OK;
}

/**
*   @brief Executes "out" command.
*
*   @param progress [in] - "cpu_store" contains all information about program
*
*   @return enum "ERRORS" error value
*/

ERRORS cmd_out(cpu_store *progress)
{
    //fprintf(stderr, "OUT\n");
    assert(progress != nullptr);

    if (stack_empty(&progress->stk)) return EMPTY_STACK;

    stack_el var = *(stack_el *) stack_front(&progress->stk);
    printf("%lg\n", var);

    return OK;
}

/**
*   @brief Executes "push" command.
*
*   @param progress [in] - "cpu_store" contains all information about program
*
*   @return enum "ERRORS" error value
*/

ERRORS cmd_push(cpu_store *progress)
{
    //fprintf(stderr, "PUSH\n");
    assert(progress != nullptr);

    --progress->execution.machine_pos;
    unsigned char cmd = *(unsigned char *) get_machine_cmd(progress, sizeof(char));

    stack_el push_val = get_push_val(progress, cmd);
    stack_push(&progress->stk, &push_val);

    return OK;
}

/**
*   @brief Reads "push" arguments and translates them in "stack_el" value.
*
*   @param progress [in] - "cpu_store" contains all information about program
*   @param cmd      [in] - cmd encodes "push" comand and one of "push" mode
*
*   @return "stack_el" value      
*/

stack_el get_push_val(cpu_store *const progress, const unsigned char cmd)
{
    assert(progress != nullptr);
    
    if (progress->version == 1) return *(stack_el *) get_machine_cmd(progress, sizeof(stack_el));
    
    stack_el val = 0;
    if (cmd & CMD_REG_ARG) val += progress->regs[*(char *) get_machine_cmd(progress, sizeof(char))];
    if (cmd & CMD_NUM_ARG) val +=            *(stack_el *) get_machine_cmd(progress, sizeof(stack_el)); 

    return val;
}

/**
*   @brief Executes "pop" command.
*
*   @param progress [in] - "cpu_store" contains all information about program
*
*   @return enum "ERRORS" error value
*/

ERRORS cmd_pop(cpu_store *progress)
{
    //fprintf(stderr, "POP\n");
    assert(progress != nullptr);

    if (stack_empty(&progress->stk)) return EMPTY_STACK;

    --progress->execution.machine_pos;
    unsigned char cmd = *(unsigned char *) get_machine_cmd(progress, sizeof(char));

    if      (cmd & CMD_NUM_ARG) stack_pop(&progress->stk);
    else if (cmd & CMD_REG_ARG)
    {
        progress->regs[*(char *) get_machine_cmd(progress, sizeof(char))] = *(stack_el *) stack_front(&progress->stk);
        stack_pop(&progress->stk);
    }

    return OK;
}

/**
*   @brief Executes "jmp" command.
*
*   @param progress [in] - "cpu_store" contains all information about program
*
*   @return enum "ERRORS" error value
*/

ERRORS cmd_jmp(cpu_store *progress)
{
    //fprintf(stderr, "JMP\n");
    assert(progress != nullptr);

    int jmp_pos = *(int *) get_machine_cmd(progress, sizeof(int));
    //-----------
    //fprintf(stderr, "jmp_pos = %d\n", jmp_pos);
    //-----------
    progress->execution.machine_pos = jmp_pos;

    return OK;
}

/**
*   @brief Checks if signature is correct. Stops execution if it is not correct.
*
*   @param progress [in] - "cpu_store" contains all information about program
*
*   @return true if signature is correct and false else
*/

bool check_signature(cpu_store *progress)
{
    assert(progress != nullptr);

    header signature = *(header *) progress->execution.machine_code;
    //------------
    //fprintf(stderr, "signature.cmd_num = %d\n", signature.cmd_num);
    //fprintf(stderr, "header_size       = %d\n", sizeof(header));
    //------------
    if (signature.fst_let != 'G' || signature.sec_let != 'D')
    {
        fprintf(stderr, "./CPU: Signature check falls\n");
        return false;
    }
    if ((progress->version = signature.version) < 1 || progress->version > 2)
    {
        fprintf(stderr, "./CPU doesn't support the version %d\n", signature.version);
        return false;
    }

    progress->execution.machine_pos = sizeof(header);
    
    return true;
}

/**
*   @brief Prints error-messages in stderr.
*
*   @param enum "ERRORS" value encoding the error
*
*   @return nothing
*/

void output_error(ERRORS status)
{
    if (status == OK)
    {
        fprintf(stderr, GREEN "%s\n" CANCEL, error_messages[status]);
        return;
    }
    fprintf(stderr, RED "ERROR: " CANCEL "%s\n", error_messages[status]);
}

/**
*   @brief Compare two double numbers with error rate DELTA.
*
*   @param a [in] - first  number to compare
*   @param b [in] - second number to compare
*
*   @return true if numbers are approximately equal and false else
*/

bool approx_equal(double a, double b)
{
    return fabs(a - b) < DELTA;
}