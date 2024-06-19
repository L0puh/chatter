#include "web.h"
#include "utils.h"
#include <stdio.h>

void init_server(int port, int *sockfd, struct sockaddr_in *servaddr, size_t sz){
   char message[32];
   sprintf(message, "SERVER IS RUNNING\nPORT: %d", port);
   logger((char*)__func__, message);

   ASSERT((*sockfd = socket(AF_INET, SOCK_STREAM, 0)));
   bzero(servaddr, sz);
   servaddr->sin_port = htons(port);
   servaddr->sin_family = AF_INET;
   servaddr->sin_addr.s_addr = INADDR_ANY; /* FIXME: set up an ip */

   ASSERT(bind(*sockfd, (const struct sockaddr*)servaddr, sz));
   ASSERT(listen(*sockfd, QUERY));
   
   sprintf(message, "SERVER IS RUNNING\nPORT: %d", port);
}

int get_recv(int client_sockfd, char *buffer, size_t sz){
   int bytes = recv(client_sockfd, buffer, sz, 0);
   char message[32];
   sprintf(message, "recieved %d", bytes);
   logger((char*)__func__, message);
   /* TODO: handle with global state if (bytes < 0) */
   return bytes; 
}
