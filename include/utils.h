#ifndef UTILS_H
#define UTILS_H

#include <errno.h>
#include <stdint.h>
#include <string.h>

#define ASSERT(res) { if (res < 0){ errorl((char*)__func__, (char*) __FILE__, __LINE__); }}
#define DB_ASSERT(res, st) { if (PQresultStatus(res) != st){ db_errorl((char*)__func__, (char*) __FILE__, __LINE__); }}
#define SSL_ASSERT(res) { if (res < 0){ ssl_error((char*)__func__, (char*) __FILE__, __LINE__); }}
#define LEN(arr) sizeof(arr)/sizeof(arr[0])

void print_usage(int argc);
uint8_t get_options(int argc, char* argv[]);

int  is_contain(char* message, char symbol);
void remove_prefix(char* str, const char* prefix);

void  update_html();
char* get_file_content(const char* filename, size_t *length, char* mode, char* filepath);
void  write_to_file(const char* filename, char* input, char* mode, char* path);
void write_input(char* buffer, size_t sz, char* data);

void logger(const char* where, char* what);
void error(const char* where, char* what);
void ssl_error(const char* where, char* file, int line);
void errorl(const char* where, char* file, int line);
void db_errorl(const char* where, char* file, int line);

char*  base64_encode(const unsigned char* message, size_t size);
size_t base64_encode_size(size_t strlen);

#endif
