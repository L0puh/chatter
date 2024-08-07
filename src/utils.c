#include "utils.h"
#include "state.h"
#include "db.h"

#include <libpq-fe.h>
#include <openssl/err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void update_html(){
   char* header = get_file_content(HEADER_PAGE,   NULL, "r", GLOBAL.HTML_DIR);
   char* sender = get_file_content(SENDER_PAGE,   NULL, "r", GLOBAL.HTML_DIR);
   char* text   = get_file_content(DATABASE_FILE, NULL, "r", GLOBAL.HTML_DIR);;
   write_to_file(INDEX_PAGE, header, "w", GLOBAL.HTML_DIR);
   if (strcmp(text, "") != 0)
      write_to_file(INDEX_PAGE, text,   "a", GLOBAL.HTML_DIR);
   write_to_file(INDEX_PAGE, sender, "a", GLOBAL.HTML_DIR);
   free(header);
   free(sender);
   free(text);
}

void write_to_file(const char* filename, char* input, char* mode, char* path){
   char name[32];
   sprintf(name, "%s/%s", path, filename);
   FILE* f = fopen(name, mode);
   if (f == NULL) 
      error(__func__, "file open error");
   fprintf(f, "%s\n", input); 
   fclose(f);
}


char* get_file_content(const char* filename, size_t *length, char* mode, char* filepath){
   FILE *fp;
   char *path;
   char *buffer;
   int size;

   path = malloc(128);
   sprintf(path, "%s/%s", filepath, filename);
   fp = fopen(path, mode);
   if (fp == NULL){
      error(path, "cannot be found");
      return NULL;
   }

   fseek(fp, 0, SEEK_END);
   size = ftell(fp);
   rewind(fp);

   if((buffer = malloc(size + 1)) == NULL){
      fclose(fp);
      error(__func__, "failed allocation");
      return NULL;
   }
   if(fread(buffer, sizeof(unsigned char), size, fp) != size){
      fclose(fp);
      error(__func__, "failed fread");
      free(buffer);
      return NULL;
   }
   fclose(fp);
   buffer[size] = 0;
   if (length != NULL)
      *length = size+1;
   return buffer;
}


int is_contain(char* message, char symbol){
   for (int i = 0; i < strlen(message); i++){
      if (message[i] == symbol) return 1;
   }
   return 0;
}

void print_usage(int argc){
   if (argc < 3){
      printf("usage: ./server <host> <port or service>\n");
      printf("options:\n\t--SSL - enable secure connection\n");
      exit(0);
   }
}

uint8_t get_options(int argc, char* argv[]){
   uint8_t options = 0;
   if (argc == 4){
      if (strcmp(argv[3],"--SSL") == 0){
         options |= SSL_flag;
         logger(__func__, "SSL option is enabled");
      }
   }
   return options;
}
void remove_prefix(char *str, const char* prefix){
   char* p;
   if ((p = strtok(str, prefix)) != NULL){
      strcpy(str, p);
   } else {
      error(prefix, "prefix isn't found");
   }
}

void fetch_posts(){
   PGresult *res;
   char *header, *sender, *new, *format;
   
   format = "<p><span style=\"color:#b0716f\"> %s:\
             </span><span>%s </span></p>";
   header = get_file_content(HEADER_PAGE,   NULL, "r", GLOBAL.HTML_DIR);
   sender = get_file_content(SENDER_PAGE,   NULL, "r", GLOBAL.HTML_DIR);
   res  = db_select_all("posts");
  
   write_to_file(INDEX_PAGE, header, "w", GLOBAL.HTML_DIR);
   int rows = PQntuples(res);
   new = malloc(MAXLEN);
   for (int i = 0; i < rows; i++){
      sprintf(new, format, PQgetvalue(res, i, 1), PQgetvalue(res, i, 2));
      write_to_file(INDEX_PAGE, new, "a", GLOBAL.HTML_DIR);
   }
   write_to_file(INDEX_PAGE, sender, "a", GLOBAL.HTML_DIR);

   free(header);
   free(sender);
   free(new);
}

void write_input_to_db(char* buffer, size_t sz, char* data){
   char *new;

   new = malloc(MAXLEN);
   sprintf(new, "INSERT INTO posts(title, content) VALUES('%s', '%s');", data, buffer);
   db_exec(new, PGRES_COMMAND_OK);
   fetch_posts();
   free(new);
}

void write_input_to_file(char* buffer, size_t sz, char* data){
   char *new, *format;
   format = "<p><span style=\"color:#b0716f\"> %s:\
             </span><span>%s </span></p>";

   new = malloc(MAXLEN);
   sprintf(new, format, data, buffer);
   write_to_file(DATABASE_FILE, new, "a", GLOBAL.HTML_DIR);
   update_html();
   free(new);
}

char* from_bytes_to_string(unsigned char* bytes, size_t len){
   char* res = malloc(len * 2 + 1);
   if (res == NULL){
      error(__func__, "failed malloc");
      return NULL;
   }
   for (int i = 0; i < len; i++)
      sprintf(&res[i*2], "%02x", bytes[i]);
   res[len*2] = '\0';
   return res;
}
/************************************************/

void logger(const char* where, char* what){
#ifdef LOG_ON
   printf("[+] LOG %s: %s\n", where, what);
#endif
}

void errorl(const char* where, char* file, int line){
#ifdef LOG_ON
    printf("[-] ERROR: %s[%s:%d]: %s\n", where, file, line, strerror(errno));
#endif
   exit(-1);
}
void db_errorl(const char* where, char* file, int line){
#ifdef LOG_ON
    printf("[-] ERROR: %s[%s:%d]: %s\n", where, file, line, db_error());
#endif
   exit(-1);
}
void ssl_error(const char* where, char* file, int line){
#ifdef LOG_ON
   printf("[-] ERROR: %s[%s:%d]: %s\n", where, file, line, ERR_error_string(ERR_get_error(), NULL));
#endif
}
void error(const char* where, char* what){
#ifdef LOG_ON
   if (errno != 0){
      printf("[-] ERROR: %s: %s - %s\n", where, what, strerror(errno));
      exit(-1);
   } else
      printf("[-] INFO %s: %s\n", where, what);
#endif
}
