#include "ssl.h"
#include "utils.h"
#include <openssl/ssl.h>

SSL_CTX* init_ssl(){

   SSL_CTX *sslctx;
   SSL_load_error_strings();
   SSL_library_init();
   OpenSSL_add_all_algorithms();

   SSL_ASSERT((sslctx = SSL_CTX_new(TLS_server_method())));
  
   SSL_CTX_set_verify(sslctx,SSL_VERIFY_NONE,NULL); 
   SSL_CTX_set_options(sslctx, SSL_OP_SINGLE_DH_USE);

   SSL_ASSERT(SSL_CTX_use_certificate_file(sslctx, "cert.pem", SSL_FILETYPE_PEM));
   SSL_ASSERT(SSL_CTX_use_PrivateKey_file(sslctx, "key.pem", SSL_FILETYPE_PEM));

   return sslctx;
}

SSL* create_ssl(int sockfd, SSL_CTX* sslctx){
   SSL *ssl_sockfd;

   if (!SSL_CTX_check_private_key(sslctx)){
      error(__func__, "key doesn't match certificate");
   }

   ssl_sockfd = SSL_new(sslctx);
   SSL_set_fd(ssl_sockfd, sockfd);
   SSL_ASSERT(SSL_accept(ssl_sockfd));

   return ssl_sockfd;
}
