![](https://github.com/L0puh/chatter/blob/master/resources/images/logo.png)
### about
tiny web chat application in pure c

### current features:
- http server
- websocket server (based on [RFC 6455](https://www.rfc-editor.org/rfc/rfc6455))
- SSL support with openSSL for https and wss
- IPv6 support
- MIME support: html/htm, jpg/jpeg, png, css, ico

### install && run:
```shell
make . && ./server <HOST> <PORT>
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




