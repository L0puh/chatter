#include "web.h"
#include "utils.h"
#include "state.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void set_current_page(char* res){
   if (strcmp(res, "/") == 0 || strcmp(res, CLEAR_COMMAND)){
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

void write_input(char* buffer, size_t sz, char* data){
   char *input, *new, *format;
   format = "<p><span style=\"color:#b0716f\"> %s: </span><span>%s </span></p>";
   new = malloc(MAXLEN); 
   
   input = get_input(get_parse(buffer, sz, "?"));
   sprintf(new, format, data, input);
   write_to_file(DATABASE_FILE, new, "a");
   update_html();

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

void handle_request(user_t user, int bytes){
   char* res;
   if (bytes > 0 && get_type_request(GLOBAL.buffer, bytes) == GET){
      res = get_parse(GLOBAL.buffer, bytes, " ");
         
      if (is_contain(res, '?')){
         write_input(res, strlen(res), user.addr);
      } else if (is_contain(res, '/')){
         set_current_page(res);

         /* FIXME: add database */
         if (strcmp(res, CLEAR_COMMAND) == 0){
            system("rm -f resources/text.txt && touch resources/text.txt"); 
            update_html();
         }
      }
   }
}

void* handle_client(void* th_user){
   int bytes;
   user_t user = *(user_t*)th_user;
   
   while((bytes = recv(user.sockfd, GLOBAL.buffer, 
                  sizeof(GLOBAL.buffer), 0)) > 0 
                  && GLOBAL.SERVER_RUNNING)
   {
      GLOBAL.buffer[bytes+1] = '\0';
      handle_request(user, bytes);
      send_request(user.sockfd, OK);
   }
   ASSERT(bytes);
   close(user.sockfd);
   pthread_exit((void*)(-1));
   return 0;
}


