#ifndef UTILS_H
#define UTILS_H

#include <errno.h>
#include <string.h>

#define ASSERT(res) { if (res == -1){ error((char*)__func__, strerror(errno)); }}
#define DIR "resources"


void print_usage(int argc);
void logger(const char* where, char* what);
void error(const char* where, char* what);

char* get_file_content(char* filename);

#endif
