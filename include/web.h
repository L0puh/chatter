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
   WS,
   NOT_FOUND,
   ERROR,
   CLOSE
} req_type;

typedef struct {
   int length;
   int code;
   char* header; 
   char* accept;
   char* content_type;
   char* cookies;
   char* content;
   char* location;
} request_t;

typedef struct {
   int sockfd;
   char* addr; 
   char* current_page;
   int is_WS;
} user_t;

/************************************************************/

static char* WS_STRING = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

static char* CLEAR_COMMAND = "/clear.html";
static char* INDEX_PAGE = "index.html";
static char* ERROR_PAGE = "error.html";
static char* HEADER_PAGE = "header.html";
static char* SENDER_PAGE = "sender.html";
static char* DATABASE_FILE = "text.txt"; 
static char* DIR = "resources";

static char* available_routs[] = {
   "",
   "/",
   "error.html",
   "websocket.html",
};

/************************************************************/

void init_server(int port, int *sockfd, struct sockaddr_in*, size_t);

int   get_type_request(char* message, size_t sz);
void  get_input(char* message);
char* header_parse(const char* message, size_t sz, const char* symbol);
char* post_parse(const char* message, size_t sz, const char* symbol);
char* get_str_addr(struct sockaddr_in addr);
char* set_cookie(char* param, char* value);
void free_request(request_t *req);

void* handle_client(void* user);
int handle_request(user_t *user, request_t *req, char* buffer, int bytes);

void free_request(request_t *req);
void set_current_page(user_t *user, char *input);
void url_decode(char* str);
void send_response(user_t user, request_t req);
void recv_loop(int client_sockfd, char* buffer, int *bytes);

char* ws_key_parse(const char* buffer);
char* ws_create_accept(const char* key);
char* ws_create_upgrade(const request_t req);
char* ws_recv_frame(char* buffer, int *res);
char* ws_recv_text(char* buffer, uint64_t msglen, uint16_t offset);

#endif
