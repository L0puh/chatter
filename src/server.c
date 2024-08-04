#include "server.h"
#include "http.h"
#include "state.h"
#include "utils.h"
#include "db.h"
#include "websocket.h"

#include <libpq-fe.h>
#include <openssl/sha.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct state GLOBAL;

void set_current_page(user_t *user, char* res){
   if (res == NULL || strcmp(res, "/") == 0 || strcmp(res, CLEAR_COMMAND) == 0 || user->current_page == NULL){
      user->current_page = INDEX_PAGE;
      return;
   }
   remove_prefix(res, "/");
   if (strcmp(res, "websocket.html") == 0){
      user->current_page = GLOBAL.DEFAULT_WEBSOCKET_PAGE;
      return;
   }
   if (strcmp(res, "post") == 0) {
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

char* get_by_login(char* what, char* login){
   int rows;
   char* query;
   PGresult *res;
   query = malloc(MAXLEN);
   sprintf(query, "SELECT %s FROM users WHERE username='%s';", what, login);
   res = db_exec(query, PGRES_TUPLES_OK);
   if (PQresultStatus(res) == PGRES_TUPLES_OK)
      return PQgetvalue(res, 0, 0);
   return NULL;
}

char* hash_password(char* pswd, char* salt){
   SHA256_CTX ctx;
   char *salted_pswd, *hex_str;
   unsigned char *buffer; 

   salted_pswd = malloc((strlen(pswd) + strlen(salt)) * CHAR_BIT); 
   sprintf(salted_pswd, "%s%s", pswd, salt);

   buffer = malloc(SHA256_DIGEST_LENGTH);
   SHA256_Init(&ctx);
   SHA256_Update(&ctx, (unsigned char*) salted_pswd, strlen(salted_pswd));
   SHA256_Final(buffer, &ctx);

   hex_str = from_bytes_to_string(buffer, SHA256_DIGEST_LENGTH);
   free(buffer);
   free(salted_pswd);
   return hex_str;
}


int add_user(char* login, char* pswd){
   unsigned char salt[32];    
   char *query, *db_pswd, *db_salt, *salt_str;
   char* buffer = malloc(SHA256_DIGEST_LENGTH);
  
   db_salt = get_by_login("salt", login);
   if (db_salt != NULL){
      db_pswd = get_by_login("password", login);
      buffer  = hash_password(pswd,  db_salt);
      if (strcmp((char*) buffer, db_pswd) != 0){
         error(__func__, "password doesn't match"); 
         free(buffer);
         return -1;
      }
   } else {
      size_t *len;
      RAND_bytes(salt, 32);
     
      salt_str = from_bytes_to_string(salt, 32); 
      buffer = hash_password(pswd, salt_str);
      query = malloc(MAXLEN);

      sprintf(query, 
            "INSERT INTO users(username, salt, password) VALUES($$%s$$, $$%s$$, $$%s$$);", 
             login, salt_str, buffer);
      
      db_exec(query, PGRES_COMMAND_OK);
      free(query);
      free(salt_str);
   }    
   free(buffer);
   return 0;
}

char* parse_username_cookies(char* buffer){
   char* p, *res;
   char* cp_buffer;
   cp_buffer = malloc(strlen(buffer));
   memcpy(cp_buffer, buffer, strlen(buffer));
   if ((p = strstr(cp_buffer, "Cookie: username=")) != NULL){
      p += strlen("Cookie: username=");
      p = strtok(p, "\r");
      res = malloc(strlen(p)+1);
      strcpy(res, p);
      res[strlen(p)] = '\0';
      free(cp_buffer);
      return res;

   }
   free(cp_buffer);
   return NULL;
}

void update_username(user_t *user, char* buffer){
   char* usrname = parse_username_cookies(buffer);
   if (usrname != NULL){
      pthread_mutex_lock(&GLOBAL.mutex); 
      strcpy(user->username, usrname);
      pthread_mutex_unlock(&GLOBAL.mutex); 
      free(usrname);
   } 
}

req_type handle_http_request(user_t *user, request_t *req, char* buffer, int bytes){
   int is_static;
   char *res, *post, *name, *pswd;
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
            if (strcmp(res, "favicon.ico") == 0){
               req->content_type = get_content_type(res, &req->content_type_i);
               req->content = get_file_content("favicon.ico", &req->length, "rb", GLOBAL.IMAGE_DIR);
               return OK;
            } else if (strcmp(res, "style.css") == 0 && is_static){
               req->content_type = get_content_type(res, &req->content_type_i);
               req->content = get_file_content("style.css", &req->length, "r", GLOBAL.STYLE_DIR);
               return OK;
            } else if (is_contain(res, '/') && !is_static){
                set_current_page(user, res);
                user->response_page = user->current_page;
                if (strcmp(res, CLEAR_COMMAND) == 0){ 
                  logger(__func__, "delete chat history");
                  db_clear_table("posts");
                  fetch_posts();
               }
            } else if (is_static)
               user->response_page = res;
            break;
         case POST:
            logger(__func__, "POST request");
            user->response_page = user->current_page;
            post = post_parse(buffer, bytes, "input=");
            if (post != NULL) {
               remove_prefix(post, "input=");
               url_decode(post);
               write_input_to_db(post, strlen(post), user->username);
               break;
            }
            post = post_parse(buffer, bytes, "login=");
            if (post != NULL){
               post +=strlen("login=");

               char* tok = strtok(post, "&");
               name = malloc(strlen(tok)+1);
               strcpy(name, tok);
               name[strlen(tok)] = '\0';

               tok = strtok(NULL, "&");
               tok+=strlen("pswd=");
               pswd = malloc(strlen(tok)+1);
               strcpy(pswd, tok);
               pswd[strlen(tok)] = '\0';
               
               if (add_user(name, pswd) != -1){
                  req->cookies = set_cookie("username", name);
                  req->is_cookie = 1;
               }
               free(name);
               free(pswd);
            }
            break;
         default:
            error(__func__, "unsupported type of request");
            return NONE;
      }
   }
   req->content_type = get_content_type(user->response_page, &req->content_type_i);
   switch(req->content_type_i){
      case IMAGE:
         req->content = get_file_content(user->response_page, &req->length, "rb", GLOBAL.IMAGE_DIR);
         break;
      case CSS:
         req->content = get_file_content(user->response_page, &req->length, "r", GLOBAL.STYLE_DIR);
         break;
      default:
         req->content = get_file_content(user->response_page, &req->length, "r", GLOBAL.HTML_DIR);
   } 

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
         message = malloc(MAXLEN);
         sprintf(message, "%s|%s", user->username, ws_buffer);
         send_text_frame(message, strlen(message));
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
  
   ws_add_connection(user);
   while((bytes = recv_buffer(user, buffer, sizeof(buffer))) > 0){
      update_username(user, buffer);
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


void handle_termination(int sig){
   ws_send_close();
   connections_cleanup();
#ifdef WITH_DB
   PQfinish(GLOBAL.DB);
#endif
   logger(__func__, "terminating the program");
   exit(0);
}


