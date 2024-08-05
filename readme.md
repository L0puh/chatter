![](https://github.com/L0puh/chatter/blob/master/resources/images/logo.png)
### about
tiny web chat application in pure c

### current features:
- http server
- websocket server (based on [RFC 6455](https://www.rfc-editor.org/rfc/rfc6455))
- SSL support with openSSL for https and wss
- IPv6 support
- MIME support: html/htm, jpg/jpeg, png, css, ico
- PostgreSQL support (with `postgresql` library) 
- hashing and salting of passwords

### install && run:
```shell
make WITH_DB=0 LOG_ON=1 DEBUG=0&& ./server <HOST> <PORT> <FLAGS> 
```
<details><summary><b>with SSL</b></summary>
  
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
4. use `--SSL` option to enable https and wss

</details>
<details><summary><b>with PostgreSQL</b></summary>

1. install `postgresql` lib. 
2. create `conninfo` file and fill in connection information 
3. compile with `WITH_DB=1` 

</details>
