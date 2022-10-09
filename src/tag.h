#ifndef TAG_H
#define TAG_H

struct mark
{
    char *mark_ptr;
    int   mark_size;

    int machine_pos;
};

struct tag
{
    mark *data;

    size_t capacity;
    size_t size;
};

bool tag_push        (tag *const label, mark push_val);

int  tag_string_find (tag *const label, const char *s);
int  tag_mark_find   (tag *const label, const mark mrk);

void tag_ctor        (tag *const label);
void tag_realloc     (tag *const label);

#endif //TAG_H