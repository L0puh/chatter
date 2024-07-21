#ifndef HTTP_H
#define HTTP_H

#include "state.h"
#include <arpa/inet.h>
#include <netdb.h>



int   init_server(char* host, char* addr, struct addrinfo *servaddr);
int   get_type_request(char* message, size_t sz);
char* header_parse(const char* message, size_t sz, const char* symbol, int* is_static);
char* post_parse(const char* message, size_t sz, const char* symbol);
char* get_str_addr(struct sockaddr_in addr);
char* set_cookie(char* param, char* value);
char* cookies_parse(char *buffer, char* key);
void  url_decode(char* str);
void  recv_loop(int client_sockfd, char* buffer, int *bytes);
int   recv_buffer(user_t *user, char* buffer, size_t buffer_size);
void  send_response(user_t *user, request_t *req);
void  create_response(request_t *req, char* response, int* len);
char* get_content_type(char* buffer, req_type* type);

#endif
