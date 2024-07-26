#ifndef DB_H
#define DB_H

#include <libpq-fe.h>

void db_init();
int  db_version();
void db_migrate_tables(const char* filename);

PGresult* db_select_all(const char* table_name);
PGresult* db_exec(const char* query, int status);

void db_insert(const char* table_name, char* values);
void db_clear_table(char* table_name);

const char* db_error();

#endif
