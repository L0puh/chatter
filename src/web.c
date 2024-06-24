#include "web.h"
#include "utils.h"
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

int get_request(char* message, size_t sz){
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


char* get_input(const char* message){
   int cur=0;
   char *token, *res[MAXLEN];
   char *cpy_msg = malloc(strlen(message) * CHAR_BIT);
   strcpy(cpy_msg, message);
   cpy_msg = remove_prefix(cpy_msg, "input=");
   if (message == NULL) return "";
   token = strtok(cpy_msg, "+");
   while (token != NULL && cur < strlen(cpy_msg)){
      res[cur] = token;    
      printf("%s\n", res[cur]);
      cur++;
      token = strtok(NULL, "+");
   }
   char *str = malloc(MAXLEN);
   for (int i = 0; i < cur; i++){
      if (res[i] == NULL) break;
      strcat(str, res[i]);
      strcat(str, " ");
   }
   return str;
}

char* get_parse(const char* message, size_t sz, const char* symbol){
   char* token; int cur = 0;
   char* res = malloc(strlen(message) * CHAR_BIT);
   char* copy_msg = malloc(strlen(message) * CHAR_BIT);
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
