#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "read_write.h"

struct cpu_store
{
    struct machine execution;
    size_t execution_size;
};

/*-----------------------------------------FUNCTION_DECLARATION-----------------------------------------*/

bool check_signature(cpu_store *progress);

/*------------------------------------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    cpu_store progress = {};
    progress.execution.machine_code = read_file(argv[1], &progress.execution_size);

    if (!check_signature(&progress)) return 1;
}

bool check_signature(cpu_store *progress)
{
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
