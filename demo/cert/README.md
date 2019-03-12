# 证书生成方法及使用说明

## 证书生成步骤
### 1、创建CA证书配置CA.cnf文件

```
[ req ]
distinguished_name  = req_distinguished_name
x509_extensions     = root_ca
 
[ req_distinguished_name ]
countryName                     = Country Name (2 letter code)
countryName_default             = CN
countryName_min                 = 2
countryName_max                 = 2
stateOrProvinceName             = State or Province Name (full name)
stateOrProvinceName_default     = ZheJiang
localityName                    = Locality Name (eg, city)
localityName_default            = HangZhou
0.organizationName              = Organization Name (eg, company)
0.organizationName_default      = Alibaba
organizationalUnitName          = Organizational Unit Name (eg, section)
organizationalUnitName_default  = Alibaba Cloud
commonName                      = Common Name (eg, fully qualified host name)
commonName_default              = Certification Authority
commonName_max                  = 64
emailAddress                    = Email Address
emailAddress_default            = CA@dev.com
emailAddress_max                = 64
 
[ root_ca ]
basicConstraints            = critical, CA:true
```
### 2、创建ssl证书cert.cnf文件

```
distinguished_name  = req_distinguished_name
 
[ req_distinguished_name ]
countryName                     = Country Name (2 letter code)
countryName_default             = CN
countryName_min                 = 2
countryName_max                 = 2
stateOrProvinceName             = State or Province Name (full name)
stateOrProvinceName_default     = ZheJiang
localityName                    = Locality Name (eg, city)
localityName_default            = HangZhou
0.organizationName              = Organization Name (eg, company)
0.organizationName_default      = Alibaba
organizationalUnitName          = Organizational Unit Name (eg, section)
organizationalUnitName_default  = Alibaba Cloud IOT
commonName                      = Common Name (eg, fully qualified host name)
commonName_default              = Certificate
commonName_max                  = 64
emailAddress                    = Email Address
emailAddress_default            = server@dev.com
emailAddress_max                = 64
```
### 3、创建ssl证书subjectName描述文件cert.ext

```
subjectAltName = @alt_names
extendedKeyUsage = serverAuth
 
[alt_names]
DNS.1 = localhost
DNS.2 = 127.0.0.1
```
### 4、创建CA+SSL证书

```
生成CA 证书
openssl req -x509 -newkey rsa:4096 -out CA.cer -outform PEM -keyout CA.pvk -days 3650 -verbose -config CA.cnf -nodes -sha256
 
生成证书请求文件
openssl req -newkey rsa:4096 -keyout cert.pvk -out cert.req -config cert.cnf -sha256 -nodes
 
生成证书
openssl x509 -req -CA CA.cer -CAkey CA.pvk -in cert.req -out cert.cer -days 3650 -extfile cert.ext -sha256 -set_serial 0x1111
```
## 使用说明
使用此方法生成ca证书CA.cer，公钥证书cert.cer，私钥证书cert.pvk，在需验证Websocket服务端(即Websocket驱动端)的情况下，需要将cert.cer及cert.pvk放在服务端，
将CA.cer放在客户端。

### 如何打开websocket的tls加密
在子设备的驱动配置中添加websocket自定义驱动,并添加自定义配置：
```
{"tls": 1}
```
