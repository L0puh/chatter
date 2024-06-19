#include "utils.h"
#include "web.h"

#include <stdio.h>
#include <stdlib.h>

char* get_file_content(char* filename){
   int cnt = 0;
   char ch, *string, name[32];
   string = (char*) malloc(MAXLEN);
   sprintf(name, "%s/%s", DIR, filename);
   FILE* file = fopen(name, "r");
   if (file == NULL) error(__func__, "open file");
   do {
      ch = fgetc(file);
      string[cnt++] = ch;
   } while (ch != EOF);
   string[cnt-1] = '\0';
   return string; 
}


void print_usage(int argc){
   if (argc != 2){
      printf("usage: ./server <PORT>\n");
      exit(0);
   }
}

void logger(const char* where, char* what){
   printf("[+] LOG %s: %s\n", where, what);
}
void error(const char* where, char* what){
   printf("[-] ERROR %s: %s\n", where, what);
}
