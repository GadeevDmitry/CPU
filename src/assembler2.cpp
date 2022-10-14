/** @file */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>

#define RED    "\e[1;31m"
#define CANCEL "\e[0m"
#define GREEN  "\e[0;32m"

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

#define DEF_CMD(name, number, ...)     \
        CMD_##name = number,

#define DEF_JMP_CMD(name, number, ...) \
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

enum MARK
{
    MARK_GET   , // 0
    MARK_CHECK   // 1
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

CMD   identify_cmd      (const char *cmd);

bool  read_push_pop_arg (source *const program, src_location *const info, machine *const cpu, unsigned char cmd);
bool  cmd_pop           (source *const program, src_location *const info, machine *const cpu);
bool  cmd_jmp           (source *const program, src_location *const info, machine *const cpu, tag *const label, const char mark_mode, unsigned char cmd);
bool  get_mark          (source *const program, src_location *const info, machine *const cpu, tag *const label, int possible_mrk_beg, const char mark_mode);
bool  is_double         (const char *s, double *const val);
bool  is_long           (const char *s, long   *const val);
bool  is_reg            (const char *s, char   *const pos);
bool  is_long_reg       (const char *s, char   *const pos);

int   read_val          (source *program, src_location *info, const char sep1, const char sep2 = ' ');

void  tag_ctor          (tag *const label);
void  add_machine_cmd   (machine *const cpu, const size_t val_size, void *val_ptr);
void  skip_spaces       (source *const program, src_location *const info);
void *assembler         (source *program, size_t *const cpu_size, tag *const label, const char mark_mode);

/*------------------------------------------------------------------------------------------------------*/

int main(int argc, const char *argv[])
{
    source program  = {};

    program.src_code  = (char *) read_file(argv[1], &program.src_size);
    if (program.src_code == nullptr)
    {
        fprintf(stderr, RED "ERROR: " CANCEL "Can't open the file \"%s\"\n", argv[1]);
        return 1;
    }

    tag label = {};
    tag_ctor(&label);

    header machine_info = {'G', 'D', 2, 0};
    if (assembler(&program, &machine_info.cmd_num, &label, MARK_GET) == nullptr) return 1;

    void *machine_data = assembler(&program, &machine_info.cmd_num, &label, MARK_CHECK);
    *(header *) machine_data = machine_info;

    if (write_file(argv[2], machine_data, machine_info.cmd_num + sizeof(header)) == false)
    {
        free(machine_data);
        fprintf(stderr, RED "ERROR: " CANCEL "Can't open the file to write the machine code in\n");
        return 1;
    }

    free(machine_data);
    fprintf(stderr, GREEN "./ASM2 IS OK\n" CANCEL);
    return 0;
}

/**
*   @brief Translates "source code" to "machine code".
*
*   @param program   [in]  - pointer to the structure with information about source
*   @param cpu_size  [out] - pointer to the variable to put the size of "machine code" (in bytes) in
*   @param label     [out] - pointer to the "tag" variable to put marks in
*   @param mark_mode [in]  - mode of cmd-jump module
*
*   @return array consisting of "machine code" 
*/

void *assembler(source *program, size_t *const cpu_size, tag *const label, const char mark_mode)
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
        int possible_mark_begin = read_val(program, &info, ':');
        CMD status_cmd = identify_cmd(info.cur_src_cmd);

        switch (status_cmd)
        {
            case CMD_NOT_EXICTING:
                if (!get_mark(program, &info, &cpu, label, possible_mark_begin, mark_mode)) error = true;
                break;

            case CMD_PUSH:
                if (!read_push_pop_arg(program, &info, &cpu, CMD_PUSH)) error = true;
                break;

            case CMD_POP:
                if (!cmd_pop(program, &info, &cpu)) error = true;
                break;

            case CMD_JMP: case CMD_JA: case CMD_JAE: case CMD_JB:
            case CMD_JBE: case CMD_JE: case CMD_JNE: case CMD_CALL:
                if (!cmd_jmp(program, &info, &cpu, label, mark_mode, status_cmd)) error = true;
                break;

            default:
                add_machine_cmd(&cpu, sizeof(char), &status_cmd);
                break;
        }
        skip_spaces(program, &info);
    }

    free(info.cur_src_cmd);
    if (error) return nullptr;

    *cpu_size = cpu.machine_pos - sizeof(header); //only machine commands (without header)

    return cpu.machine_code;
}

/**
*   @brief Reads another command or argument from source and puts it in the array "info->cur_src_cmd". Skips all spaces before and after command.
*
*   @param program [in]  - pointer   to the structure with information about source
*   @param info    [out] - pointer   to the structure with information abour location in source
*   @param sep1    [in]  -         character to stop reading after meeting it
*   @param sep2    [in]  - another character to stop reading after meeting it
*   
*   @return  index of the first character of the command in the "program->src_code"
*
*   @note reading also stops after meeting a space-character
*/

int read_val(source *program, src_location *info, const char sep1, const char sep2)
{
    assert(program != nullptr);
    assert(info    != nullptr);

    skip_spaces(program, info);
    
    int ans = info->cur_src_pos;
    int cur_char = 0;
    int cmd_counter = 0;

    while (info->cur_src_pos < program->src_size &&
           !isspace(cur_char = program->src_code[info->cur_src_pos]) && cur_char != sep1 && cur_char != sep2)
    {
        info->cur_src_cmd[cmd_counter++] = cur_char;
        info->cur_src_pos++;
    }
    
    info->cur_src_cmd[cmd_counter] = '\0';

    return ans;
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

    #define DEF_CMD(name, ...)                              \
            if (!strcasecmp(cmd, #name)) return CMD_##name;
    
    #define DEF_JMP_CMD(name, ...)                          \
            if (!strcasecmp(cmd, #name)) return CMD_##name;

    #include "cmd.h"
    #undef DEF_CMD
    #undef DEF_JMP_CMD

    return CMD_NOT_EXICTING;
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

    unsigned char cmd = CMD_POP;
    skip_spaces(program, info);

    if (program->src_code[info->cur_src_pos] == '[') return read_push_pop_arg(program, info, cpu, CMD_POP);

    char reg_arg = 0;
    read_val(program, info, ' ');
    if (!strcmp(info->cur_src_cmd, "void"))
    {
        cmd = cmd | CMD_NUM_ARG;
        add_machine_cmd(cpu, sizeof(char), &cmd);
        
        return true;
    }
    else if (is_reg(info->cur_src_cmd, &reg_arg) || is_long_reg(info->cur_src_cmd, &reg_arg))
    {
        cmd = cmd | CMD_REG_ARG;
        add_machine_cmd(cpu, sizeof(char), &cmd);
        add_machine_cmd(cpu, sizeof(char), &reg_arg);

        return true;
    }

    fprintf(stderr, "line %4d: " RED "ERROR: " CANCEL "\"%s\" is not a valid pop-argument\n", info->cur_src_line, info->cur_src_cmd);
    return false;
}

#define MEM_SYNTAX_CHECK                                                                                                        \
        if  (cmd & CMD_MEM_ARG)                                                                                                 \
        {                                                                                                                       \
            if (info->cur_src_pos < program->src_size && program->src_code[info->cur_src_pos] == ']') ++info->cur_src_pos;      \
            else                                                                                                                \
            {                                                                                                                   \
                fprintf(stderr, "line %4d: " RED "ERROR: " CANCEL "there is no \']\' character\n", info->cur_src_line);         \
                return false;                                                                                                   \
            }                                                                                                                   \
        }

/**
*   @brief Reads push and pop arguments. Checks if they are valid. There is not more than one "double" argument and one "register_name" argument.
*   @brief Adds commands and arguments in "cpu->machine.code".
*
*   @param program [in]  - pointer to the structure with information about source
*   @param info    [in]  - pointer to the structure with information abour location in source
*   @param cpu     [out] - pointer to the struct "machine" to add the command and arguments in "cpu->machine_code"
*   @param cmd     [in]  - value equal to CMD_POP to read pop-arguments and CMD_PUSH to read push-arguments
*
*   @return true if arguments are correct and false else
*/

bool read_push_pop_arg(source *const program, src_location *const info, machine *const cpu, unsigned char cmd)
{
    assert(program != nullptr);
    assert(info    != nullptr);
    assert(cpu     != nullptr);

    assert(cmd == CMD_POP || cmd == CMD_PUSH);

    skip_spaces(program, info);
    if (program->src_code[info->cur_src_pos] == '[')
    {
        cmd = cmd | CMD_MEM_ARG;
        ++info->cur_src_pos;

        read_val(program, info, '+', ']');
        //------------
        //fprintf(stderr, "arg = %s\n", info->cur_src_cmd);
        //------------
        long long_arg = 0;
        char long_reg = 0;
        if (is_long(info->cur_src_cmd, &long_arg))
        {
            //------------------
            //fprintf(stderr, "arg = \"%s\" is long\n", info->cur_src_cmd);
            //------------------
            cmd = cmd | CMD_NUM_ARG;
            skip_spaces(program, info);

            if (info->cur_src_pos < program->src_size && program->src_code[info->cur_src_pos] == '+')
            {
                ++info->cur_src_pos;
                read_val(program, info, ']');

                if (is_long_reg(info->cur_src_cmd, &long_reg))
                {
                    MEM_SYNTAX_CHECK
                    cmd = cmd | CMD_REG_ARG;

                    add_machine_cmd(cpu, sizeof(char), &cmd);
                    add_machine_cmd(cpu, sizeof(char), &long_reg);
                    add_machine_cmd(cpu, sizeof(long), &long_arg);

                    return true;
                }
                else
                {
                    fprintf(stderr, "line %4d: " RED "ERROR: " CANCEL "\"%s\" is not a long-type register name\n", info->cur_src_line, info->cur_src_cmd);
                    return false;
                }
            } //if only long arg
            else
            {
                MEM_SYNTAX_CHECK
                
                add_machine_cmd(cpu, sizeof(char), &cmd);
                add_machine_cmd(cpu, sizeof(long), &long_arg);

                return true;
            }
        } //if first argument is not long
        else if (is_long_reg(info->cur_src_cmd, &long_reg))
        {
            cmd = cmd | CMD_REG_ARG;
            skip_spaces(program, info);

            if (info->cur_src_pos < program->src_size && program->src_code[info->cur_src_pos] == '+')
            {
                ++info->cur_src_pos;
                read_val(program, info, ']');

                if (is_long(info->cur_src_cmd, &long_arg))
                {
                    MEM_SYNTAX_CHECK
                    cmd = cmd | CMD_NUM_ARG;

                    add_machine_cmd(cpu, sizeof(char), &cmd);
                    add_machine_cmd(cpu, sizeof(char), &long_reg);
                    add_machine_cmd(cpu, sizeof(long), &long_arg);

                    return true;
                }
                else
                {
                    fprintf(stderr, "line %4d: " RED "ERROR: " CANCEL "\"%s\" is not a valid long\n", info->cur_src_line, info->cur_src_cmd);
                    return false;
                }
            } //if only register arg
            else
            {
                MEM_SYNTAX_CHECK

                add_machine_cmd(cpu, sizeof(char), &cmd);
                add_machine_cmd(cpu, sizeof(char), &long_reg);

                return true;
            }
        } //if invalid arguments
        fprintf(stderr, "line %4d: " RED "ERROR: " CANCEL "\"%s\" is not a valid RAM-argument\n", info->cur_src_line, info->cur_src_cmd);
        return false;
    } //if not MEM arguments

    read_val(program, info, '+', ']');

    double dbl_arg = 0;
    char   reg_arg = 0;
    if (is_double(info->cur_src_cmd, &dbl_arg))
    {
        cmd = cmd | CMD_NUM_ARG;
        skip_spaces(program, info);

        if (info->cur_src_pos < program->src_size && program->src_code[info->cur_src_pos] == '+')
        {
            ++info->cur_src_pos;
            read_val(program, info, ']');

            if (is_reg(info->cur_src_cmd, &reg_arg) || is_long_reg(info->cur_src_cmd, &reg_arg))
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
    else if (is_reg(info->cur_src_cmd, &reg_arg) || is_long_reg(info->cur_src_cmd, &reg_arg))
    {
        cmd = cmd | CMD_REG_ARG;
        skip_spaces(program, info);

        if (info->cur_src_pos < program->src_size && program->src_code[info->cur_src_pos] == '+')
        {
            ++info->cur_src_pos;
            read_val(program, info, ']');

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
    fprintf(stderr, "line %4d: " RED "ERROR: " CANCEL "\"%s\" is not a valid argument\n", info->cur_src_line, info->cur_src_cmd);
    return false;
}   

/**
*   @brief Reads jmp-arguments. Works in two modes.
*   @brief If "mark_mode" is MARK_GET,   in case of non-existent mark it skips this mark and continue the assembler. (This mark can appear in the code below).
*   @brief If "mark_mode" is MARK_CHECK, in case of non-existent mark it give an error-message.  
*
*   @param program   [in]      - pointer to the structure with information about source
*   @param info      [in]      - pointer to the structure with information abour location in source
*   @param cpu       [out]     - pointer to the struct "machine" to add the command and arguments in "cpu->machine_code"
*   @param label     [in][out] - pointer to the store of marks
*   @param mark_mode [in]      - mode of the function
*
*   @return in MARK_CHECK-mode in case of non-existent mark returns false and true else
*/

bool cmd_jmp(source *const program, src_location *const info, machine *const cpu, tag *const label, const char mark_mode, unsigned char cmd)
{
    assert(program != nullptr);
    assert(info    != nullptr);
    assert(cpu     != nullptr);
    assert(label   != nullptr);

    assert(mark_mode == 0 || mark_mode == 1);

    read_val(program, info, ' ');

    int  label_pos = 0;
    if ((label_pos = tag_string_find(label, info->cur_src_cmd)) != -1)
    {
        add_machine_cmd(cpu, sizeof(char), &cmd);
        add_machine_cmd(cpu, sizeof(int) , &label->data[label_pos].machine_pos);

        return true;
    }

    if (mark_mode == MARK_GET)
    {
        char temp_invalid_ptr = -1;
        add_machine_cmd(cpu, sizeof(char), &cmd);
        add_machine_cmd(cpu, sizeof(int) , &temp_invalid_ptr);
        return true;
    }

    fprintf(stderr, "line %4d: " RED "ERROR: " CANCEL "\"%s\" is not a mark\n", info->cur_src_line, info->cur_src_cmd);
    return false;
}

/**
*   @brief Function working with marks. Works in two modes.
*   @brief In MARK_GET-mode it determines if another string is a mark declaration or not. In the first case it puts the mark in "label".
*   @brief In MARK_CHECK-mode it immediately returns.
*
*   @param program          [in]      - pointer to the structure with information about source
*   @param info             [in]      - pointer to the structure with information abour location in source
*   @param cpu              [out]     - pointer to the struct "machine" to add the command and arguments in "cpu->machine_code"
*   @param label            [in][out] - pointer to the store of marks
*   @param possible_mrk_beg [in]      - index   of the beginning of possible mark 
*   @param mark_mode        [in]      - mode of the function
*
*   @return in MARK_GET-mode in case of invalid mark(already declareted mark or string with no ':' character in the end) returns false and true else
*/

bool get_mark(source *const program, src_location *const info, machine *const cpu, tag *const label, const int possible_mrk_beg, const char mark_mode)
{
    assert(program != nullptr);
    assert(info    != nullptr);
    assert(cpu     != nullptr);
    assert(label   != nullptr);

    assert(mark_mode == 0 || mark_mode == 1);

    if (mark_mode == MARK_CHECK)
    {
        ++info->cur_src_pos;
        return true;
    }

    int cur_line = info->cur_src_line;
    skip_spaces(program, info);
    
    if (info->cur_src_pos < program->src_size)
    {
        int mrk_size = strlen(info->cur_src_cmd);
        if (program->src_code[info->cur_src_pos] == ':' && tag_push(label, {program->src_code + possible_mrk_beg, mrk_size, cpu->machine_pos}))
        {
            ++info->cur_src_pos;
            return true;
        }
        if (program->src_code[info->cur_src_pos] == ':')
        {
            fprintf(stderr, "line %4d: " RED "ERROR: " CANCEL "the mark \"%s\" has already met\n", cur_line, info->cur_src_cmd);
            
            ++info->cur_src_pos;
            return false;
        }
    }

    fprintf(stderr, "line %4d: " RED "ERROR: " CANCEL "command \"%s\" is not existing\n", cur_line, info->cur_src_cmd);
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
*   @brief Checks if "*s" is valid long. Puts the value in "val".
*
*   @param s   [in]  - pointer to the first byte of null-terminated byte string to check
*   @param val [out] - pointer to the "long" variable to put the result
*
*   @return true if argument is valid and false else
*
*   @note you should ignore "*val" if "is_long()" returns false
*/

bool is_long(const char *s, long *const val)
{
    assert (s != nullptr);

    char *check = nullptr;
    *val = strtol(s, &check, 10);

    return !(*check) && check != s;
}

/**
*   @brief Checks if "*s" is the "double type" rigister name. Puts number of register in "pos".
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

    for (char reg_cnt = REG_NUM / 2 + 1; reg_cnt <= REG_NUM; ++reg_cnt)
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
*   @brief Checks if "*s" is the name of "long" type rigister. Puts number of register in "pos".
*
*   @param s   [in]  - pointer to the first byte of null-terminated byte string to check
*   @param pos [out] - pointer to the number of register
*
*   @return true if "s" - name of "int" type register and false else
*
*   @note you should ignore "*pos" if "is_long_reg()" returns false
*/

bool is_long_reg(const char *s, char *const pos)
{
    assert(s   != nullptr);
    assert(pos != nullptr);

    for (char reg_cnt = 1; reg_cnt <= REG_NUM / 2; ++reg_cnt)
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