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

void send_request(user_t user, enum req_t type){
   size_t msg_sz = MAXLEN;
   int bytes_sent, total;
   char *header, *message, *result;

   switch(type){
      case OK:
         header = get_file_content(req_t_OK, 0);
         break;
      default:
         header = get_file_content(req_t_OK, 0);

   }
   message = get_file_content(user.current_page, &msg_sz);
   result = malloc(msg_sz + MAXLEN);
   
   sprintf(result, "%s%lu\n\n%s", header, strlen(message), message);
   bytes_sent = 0, total = strlen(result);
   
   while (bytes_sent < total){
      bytes_sent = send(user.sockfd, result, strlen(result), 0);
      ASSERT(bytes_sent);
   }
  
   free(header); free(result); free(message); 
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
   handle_request(user, buffer, bytes);
   send_request(user, OK);
   ASSERT(bytes);
   pthread_exit(0);
   return 0;
}


