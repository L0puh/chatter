#ifndef WEB_H
#define WEB_H

#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <strings.h>

#include "state.h"

void     set_current_page(user_t *user, char *buffer);

void*    handle_client(void* user);
void     handle_ws_request(user_t *user, char* buffer, int bytes);
req_type handle_http_request(user_t *user, request_t *req, char* buffer, int bytes);

void connections_cleanup();
void send_text_frame(char* message, int len);

#endif
