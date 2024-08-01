#include "db.h"
#include "state.h"
#include "utils.h"
#include <libpq-fe.h>
#include <stdio.h>

void db_migrate_tables(const char* filename){
   size_t len;
   char *query = get_file_content(filename, &len, "r", "resources/");
   DB_ASSERT(PQexec(GLOBAL.DB, query), PGRES_COMMAND_OK);
   logger(filename, "fetched successfully");
   free(query);
}

void db_clear_table(char* table_name){
   char* query;
   query = malloc(MAXLEN);
   sprintf(query, "TRUNCATE %s;", table_name);
   db_exec(query, PGRES_COMMAND_OK);
   logger(table_name, "cleared");
   free(query);
}

void db_insert(const char* table_name, char* values){
   char* query;
   query = malloc(MAXLEN);
   sprintf(query, "INSERT INTO %s VALUES(%s);", table_name, values);
   db_exec(query, PGRES_COMMAND_OK);
   free(query);
}

PGresult* db_exec(const char* query, int status){
   PGresult *res;
   res = PQexec(GLOBAL.DB, query);
   DB_ASSERT(res, status);
   return res;
}

PGresult* db_select_all(const char* table_name){
   char* query;
   PGresult *res;
   
   query = malloc(MAXLEN);
   sprintf(query, "SELECT * FROM %s", table_name);
   res = db_exec(query, PGRES_TUPLES_OK);
   free(query);
   
   return res;
}

void db_init(){
   size_t len;
   char* conninfo;

   conninfo = get_file_content("conninfo", &len, "r", "resources/");
   GLOBAL.DB = PQconnectdb(conninfo);
   if (PQstatus(GLOBAL.DB) == CONNECTION_BAD)
      db_errorl(__func__, __FILE__, __LINE__);
   free(conninfo);

   logger(PQdb(GLOBAL.DB), "connected");
   db_migrate_tables("database.sql");
}

const char* db_error() { return PQerrorMessage(GLOBAL.DB); }
int db_version()       { return PQlibVersion(); }
