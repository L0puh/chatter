#include "utils.h"
#include "web.h"

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

char* ws_decode(char* buffer){
    int i;
    uint8_t fin, mask;
    uint8_t opcode;
    uint16_t offset = 2;
    uint64_t msglen;

    fin = (buffer[0] & 0x80)  != 0;
    mask = (buffer[1] & 0x80) != 0;
    opcode = buffer[0] & 0x0F;
    msglen = (uint64_t)buffer[1] & 0x7F;

    if (mask && msglen < 126) {
        uint8_t decoded[msglen];
        uint8_t masks[4] = {   buffer[offset], 
                               buffer[offset+1], 
                               buffer[offset+2], 
                               buffer[offset+3]  };
        offset += 4;
        for (i = 0; i < msglen; ++i)
            decoded[i] = (uint8_t)(buffer[offset + i] ^ masks[i % 4]);

        char *text = (char*)malloc(msglen + 1);
       
        for (i = 0; i < msglen; ++i)
            text[i] = decoded[i];
       
        text[msglen] = '\0';
        return text;

    } else if (!mask)
        error(__func__, "mask bit not set");

    return NULL;
 }
