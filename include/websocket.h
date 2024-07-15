#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "state.h"

static char* WS_STRING = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

#define FIN 0x80
#define WS_TEXT 0x01
#define WS_BINARY 0x02
#define WS_CLOSE 0x08
#define WS_MAXLEN 65536

typedef struct {
   uint8_t opcode;
   uint64_t payload_len;
   char* data;
} ws_frame_t;

char* ws_key_parse(const char* buffer);
char* ws_create_accept(const char* key);
char* ws_recv_frame(char* buffer, req_type *res);
char* ws_create_upgrade(const request_t *req);
char* ws_get_frame(ws_frame_t frame, uint64_t *res_size);
char* ws_recv_text(char* buffer, uint64_t msglen, uint16_t offset);

void ws_establish_connection(char* buffer, request_t *req, user_t *user);
int  ws_parse_message(char* ws_buffer);
void ws_send_broadcast(char* buffer, uint64_t buffer_sz);
void ws_send(user_t *user, char* buffer, uint64_t buffer_sz);
void ws_send_close();

int is_connection_exists(user_t *user);

#endif 
