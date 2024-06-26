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

static char* FORMAT = "<p>%s</p>\n";
static char* INDEX_PAGE = "index.html";
static char* ERROR_PAGE = "error.html";
static char* HEADER_PAGE = "header.html";
static char* SENDER_PAGE = "sender.html";
static char* DATABASE_FILE = "text.txt"; 

static char* REQ_OK = "request_ok.txt";


static char* available_routs[] = {
   "index.html", 
   "",
   "/",
   "error.html"
};

/************************************************************/

void init_server(int port, int *sockfd, struct sockaddr_in*, size_t);

int get_type_request(char* message, size_t sz);
char* get_input(const char* message);
char* get_parse(const char* message, size_t sz, const char* symbol);
char* get_str_addr(struct sockaddr_in addr);

void handle_client(int sockfd);
void handle_request(int bytes);

void set_current_page(char *input);
void send_request(int client_sockfd, enum REQ type);
void recv_loop(int client_sockfd, int *bytes);

#endif
