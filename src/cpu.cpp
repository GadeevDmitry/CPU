#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "read_write.h"

struct cpu_store
{
    struct machine execution;
    size_t execution_size;
};

int main(int argc, char *argv[])
{
    struct cpu_store progress = {};
    progress.execution.machine_code = read_file(argv[1], &progress.execution_size);
}

