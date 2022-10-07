/** @file */

/**
*   @brief enum "STATUS" contains the cpu numbers of commands
*
*   @param HLT           - stop  the program
*   @param PUSH          - push  the another double-number in the stack
*   @param ADD           - push  the sum            of last two elements of the stack instead of them
*   @param SUB           - push  the diffrence      of last two elements of the stack instead of them
*   @param MUL           - push  the multiplication of last two elements of the stack instead of them
*   @param DIV           - push  the division       of last two elements of the stack insteat of them
*   @param OUT           - print the element on the screen  and pop it from the stack
*   @param UNDEFINED_CMD - the command is not belong to the command list
*   @param OK            - no errors found
*/

enum STATUS
{
    HLT = 67,
    PUSH,
    ADD,
    SUB,
    MUL,
    DIV,
    OUT,

    UNDEFINED_CMD,
    OK
};

const size_t CMD_LEN    = 4;
const size_t DOUBLE_LEN = 100;

struct supportive
{
    char code_word[4];
    char version;

    unsigned commands_number;
};

struct program
{
    char *data;

    size_t       Size;
    supportive   info;
};

void *assembler(program *source, size_t *const machine_code_size);

STATUS get_param(const char *const data,
                 const size_t      data_size,
                 int  *const       data_pos,
                 char *const       str,
                 const size_t      str_size);

STATUS add_cmd(const char *cmd, void *cpu_cmd_ptr, int *const cpu_pos);

STATUS get_double(const char *const data,
                  const size_t      data_size,
                  int    *const     data_pos,
                  double *const     value);

int pass_spaces(const char *const data, const size_t data_size, int *const data_pos);

char *read_file(const char *file_name, size_t *const Size_ptr);

unsigned get_file_size(const char *file_name);

void write_machine_code(void *cpu_cmd_ptr, const size_t cpu_size);