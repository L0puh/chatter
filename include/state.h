#ifndef STATE_H
#define STATE_H

#include "web.h"

struct state{
   int SERVER_RUNNING;
   char buffer[MAXLEN];
   char* current_page;
};

extern struct state GLOBAL;

#endif 