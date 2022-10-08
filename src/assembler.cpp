/** @file */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#include <sys/stat.h>

#define RED    "\e[1;31m"
#define CANCEL "\e[0m"

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

struct header
{
    char fst_let;
    char sec_let;
    char version;

    size_t cmd_num;
};

enum CMD
{
    CMD_HLT          ,
    CMD_PUSH         ,
    CMD_ADD          ,
    CMD_SUB          ,
    CMD_MUL          ,
    CMD_DIV          ,
    CMD_OUT          ,
    CMD_NOT_EXICTING
};

/*-----------------------------------------FUNCTION_DECLARATION-----------------------------------------*/

bool     read_double   (source *const program, src_location *const info, double *const arg);

CMD      identify_cmd  (const char *cmd);

char    *read_file     (const char *file_name, size_t *const size_ptr);

void     write_file    (void *data, const int data_size);
void     read_cmd      (source *program, src_location *info);
void     skip_spaces   (source *const program, src_location *const info);
void    *assembler     (source *program, size_t *const cpu_size);

unsigned get_file_size (const char *file_name);

/*------------------------------------------------------------------------------------------------------*/

int main(int argc, const char *argv[])
{
    source program = {};

    program.src_code = read_file(argv[1], &program.src_size);

    if (program.src_code == nullptr)
    {
        fprintf(stderr, RED "ERROR: " CANCEL "Can't open the file \"%s\"\n", argv[1]);
        return 1;
    }

    header machine_info = {'G', 'D', 1, 0};
    void  *machine_data = assembler(&program, &machine_info.cmd_num);

    if (machine_data == nullptr) return 1;

    *(header *) machine_data = machine_info;

    write_file(machine_data, machine_info.cmd_num + sizeof(header));
}

/**
*   @brief Translates "source code" to "machine code".
*
*   @param program  [in]  - pointer to the structure with information about source
*   @param cpu_size [out] - pointer to the variable to put the size of "machine code" (in bytes) in
*
*   @return array consisting of "machine code" 
*/

void *assembler(source *program, size_t *const cpu_size)
{
    assert(program != nullptr);

    src_location info = {0, 1, (char *) calloc(sizeof(char), program->src_size + 1)};
    assert(info.cur_src_cmd != nullptr);

    machine cpu = {calloc(sizeof(double), program->src_size), sizeof(header)};
    assert( cpu.machine_code != nullptr);

    bool error = false;

    skip_spaces(program, &info);

    while (info.cur_src_pos < program->src_size)
    {
        read_cmd   (program, &info);
        CMD status_cmd = identify_cmd(info.cur_src_cmd);

        if (status_cmd == CMD_NOT_EXICTING)
        {
            error = true;
            fprintf(stderr, "line %d: " RED "ERROR: " CANCEL "command \"%s\" is not existing\n", info.cur_src_line, info.cur_src_cmd);
        }
        else
        {
            *((char *) cpu.machine_code + cpu.machine_pos) = status_cmd;
            cpu.machine_pos += sizeof(char);

            if (status_cmd == CMD_PUSH)
            {
                double arg = 0;
                if (read_double(program, &info, &arg))
                {
                    *(double *) ((char *) cpu.machine_code + cpu.machine_pos) = arg;
                    cpu.machine_pos += sizeof(double);
                }
                else
                {
                    error = true;
                    fprintf(stderr, "line %d: " RED "ERROR: " CANCEL "\"%s\" is not valid double\n", info.cur_src_line, info.cur_src_cmd);
                }
            }
        }
        
        skip_spaces(program, &info);
    }

    if (error) return nullptr;

    *cpu_size = cpu.machine_pos - sizeof(header); //only machine commands (without header)
    return cpu.machine_code;
}

/**
*   @brief Reads another command from source and puts it in the array "info->cur_src_cmd".
*
*   @param program [in]  - pointer to the structure with information about source
*   @param info    [out] - pointer to the structure with information abour location in source
*
*   @return nothing
*/

void read_cmd(source *program, src_location *info)
{
    assert(program != nullptr);
    assert(info    != nullptr);

    int cur_char = 0;
    int cmd_counter = 0;

    while (info->cur_src_pos < program->src_size &&
           !isspace(cur_char = program->src_code[info->cur_src_pos]))
    {
        info->cur_src_cmd[cmd_counter++] = cur_char;
        info->cur_src_pos++;
    }
    
    info->cur_src_cmd[cmd_counter] = '\0';
}

/**
*   @brief Identifies the command "cmd". Returns corresponding value from enum "CMD".
*
*   @param cmd [in] - pointer to the first byte of null-terminated byte string coding the command
*
*   @return the value from enum "CMD" that corresponds to "cmd"
*
*   @note return "CMD_NOT_EXICTING" if command "cmd" is invalid
*/

CMD identify_cmd(const char *cmd)
{
    assert(cmd != nullptr);

    if (strcasecmp(cmd, "HLT" ) == 0) return CMD_HLT ;
    if (strcasecmp(cmd, "PUSH") == 0) return CMD_PUSH;
    if (strcasecmp(cmd, "ADD" ) == 0) return CMD_ADD ;
    if (strcasecmp(cmd, "SUB" ) == 0) return CMD_SUB ;
    if (strcasecmp(cmd, "MUL" ) == 0) return CMD_MUL ;
    if (strcasecmp(cmd, "DIV" ) == 0) return CMD_DIV ;
    if (strcasecmp(cmd, "OUT" ) == 0) return CMD_OUT ;

    return CMD_NOT_EXICTING;
}

/**
*   @brief Reads another argument from source. Checks if the argument is valid double. Puts the argument in "*arg".
*
*   @param program [in]  - pointer to the structure with information about source
*   @param info    [out] - pointer to the structure with information about location in source
*   @param arg     [out] - pointer to the double variable to put in
*
*   @return true if argument is valid and false else
*
*   @note you should ignore the value of "*arg" if argument is invalid
*/

bool read_double(source *const program, src_location *const info, double *const arg)
{
    assert(program != nullptr);
    assert(info    != nullptr);

    skip_spaces(program, info);
    read_cmd(program, info);

    char *check = nullptr;
    *arg = strtod(info->cur_src_cmd, &check);

    return !(*check);
}

/**
*   @brief Skips space_characters. Stops when non-space_character founded. Char "c" is the space_character if isspace("c") is true.
*   @brief Changes "info->cur_src_pos"  if space_characters  are founded.
*   @brief Changes "info->cur_src_line" if backslash_n chars are founded.
*
*   @param program [in]  - pointer to the structure with information about source
*   @param info    [out] - pointer to the structure with information about location in source
*
*   @return nothing
*/

void skip_spaces(source *const program, src_location *const info)
{
    assert(program != nullptr);
    assert(info    != nullptr);

    int cur_char = 0;

    while (info->cur_src_pos < program->src_size &&
           isspace(cur_char = program->src_code[info->cur_src_pos]))
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