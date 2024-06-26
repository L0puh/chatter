#include "utils.h"
#include "web.h"
#include "state.h"

#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h>
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
      logger("connection from", get_str_addr(cliaddr));
      handle_client(client_sockfd);
   }

   close(client_sockfd);
   close(sockfd);
   return 0;
}


