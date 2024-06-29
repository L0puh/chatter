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

typedef enum {
   NONE,
   GET,
   POST,
   OK,
   NOT_FOUND,
} reqtype_t;

typedef struct {
   int length;
   int code;
   char* header; 
   char* content_type;
   char* cookies;
   char* content;
} request_t;

typedef struct {
   int sockfd;
   char* addr; 
   char* current_page;
} user_t;

/************************************************************/

static char* CLEAR_COMMAND = "/clear.html";
static char* INDEX_PAGE = "index.html";
static char* ERROR_PAGE = "error.html";
static char* HEADER_PAGE = "header.html";
static char* SENDER_PAGE = "sender.html";
static char* DATABASE_FILE = "text.txt"; 
static char* DIR = "resources";

static char* available_routs[] = {
   "index.html", 
   "",
   "/",
   "error.html",
};

/************************************************************/

void init_server(int port, int *sockfd, struct sockaddr_in*, size_t);

int   get_type_request(char* message, size_t sz);
void  get_input(char* message);
char* header_parse(const char* message, size_t sz, const char* symbol);
char* post_parse(const char* message, size_t sz, const char* symbol);
char* get_str_addr(struct sockaddr_in addr);
char* set_cookie(char* param, char* value);

void* handle_client(void* user);
void  handle_request(user_t user, char* buffer, int bytes);

void free_request(request_t *req);
void set_current_page(user_t *user, char *input);
void url_decode(char* str);
void send_request(user_t user, request_t req);
void recv_loop(int client_sockfd, char* buffer, int *bytes);

#endif
