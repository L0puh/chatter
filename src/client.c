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
   int bytes, cnt = 0;  
   
   recv_loop(client_sockfd, &bytes);

   if (bytes > 0 && get_request(GLOBAL.buffer, bytes) == GET){
      cnt++;
      char *res, *input, *new;
      res = get_parse(GLOBAL.buffer, bytes, res, " ");
      if (is_contain(res, '?')){
         res = get_parse(res, strlen(res), res, "?");
         input = get_input(res);
         new = malloc(MAXLEN); 
         sprintf(new, "<p>%s</p>\n", input);
         write_to_file("text.txt", new, "a");
         write_html();
         printf("INPUT: %s\n", input);
         free(input); free(new);
      } else if (is_contain(res, '/')){
         res = remove_prefix(res, "/");
         GLOBAL.current_page = res;
      }
      /* free(res); free(new); free(input); */
   }
   /*FIXME: this is a hack, fix it later*/

   size_t sz = MAXLEN;
   char* h = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
   char* m = get_file_content(GLOBAL.current_page, &sz);
   char* r = malloc(sz + MAXLEN);
   sprintf(r, "%s%lu\n\n%s", h, strlen(m), m);
   int bytes_sent = 0, total = strlen(r);
   
   while (bytes_sent < total){
      bytes_sent = send(client_sockfd, r, strlen(r), 0);
      ASSERT(bytes_sent);
   }
  
   free(r); free(m); 
}
