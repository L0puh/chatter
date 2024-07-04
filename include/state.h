#ifndef STATE_H
#define STATE_H

#define QUERY 10
#define MAXLEN 4096


extern struct state GLOBAL;

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
 
   int id;
   int sockfd;
   int is_WS;
   
   char* addr; 
   char* current_page;

} user_t;

struct state{
   int SERVER_RUNNING;
   char* DEFAULT_PAGE;
   int connections_size;
   user_t connections[QUERY];
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
   "error.html",
   "websocket.html",
};

#endif 
