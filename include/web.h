/**********************
 
library which provides 
simple http web server

***********************/

#ifndef WEB_H
#define WEB_H
#define QUERY 10
#define MAXLEN 4028

#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>

void init_server(int port, int *sockfd, struct sockaddr_in *servaddr, size_t sz);
int get_recv(int client_sockfd, char *buffer, size_t sz);

#endif
