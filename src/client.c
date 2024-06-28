#include "web.h"
#include "utils.h"
#include "state.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct state GLOBAL;

void set_current_page(user_t *user, char* res){
   if (strcmp(res, "/") == 0 || strcmp(res, CLEAR_COMMAND) == 0){
      user->current_page = INDEX_PAGE;
      return;
   }
   remove_prefix(res, "/");
   for (int i = 0; i < LEN(available_routs); i++){
      if (strcmp(res, available_routs[i]) == 0){
         user->current_page = res;
         return;
      }
   }
   user->current_page = ERROR_PAGE;
   logger(res, "page isn't found");
   return;
}

void recv_loop(int client_sockfd, char* buffer, int *bytes){
   while ((*bytes = recv(client_sockfd, 
                        buffer, MAXLEN, 0)) <= 0){}
   ASSERT(bytes);
}

void write_input(char* buffer, size_t sz, char* data){
   char *input, *new, *format;
   format = "<p><span style=\"color:#b0716f\"> %s:\
            </span><span>%s </span></p>";

   new = malloc(MAXLEN); 
   input = get_parse(buffer, sz, "?"); 
   get_input(input);
   sprintf(new, format, data, input);
   write_to_file(DATABASE_FILE, new, "a");
   update_html();

   free(new);
}

void send_request(user_t user, enum req_type_t type){
   char* format = "HTTP/1.1 %d %s\nContent-Type: %s\n\
                   Set-Cookie: %s\nContent-Length:%d\n\n%s\n";
   char* result; 
   request_t req;
   size_t msg_sz = MAXLEN;
   int bytes_sent, total;

   switch(type){
      case OK:
         req.header = "OK";
         req.code = 200;
         break;
      default:
         error(__func__, "type of request isn't supported");
         return;

   }

   req.content = get_file_content(user.current_page, &msg_sz);
   req.content_type = "text/html";
   req.cookies = "test=123";
   req.length = strlen(req.content);

   result = malloc(msg_sz + MAXLEN);
   
   sprintf(result, format, req.code, req.header, 
           req.content_type, req.cookies, 
           req.length, req.content);

   bytes_sent = 0, total = strlen(result);
   
   while (bytes_sent < total){
      bytes_sent = send(user.sockfd, result, strlen(result), 0);
      ASSERT(bytes_sent);
   }
  
   free(result);  
}

void handle_request(user_t user, char* buffer, int bytes){
   char* res;
   if (bytes > 0 && get_type_request(buffer, bytes) == GET){
      res = get_parse(buffer, bytes, " ");
      if (is_contain(res, '?')){
         write_input(res, strlen(res), user.addr);
      } else if (is_contain(res, '/')){
         set_current_page(&user, res);
         /* FIXME: add database */
         if (strcmp(res, CLEAR_COMMAND) == 0){
            logger(__func__, "delete chat history");
            system("rm -f resources/text.txt && touch resources/text.txt"); 
            update_html();
         }
      }
   }
}

void* handle_client(void* th_user){
   char buffer[MAXLEN]; int bytes;
   user_t user = *(user_t*)th_user;
   recv_loop(user.sockfd, buffer, &bytes); 
   buffer[bytes] = '\0';

   printf("%s\n", buffer);
   
   handle_request(user, buffer, bytes);
   send_request(user, OK);
   ASSERT(bytes);
   pthread_exit(0);
   return 0;
}


