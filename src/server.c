#include "server.h"
#include "http.h"
#include "state.h"
#include "utils.h"
#include "websocket.h"

#include <openssl/ssl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct state GLOBAL;

void get_favicon(user_t *user, request_t *req){
}

void set_current_page(user_t *user, char* res){
   if (strcmp(res, "/") == 0 || strcmp(res, CLEAR_COMMAND) == 0){
      user->current_page = INDEX_PAGE;
      return;
   }
   remove_prefix(res, "/");
   res = strtok(res, "\r");
   if (strcmp(res, "websocket.html") == 0){
      user->current_page = GLOBAL.DEFAULT_WEBSOCKET_PAGE;
      return;
   }
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
               ws_establish_connection(buffer, req, user);
               return WS;
            }
            res = header_parse(buffer, bytes, " ");
            if (strcmp(res, "/favicon.ico") == 0) {
               size_t size;
               req->header = "OK";
               req->code = 200;
               req->content_type = "image/x-icon";
               req->content = get_binary("favicon.ico", &size);
               req->length = size;
               req->is_cookie = 0;
               return OK;
            } else if (is_contain(res, '/')){
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
   req->is_cookie = 0;
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
   user->ws_state = WS_TEXT;

   switch (res){
      case ERROR:
      case CLOSE:
         pthread_mutex_lock(&user->mutex);
         user->is_ws = 0;
         user->ws_state = WS_CLOSE;
         pthread_mutex_unlock(&user->mutex);
         break;
      case OK:
         res = ws_parse_message(ws_buffer);
         if (res == NAME)
            user->username = ws_buffer;
         else{
            message = malloc(MAXLEN);
            sprintf(message, "%s|%s", user->username, ws_buffer);
            len = strlen(message);
            frame.opcode = WS_TEXT;
            frame.payload_len = len;
            strcpy(frame.data, message);
            
            char* res = ws_get_frame(frame, &buffer_sz);
            if (res != NULL && buffer_sz > 0)
               ws_send_broadcast(res, buffer_sz);
            else error(__func__,"corrupted frame");
            free(message);
         }
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
  
   while((bytes = recv_buffer(user, buffer, sizeof(buffer))) > 0){
      if (!user->is_ws){
         res = handle_http_request(user, &req, buffer, bytes);
         send_response(user, &req);
      } else {
         handle_ws_request(user, buffer, bytes);
      }
   }

   SSL_free(user->SSL_sockfd);
   close(user->sockfd);
   pthread_exit(0);

   return 0;
}

void connections_cleanup(){
   for (int i = 0; i < GLOBAL.connections_size; i++)
      free(GLOBAL.connections[i]);
   GLOBAL.connections_size = 0;
}
