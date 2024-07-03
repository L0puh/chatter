#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "state.h"
#include <stdio.h>
#include <stdint.h>

static char* WS_STRING = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

char* ws_key_parse(const char* buffer);
char* ws_create_accept(const char* key);
char* ws_create_upgrade(const request_t req);
char* ws_recv_frame(char* buffer, int *res);
char* ws_recv_text(char* buffer, uint64_t msglen, uint16_t offset);
void  ws_send_response(user_t user, char* message, unsigned short msglen);

#endif 
