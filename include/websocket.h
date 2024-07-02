#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "state.h"
#include <stdint.h>

char* ws_key_parse(const char* buffer);
char* ws_create_accept(const char* key);
char* ws_create_upgrade(const request_t req);
char* ws_recv_frame(char* buffer, int *res);
char* ws_recv_text(char* buffer, uint64_t msglen, uint16_t offset);

#endif 
