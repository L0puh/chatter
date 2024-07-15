#include "http.h"
#include "utils.h"
#include "websocket.h"

#include <ctype.h>
#include <limits.h>
#include <openssl/ssl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int init_server(char* host, char* port, struct addrinfo *servaddr){
   int enable = 1, err, sockfd;
   struct addrinfo hints; 

   bzero(&hints, sizeof(hints));
   hints.ai_flags = AI_PASSIVE;
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   
   if ((err = getaddrinfo(host, port, &hints, &servaddr)) != 0)
      error(__func__, (char*)gai_strerror(err));

   do {
      ASSERT((sockfd = socket(servaddr->ai_family, servaddr->ai_socktype, servaddr->ai_protocol)));
      ASSERT(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)));
      ASSERT((err = bind(sockfd, servaddr->ai_addr, servaddr->ai_addrlen)));
      if (err == 0) break;

      close(sockfd);
   } while ((servaddr->ai_next));

   ASSERT(listen(sockfd, QUERY));
   
   printf("[+] %s is running on port %s ", host, port);
   switch(servaddr->ai_family){
      case AF_INET:
         printf("[IPv4]\n");
         return sockfd;
      case AF_INET6:
         printf("[IPv6]\n");
         return sockfd;
   }
   return -1;
}

char* get_str_addr(struct sockaddr_in addr){
   size_t len = INET_ADDRSTRLEN;
   if (addr.sin_family == AF_INET6)
      len = INET6_ADDRSTRLEN;
   char* str_addr = malloc(len+1); 
   ASSERT(inet_ntop(addr.sin_family, &(addr.sin_addr), str_addr, len));
   str_addr[len] = '\0';
   return str_addr;
}


void recv_loop(int client_sockfd, char* buffer, int *bytes){
   while ((*bytes = recv(client_sockfd, 
                        buffer, MAXLEN, 0)) <= 0){}
   ASSERT(bytes);
}

int get_type_request(char* message, size_t sz){
   if (strstr(message, "GET") != NULL)
      return GET;
   if (strstr(message, "POST") != NULL)
      return POST;
   return NONE;
}

char* post_parse(const char* message, size_t sz, const char* symbol){
   char *ptr, *res;
   if ((ptr = strstr(message, symbol)) != NULL){
      return ptr;
   }
   return NULL;
}
char* header_parse(const char* message, size_t sz, const char* symbol){
   char* token; int cur = 0;
   char* res = malloc(sz * CHAR_BIT);
   char* copy_msg = malloc(sz * CHAR_BIT);
   strcpy(copy_msg, message);
   
   token = strtok(copy_msg, symbol); 

   while(token != NULL){
      token = strtok(NULL, " ");
      if (cur == 0){
         res = token;
         if (res == NULL) 
            res = "";
         return res;
      }
      cur++;
   }
   free(token); 
   return res;
}

void url_decode(char* str){
   char* p = str;
   while(*p){
      if (*p == '+'){
         *str++ = ' ';
         p++;
      } else if (*p == '%'){
         p++;
         if (isxdigit(*p)){
            if (isxdigit(p[1])){
               uint8_t x;
               sscanf(p, "%02hhx", &x);
               *str++=x;
               p+=2;
            } else { 
               *str++='%';
               p++;
            }
         } else p++;
      } else *str++ = *p++;
   }
   *str='\0';
}

char* set_cookie(char* param, char* value){
   char* cookie = malloc(MAXLEN);
   sprintf(cookie, "%s=%s;", param, value);
   return cookie;
}

char* cookies_parse(char *buffer, char* key){
   size_t len;
   char *p, *token, *res;
   if ((p = strstr(buffer, "Cookie:")) != NULL && (p = strstr(p, key)) != NULL){
      token = strtok(p, "=");
      token = strtok(NULL, "="); 
      token = strtok(token, "\r");
      len = sizeof(char) * strlen(token);
      res = malloc(len + 1);
      strcpy(res, token);
      res[len] = '\0';
      return res;
   }
   return NULL;
}

void send_response(user_t *user, request_t *req){
   char* format;
   char* result; 
   int bytes_sent, total;
   result = malloc(MAXLEN);
   if (req->code == 200){
      format = "HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n";
      sprintf(result, format, req->code, req->header, req->content_type, req->length);
   }
   else if (req->code == 301){
       format = "HTTP/1.1 %d %s\r\nLocation: %s\r\n\r\n";
       sprintf(result, format, req->code, req->header, req->location);
       logger(__func__, "Moved Permanently response");
   }
   else if (req->code == 101){
      result = ws_create_upgrade(req);
   }

   pthread_mutex_lock(&user->mutex);
   total = strlen(result), bytes_sent = 0;
   while (bytes_sent < total){
      if (user->is_ssl){
         SSL_ASSERT((bytes_sent += SSL_write(user->SSL_sockfd, result, strlen(result))));
      } else 
         ASSERT((bytes_sent = send(user->sockfd, result, strlen(result), 0)));
   }
   bytes_sent = 0, total = req->length;
   while(bytes_sent < total){
      if (user->is_ssl){
         SSL_ASSERT((bytes_sent += SSL_write(user->SSL_sockfd, req->content, req->length)));
      } else 
         ASSERT((bytes_sent = send(user->sockfd, req->content, req->length, 0)));
   }    
   pthread_mutex_unlock(&user->mutex);
   free(result);  
}

int recv_buffer(user_t *user, char *buffer, size_t buffer_size){
   int bytes;
   if (user->is_ssl && user->SSL_sockfd){
      bytes = SSL_read(user->SSL_sockfd, buffer, buffer_size);
      SSL_ASSERT(bytes);
   } else if (user->sockfd != -1){
      bytes = recv(user->sockfd, buffer, buffer_size, 0);
      ASSERT(bytes);
   }
   if (bytes > 0)
      buffer[bytes] = '\0';

   return bytes;
}
