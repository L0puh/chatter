#include "server.h"
#include "http.h"
#include "utils.h"
#include "websocket.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

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
   return;
}

int handle_request(user_t *user, request_t *req, char* buffer, int bytes){
   char *res, *ws;
   req_type type = get_type_request(buffer, bytes);
   

   if (bytes > 0 && type == POST && !user->is_ws){
      logger(__func__, "POST request");
      
      res = post_parse(buffer, bytes, "input=");
      remove_prefix(res, "input=");
      url_decode(res);
      write_input(res, strlen(res), user->addr);

   } else if (bytes > 0 && type == GET){
      if (strstr(buffer, "Sec-WebSocket-Key") != 0){
         logger(__func__, "WebSocket request");
         ws = malloc(128);
         ws = ws_key_parse(buffer);
         if (ws == NULL) 
            error(__func__, "error in parsing WS key");
         req->header = "Switching Protocols";
         req->code = 101;
         req->accept = ws_create_accept(ws);
         user->is_ws = 1;
         user->ws_id = GLOBAL.connections_size;
         user->ws_state = WS_TEXT;
         pthread_mutex_lock(&GLOBAL.mutex);
         if (GLOBAL.connections_size+1 < QUERY)
            GLOBAL.connections[GLOBAL.connections_size++] = user;
         pthread_mutex_unlock(&GLOBAL.mutex);
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


void handle_ws(user_t *user, char* buffer, int bytes){
   int res;
   ws_frame_t frame;
   uint64_t buffer_sz;

   char* ws_buffer = ws_recv_frame(buffer, &res);
   if (ws_buffer != NULL && res != ERROR && res != CLOSE){

      frame.opcode = WS_TEXT;
      frame.payload_len = strlen(ws_buffer);
      frame.data = ws_buffer;
      char* res = ws_get_frame(frame, &buffer_sz);
      
      if (res != NULL && buffer_sz > 0)
         ws_send_broadcast(res, buffer_sz);
   }
   else if (res == CLOSE) {
      user->is_ws = 0;
      user->ws_state = WS_CLOSE;
   }
}

void* handle_client(void* th_user){
   user_t user; 
   request_t req;
   int bytes, res;
   char buffer[MAXLEN];

   user = *(user_t*)th_user;
   while ((bytes = recv(user.sockfd, buffer, sizeof(buffer), 0)) > 0){
      buffer[bytes] = '\0';
     
      res = handle_request(&user, &req, buffer, bytes);
      if (user.is_ws == 1) handle_ws(&user, buffer, bytes);
      if (res == WS || user.is_ws == 0)
         send_response(user, req);
   }
   ASSERT(bytes);
   close(user.sockfd);
   pthread_exit(0);
   return 0;
}

