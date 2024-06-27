#include "utils.h"
#include "web.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void update_html(){
   char* header = get_file_content(HEADER_PAGE, 0);
   char* sender = get_file_content(SENDER_PAGE, 0);
   char* text   = get_file_content(DATABASE_FILE, 0); 
   write_to_file(INDEX_PAGE, header, "w");
   if (strcmp(text, "") != 0)
      write_to_file(INDEX_PAGE, text,   "a");
   write_to_file(INDEX_PAGE, sender, "a");
}

void write_to_file(const char* filename, char* input, char* mode){
   char name[32];
   sprintf(name, "%s/%s", DIR, filename);
   FILE* f = fopen(name, mode);
   if (f == NULL) 
      error(__func__, "file open error");
   fprintf(f, "%s\n", input); 
   fclose(f);
}

char* get_file_content(const char* filename, size_t *init_size){
   FILE *file;
   char ch, *string, name[64]; 
   size_t size = MAXLEN, total_cnt = 0;

   if (init_size != 0 && init_size != NULL) 
      size = *init_size;

   if ((string = malloc(size * CHAR_BIT)) == NULL) {
      error(__func__, "failed allocation of string");
      return NULL;
   }

   bzero(string, strlen(string));
   sprintf(name, "%s/%s", DIR, filename);

   file = fopen(name, "r");
   if (file == NULL){
      error(__func__, name);
      return "";
   }

    while((ch = fgetc(file)) != EOF){
       string[total_cnt++] = ch;
       if (total_cnt >= size){
          size+=MAXLEN;
          if ((string = realloc(string, size * CHAR_BIT)) == NULL){
             error(__func__, "failed realloc");
             return NULL;
          }
          logger(__func__, "realloc new memory");
       }
    }

  
   if (init_size != 0 && init_size != NULL)
      *init_size = size;
   if (total_cnt < size) 
      string[total_cnt] = '\0';

   fclose(file);
   return string;
   
}


int is_contain(char* message, char symbol){
   for (int i = 0; i < strlen(message); i++){
      if (message[i] == symbol) return 1;
   }
   return 0;
}

void print_usage(int argc){
   if (argc != 2){
      printf("usage: ./server <PORT>\n");
      exit(0);
   }
}

void remove_prefix(char *str, const char* prefix){
   char* p;
   if ((p = strstr(str, prefix)) != NULL){
      p+=strlen(prefix);
      if (p) strcpy(str, p);
   } else {
      error(prefix, "prefix isn't found");
   }
}
/************************************************/

void logger(const char* where, char* what){
   printf("[+] LOG %s: %s\n", where, what);
}
void error(const char* where, char* what){
   printf("[-] ERROR %s: %s...", where, what);
   printf("%s\n", strerror(errno));
}
