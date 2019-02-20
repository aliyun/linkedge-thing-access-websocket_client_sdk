## 阿里云 IoT LinkEdge WebSocket设备接入 SDK 

### 介绍

本 SDK 根据阿里云IoT LinkEdge WebSocket client接入协议，屏蔽了协议的封装和解析细节，并整合了一款刚广泛应用的websocket库，方便开发者快速接入.

### 通讯协议

* [通讯协议完整版](protocol.md)

### 系统依赖

* [libwebsockets](https://github.com/warmcat/libwebsockets)
* 任何一种TLS库, 默认是[openssl](https://github.com/openssl/openssl), 其他可以是mbedTLS, wolfSSL, BoringSSL等.
* pthread 库.

### 编译指南

linux环境下

* 修改demo.c中g_devices中的pk,dn
* 执行编译命令: `make`
* 在sdk/export/lib目录会生成libleda.so, 在demo目录下生成demo可执行程序

### demo运行方法

linux环境下
    
    ./start_demo.sh [ip] <tls>
    
 
