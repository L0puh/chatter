#include "utils.h"
#include "web.h"

#include <openssl/sha.h>
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
