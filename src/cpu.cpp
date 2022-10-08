#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define RED    "\e[1;31m"
#define CANCEL "\e[0m"

#include "cpu.h"
#include "read_write.h"

typedef double Stack_elem;
#include "stack.h"

struct cpu_store
{
    struct machine execution;
    size_t execution_size;

    Stack stack;
};

/*-----------------------------------------FUNCTION_DECLARATION-----------------------------------------*/

bool check_signature(cpu_store *progress);
bool execution      (cpu_store *progress);

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

bool cmd_add(cpu_store *progress)
{
    assert(progress != nullptr);

    double var_1 = 0, var_2 = 0;

    bool is_empty = false;
    Stack_IsEmpty(&progress->stack, &is_empty);

    if (is_empty) return false;

    StackPop     (&progress->stack, &var_1);
    Stack_IsEmpty(&progress->stack, &is_empty);

    if (is_empty) return false;

    StackPop(&progress->stack, &var_2);

    return var_1 + var_2;
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
