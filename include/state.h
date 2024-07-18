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
   WS,
   NOT_FOUND,
   ERROR,
   CLOSE,
   NAME,
   TEXT,
   WS_CONNECT,
   
   OK = 200,
   MOVED_PERMANENTLY = 300,
   SWITCHING_PROTOCOLS = 101,
} req_type;


typedef enum {
   SSL_flag = 0x0001,
} options_flags;

typedef struct {
   int code;
   int is_cookie;
   size_t length;
   char* header; 
   char* accept;
   char* cookies;
   char* content;
   char* location;
   char* content_type;
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
   char* response_page;
   char* username;
   pthread_mutex_t mutex;

} user_t;

struct state{
   int SERVER_RUNNING;
   char* DEFAULT_PAGE;
   char* DEFAULT_WEBSOCKET_PAGE;
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
   "websocket_ssl.html",
   "style.css",
   "logo.png",
};

#endif 
