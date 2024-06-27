#include "utils.h"
#include "web.h"
#include "state.h"

#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

struct state GLOBAL;

int main(int argc, char* argv[]){
   pthread_t ptr;
   int sockfd, client_sockfd;
   struct sockaddr_in servaddr, cliaddr;
   socklen_t cliaddr_sz;
  
   print_usage(argc);
   init_server(atoi(argv[1]), &sockfd, &servaddr, sizeof(servaddr));
   GLOBAL.SERVER_RUNNING = 1;
   GLOBAL.current_page = INDEX_PAGE;
   cliaddr_sz = sizeof(cliaddr); 
   
   while(GLOBAL.SERVER_RUNNING){ 
      ASSERT((client_sockfd = accept(sockfd, (struct sockaddr*)&cliaddr, &cliaddr_sz)));
      user_t user;
      
      user.sockfd = client_sockfd;
      user.addr = get_str_addr(cliaddr);

      ASSERT(pthread_create(&ptr, NULL, handle_client, (void*)&user));
      ASSERT(pthread_detach(ptr));
   }

   close(client_sockfd);
   close(sockfd);
   return 0;
}


