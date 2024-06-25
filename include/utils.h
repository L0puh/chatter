#ifndef UTILS_H
#define UTILS_H

#include <errno.h>
#include <string.h>

#define ASSERT(res) { if (res < 0){ error((char*)__func__, strerror(errno)); }}
#define LEN(arr) sizeof(arr)/sizeof(arr[0])
#define DIR "resources"

void print_usage(int argc);
void logger(const char* where, char* what);
void error(const char* where, char* what);
int is_contain(char* message, char symbol);

void write_html();
void write_to_file(const char* filename, char* input, char* mode);
char* remove_prefix(const char* msg, const char* x);
char* get_file_content(const char* filename, size_t *init_size);

#endif
