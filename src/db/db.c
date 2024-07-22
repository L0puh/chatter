#include "db.h"
#include "state.h"
#include "utils.h"

int db_version(){
  return PQlibVersion();
}

void db_init(){
   size_t len;
   char* conninfo = get_file_content("conninfo", &len, "r", "resources/");
   GLOBAL.DB = PQconnectdb(conninfo);
   DB_ASSERT(GLOBAL.DB, CONNECTION_OK);
   logger(PQdb(GLOBAL.DB), "connected");
}

const char* db_error(){
   return PQerrorMessage(GLOBAL.DB);
}
