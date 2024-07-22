#ifndef DB_H
#define DB_H

#include <libpq-fe.h>

void db_init();
int db_version();
const char* db_error();

#endif
