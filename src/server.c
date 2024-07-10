#include "server.h"
#include "http.h"
#include "state.h"
#include "utils.h"
#include "websocket.h"

#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct state GLOBAL;

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


void set_current_page(user_t *user, char* res){
   if (strcmp(res, "/favicon.ico") == 0) return;
   if (strcmp(res, "/") == 0 || strcmp(res, CLEAR_COMMAND) == 0){
      user->current_page = INDEX_PAGE;
      return;
   }
   remove_prefix(res, "/");
   res = strtok(res, "\r");
   for (int i = 0; i < LEN(available_routs); i++){
      if (strcmp(res, available_routs[i]) == 0){
         user->current_page = res;
         return;
      }
   }
   user->current_page = ERROR_PAGE;
   return;
}


req_type handle_http_request(user_t *user, request_t *req, char* buffer, int bytes){
   char *res;
   req_type type = get_type_request(buffer, bytes);
   if (bytes > 0){
      switch(type){
         case GET:
            if (strstr(buffer, "Sec-WebSocket-Key") != NULL){
               ws_establish_connections(buffer, req, user);
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
            break;
         case POST:
            logger(__func__, "POST request");
            
            res = post_parse(buffer, bytes, "input=");
            if (res != NULL) {
               remove_prefix(res, "input=");
               url_decode(res);
               write_input(res, strlen(res), user->addr);
            }
            break;
         default:
            error(__func__, "unsupported type of request");
            return NONE;
      }
   }


   req->header = "OK";
   req->code = 200;
   req->content = get_file_content(user->current_page, 0);
   req->content_type = "text/html";
   req->cookies = set_cookie("test", "cookie");
   req->length = strlen(req->content);

   return OK;
}


void handle_ws_request(user_t *user, char* buffer, int bytes){
   int len;
   req_type res;
   char* message;
   uint64_t buffer_sz;
   ws_frame_t frame;

   char* ws_buffer = ws_recv_frame(buffer, &res);
   if (ws_buffer == NULL) return;
   res = ws_parse_message(ws_buffer);

   switch (res){
      case ERROR:
      case CLOSE:
         pthread_mutex_lock(&GLOBAL.mutex);
         user->is_ws = 0;
         user->ws_state = WS_CLOSE;
         pthread_mutex_unlock(&GLOBAL.mutex);
         break;
      case NAME:
         user->username = ws_buffer;
         break;
      case TEXT:
         len = strlen(ws_buffer) + strlen(user->username)+1;
         message = malloc(len*sizeof(char)+1); 
         message[len*sizeof(char)] = '\0';
         sprintf(message, "%s|%s", user->username, ws_buffer);
        
         frame.opcode = WS_TEXT;
         frame.payload_len = len * sizeof(char)+1;
         
         strcpy(frame.data, message);
         char* res = ws_get_frame(frame, &buffer_sz);
         if (res != NULL && buffer_sz > 0)
            ws_send_broadcast(res, buffer_sz);
         else error(__func__,"corrupted frame");
         free(message);
         break;
      default:
         return;
   }
}

void* handle_client(void* th_user){
   int bytes;
   user_t *user; 
   request_t req;
   req_type res;
   char buffer[MAXLEN];
  
   user = (user_t*)th_user;

   while ((bytes = recv(user->sockfd, buffer, sizeof(buffer), 0)) > 0){
      buffer[bytes] = '\0';

      if (!user->is_ws){
         res = handle_http_request(user, &req, buffer, bytes);
         send_response(*user, req);
      } else 
         handle_ws_request(user, buffer, bytes);
   }
   
   ASSERT(bytes);
   close(user->sockfd);
   pthread_exit(0);
   free(th_user);

   return 0;
}

