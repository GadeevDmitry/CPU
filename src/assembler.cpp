/** @file */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#include <sys/stat.h>

#include "logs.h"

typedef int elem_t;

struct source
{
    char  *src_code;
    size_t src_size;
};

struct src_location
{
    int cur_src_pos;
    int cur_src_line;

    char *cur_src_cmd;
};

struct machine
{
    void *machine_code;
    int   machine_pos;
};

/*-----------------------------------------FUNCTION_DECLARATION-----------------------------------------*/

void write_file(void *data, const int data_size);

char *read_file(const char *file_name, size_t *const size_ptr);

unsigned get_file_size(const char *file_name);

/*------------------------------------------------------------------------------------------------------*/


int main(int argc, const char *argv[])
{
    source program = {};

    program.src_code = read_file(argv[1], &program.src_size);

    if (program.src_code == nullptr)
    {
        fprintf(stderr, "Can't open the file \"%s\"\n", argv[1]);
        return 1;
    }
}

void *assembler(source *program)
{
    assert(program != nullptr);

    src_location info = {0, 1, (char *) calloc(sizeof(char), program->src_size)};
    assert(info.cur_src_cmd != nullptr);

    machine cpu = {calloc(sizeof(elem_t), program->src_size), 0};
    assert( cpu.machine_code != nullptr);
}

/**
*   @brief Skips space_characters. Stops when non-space_character founded. Char "c" is the space_character if isspace("c") is true.
*   @brief Changes "info->cur_src_pos"  if space_characters  are founded.
*   @brief Changes "info->cur_src_line" if backslash_n chars are founded.
*
*   @param program [in] - pointer to the structure with information about source
*   @param info    [in] - pointer to the structure with information about location in source
*
*   @return nothing
*/

void skip_spaces(source *const program, src_location *const info)
{
    assert(program != nullptr);
    assert(info    != nullptr);

    int cur_char = 0;

    while (info->cur_src_pos < program->src_size && !isspace(cur_char = program->src_code[info->cur_src_pos]))
    {
        if (cur_char == '\n') ++info->cur_src_line;

        ++info->cur_src_pos;
    }
}

/**
*   @brief Writes machine code from "data" into the "machine.cpu".
*
*   @param data      [in] - pointer to the first element of array with machine code.
*   @param data_size [in] - size (in bytes) of "data"
*
*   @return nothing 
*/

void write_file(void *data, const int data_size)
{
    FILE  *stream = fopen("machine.cpu", "wb");
    assert(stream != nullptr);
    assert(data   != nullptr);

    fwrite(data, sizeof(char), data_size, stream);
    fclose(stream);
}

/**
*   @brief Allocates memory for input data and reads it from the file "file_name".
*
*   @param file_name [in]  - name of the file to read from
*   @param size_ptr  [out] - pointer to the size of the file "file_name"
*
*   @return pointer to the array with data and nullptr in case of error
*/

char *read_file(const char *file_name, size_t *const size_ptr)
{
    assert(file_name != nullptr);
    assert(size_ptr  != nullptr);

    *size_ptr = get_file_size(file_name);

    log_message(BLUE, "*size_ptr = %u\n", *size_ptr);

    if (*size_ptr == -1) return nullptr;

    FILE *stream  = fopen(file_name, "r");
    if (  stream == nullptr) return nullptr;

    char *data_ptr = (char *) calloc(*size_ptr, sizeof(char));

    fread (data_ptr, sizeof(char), *size_ptr, stream);
    fclose(stream);

    return data_ptr;
}

/**
*   @brief Determines the size (in bytes) of file "file_name".
*
*   @param file_name [in] - name of the file to determine
*
*   @return size (in bytes) of the file
*
*   @note return -1 in case of error
*/

unsigned get_file_size(const char *file_name)
{
    assert(file_name != nullptr);

    struct stat BuffSize = {};

    int StatRet = stat(file_name, &BuffSize);
    if (StatRet == -1) return -1;

    return BuffSize.st_size;
}