/** @file */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>

#define RED    "\e[1;31m"
#define CANCEL "\e[0m"

#include "read_write.h"
#include "tag.h"

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
    CMD_NUM_ARG      = 1 << 4 ,
    CMD_REG_ARG      = 1 << 5 ,
    CMD_MEM_ARG      = 1 << 6
};

const int REG_NUM = 8;
const char *reg_names[] = 
{
    "empty",
    "rax"  ,
    "rbx"  ,
    "rcx"  ,
    "rdx"  ,
    "rex"  ,
    "rfx"  ,
    "rgx"  ,
    "rhx"
};

/*-----------------------------------------FUNCTION_DECLARATION-----------------------------------------*/

CMD      identify_cmd    (const char *cmd);

bool     cmd_push        (source *const program, src_location *const info, machine *const cpu);
bool     cmd_pop         (source *const program, src_location *const info, machine *const cpu);
bool     cmd_jmp         (source *const program, src_location *const info, machine *const cpu, tag *const label);
bool     is_double       (const char *s, double *const val);
bool     is_reg          (const char *s, char   *const pos);

void     tag_ctor        (tag *const label);
void     add_machine_cmd (machine *const cpu, const size_t val_size, void *val_ptr);
void     read_val        (source *program, src_location *info, const char sep);
void     skip_spaces     (source *const program, src_location *const info);
void    *assembler       (source *program, size_t *const cpu_size, tag *const label);

/*------------------------------------------------------------------------------------------------------*/

int main(int argc, const char *argv[])
{
    source program = {};
    program.src_code = (char *) read_file(argv[1], &program.src_size);

    if (program.src_code == nullptr)
    {
        fprintf(stderr, RED "ERROR: " CANCEL "Can't open the file \"%s\"\n", argv[1]);
        return 1;
    }

    tag label = {};
    tag_ctor(&label);

    header machine_info = {'G', 'D', 2, 0};
    void  *machine_data = assembler(&program, &machine_info.cmd_num, &label);

    if (machine_data == nullptr) return 1;

    *(header *) machine_data = machine_info;

    if (write_file(argv[2], machine_data, machine_info.cmd_num + sizeof(header)) == false)
    {
        fprintf(stderr, RED "ERROR: " CANCEL "Can't open the file to write the machine code in\n");
        return 1;
    }
    return 0;
}

/**
*   @brief Translates "source code" to "machine code".
*
*   @param program  [in]  - pointer to the structure with information about source
*   @param cpu_size [out] - pointer to the variable to put the size of "machine code" (in bytes) in
*
*   @return array consisting of "machine code" 
*/

void *assembler(source *program, size_t *const cpu_size, tag *const label)
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
        read_val(program, &info, ':');
        CMD status_cmd = identify_cmd(info.cur_src_cmd);

        //possibly mark
        if (status_cmd == CMD_NOT_EXICTING && info.cur_src_pos < program->src_size)
        {
            int mrk_size              = strlen(info.cur_src_cmd);
            char *possible_mark_begin = program->src_code + (info.cur_src_pos - mrk_size);
            skip_spaces(program, &info);

            if (program->src_code[info.cur_src_pos] != ':' || !tag_push(label, {possible_mark_begin, mrk_size, cpu.machine_pos}))
            {
                fprintf(stderr, "line %4d: " RED "ERROR: " CANCEL "the mark \"%s\" has already met\n", info.cur_src_line, info.cur_src_cmd);
            }
            
            ++info.cur_src_pos;
            skip_spaces(program, &info);
            continue;
        }

        switch (status_cmd)
        {
            case CMD_NOT_EXICTING:
                error = true;
                fprintf(stderr, "line %4d: " RED "ERROR: " CANCEL "command \"%s\" is not existing\n", info.cur_src_line, info.cur_src_cmd);
                break;
            
            case CMD_PUSH:
                if (!cmd_push(program, &info, &cpu)) error = true;
                break;

            case CMD_POP:
                if (!cmd_pop(program, &info, &cpu)) error = true;
                break;

            case CMD_JMP:
                if (!cmd_jmp(program, &info, &cpu, label)) error = true;
                break;

            default:
                add_machine_cmd(&cpu, sizeof(char), &status_cmd);
                break;
        }

        skip_spaces(program, &info);
    }

    if (error) return nullptr;

    *cpu_size = cpu.machine_pos - sizeof(header); //only machine commands (without header)
    return cpu.machine_code;
}

/**
*   @brief Reads another command or argument from source and puts it in the array "info->cur_src_cmd".
*
*   @param program [in]  - pointer   to the structure with information about source
*   @param info    [out] - pointer   to the structure with information abour location in source
*   @param sep     [in]  - character to stop reading after meeting it 
*   
*   @return nothing
*
*   @note reading also stops after meeting a space-character
*/

void read_val(source *program, src_location *info, const char sep)
{
    assert(program != nullptr);
    assert(info    != nullptr);

    int cur_char = 0;
    int cmd_counter = 0;

    while (info->cur_src_pos < program->src_size &&
           !isspace(cur_char = program->src_code[info->cur_src_pos]) && cur_char != sep)
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
    if (strcasecmp(cmd, "POP" ) == 0) return CMD_POP ;
    if (strcasecmp(cmd, "JMP" ) == 0) return CMD_JMP ;

    return CMD_NOT_EXICTING;
}

/**
*   @brief Reads push-arguments. Checks if they are valid. There is not more than one "double" argument and one "register_name" argument.
*   @brief Adds commands and arguments in "cpu->machine.code".
*
*   @param program [in]  - pointer to the structure with information about source
*   @param info    [in]  - pointer to the structure with information abour location in source
*   @param cpu     [out] - pointer to the struct "machine" to add the command and arguments in "cpu->machine_code"
*
*   @return true if arguments are correct and false else
*/

bool cmd_push(source *const program, src_location *const info, machine *const cpu)
{
    assert(program != nullptr);
    assert(info    != nullptr);
    assert(cpu     != nullptr);

    char cmd = CMD_PUSH;

    skip_spaces(program, info);
    read_val   (program, info, '+');

    double dbl_arg = 0;
    char   reg_arg = 0;
    if (is_double(info->cur_src_cmd, &dbl_arg))
    {
        cmd = cmd | CMD_NUM_ARG;
        
        skip_spaces(program, info);
        if (info->cur_src_pos < program->src_size && program->src_code[info->cur_src_pos] == '+')
        {
            ++info->cur_src_pos;
            read_val(program, info, ' ');

            if (is_reg(info->cur_src_cmd, &reg_arg))
            {
                cmd = cmd | CMD_REG_ARG;

                add_machine_cmd(cpu, sizeof(char)  , &cmd);
                add_machine_cmd(cpu, sizeof(char)  , &reg_arg);
                add_machine_cmd(cpu, sizeof(double), &dbl_arg);

                return true;
            }
            else
            {
                fprintf(stderr, "line %4d: " RED "ERROR: " CANCEL "\"%s\" is not a register name\n", info->cur_src_line, info->cur_src_cmd);
                return false;
            }
        } //if only double arg
        else
        {
            add_machine_cmd(cpu, sizeof(char)  , &cmd);
            add_machine_cmd(cpu, sizeof(double), &dbl_arg);

            return true;
        }
    } //if first argument is not "double"
    else if (is_reg(info->cur_src_cmd, &reg_arg))
    {
        cmd = cmd | CMD_REG_ARG;

        skip_spaces(program, info);
        if (info->cur_src_pos < program->src_size && program->src_code[info->cur_src_pos] == '+')
        {
            ++info->cur_src_pos;
            read_val(program, info, ' ');

            if (is_double(info->cur_src_cmd, &dbl_arg))
            {
                cmd = cmd | CMD_NUM_ARG;

                add_machine_cmd(cpu, sizeof(char)  , &cmd);
                add_machine_cmd(cpu, sizeof(char)  , &reg_arg);
                add_machine_cmd(cpu, sizeof(double), &dbl_arg);

                return true;
            }
            else
            {
                fprintf(stderr, "line %4d: " RED "ERROR: " CANCEL "\"%s\" is not a valid double\n", info->cur_src_line, info->cur_src_cmd);
                return false;
            }
        } //if only register arg
        else
        {
            add_machine_cmd(cpu, sizeof(char), &cmd);
            add_machine_cmd(cpu, sizeof(char), &reg_arg);

            return true;
        }
    } //if invalid arguments
    
    fprintf(stderr, "line %4d: " RED "ERROR: " CANCEL "\"%s\" is not a valid push-argument\n", info->cur_src_line, info->cur_src_cmd);
    return false;
}

/**
*   @brief Reads pop-arguments. Checks if they are valid. Adds commands and arguments in "cpu->machine.code".
*
*   @param program [in]  - pointer to the structure with information about source
*   @param info    [in]  - pointer to the structure with information abour location in source
*   @param cpu     [out] - pointer to the struct "machine" to add the command and arguments in "cpu->machine_code"
*
*   @return true if arguments are correct and false else
*/

bool cmd_pop(source *const program, src_location *const info, machine *const cpu)
{
    assert(program != nullptr);
    assert(info    != nullptr);
    assert(cpu     != nullptr);

    char cmd = CMD_POP;

    skip_spaces(program, info);
    read_val   (program, info, ' ');

    char reg_arg = 0;
    if (!strcmp(info->cur_src_cmd, "void"))
    {
        cmd = cmd | CMD_NUM_ARG;
        add_machine_cmd(cpu, sizeof(char), &cmd);
        
        return true;
    }
    else if (is_reg(info->cur_src_cmd, &reg_arg))
    {
        cmd = cmd | CMD_REG_ARG;
        add_machine_cmd(cpu, sizeof(char), &cmd);
        add_machine_cmd(cpu, sizeof(char), &reg_arg);

        return true;
    }

    fprintf(stderr, "line %4d: " RED "ERROR: " CANCEL "\"%s\" is not a valid pop-argument\n", info->cur_src_line, info->cur_src_cmd);
    return false;
}

/**
*   @brief Reads jmp-arguments. Check if they are valid. Adds commands and arguments in "cpu->machine.code".
*
*   @param program [in]  - pointer to the structure with information about source
*   @param info    [in]  - pointer to the structure with information abour location in source
*   @param cpu     [out] - pointer to the struct "machine" to add the command and arguments in "cpu->machine_code"
*
*   @return true if arguments are correct and false else
*/

bool cmd_jmp(source *const program, src_location *const info, machine *const cpu, tag *const label)
{
    assert(program != nullptr);
    assert(info    != nullptr);
    assert(cpu     != nullptr);
    assert(label   != nullptr);

    char cmd = CMD_JMP;

    skip_spaces(program, info);
    read_val   (program, info, ' ');

    int  label_pos = 0;
    if ((label_pos = tag_string_find(label, info->cur_src_cmd)) != -1)
    {
        add_machine_cmd(cpu, sizeof(char), &cmd);
        add_machine_cmd(cpu, sizeof(int) , &label->data[label_pos].machine_pos);

        return true;
    }

    fprintf(stderr, "line %4d: " RED "ERROR: " CANCEL "\"%s\" is not a mark\n", info->cur_src_line, info->cur_src_cmd);
    return false;
}

/**
*   @brief Adds command or argument in "cpu->machine_code".
*
*   @param cpu      - [out] pointer to the struct "machine" to add the command in "cpu->machine_code"
*   @param val_size - [in]  size (in bytes) of value to put
*   @param val_ptr  - [in]  pointer to the value to put
*
*   @return nothing
*/

void add_machine_cmd (machine *const cpu, const size_t val_size, void *val_ptr)
{
    assert(cpu     != nullptr);
    assert(val_ptr != nullptr);

    memcpy((char *) cpu->machine_code + cpu->machine_pos, val_ptr, val_size);
    cpu->machine_pos += val_size;
}

/**
*   @brief Checks if "*s" is valid double. Puts the value in "val".
*
*   @param s   [in]  - pointer to the first byte of null-terminated byte string to check
*   @param val [out] - pointer to the "double" variable to put the result
*
*   @return true if argument is valid and false else
*
*   @note you should ignore "*val" if "is_double()" returns false
*/

bool is_double(const char *s, double *const val)
{
    assert(s != nullptr);

    char  *check = nullptr;
    *val = strtod(s, &check);

    return !(*check) && check != s;
}

/**
*   @brief Checks if "*s" is the rigister name. Puts number of register in "pos".
*
*   @param s   [in]  - pointer to the first byte of null-terminated byte string to check
*   @param pos [out] - pointer to the number of register
*
*   @return true if "s" - name of register and false else
*
*   @note you should ignore "*pos" id "is_reg()" returns false
*/

bool is_reg(const char *s, char *const pos)
{
    assert(s   != nullptr);
    assert(pos != nullptr);

    for (char reg_cnt = 1; reg_cnt <= REG_NUM; ++reg_cnt)
    {
        if (!strcmp(s, reg_names[reg_cnt]))
        {
            *pos = reg_cnt;
            return true;
        }
    }
    return false;
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