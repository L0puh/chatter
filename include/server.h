#ifndef WEB_H
#define WEB_H

#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <strings.h>

#include "state.h"

void  init_server(int port, int *sockfd, struct sockaddr_in*, size_t);
void  set_current_page(user_t *user, char *input);

void* handle_client(void* user);
void  handle_ws(user_t *user, char* buffer, int bytes);
int   handle_request(user_t *user, request_t *req, char* buffer, int bytes);

#endif
