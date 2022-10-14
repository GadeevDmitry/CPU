DEF_CMD(HLT, 0,
{
    return true;
})

DEF_CMD(PUSH, 1,
{
    cmd_push(progress);
})

DEF_CMD(ADD, 2,
{
    GET_STK_TWO()
    PUSH(a + b)
})


DEF_CMD(SUB, 3,
{
    GET_STK_TWO()
    PUSH(b - a)
})

DEF_CMD(MUL, 4,
{
    GET_STK_TWO()
    PUSH(a * b)
})

DEF_CMD(DIV, 5,
{
    GET_STK_TWO()
    ZERO_CHECK(a)
    PUSH(b / a)
})

DEF_CMD(OUT, 6,
{
    GET_STK_ONE()
    PRINT(a)
})

DEF_CMD(NOT_EXICTING, 7, {
    return true;
})

DEF_CMD(POP, 8,
{
    ERRORS status = cmd_pop(progress);

    if (status != OK)
    {
        output_error(status);
        return false;
    }
})


DEF_CMD(JMP, 9,
{
    cmd_jmp(progress);
})

DEF_JMP_CMD(JA , 10, >)
DEF_JMP_CMD(JAE, 11, >=)
DEF_JMP_CMD(JB , 12, <)
DEF_JMP_CMD(JBE, 13, <=)
DEF_JMP_CMD(JE , 14, ==)
DEF_JMP_CMD(JNE, 15, !=)