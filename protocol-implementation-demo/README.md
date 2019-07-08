## `WebSocket设备接入协议参考实现版本更新注意事项`：

 <b>WebSocket设备接入协议参考实现在v1.0.0版本做了重大修改，不再兼容老的参考实现版本，即使用v1.0.0之前参考实现版本无法使用v1.0.0版本参考实现进行升级；</b>

## Link IoT Edge WebSocket接入协议实现参考

### 介绍

为了方便用户理解WebSocket接入协议，本文给出了一份WebSocket接入协议C语言实现作为协议理解参考。用户可以参考本文给的实现参考设计符合自己技术栈的WebSocket接入协议。

### 协议规范参考

* [通讯协议完整版](../protocol-design-description.md)

### 实现参考演示
`demo_led`示例演示使用WebSocket接入协议接入Link IoT Edge过程。

#### 依赖工具

本工程需要的编译工具版本保证和表格中列举版本一致或更高版本，否则将编译可能会失败

Tool           | Version |
---------------|---------|
gcc            | 4.8.5+  |
make           | 3.82+   |
ld             | 2.17+   |
cmake          | 3.11.1+ |
autoconf       | 2.69+   |
libtool        | 2.4.6+  |


#### 系统组件

* [libwebsockets](https://github.com/warmcat/libwebsockets)
* 任何一种TLS库, 默认是[openssl](https://github.com/openssl/openssl), 其他可以是mbedTLS, wolfSSL, BoringSSL
* zlib

Componet       | Version |
---------------|---------|
libwebsockets  | 3.1.0+  |
openssl        | 1.0.2p+  |
zlib           | 1.2.11+  |

#### Demo编译

``` sh
    $git clone https://github.com/aliyun/linkedge-thing-access-websocket_client_sdk.git

    $cd linkedge-thing-access-websocket_client_sdk/protocol-reference-implementation

    $make prepare              #预编译生成外部依赖库

    $make                      #生成sdk和demo
```

#### Demo演示

``` sh
    $cd demo/linux/

    $./start_demo.sh [$server_ip] [$server_port] <$use_tls>
```
<b>`说明`：$server_ip对应WebSocket驱动配置页配置的IP， $server_port对应WebSocket驱动配置页配置的Port， $use_tls是加密开关，填0（关闭）或1（打开），不填默认0，填1时需要将根证书放置在cert文件夹下，且名称为CA.cer</b>

1. 创建一个产品，名称为`demo_led`。该产品包含一个`power`属性（int32类型），一个`brightness`属性（int32类型）和一个`ledBroken`事件（无输出数据）。
2. 创建一个名为`demo_led`的上述产品的设备。
3. 创建一个新的分组，并将Link IoT Edge网关设备加入到分组。
4. 进入设备驱动配置页，添加`WebSocket`驱动（根据不同硬件平台进行选择）。
5. 配置`WebSocket`[驱动配置](https://help.aliyun.com/document_detail/122583.html)，填写驱动监听IP地址和Port端口，选择加密，进入步骤6，选择不加密，进入步骤7。
6. 上传公钥文件（后缀为.cer）和私钥文件（后缀为.pvk）。
7. 将`demo_led`设备分配到`WebSocket`驱动。
8. 进入消息路由页，使用如下配置添加*消息路由*：
  * 消息来源：`demo_led`设备
  * TopicFilter：属性
  * 消息目标：IoT Hub
9. 部署分组。`demo_led`设备将每隔5秒上报属性到云端，可在Link IoT Edge控制台设备运行状态页面查看。

<b>`注：` 根证书、公钥文件和私钥文件生成方法见[证书生成流程](demo/cert/README.md)。</b>
