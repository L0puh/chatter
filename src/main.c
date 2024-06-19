#include "utils.h"
#include "web.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]){
   print_usage(argc);
   int sockfd, client_sockfd;
   int RUN_SERVER=1;
   struct sockaddr_in servaddr;
   char buffer[MAXLEN];
   init_server(atoi(argv[1]), &sockfd, &servaddr, sizeof(servaddr));
  
   while(RUN_SERVER){ 
      ASSERT((client_sockfd = accept(sockfd, NULL, NULL)));
      while (get_recv(client_sockfd, buffer, sizeof(buffer)) > 0){
         printf("%s\n", buffer);

         /*FIXME: this is a hack, fix it later*/
         char* h = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
         char* m = get_file_content("index.html");
         char* r = malloc(MAXLEN);

         sprintf(r, "%s %lu\n\n%s\n", h, strlen(m), m);
         send(client_sockfd, r, strlen(r), 0);
         free(r); free(m); 
      }
      
      close(sockfd);
      RUN_SERVER=0;
   }
}


