#ifndef UTILS_H
#define UTILS_H

#include <errno.h>
#include <string.h>

#define ASSERT(res) { if (res < 0){ errorl((char*)__func__, (char*) __FILE__, __LINE__); }}
#define LEN(arr) sizeof(arr)/sizeof(arr[0])

void print_usage(int argc);

int  is_contain(char* message, char symbol);
void remove_prefix(char* str, const char* prefix);

void  update_html();
char* get_file_content(const char* filename, size_t *init_size);
void  write_to_file(const char* filename, char* input, char* mode);
void write_input(char* buffer, size_t sz, char* data);

void logger(const char* where, char* what);
void error(const char* where, char* what);
void errorl(const char* where, char* file, int line);


char*  base64_encode(const unsigned char* message, size_t size);
size_t base64_encode_size(size_t strlen);

#endif
