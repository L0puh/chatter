#ifndef DB_H
#define DB_H

#include "state.h"
#include <libpq-fe.h>

void db_init();
void db_fetch_tables(char* filename);

inline const char* db_error(){ return PQerrorMessage(GLOBAL.DB); }
inline int db_version(){ return PQlibVersion(); }

#endif
