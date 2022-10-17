#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <SFML/Graphics.hpp>

#define RED    "\e[1;31m"
#define CANCEL "\e[0m"
#define GREEN  "\e[0;32m"

#include "read_write.h"
#include "stack.h"

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

const int REG_NUM = 8;
const int RAM_NUM = 10000;
const int RAM_STR = 100;

#define DEF_CMD(name, number, code)      \
        CMD_##name = number,
#define DEF_JMP_CMD(name, number, code)  \
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

struct cpu_store
{
    machine execution;
    size_t  execution_size;

    char version;

    stack calls;
    stack stk;
    stack_el  ram [RAM_NUM];
    stack_el  regs[REG_NUM / 2];
    long long_regs[REG_NUM / 2 + 1]; //zero register is invalid
    
};

enum ERRORS
{
    OK            ,
    ZERO_DIVISION ,
    EMPTY_STACK   ,
    EMPTY_CALLS   ,
    UNDEFINED_CMD ,
    MEMORY_LIMIT  ,
    NEG_VALUE
};

const char *error_messages[] = 
{
    "./CPU IS OK"            ,
    "DIVISION BY ZERO"       ,
    "STACK IS EMPTY"         ,
    "CALLS STACK IS EMPTY"   ,
    "UNDEFINED COMMAND"      ,
    "MEMORY LIMIT EXCEEDED"  ,
    "SQRT OF NEGATIVE VALUE"
};


/*-----------------------------------------FUNCTION_DECLARATION-----------------------------------------*/

bool     check_signature  (cpu_store *progress);
bool     execution        (cpu_store *progress);
bool     approx_equal     (const double a,   const double b);
bool     approx_cmp       (const stack_el a, const stack_el b, char *type);

ERRORS   cmd_push         (cpu_store *progress);
ERRORS   cmd_pop          (cpu_store *progress);
ERRORS   cmd_jmp          (cpu_store *progress);

long     get_memory_val   (cpu_store *const progress, const unsigned char cmd);

void    *get_machine_cmd  (cpu_store *const progress, const size_t val_size);
void     output_error     (ERRORS status);

stack_el get_reg_val      (cpu_store *const progress, const char reg_num);
stack_el get_stack_el_val (cpu_store *progress, const unsigned char cmd);

/*------------------------------------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    cpu_store progress = {};
    stack_ctor(&progress.stk  , sizeof(stack_el));
    stack_ctor(&progress.calls, sizeof(int));

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

#define EMPTY_CHECK()                                                               \
        if (stack_empty(&progress->stk))                                            \
        {                                                                           \
            output_error(EMPTY_STACK);                                              \
            return false;                                                           \
        }

#define EMPTY_CALLS()                                                               \
        if (stack_empty(&progress->calls))                                          \
        {                                                                           \
            output_error(EMPTY_CALLS);                                              \
            return false;                                                           \
        }

#define ZERO_CHECK(val)                                                             \
        if (approx_equal(val, 0))                                                   \
        {                                                                           \
            output_error(ZERO_DIVISION);                                            \
            return false;                                                           \
        }

#define PUSH(val)                                                                   \
        stack_el push_val = val;                                                    \
                                                                                    \
        stack_push(&progress->stk, &push_val);

#define POP()                                                                       \
        EMPTY_CHECK()                                                               \
        stack_pop(&progress->stk);

#define GET_STK_ONE()                                                               \
        EMPTY_CHECK()                                                               \
        stack_el a = *(stack_el *) stack_front(&progress->stk);

#define GET_STK_TWO()                                                               \
        GET_STK_ONE()                                                               \
        POP()                                                                       \
        EMPTY_CHECK()                                                               \
        stack_el b = *(stack_el *) stack_front(&progress->stk);                     \
        POP()

#define PRINT(val)                                                                  \
        printf("%lg\n", val);

#define ADD_POINT()                                                                 \
        int tmp_ret_val = progress->execution.machine_pos + sizeof(int);            \
        stack_push(&progress->calls, &tmp_ret_val);

#define DEL_POINT()                                                                 \
        EMPTY_CALLS()                                                               \
        stack_pop(&progress->calls);

#define RETURN()                                                                    \
        EMPTY_CALLS()                                                               \
        progress->execution.machine_pos = *(int *) stack_front(&progress->calls);

#define NEG_CHECK(val)                                                              \
        if (!approx_equal(val, 0) && val < 0)                                       \
        {                                                                           \
            output_error(NEG_VALUE);                                                \
            return false;                                                           \
        }

#define DRAW_RAM()                                                                  \
                                                                                    \
    /* for (int i = 0; i <= 10; ++i)                                                \
    {                                                                               \
        fprintf(stderr, "%lg ", progress->ram[i]);                                  \
    }                                                                               \
    fprintf(stderr, "\n");*/                                                        \
                                                                                    \
    const int  wnd_size = 500;                                                      \
    const int cell_size = 5;                                                        \
    sf::Uint32 pixels[wnd_size][wnd_size] = {};                                     \
                                                                                    \
    for (int RAM_cnt = 0; RAM_cnt < RAM_NUM; ++RAM_cnt)                             \
    {                                                                               \
        if (!approx_equal(progress->ram[RAM_cnt], 0))                               \
        {                                                                           \
            int y0 = cell_size * (RAM_cnt / RAM_STR) + 1;                           \
            int x0 = cell_size * (RAM_cnt % RAM_STR) + 1;                           \
                                                                                    \
            for (int y = y0; y < y0 + 3; ++y)                                       \
            {                                                                       \
                for (int x = x0; x < x0 + 3; ++x)                                   \
                {                                                                   \
                    pixels[y][x] = 0xFF00FF00; /* GREEN */                          \
                }                                                                   \
            }                                                                       \
        }                                                                           \
    }                                                                               \
                                                                                    \
    sf::Texture tx;                                                                 \
    tx.create(500, 500);                                                            \
    tx.update((sf::Uint8 *) pixels, wnd_size, wnd_size, 0, 0);                      \
                                                                                    \
    sf::Sprite sprite(tx);                                                          \
    sprite.setPosition(0, 0);                                                       \
                                                                                    \
    wnd.clear(sf::Color::Red);                                                      \
    wnd.draw(sprite);                                                               \
    wnd.display();


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

    sf::RenderWindow wnd(sf::VideoMode(500, 500), "RAM");
    bool is_hlt = false;

    while (wnd.isOpen())
    {
        sf::Event event;
        while (wnd.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                wnd.close();
                break;
            }
        }

        progress->execution.machine_pos = sizeof(header);
        while (progress->execution.machine_pos < progress->execution_size && is_hlt == false)
        {
            unsigned char cmd = *(unsigned char *) get_machine_cmd(progress, sizeof(char));

            #define DEF_CMD(name, number, code)                                 \
                    case CMD_##name:                                            \
                        code                                                    \
                        break;

            #define DEF_JMP_CMD(name, number, cmp)                              \
                    case CMD_##name:                                            \
                    {                                                           \
                        GET_STK_TWO()                                           \
                        if (approx_cmp(b, a, #cmp)) cmd_jmp(progress);          \
                        else progress->execution.machine_pos += sizeof(int);    \
                        break;                                                  \
                    }
                
            switch ((cmd & mask01))
            {
                #include "cmd.h"
                default:
                    output_error(UNDEFINED_CMD);
                    return false;
            }
            #undef DEF_CMD
            #undef DEF_JMP_CMD
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

    if (cmd & CMD_MEM_ARG)
    {
        long ram_index = get_memory_val(progress, cmd);

        if (ram_index >= RAM_NUM) return MEMORY_LIMIT;
        
        stack_push(&progress->stk, &progress->ram[ram_index]);
        return OK;
    }
    
    stack_el push_val = get_stack_el_val(progress, cmd);
    stack_push(&progress->stk, &push_val);

    return OK;
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
    assert(progress != nullptr);

    if (stack_empty(&progress->stk)) return EMPTY_STACK;

    --progress->execution.machine_pos;
    unsigned char cmd = *(unsigned char *) get_machine_cmd(progress, sizeof(char));
    
    if (cmd & CMD_MEM_ARG)
    {
        long ram_index = get_memory_val(progress, cmd);

        if (ram_index >= RAM_NUM) return MEMORY_LIMIT;

        progress->ram[ram_index] = *(stack_el *) stack_front(&progress->stk);
        stack_pop(&progress->stk);
        
        return OK;
    }
    if (cmd & CMD_REG_ARG)
    {
        char reg_pos = *(char *) get_machine_cmd(progress, sizeof(char));
        if (reg_pos <= REG_NUM / 2) progress->long_regs[reg_pos] = (long) *(stack_el *) stack_front(&progress->stk);
        else
        {
            reg_pos -= 5; //5 - number of long-type registers
            progress->regs[reg_pos] = *(stack_el *) stack_front(&progress->stk);
        }
        stack_pop(&progress->stk);

        return OK;
    }
    if (cmd & CMD_NUM_ARG)
    {
        stack_pop(&progress->stk);
    }
    return OK;
}

long get_memory_val(cpu_store *const progress, const unsigned char cmd)
{
    assert(progress != nullptr);

    long ram_index = 0;
    if (cmd & CMD_REG_ARG) ram_index += get_reg_val(progress, *(char *) get_machine_cmd(progress, sizeof(char)));
    if (cmd & CMD_NUM_ARG) ram_index += *(long *) get_machine_cmd(progress, sizeof(long));

    return ram_index;
}

stack_el get_stack_el_val(cpu_store *progress, const unsigned char cmd)
{
    assert(progress != nullptr);

    stack_el val = 0;
    if (cmd & CMD_REG_ARG) val += get_reg_val(progress, *(char *) get_machine_cmd(progress, sizeof(char)));
    if (cmd & CMD_NUM_ARG) val += *(stack_el *) get_machine_cmd(progress, sizeof(stack_el));

    return val;
}

/**
*   @brief Gets value from "*progress" registers by the register number.
*
*   @param progress [in] - "cpu_store" contains all information about program
*   @param reg_num  [in] - number of register to take the value from
*
*   @return value from the register
*/

stack_el get_reg_val(cpu_store *const progress, const char reg_num)
{
    assert(progress != nullptr);

    if (reg_num <= REG_NUM / 2) return progress->long_regs[reg_num];
    return progress->regs[reg_num - 5];
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

bool approx_cmp(const stack_el a, const stack_el b, char *type)
{
    assert (type != nullptr);

    bool is_equal = approx_equal(a, b);

    if (!strcmp(type, ">")) return !is_equal && a > b;
    if (!strcmp(type, ">=")) return is_equal || a > b;
    if (!strcmp(type, "<")) return !is_equal && a < b;
    if (!strcmp(type, "<=")) return is_equal || a < b;
    if (!strcmp(type, "==")) return is_equal;
    if (!strcmp(type, "!=")) return !is_equal;

    return false;
}

/**
*   @brief Compare two double numbers with error rate DELTA.
*
*   @param a [in] - first  number to compare
*   @param b [in] - second number to compare
*
*   @return true if numbers are approximately equal and false else
*/

bool approx_equal(const double a, const double b)
{
    return fabs(a - b) < DELTA;
}