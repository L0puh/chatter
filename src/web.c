#include "web.h"
#include "utils.h"
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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
   char *s = malloc(MAXLEN);
   for (int i = 0; i < sz; i++){
      s[i] = message[i]; 
      if (strcmp(s, "GET")){
         return GET;
      } else if (strcmp(s, "OK")){
         return OK;
      }
   }
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

char* get_parse(const char* message, size_t sz, const char* symbol){
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
         *str++ =' ';
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
