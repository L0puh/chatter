#include "web.h"
#include "utils.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>


void init_server(int port, int *sockfd, struct sockaddr_in *servaddr, size_t sz){
   char message[32]; int enable = 1;
   sprintf(message, "SERVER IS RUNNING\nPORT: %d", port);
   logger((char*)__func__, message);

   ASSERT((*sockfd = socket(AF_INET, SOCK_STREAM, 0)));
   ASSERT(setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)));
   bzero(servaddr, sz);
   servaddr->sin_port = htons(port);
   servaddr->sin_family = AF_INET;
   servaddr->sin_addr.s_addr = INADDR_ANY; 

   ASSERT(bind(*sockfd, (const struct sockaddr*)servaddr, sz));
   ASSERT(listen(*sockfd, QUERY));
   
   sprintf(message, "SERVER IS RUNNING\nPORT: %d", port);
}

int get_type_request(char* message, size_t sz){
   if (strstr(message, "GET") != NULL)
      return GET;
   if (strstr(message, "POST") != NULL)
      return POST;
   return NONE;
}

char* get_str_addr(struct sockaddr_in addr){
   char* str_addr = malloc(INET_ADDRSTRLEN+1); 
   ASSERT(inet_ntop(addr.sin_family, &(addr.sin_addr), str_addr, INET_ADDRSTRLEN));
   str_addr[INET_ADDRSTRLEN] = '\0';
   return str_addr;
}

char* get_sentence(char* str) {
   int cur = 1, total_len = 0;
   char* token = strtok(str, "+");
   
   char *res = malloc((MAXLEN+1) * CHAR_BIT);
   bzero(res, strlen(res));

   while (token != NULL && cur < MAXLEN && total_len < MAXLEN){
      strcat(res, token);
      strcat(res, " ");
      total_len += strlen(token) + 1;
      token = strtok(NULL, "+");
   }
   res[total_len] = '\0';
   return str;
}
void get_input(char* message){
   remove_prefix(message, "input=");
   url_decode(message);
}

char* post_parse(const char* message, size_t sz, const char* symbol){
   char *ptr, *res;
   if ((ptr = strstr(message, symbol)) != NULL){
      return ptr;
   }
   return "";
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

void send_response(user_t user, request_t req){
   char* format;
   char* result; 
   int bytes_sent, total;
   result = malloc(req.length + MAXLEN);
   if (req.code == 200){
       format = "HTTP/1.1 %d %s\r\nSet-Cookie: %s; Secure; HttpOnly\r\n"
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

