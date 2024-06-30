#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

const char base64_table[] = """ABCDEFGHIJKLMNOPQRSTUVWXYZ"""
                            """abcdefghijklmnopqrstuvwxyz0123456789+/";

char* base64_encode(const unsigned char* message, size_t len){
   char* out;
   size_t enc_len, i, j, v;
   
   enc_len = base64_encode_size(len);
   out = malloc(enc_len+1);
   out[enc_len] = '\0';

   for (i=0, j=0; i < len; i+=3, j+=4){
      v = message[i];

      v <<= 8;
      if (i+1 < len) v |= message[i+1];
      v <<= 8;
      if (i+2 < len) v |= message[i+2];
     
      out[j]   = base64_table[(v >> 18) & 0x3f];
      out[j+1] = base64_table[(v >> 12) & 0x3f];

      out[j+2] = '=';
      out[j+3] = '=';
      
      if (i+1 < len) 
         out[j+2] = base64_table[(v>>6) & 0x3f];

      if (i+2 < len) 
         out[j+3] = base64_table[v & 0x3f];
   }
   return out;
}

size_t base64_encode_size(size_t strlen){
   if (strlen % 3 == 0) return (strlen/3)*4;
   return ((strlen + 3 - (strlen % 3))/3) * 4;
}
