#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "stack.h"


void stack_ctor(stack *const stk, const size_t el_size)
{
    assert(stk != nullptr);

    *stk = {};
    stk->el_size  = el_size;
    stk->data     = calloc(el_size, 4); //default elementary capacity
    stk->capacity = 4;
}

void stack_push(stack *const stk, const void *push_val)
{
    assert(stk      != nullptr);
    assert(push_val != nullptr);

    stack_realloc(stk);
    memcpy((char *) stk->data + stk->size * stk->el_size, push_val, stk->el_size);
    ++stk->size;
    stack_realloc(stk);
}

void *stack_pop(stack *const stk)
{
    assert(stk != nullptr);

    stack_realloc(stk);
    --stk->size;
    stack_realloc(stk);

    return (char *) stk->data + stk->size * stk->el_size;
}

void *stack_front(stack *const stk)
{
    assert(stk != nullptr);

    return (char *) stk->data + (stk->size - 1) * stk->el_size;
}

bool stack_empty(stack *const stk)
{
    assert(stk != nullptr);

    return stk->size == 0;
}

void stack_realloc(stack *const stk)
{
    assert(stk != nullptr);

    if (stk->size == stk->capacity)
    {
        stk->capacity *= 2;
        stk->data      = realloc(stk->data, stk->el_size * stk->capacity);
    }
    else if (4 * stk->size < stk->capacity)
    {
        stk->capacity = (stk->size >= 2) ? 2 * stk->size : 4;
        stk->data     = realloc(stk->data, stk->el_size * stk->capacity);
    }
}