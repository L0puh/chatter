#ifndef STATE_H
#define STATE_H

#define QUERY 10
#define MAXLEN 4096

#include <pthread.h>
#include <openssl/ssl.h>

extern struct state GLOBAL;

typedef enum {
   NONE,
   GET,
   POST,
   OK,
   WS,
   NOT_FOUND,
   ERROR,
   CLOSE,
   NAME,
   TEXT,
   WS_CONNECT,
} req_type;


typedef enum {
   SSL_flag = 0x0001,
} options_flags;

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
   SSL *SSL_sockfd;
   
   int is_ws;
   int is_ssl;
   int ws_id;
   int ws_state;
   int port;

   char* addr; 
   char* current_page;
   char* username;

} user_t;

struct state{
   int SERVER_RUNNING;
   char* DEFAULT_PAGE;
   int connections_size;
   user_t* connections[QUERY];
   pthread_mutex_t mutex;
};

/************************************************************/


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
   "index.html",
   "error.html",
   "websocket.html",
};

#endif 
