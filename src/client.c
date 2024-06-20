#include "web.h"
#include "utils.h"
#include "state.h"

#include "state.h"
#include <stdio.h>
#include <stdlib.h>

void recv_loop(int client_sockfd, int *bytes){
   while ((*bytes = recv(client_sockfd, GLOBAL.buffer, sizeof(GLOBAL.buffer), 0)) <= 0){}
   ASSERT(bytes);
}

void handle_client(int client_sockfd){
   logger("INFO", "new connection");
   int bytes, cnt = 0;  
   
   recv_loop(client_sockfd, &bytes);

   if (bytes > 0 && get_request(GLOBAL.buffer, bytes) == GET){
      cnt++;
      char *res, *input;
      res = parse_get(GLOBAL.buffer, bytes, res, " ");
      res = parse_get(res, strlen(res), res, "?");
      input = get_input(res);
      printf("INPUT: %s\n", input);
   }
   
   /*FIXME: this is a hack, fix it later*/

   char* h = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
   char* m = get_file_content("index.html");
   char* r = malloc(MAXLEN);

   sprintf(r, "%s%lu\n\n%s", h, strlen(m), m);
   int bytes_sent = 0, total = strlen(r);
   
   while (bytes_sent < total){
      bytes_sent = send(client_sockfd, r, strlen(r), 0);
      ASSERT(bytes_sent);
   }
  
   free(r); free(m); 
}
