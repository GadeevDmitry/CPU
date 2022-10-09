#ifndef READ_WRITE
#define READ_WRITE

void     *read_file   (const char *file_name, size_t *const size_ptr);
bool     write_file   (const char *file_name, void *data, const int data_size);

unsigned get_file_size(const char *file_name);

#endif //READ_WRITE