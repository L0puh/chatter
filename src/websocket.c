#include "utils.h"
#include "web.h"

#include <netinet/in.h>
#include <openssl/sha.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

char* ws_recv_frame(char* buffer, int *res){
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
   printf("RECIEVED: %lu\n", msglen);

   switch(opcode){
      case 0x01:
         *res = OK;
         return ws_recv_text(buffer, msglen, offset);
      case 0x08:
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
