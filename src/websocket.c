#include "websocket.h"
#include "utils.h"

#include <netinet/in.h>
#include <openssl/sha.h>
#include <openssl/ssl.h>
#include <pthread.h>

#include <stdint.h>
#include <stdio.h>

char* ws_key_parse(const char* buffer){
   char *p, *token, *res = malloc(128);
   if ((p = strstr(buffer, "Sec-WebSocket-Key:")) != NULL) {
      token = strtok(p, " "); 
      token = strtok(NULL, " ");
      token = strtok(token, "\r");
      strcpy(res, token);
      return res;
   }
   return NULL;
}

char* ws_create_accept(const char* key){
   char *str;
   size_t total_size;
   SHA_CTX ctx;
   unsigned char digest[SHA_DIGEST_LENGTH];
  
   total_size = strlen(key) + strlen(WS_STRING);
   str = malloc(total_size+1);

   sprintf(str, "%s%s", key, WS_STRING);
   str[total_size] = '\0';

   SHA1_Init(&ctx);
   SHA1_Update(&ctx, str, strlen(str));
   SHA1_Final(digest, &ctx);
   
   return base64_encode((const unsigned char*) digest, SHA_DIGEST_LENGTH);
}

char* ws_create_upgrade(const request_t req){
   char *format, *result;
   result = malloc(MAXLEN);
   format = "HTTP/1.1 %d %s\r\nUpgrade: websocket\r\n"
                  "Connection: Upgrade\r\nSec-WebSocket-Accept: %s\r\n\r\n";
   sprintf(result, format, req.code, req.header, req.accept);
   return result;
}

char* ws_recv_frame(char* buffer, req_type *res){
   uint16_t offset;
   uint64_t msglen;
   uint8_t fin, mask, opcode;
   
   fin    = (buffer[0] & 0x80) != 0;
   opcode = buffer[0] & 0x0F;
   mask   = (buffer[1] & 0x80) != 0;
   msglen = (uint64_t) buffer[1] & 0x7F;
   
   if (!mask || fin == 0 || msglen == 0) 
      return NULL;

   if (msglen <= 125){
      offset = 2;
   } else if (msglen == 126) {
       memcpy(&msglen, &buffer[2], 2);
       msglen = ntohs(msglen);
       offset = 4;
   } else {
      error(__func__, "frame length exceeds");
      return NULL;
   }

   switch(opcode){
      case WS_TEXT:
         *res = OK;
         return ws_recv_text(buffer, msglen, offset);
      case WS_CLOSE:
         logger(__func__, "connection close");
         *res = CLOSE;
         return ws_recv_text(buffer, msglen, offset);
      default:
         *res = ERROR;
         error(__func__, "WS: unsupported type");
   }

   return NULL;
}

char* ws_recv_text(char* buffer, uint64_t msglen, uint16_t offset){
   uint8_t mask[4];
   uint8_t decoded[msglen];
   
   mask[0] = buffer[offset++];
   mask[1] = buffer[offset++];
   mask[2] = buffer[offset++];
   mask[3] = buffer[offset++];

   for (int i = 0; i < msglen; ++i)
      decoded[i] = (uint8_t)(buffer[offset + i] ^ mask[i % 4]);
   
   char *text = (char*) malloc(msglen + 1);
   for (int i = 0; i < msglen; ++i)
      text[i] = decoded[i];
   text[msglen] = '\0';
   
   return text;
}

char* ws_get_frame(ws_frame_t frame, uint64_t *res_size){
   int bytes, offset;
   char* buffer = malloc(MAXLEN);
   
   bzero(buffer, MAXLEN);
   buffer[0] = FIN | frame.opcode;
   *res_size = 0;
   
   switch(frame.opcode){
      case WS_TEXT:
         if (frame.payload_len < 126) {
            buffer[1] = (char)frame.payload_len;
            offset = 2;
         } else if (frame.payload_len < WS_MAXLEN) {
            buffer[1] = 0x7E; 
            buffer[2] = (frame.payload_len >> 8) & 0xFF;
            buffer[3] =  frame.payload_len & 0xFF;
            offset = 4;
         } else {
            error(__func__, "buffer length exceeds");
            return NULL;
         }
         memcpy(buffer+offset, frame.data, frame.payload_len);
         buffer[frame.payload_len+offset] = '\0';
         break;
      case WS_CLOSE:
         buffer[1] = (char)frame.payload_len;
         offset=2;
         break;
      default:
         error(__func__, "unsupported type");
         return NULL;
   }
   *res_size = frame.payload_len + offset;
   return buffer;
}
void ws_send_broadcast(char* buffer, uint64_t buffer_sz){
   pthread_mutex_lock(&GLOBAL.mutex);
   for (int i = 0; i < GLOBAL.connections_size; i++){
      user_t *user = GLOBAL.connections[i];
      if (user->is_ws && user->ws_state != WS_CLOSE) 
         ws_send(user, buffer, buffer_sz);
   }
   pthread_mutex_unlock(&GLOBAL.mutex);
}
void ws_send(user_t *user, char* buffer, uint64_t buffer_sz){
   if (user->sockfd != -1){
      pthread_mutex_lock(&user->mutex);
      if (user->is_ssl && user->SSL_sockfd){
         int bytes = SSL_write(user->SSL_sockfd, buffer, (int)buffer_sz);
      } else if (!user->is_ssl)
         ASSERT(send(user->sockfd, buffer, buffer_sz, 0));
      pthread_mutex_unlock(&user->mutex);
   } else 
      error(__func__, "invalid socket");
}

void ws_send_close(){
   uint64_t buffer_sz;
   ws_frame_t frame;
   frame.opcode = WS_CLOSE;
   frame.payload_len = 0;
   frame.data = NULL;
   char* res = ws_get_frame(frame, &buffer_sz);
   ws_send_broadcast(res, buffer_sz);
}


int ws_parse_message(char* ws_buffer){
   char* p, *text = "TEXT: ", *name = "NAME: ";
   if ((p = strstr(ws_buffer, text)) != NULL){
      p+=strlen(text);
      strcpy(ws_buffer, p);
      return TEXT;
   } else if ((p = strstr(ws_buffer, name)) != NULL){
      p+=strlen(name);
      strcpy(ws_buffer, p);
      return NAME;
   }
   return NONE;
}

void ws_establish_connection(char* buffer, request_t *req, user_t *user){
   char* ws;
   logger(__func__, "WebSocket request");
   
   ws = ws_key_parse(buffer);
   if (ws == NULL) 
      error(__func__, "error in parsing WS key");
   
   pthread_mutex_lock(&user->mutex);
   user->is_ws = 1;
   user->ws_state = WS_CONNECT;
   req->code = 101;
   req->header = "Switching Protocols";
   req->accept = ws_create_accept(ws);
   
   if (!is_connection_exists(user)){
      if (GLOBAL.connections_size+1 >= QUERY)
         error(__func__, "limit of connections");
      else {
         user->ws_id = GLOBAL.connections_size;
         GLOBAL.connections[GLOBAL.connections_size++] = user;
      }
   } 
   pthread_mutex_unlock(&user->mutex);
   free(ws);
}

int is_connection_exists(user_t *user){

   pthread_mutex_lock(&GLOBAL.mutex);

   for (int i = 0; i < GLOBAL.connections_size; i++){
      user_t *u = GLOBAL.connections[i];
      if (strcmp(u->addr, user->addr) == 0 && !u->is_ws)
      {
         user->username = u->username;
         u = user;
         GLOBAL.connections[i] = u;
         pthread_mutex_unlock(&GLOBAL.mutex);
         return 1;
      } 
   }

   pthread_mutex_unlock(&GLOBAL.mutex);
   return 0;
}
