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
   cliaddr_sz = sizeof(cliaddr); 
   init_server(atoi(argv[1]), &sockfd, &servaddr, sizeof(servaddr));
  
   while(GLOBAL.SERVER_RUNNING){ 
      ASSERT((client_sockfd = accept(sockfd, (struct sockaddr*)&cliaddr, &cliaddr_sz)));
     
      user.id = -1;
      user.sockfd = client_sockfd;
      user.addr = get_str_addr(cliaddr);
      user.current_page = GLOBAL.DEFAULT_PAGE;

      //FIXME: add mutex 
      for (int i = 0; i < GLOBAL.connections_size; i++){
         if (strcmp(GLOBAL.connections[i].addr, user.addr) == 0){
            user.id = GLOBAL.connections[i].id;
            break;
         }
      }
      if (user.id == -1 && GLOBAL.connections_size < QUERY){
         user.id = GLOBAL.connections_size;
         GLOBAL.connections[GLOBAL.connections_size++] = user;
       }

      ASSERT(pthread_create(&ptr, NULL, handle_client, (void*)&user));
      ASSERT(pthread_detach(ptr));
   }
   close(sockfd);
   return 0;
}


