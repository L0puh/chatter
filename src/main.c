#include "utils.h"
#include "web.h"
#include "state.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct state GLOBAL;

int main(int argc, char* argv[]){
   int sockfd, client_sockfd;
   struct sockaddr_in servaddr;

   print_usage(argc);
   init_server(atoi(argv[1]), &sockfd, &servaddr, sizeof(servaddr));
   
   GLOBAL.SERVER_RUNNING = 1;
   GLOBAL.current_page = "index.html";
   while(GLOBAL.SERVER_RUNNING){ 
      ASSERT((client_sockfd = accept(sockfd, NULL, NULL)));
      handle_client(client_sockfd);
   }

   close(client_sockfd);
   close(sockfd);
   return 0;
}


