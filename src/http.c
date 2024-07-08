#include "http.h"
#include "utils.h"
#include "websocket.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* get_str_addr(struct sockaddr_in addr){
   char* str_addr = malloc(INET_ADDRSTRLEN+1); 
   ASSERT(inet_ntop(addr.sin_family, &(addr.sin_addr), str_addr, INET_ADDRSTRLEN));
   str_addr[INET_ADDRSTRLEN] = '\0';
   return str_addr;
}


void recv_loop(int client_sockfd, char* buffer, int *bytes){
   while ((*bytes = recv(client_sockfd, 
                        buffer, MAXLEN, 0)) <= 0){}
   ASSERT(bytes);
}

int get_type_request(char* message, size_t sz){
   if (strstr(message, "GET") != NULL)
      return GET;
   if (strstr(message, "POST") != NULL)
      return POST;
   return NONE;
}

char* post_parse(const char* message, size_t sz, const char* symbol){
   char *ptr, *res;
   if ((ptr = strstr(message, symbol)) != NULL){
      return ptr;
   }
   return NULL;
}
char* header_parse(const char* message, size_t sz, const char* symbol){
   char* token; int cur = 0;
   char* res = malloc(sz * CHAR_BIT);
   char* copy_msg = malloc(sz * CHAR_BIT);
   strcpy(copy_msg, message);
   
   token = strtok(copy_msg, symbol); 

   while(token != NULL){
      token = strtok(NULL, " ");
      if (cur == 0){
         res = token;
         if (res == NULL) 
            res = "";
         return res;
      }
      cur++;
   }
   free(token); 
   return res;
}

void url_decode(char* str){
   char* p = str;
   while(*p){
      if (*p == '+'){
         *str++ = ' ';
         p++;
      } else if (*p == '%'){
         p++;
         if (isxdigit(*p)){
            if (isxdigit(p[1])){
               uint8_t x;
               sscanf(p, "%02hhx", &x);
               *str++=x;
               p+=2;
            } else { 
               *str++='%';
               p++;
            }
         } else p++;
      } else *str++ = *p++;
   }
   *str='\0';
}

char* set_cookie(char* param, char* value){
   char* cookie = malloc(MAXLEN);
   sprintf(cookie, "%s=%s;", param, value);
   return cookie;
}

char* cookies_parse(char *buffer, char* key){
   size_t len;
   char *p, *token, *res;
   if ((p = strstr(buffer, "Cookie:")) != NULL && (p = strstr(p, key)) != NULL){
      token = strtok(p, "=");
      token = strtok(NULL, "="); 
      token = strtok(token, "\r");
      len = sizeof(char) * strlen(token);
      res = malloc(len + 1);
      strcpy(res, token);
      res[len] = '\0';
      return res;
   }
   return NULL;
}

void send_response(user_t user, request_t req){
   char* format;
   char* result; 
   int bytes_sent, total;
   result = malloc(req.length + MAXLEN);
   if (req.code == 200){
       format = "HTTP/1.1 %d %s\r\nSet-Cookie: %s HttpOnly\r\n"
                     "Content-Type: %s\r\nContent-Length:%d\r\n\r\n%s\r\n\r\n";
       sprintf(result, format, req.code, req.header, 
               req.cookies, req.content_type, 
               req.length, req.content);

   }
   else if (req.code == 301){
       format = "HTTP/1.1 %d %s\r\nLocation: %s\r\n\r\n";
       sprintf(result, format, req.code, req.header, req.location);
       logger(__func__, "Moved Permanently response");
   }
   else if (req.code == 101){
      result = ws_create_upgrade(req);
   }

   bytes_sent = send(user.sockfd, result, strlen(result), 0);
   ASSERT(bytes_sent);
  
   free(result);  
}

