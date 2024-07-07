#ifndef HTTP_H
#define HTTP_H

#include "state.h"
#include <arpa/inet.h>

int   get_type_request(char* message, size_t sz);
char* header_parse(const char* message, size_t sz, const char* symbol);
char* post_parse(const char* message, size_t sz, const char* symbol);
char* get_str_addr(struct sockaddr_in addr);
char* set_cookie(char* param, char* value);
char* cookies_parse(char *buffer, char* key);
void  url_decode(char* str);
void  send_response(user_t user, request_t req);
void  recv_loop(int client_sockfd, char* buffer, int *bytes);

#endif
