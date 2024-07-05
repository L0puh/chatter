#include "http.h"
#include "utils.h"
#include "server.h"
#include "state.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
   user_t user;
   pthread_t ptr;
   socklen_t cliaddr_sz;
   int sockfd, client_sockfd;
   struct sockaddr_in servaddr, cliaddr;

   print_usage(argc);

   GLOBAL.SERVER_RUNNING = 1;
   GLOBAL.DEFAULT_PAGE = INDEX_PAGE;
   GLOBAL.connections_size = 0;

   pthread_mutex_init(&GLOBAL.mutex, 0);
   init_server(atoi(argv[1]), &sockfd, &servaddr, sizeof(servaddr));
   
   cliaddr_sz = sizeof(cliaddr); 
  
   while(GLOBAL.SERVER_RUNNING){ 
      ASSERT((client_sockfd = accept(sockfd, (struct sockaddr*)&cliaddr, &cliaddr_sz)));
     
      user.addr = get_str_addr(cliaddr);
      user.sockfd = client_sockfd;
      user.current_page = GLOBAL.DEFAULT_PAGE;
      user.is_ws = 0;

      ASSERT(pthread_create(&ptr, NULL, handle_client, (void*)&user));
      ASSERT(pthread_detach(ptr));
   }
   close(sockfd);
   return 0;
}


