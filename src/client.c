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
   char *new, *format;
   format = "<p><span style=\"color:#b0716f\"> %s:\
             </span><span>%s </span></p>";

   new = malloc(MAXLEN); 
   sprintf(new, format, data, buffer);
   write_to_file(DATABASE_FILE, new, "a");
   update_html();

   free(new);
}

int handle_request(user_t *user, request_t *req, char* buffer, int bytes){
   char *res, *ws;
   reqtype_t type = get_type_request(buffer, bytes);
   
   if (bytes > 0 && type == POST){
      logger(__func__, "POST request");
      
      res = post_parse(buffer, bytes, "input=");
      get_input(res);
      write_input(res, strlen(res), user->addr);

   } else if (bytes > 0 && type == GET ){
      if (strstr(buffer, "Upgrade: websocket") != 0){
         logger(__func__, "WebSocket request");
         ws = malloc(128);
         ws = ws_key_parse(buffer);
         if (ws == NULL) 
            error(__func__, "error in parsing WS key");
         req->header = "Switching Protocols";
         req->code = 101;
         req->accept = ws_create_accept(ws);
         return WS;
      } 
      res = header_parse(buffer, bytes, " ");
      if (is_contain(res, '/')){
         set_current_page(user, res);
         
         if (strcmp(res, CLEAR_COMMAND) == 0){ //FIXME: add database
            logger(__func__, "delete chat history");
            system("rm -f resources/text.txt && touch resources/text.txt"); 
            update_html();
         }
      }
   }

   req->header = "OK";
   req->code = 200;
   req->content = get_file_content(user->current_page, 0);
   req->content_type = "text/html";
   req->cookies = set_cookie("test", "test");
   req->length = strlen(req->content);

   return OK;
}


void* handle_client(void* th_user){
   int bytes;
   user_t user;
   request_t req;
   char buffer[MAXLEN]; 

   user = *(user_t*)th_user;
   recv_loop(user.sockfd, buffer, &bytes); 
   buffer[bytes] = '\0';
   printf("%s\n", buffer);

   handle_request(&user, &req, buffer, bytes);
   send_response(user, req);

   pthread_exit(0);
   return 0;
}


