#ifndef STATE_H
#define STATE_H

#include "web.h"

struct state{
   int SERVER_RUNNING;
   char* DEFAULT_PAGE;
   
};

extern struct state GLOBAL;

#endif 
