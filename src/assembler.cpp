/** @file */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#include <sys/stat.h>

#include "logs.h"

struct source
{
    char  *source_code;
    size_t source_size;
};

/*-----------------------------------------FUNCTION_DECLARATION-----------------------------------------*/

char *read_file(const char *file_name, size_t *const size_ptr);

unsigned get_file_size(const char *file_name);

/*------------------------------------------------------------------------------------------------------*/

int main(int argc, const char *argv[])
{
    source program = {};

    program.source_code = read_file(argv[1], &program.source_size);

    if (program.source_code == nullptr)
    {
        fprintf(stderr, "Can't open the file \"%s\"\n", argv[1]);
        return 1;
    }

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
