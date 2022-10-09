#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tag.h"

void tag_ctor(tag *const label)
{
    assert(label !=nullptr);

    *label = {};
    label->data     = (mark *) calloc(sizeof(mark), 4); //elementary tag cpapacity
    label->capacity = 4;
}

bool tag_push(tag *const label, mark push_val)
{
    assert(label != nullptr);

    if (tag_mark_find(label, push_val) != -1) return false;

    tag_realloc(label);
    label->data[label->size++] = push_val;

    return true;
}

int tag_string_find(tag *const label, const char *s)
{
    assert(label != nullptr);
    assert(s     != nullptr);

    for (int i = 0; i < label->size; ++i)
    {
        int len = strlen(s);
        if (len == label->data[i].mark_size && !strncmp(s, label->data[i].mark_ptr, len)) return i;
    }
    return -1;
}

int tag_mark_find(tag *const label, const mark mrk)
{
    assert(label != nullptr);

    for (int i = 0; i < label->size; ++i)
    {
        if (mrk.mark_size == label->data[i].mark_size && !strncmp(mrk.mark_ptr, label->data[i].mark_ptr, mrk.mark_size)) return i;
    }
    return -1;
}

void tag_realloc(tag *const label)
{
    assert(label != nullptr);

    if (label->size == label->capacity)
    {
        label->capacity *= 2;
        label->data      = (mark *) realloc(label->data, sizeof(mark) * label->capacity);
    }
}