#include "http.h"
#include "utils.h"
#include "server.h"
#include "state.h"
#include "websocket.h"
#include "ssl.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char* argv[]){
   user_t *user;
   pthread_t ptr;
   socklen_t cliaddr_sz;
   uint8_t options;
   int sockfd, client_sockfd;
   SSL_CTX* ctx;
   struct sockaddr_in servaddr, cliaddr;

   print_usage(argc);
   options = get_options(argc, argv);
   if (options & SSL_flag){
      ctx = init_ssl();
      GLOBAL.DEFAULT_WEBSOCKET_PAGE = "websocket_ssl.html";
   } else 
      GLOBAL.DEFAULT_WEBSOCKET_PAGE = "websocket.html";

   GLOBAL.SERVER_RUNNING = 1;
   GLOBAL.DEFAULT_PAGE = INDEX_PAGE;
   GLOBAL.connections_size = 0;

   pthread_mutex_init(&GLOBAL.mutex, 0);
   init_server(atoi(argv[1]), &sockfd, &servaddr, sizeof(servaddr));
   
   cliaddr_sz = sizeof(cliaddr); 
  
   while(GLOBAL.SERVER_RUNNING){ 
      ASSERT((client_sockfd = accept(sockfd, (struct sockaddr*)&cliaddr, &cliaddr_sz)));
      
      user = malloc(sizeof(user_t));
      if (options & SSL_flag){
         user->is_ssl = 1;
         user->SSL_sockfd = create_ssl(client_sockfd, ctx);
      } else {
         user->is_ssl = 0;
         user->SSL_sockfd = NULL;
      }
      user->sockfd = client_sockfd;
      user->addr = get_str_addr(cliaddr);
      user->port = ntohs(cliaddr.sin_port);
      user->current_page = GLOBAL.DEFAULT_PAGE;
      user->is_ws = 0;
      user->username = user->addr;
      pthread_mutex_init(&user->mutex, 0);

      ASSERT(pthread_create(&ptr, NULL, handle_client, (void*)user));
      ASSERT(pthread_detach(ptr));
   }
   ws_send_close();
   SSL_CTX_free(ctx);
   close(sockfd);
   return 0;
}


