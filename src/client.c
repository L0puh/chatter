#include "web.h"
#include "utils.h"
#include "state.h"

#include "state.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const char* available_routs[] = {
   "index.html", 
   "",
   "/",
   "error.html"
};

void set_current_page(char* res){
   if (strcmp(res, "/") == 0 || strcmp(res, "/index.html") == 0){
      GLOBAL.current_page = "index.html";
      return;
   }
   res = remove_prefix(res, "/");
   for (int i = 0; i < LEN(available_routs); i++){
      if (strcmp(res, available_routs[i]) == 0){
         GLOBAL.current_page = res;
         return;
      }
   }
   GLOBAL.current_page = "error.html";
   logger(res, "page isn't found");
   return;
}

void recv_loop(int client_sockfd, int *bytes){
   while ((*bytes = recv(client_sockfd, GLOBAL.buffer, sizeof(GLOBAL.buffer), 0)) <= 0){}
   ASSERT(bytes);
}

void write_input(char* buffer, size_t sz){
   char *input, *new;
   new = malloc(MAXLEN); 

   input = get_input(get_parse(buffer, sz, "?"));
   sprintf(new, "<p>%s</p>\n", input);

   write_to_file("text.txt", new, "a");
   write_html();

   free(input); free(new);
}

void handle_client(int client_sockfd){
   int bytes;  
   char* res;
   
   recv_loop(client_sockfd, &bytes);
   GLOBAL.buffer[bytes+1] = '\0';

   if (bytes > 0 && get_request(GLOBAL.buffer, bytes) == GET){
      res = get_parse(GLOBAL.buffer, bytes, " ");
      if (is_contain(res, '?')){
         write_input(res, strlen(res));
      } else if (is_contain(res, '/')){
         set_current_page(res);
      }
   }
   /*FIXME: this is a hack, fix it later*/
   size_t msg_sz = MAXLEN;
   char* h = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
   char* m = get_file_content(GLOBAL.current_page, &msg_sz);
   char* r = malloc(msg_sz + MAXLEN);
   sprintf(r, "%s%lu\n\n%s", h, strlen(m), m);
   int bytes_sent = 0, total = strlen(r);
   
   while (bytes_sent < total){
      bytes_sent = send(client_sockfd, r, strlen(r), 0);
      ASSERT(bytes_sent);
   }
  
   free(r); free(m); 
}
