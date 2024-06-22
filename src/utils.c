#include "utils.h"
#include "web.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void write_html(){
   size_t sz_header, sz_sender, sz_text;
   sz_header = sz_sender = sz_text = MAXLEN;
   char* header = get_file_content("header.html", &sz_header);
   char* sender = get_file_content("sender.html", &sz_sender);
   char* text   = get_file_content("text.txt", &sz_text); //FIXME
   write_to_file("index.html", header, "w");
   write_to_file("index.html", text, "a");
   write_to_file("index.html", sender, "a");
}

void write_to_file(char* filename, char* input, char* mode){
   char name[32];
   sprintf(name, "%s/%s", DIR, filename);
   FILE* f = fopen(name, mode);
   if (f == NULL) 
      error(__func__, "file open error");
   fprintf(f, "%s\n", input); 
   fclose(f);
}

char* get_file_content(char* filename, size_t *sz){
   /* FIXME: reallocate stuff properly */
   int cnt = 0;
   size_t size = *sz;
   char ch, *string, name[64];
   
   string = (char*) malloc(size * CHAR_BIT);
   sprintf(name, "%s/%s", DIR, filename);
   FILE* file = fopen(name, "r");
   if (file == NULL) 
      error(__func__, "open file");
   do {
      ch = fgetc(file);
      string[cnt++] = ch;
      if (cnt >= size){
         size*=2;
         if (realloc(string, size) == NULL) 
            error(__func__, "realloc error");
         logger(__func__, "allocate more memory");
      }
   } while (ch != EOF);
   string[cnt-1] = '\0';
   *sz = size;
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

char* remove_prefix(char* msg, const char* x){
   int cnt = 0;
   char* prefix = malloc(strlen(msg));
   while (strcmp(prefix, x) != 0){
      if (msg[cnt] == ' ') {
         logger(__func__, "prefix isn't found");
         return NULL;
      }
      prefix[cnt] = msg[cnt];
      cnt++;
   }
   char* res = malloc(strlen(msg) - cnt);
   for(int i = cnt; i < strlen(msg); i++){
      res[i-cnt] = msg[i];
   }
   return res;
}

/************************************************/

void logger(const char* where, char* what){
   printf("[+] LOG %s: %s\n", where, what);
}
void error(const char* where, char* what){
   printf("[-] ERROR %s: %s...", where, what);
   printf("%s\n", strerror(errno));
}
