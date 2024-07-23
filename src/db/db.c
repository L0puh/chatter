#include "db.h"
#include "state.h"
#include "utils.h"
#include <libpq-fe.h>


void db_fetch_tables(char* filename){
   size_t len;
   char *query = get_file_content(filename, &len, "r", "resources/");
   DB_ASSERT(PQexec(GLOBAL.DB, query), PGRES_COMMAND_OK);
   logger(filename, "fetched successfully");
}

void db_init(){
   size_t len;
   char* conninfo = get_file_content("conninfo", &len, "r", "resources/");
   GLOBAL.DB = PQconnectdb(conninfo);
   if (PQstatus(GLOBAL.DB) == CONNECTION_BAD)
      db_errorl(__func__, __FILE__, __LINE__);
   logger(PQdb(GLOBAL.DB), "connected");
   db_fetch_tables("database.sql");
}

