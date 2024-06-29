#ifndef UTILS_H
#define UTILS_H

#include <errno.h>
#include <string.h>

#define ASSERT(res) { if (res < 0){ error((char*)__func__, strerror(errno)); }}
#define LEN(arr) sizeof(arr)/sizeof(arr[0])

void print_usage(int argc);

int  is_contain(char* message, char symbol);
void remove_prefix(char* str, const char* prefix);

void  update_html();
char* get_file_content(const char* filename, size_t *init_size);
void  write_to_file(const char* filename, char* input, char* mode);

void logger(const char* where, char* what);
void error(const char* where, char* what);

#endif
