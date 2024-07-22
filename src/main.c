#include "db.h"
#include "http.h"
#include "utils.h"
#include "server.h"
#include "state.h"
#include "websocket.h"
#include "ssl.h"

#include <netdb.h>
#include <pthread.h>
#include <signal.h>
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
   struct addrinfo *servaddr;
   struct sockaddr_storage cliaddr;

   print_usage(argc);
   
   options = get_options(argc, argv);
   if (options & SSL_flag){
      ctx = init_ssl();
      GLOBAL.DEFAULT_WEBSOCKET_PAGE = "websocket_ssl.html";
   } else 
      GLOBAL.DEFAULT_WEBSOCKET_PAGE = "websocket.html";

   GLOBAL.SERVER_RUNNING = 1;
   GLOBAL.STYLE_DIR = "resources/style/";
   GLOBAL.IMAGE_DIR = "resources/images/";
   GLOBAL.HTML_DIR  = "resources/html/";

   GLOBAL.DEFAULT_PAGE = INDEX_PAGE;
   GLOBAL.connections_size = 0;

   pthread_mutex_init(&GLOBAL.mutex, 0);
   sockfd = init_server(argv[1], argv[2], servaddr);

#ifdef WITH_DB
   db_init();
#endif
   
   cliaddr_sz = sizeof(cliaddr); 
   signal(SIGINT, handle_termination);
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
      struct sockaddr_in *sin = (struct sockaddr_in *)&cliaddr;
      user->sockfd = client_sockfd;
      user->addr = get_str_addr(*sin);
      user->port = ntohs(sin->sin_port);
      user->current_page = GLOBAL.DEFAULT_PAGE;
      user->is_ws = 0;
      user->username = user->addr;
      pthread_mutex_init(&user->mutex, 0);

      ASSERT(pthread_create(&ptr, NULL, handle_client, (void*)user));
      ASSERT(pthread_detach(ptr));
   }
   ws_send_close();
   SSL_CTX_free(ctx);
   freeaddrinfo(servaddr);
   connections_cleanup();
   close(sockfd);
   return 0;
}


