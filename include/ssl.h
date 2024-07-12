#ifndef SSL_H
#define SSL_H

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>

SSL_CTX* init_ssl();
SSL* create_ssl(int sockfd, SSL_CTX* sslctx);

#endif
