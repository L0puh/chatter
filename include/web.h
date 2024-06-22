/**********************
 
library which provides 
simple http web server

***********************/

#ifndef WEB_H
#define WEB_H

#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>


#define QUERY 10
#define MAXLEN 4096

enum REQ {
   NONE,
   GET,
   OK
};

/************************************************************/

void init_server(int port, int *sockfd, struct sockaddr_in *servaddr, size_t sz);
int get_request(char* message, size_t sz);
char* get_input(char* message);
char* get_parse(char* message, size_t sz, char* res, char* symbol);

void handle_client(int sockfd);

#endif
