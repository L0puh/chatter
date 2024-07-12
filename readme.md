### CHATTER
tiny web chat application in pure c

### current features:
- http server
- websocket server (based on [RFC 6455](https://www.rfc-editor.org/rfc/rfc6455))
- SSL support with openSSL for https and wss

### install && run:
#### with SSL
1. generate private key:
```shell
openssl genrsa -out key.pem 2048
```
2. create a CSR
```shell
openssl req -new -key key.pem -out csr
```
3. create a self-signed certificate
```shell
openssl x509 -req -in csr -signkey key.pem -out cert.pem -days 365
```
#### run 
```shell
make . && ./server <PORT>
```  


