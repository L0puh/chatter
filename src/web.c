#include "web.h"
#include "state.h"
#include "utils.h"
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
   servaddr->sin_addr.s_addr = INADDR_ANY; /* FIXME: set up an ip */

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

char* remove_prefix(const char* msg, const char* x){
   int cnt = 0;
   char* prefix = malloc(strlen(msg));
   while (strcmp(prefix, x) != 0){
      prefix[cnt] = msg[cnt];
      cnt++;
   }
   char* res = malloc(strlen(msg) - cnt);
   for(int i = cnt; i < strlen(msg); i++){
      res[i-cnt] = msg[i];
   }
   return res;
}

char* get_input(char* message){
   char *token, *res[MAXLEN];
   int cur=0;
   token = strtok(remove_prefix(message, "input="), "+");
   while (token != NULL){
      res[cur] = token;    
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

char* parse_get(char* message, size_t sz, char* res, char* symbol){
   char* token; int cur = 0;

   if (res == NULL) 
      error(__func__, "given string is NULL");

   res = malloc(32);
   token = strtok(message, symbol); 

   while(token != NULL){
      token = strtok(NULL, " ");
      if (cur == 0){
         res = token;
         if (res == NULL) res = "";
         return res;
      }
      cur++;
   }
   free(token); 
   return res;
}
