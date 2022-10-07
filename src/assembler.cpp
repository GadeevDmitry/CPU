/** @file */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#include <sys/stat.h>

#include "assembler.h"
#include "logs.h"

/* const char *help =  "./asm [option] file\n"
                    "Option: --help       Display the information\n"; */

int main(int argc, const char *argv[])
{
    log_message(USUAL, "main(int argc = %d, const char argv[])\n\n", argc);

    program source = {nullptr, 0, "STK", 1, 0};

    source.data = read_file(argv[1], &source.Size);

    log_message(BLUE, "source: data =\n"
                      "\"%s\"\n"
                      "Size = %u\n\n", source.data, source.Size);
    
    if (source.data == nullptr)
    {
        fprintf(stderr, "Can't open the file\n");
        return 1;
    }

    size_t machine_code_size = 0;
    void *cpu_cmd_ptr = assembler(&source, &machine_code_size);

    if (cpu_cmd_ptr == nullptr) return 1;

    write_machine_code(cpu_cmd_ptr, machine_code_size);

    log_func_end(0);
}

/**
*   @brief Translates commands from "source" to machine code using enum STATUS.
*
*   @param source            [in]  - pointer to the struct "program" which contains supportive information and source code
*   @param machine_code_size [out] - size (in bytes) of cpu commands and their arguments
*
*   @return pointer to the array with all cpu commands and arguments
*/

void *assembler(program *source, size_t *const machine_code_size)
{
    log_message(USUAL, "assembler(source = %p, machine_code_size = %p)\n\n", source, machine_code_size);

    void  *cpu_cmd_ptr  = calloc(20, source->Size);

    //---------------------------------------------

    log_message(BLUE, "cpu_cmd_ptr = %p\n", cpu_cmd_ptr);

    for (int i = 0; i < source->Size - 1; ++i)
    {
        *((char *) cpu_cmd_ptr + i) = 'X';
    }
    //---------------------------------------------

    int    cpu_pos      = sizeof(supportive);

    assert(cpu_cmd_ptr != nullptr);

    int  data_pos               = 0;
    int  current_line           = 1 + pass_spaces(source->data, source->Size, &data_pos);

    while (data_pos < source->Size)
    {
        char cmd[CMD_LEN + 1]       = ""; //CMD_LEN + (1) <- for null-character in the end

        STATUS cmd_status = get_param(source->data, source->Size, &data_pos, cmd, CMD_LEN);
        cmd_status = (cmd_status == UNDEFINED_CMD) ? UNDEFINED_CMD : add_cmd(cmd, cpu_cmd_ptr, &cpu_pos); 
        
        current_line += pass_spaces(source->data, source->Size, &data_pos);

        if (cmd_status == PUSH)
        {
            ++source->info.commands_number;

            double value = 0;
            STATUS value_status = get_double(source->data, source->Size, &data_pos, &value);

            if (value_status)
            {
                log_message(BLUE, "value = %lg\n\n", value);

                *(double *)((char *) cpu_cmd_ptr + cpu_pos) = value;
                cpu_pos += sizeof(double);
                
                current_line += pass_spaces(source->data, source->Size, &data_pos);
            }
            else
            {
                fprintf(stderr, "ERROR: Undefined double value\n"
                                " line: %d\n", current_line);

                log_func_end(1);
                return nullptr;
            }
        }
        else if (cmd_status == UNDEFINED_CMD)
        {
            fprintf(stderr, "ERROR: Undefined command\n"
                            " line: %d\n", current_line);
            
            log_func_end(1);
            return nullptr;
        }

        log_message(BLUE, "cpu_cmd_ptr = \"%s\"\n\n", (char *) cpu_cmd_ptr);

        current_line += pass_spaces(source->data, source->Size, &data_pos);
        ++source->info.commands_number;
    }

    *((supportive *) cpu_cmd_ptr) = source->info;
    *machine_code_size = cpu_pos;

    log_func_end(0);
    return cpu_cmd_ptr;
}

/**
*   @brief Reads another command or argument from array "data". Stops reading when space-character is found.
*   @brief Space-character is the character for which isspace() is true.
*
*   @param data      [in]  - pointer to the first element of the source commands array
*   @param data_size [in]  - size of the array "data"
*   @param data_pos  [in]  - index of the current element in "data"
*   @param str       [out] - pointer to the first element of the array to put the command in
*   @param str_size  [in]  - size of the array "str"
*
*   @return  enum "STATUS" value "OK" if size of the command not more than size of the "str" and value "UNDEFINED_CMD" else.
*/

STATUS get_param(const char *const data,
                 const size_t      data_size,
                 int  *const       data_pos,
                 char *const       str,
                 const size_t      str_size)
{
    assert(data     != nullptr);
    assert(data_pos != nullptr);
    assert(str      != nullptr);

    log_message(USUAL, "get_param(data =\n"
                       "\"%s\",\n"
                       "          data_size = %u, *data_pos = %d, str = %p, str_size)\n\n", data, data_size, *data_pos, str);

    for (int str_pos = 0; (*data_pos < data_size) && (str_pos < str_size); ++str_pos)
    {
        char cur_char = data[*data_pos];

        if (!isspace(cur_char))
        {
            ++*data_pos;
            str[str_pos] = cur_char;
        }
        else break;
    }

    if (!isspace(data[*data_pos]))
    {
    log_func_end(1);
    return UNDEFINED_CMD;
    }

    log_func_end(0);
    return OK;

}

#define _cmd_cmp(real_cmd, user_cmd, cmd_name)                                  \
        if (strcmp(real_cmd, user_cmd) == 0)                                    \
        {                                                                       \
            *((char *) cpu_cmd_ptr + (*cpu_pos)++) = (char) cmd_name;           \
            log_message(BLUE, "cmd_name = %d\n", cmd_name);                     \
                                                                                \
            log_func_end(0);                                                    \
            return cmd_name;                                                    \
        }

/**
*   @brief Determines the another command "cmd" and writes it in the "cpu_cmd_ptr" array.
*
*   @param cmd         [in]      - pointer to the first byte of the another command
*   @param cpu_cmd_ptr [in][out] - pointer to the first byte of the array to write in
*   @param cpu_pos     [in]      - number of employed bytes in the "cpu_cmd_ptr"
*
*   @return enum "STATUS" value meaning the cpu command
*
*   @note returns "UNDEFINED_CMD" if "cmd" is a non-existent command
*/

STATUS add_cmd(const char *cmd, void *cpu_cmd_ptr, int *const cpu_pos)
{
    log_message(USUAL, "add_cmd(cmd = \"%s\", cpu_cmd_ptr = %p, *cpu_pos = %d)\n\n", cmd, cpu_cmd_ptr, *cpu_pos);

    assert(cmd         != nullptr);
    assert(cpu_cmd_ptr != nullptr);
    assert(cpu_pos     != nullptr);

    _cmd_cmp("hlt" , cmd, HLT) ;
    _cmd_cmp("push", cmd, PUSH);
    _cmd_cmp("add" , cmd, ADD) ;
    _cmd_cmp("sub" , cmd, SUB) ;
    _cmd_cmp("mul" , cmd, MUL) ;
    _cmd_cmp("div" , cmd, DIV) ;
    _cmd_cmp("out" , cmd, OUT) ;
    
    log_func_end(1);
    return UNDEFINED_CMD;
}

#undef _cmd_cmp

/**
*   @brief Reads a "double" number from array "data". Stops reading when space-character is found. Space-character is the character for which isspace() is true.
*   @brief Determines if the double number representation is valid.
*
*   @param data      [in]  - pointer to the first element of source commands array
*   @param data_size [in]  - size of array "data"
*   @param data_pos  [in]  - index of the current element in "data"
*   @param value     [out] - pointer to the "double" number
*
*   @return enum "STATUS" value "OK" if double_number is valid and "UNDEFINED_CMD" else
*/

STATUS get_double(const char *const data,
                  const size_t      data_size,
                  int    *const     data_pos,
                  double *const     value)
{
    log_message(USUAL, "get_double(data =\n"
                        "\"%s\",\n"
                        "data_size = %u, *data_pos = %d, value)\n\n", data, data_size, *data_pos);

    assert(data     != nullptr);
    assert(data_pos != nullptr);
    assert(value    != nullptr);

    char double_num[DOUBLE_LEN + 1] = ""; //DOUBLE_LEN + (1) <- for null-character in the end

    STATUS num_status = get_param(data, data_size, data_pos, double_num, DOUBLE_LEN);

    if (num_status == OK)
    {
        char *str_end = nullptr;

        *value = strtod(double_num, &str_end);
        assert(str_end != nullptr);

        if (str_end != double_num && !*str_end)
        {
            log_func_end(0);
            return OK;
        }
    }

    log_func_end(1);
    return UNDEFINED_CMD;
}

/**
*   @brief Skips all space-characters. Stops skipping when a non-space-character is found. Space-character is the character for which isspace() is true.
*   @brief Counts backslash_n characters on the way. It is necessary to determine a number of current line in every moment.
*
*   @param data      [in] - pointer to the first element of source commands array
*   @param data_size [in] - size of array "data"
*   @param data_pos  [in] - index of the current element in "data"
*
*   @return number of backslash_n characters founded
*/

int pass_spaces(const char *const data, const size_t data_size, int *const data_pos)
{
    assert(data     != nullptr);
    assert(data_pos != nullptr);

    int backslash_n = 0;

    while (*data_pos < data_size && isspace(data[*data_pos]))
    {
        if (data[*data_pos] == '\n') ++backslash_n;
        ++*data_pos;
    }

    return backslash_n;
}

/**
*   @brief Allocates memory for input data and reads it from file "file_name".
*
*   @param file_name [in] - name of the file to read from
*   @param Size_ptr  [out] - pointer to the size of the file "file_name"
*
*   @return pointer to the array with data
*/

char *read_file(const char *file_name, size_t *const Size_ptr)
{
    assert(file_name != nullptr);
    assert(Size_ptr  != nullptr);

    *Size_ptr      = get_file_size(file_name);

    log_message(BLUE, "*Size_ptr = %u\n", *Size_ptr);

    if (*Size_ptr == -1) return nullptr;

    FILE  *stream  = fopen(file_name, "rb");
    if (   stream == nullptr) return nullptr;

    char *data_ptr = (char *) calloc(*Size_ptr, sizeof(char));

    fread (data_ptr, sizeof(char), *Size_ptr, stream);
    fclose(stream);

    return data_ptr;
}

/**
*   @brief Determines the size (in bytes) of file "file_name".
*
*   @param file_name [in] - name of the file to determine
*
*   @return size (in bytes) of the file
*/

unsigned get_file_size(const char *file_name)
{
    assert(file_name != nullptr);

    struct stat BuffSize = {};

    int StatRet = stat(file_name, &BuffSize);
    if (StatRet == -1) return -1;

    return BuffSize.st_size;
}

/**
*   @brief Writes the machine code in the "cpu.stk".
*
*   @param cpu_cmd_ptr - pointer to the translated from source to machine code
*   @param cpu_size    - size (in bytes) of "cpu_cmd_ptr"
*
*   @return nothing
*/

void write_machine_code(void *cpu_cmd_ptr, const size_t cpu_size)
{
    log_message(USUAL, "write_machine_code(cpu_cmd_ptr = \"%s\",\n"
                       "cpu_size = %u)\n\n", (char *)cpu_cmd_ptr, cpu_size);

    assert(cpu_cmd_ptr != nullptr);

    FILE  *stream = fopen("cpu.stk", "wb");
    assert(stream != nullptr);

    fwrite(cpu_cmd_ptr, 1, cpu_size, stream);
    fclose(stream);

    log_func_end(0);
}