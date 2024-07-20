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

void set_current_page(user_t *user, char* res){
   if (res == NULL || strcmp(res, "/") == 0 || strcmp(res, CLEAR_COMMAND) == 0){
      user->current_page = INDEX_PAGE;
      return;
   }
   remove_prefix(res, "/");
   if (strcmp(res, "websocket.html") == 0){
      user->current_page = GLOBAL.DEFAULT_WEBSOCKET_PAGE;
      return;
   }
   if (strcmp(res, "post") == 0) return;
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
   int is_static;
   char *res, *post;
   req_type type = get_type_request(buffer, bytes);

   req->header = "OK";
   req->code = OK;
   req->is_cookie = 0;
   
   if (bytes > 0){
      switch(type){
         case GET:
            if (strstr(buffer, "Sec-WebSocket-Key") != NULL){
               ws_establish_connection(buffer, req, user);
               return WS;
            }
            res = header_parse(buffer, bytes, " ", &is_static);
            if (strcmp(res, "favicon.ico") == 0 && is_static) {
               req->content_type = get_content_type(res);
               req->content = get_file_content("favicon.ico", &req->length, "rb");
               return OK;
            } else if (strcmp(res, "style.css") == 0 && is_static){
               req->content_type = get_content_type(res);
               req->content = get_file_content("style.css", &req->length, "r");
               return OK;
            } else if (is_contain(res, '/') && !is_static){
                set_current_page(user, res);
                user->response_page = user->current_page;
                if (strcmp(res, CLEAR_COMMAND) == 0){ //FIXME: add database
                  logger(__func__, "delete chat history");
                  system("rm -f resources/text.txt && touch resources/text.txt");
                  update_html();
               }

            } else if (is_static)
               user->response_page = res;
            break;
         case POST:
            logger(__func__, "POST request");
            
            post = post_parse(buffer, bytes, "input=");
            if (res != NULL) {
               remove_prefix(post, "input=");
               url_decode(post);
               write_input(post, strlen(post), user->username);
            }
            break;
         default:
            error(__func__, "unsupported type of request");
            return NONE;
      }
   }
   req->content_type = get_content_type(user->response_page);
   if (strstr(req->content_type, "image") != NULL){
      req->content = get_file_content(user->response_page, &req->length, "rb");
   } else
      req->content = get_file_content(user->response_page, &req->length, "r");

   return OK;
}

void send_text_frame(char* message, int len){
   ws_frame_t frame;
   uint64_t buffer_sz;
   frame.opcode = WS_TEXT;
   frame.payload_len = len;
   frame.data = message;
   char* res = ws_get_frame(frame, &buffer_sz);
   if (res != NULL && buffer_sz > 0)
      ws_send_broadcast(res, buffer_sz);
   else error(__func__,"corrupted frame");
   free(res);
}
void handle_ws_request(user_t *user, char* buffer, int bytes){
   req_type res;
   char* message;
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
         message = malloc(MAXLEN);
         sprintf(message, "%s|left chat room", user->username);
         send_text_frame(message, strlen(message));
         free(message);
         break;
      case OK:
         res = ws_parse_message(ws_buffer);
         if (res == NAME)
            user->username = ws_buffer;
         else{
            message = malloc(MAXLEN);
            sprintf(message, "%s|%s", user->username, ws_buffer);
            send_text_frame(message, strlen(message));
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
