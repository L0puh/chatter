#include "web.h"
#include "utils.h"
#include "state.h"

#include "state.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void set_current_page(char* res){
   if (strcmp(res, "/") == 0 || strcmp(res, "/index.html") == 0){
      GLOBAL.current_page = INDEX_PAGE;
      return;
   }
   res = remove_prefix(res, "/");
   for (int i = 0; i < LEN(available_routs); i++){
      if (strcmp(res, available_routs[i]) == 0){
         GLOBAL.current_page = res;
         return;
      }
   }
   GLOBAL.current_page = ERROR_PAGE;
   logger(res, "page isn't found");
   return;
}

void recv_loop(int client_sockfd, int *bytes){
   while ((*bytes = recv(client_sockfd, 
                        GLOBAL.buffer, 
                        sizeof(GLOBAL.buffer), 0)) <= 0){}
   ASSERT(bytes);
}

void write_input(char* buffer, size_t sz){
   char *input, *new;
   new = malloc(MAXLEN); 

   input = get_input(get_parse(buffer, sz, "?"));
   sprintf(new, FORMAT, input);

   write_to_file(DATABASE_FILE, new, "a");
   write_html();

   free(input); free(new);
}

void send_request(int client_sockfd, enum REQ type){
   size_t msg_sz = MAXLEN;
   int bytes_sent, total;
   char *header, *message, *result;

   switch(type){
      case OK:
         header = get_file_content(REQ_OK, 0);
         break;
      default:
         header = get_file_content(REQ_OK, 0);

   }
   message = get_file_content(GLOBAL.current_page, &msg_sz);
   result = malloc(msg_sz + MAXLEN);
   
   sprintf(result, "%s%lu\n\n%s", header, strlen(message), message);
   bytes_sent = 0, total = strlen(result);
   
   while (bytes_sent < total){
      bytes_sent = send(client_sockfd, result, strlen(result), 0);
      ASSERT(bytes_sent);
   }
  
   free(header); free(result); free(message); 
}

void handle_request(int bytes){
   char* res;
   if (bytes > 0 && get_request(GLOBAL.buffer, bytes) == GET){
      res = get_parse(GLOBAL.buffer, bytes, " ");
      if (is_contain(res, '?')){
         write_input(res, strlen(res));
      } else if (is_contain(res, '/')){
         set_current_page(res);
      }
   }
}

void handle_client(int client_sockfd){
   int bytes;  
  
   recv_loop(client_sockfd, &bytes);
   GLOBAL.buffer[bytes+1] = '\0';

   handle_request(bytes);
   send_request(client_sockfd, OK);
}
