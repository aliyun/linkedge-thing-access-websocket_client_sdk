# Link IoT Edge WebSocket接入协议

## 目录
* [简介](#简介)
* [消息体格式](#消息体格式)
* [设备上线](#设备上线)
* [设备下线](#设备下线)
* [上报属性](#上报属性)
* [上报事件](#上报事件)
* [获取属性](#获取属性)
* [设置属性](#设置属性)
* [方法调用](#方法调用)

## 简介

本文基于[阿里云物模型](https://help.aliyun.com/document_detail/73727.html?spm=a2c4g.11186623.3.4.4a01f9bcabQ3fn)及WebSocket通信方式，定义了一套JSON规范的设备接入Link IoT Edge协议，完成设备上/下线，上报属性，上报事件，获取属性，设置属性，方法调用等功能。协议约定设备侧作为client, 边缘网关作为server。

## 消息体格式
- 示例:

```
请求
{
    "version": "1.0",
    "method": "onlineDevice",
    "messageId": 0,
    "payload":{
        "productKey": "product_key",
        "deviceName": "device_name"
    }
}

应答
{
    "code": 0,
    "message": "Success",
    "messageId": 0,
    "payload": {}
}
```

- 参数名称及其解释

|名称 |类型 |取值 |描述 |
|:---: |--- |--- |--- |
|method |字符串 |"onlineDevice",<br>"offlineDevice",<br>"reportProperty",<br>"reportEvent",<br>"setProperty",<br>"getProperty",<br>"callService" |执行动作 |
|code |32位无符号整型 |[code取值](#Code取值) |请求结果返回码。|
|message |字符串 |- |与code对应的提示信息 |
|messageId |32位无符号整型 |-|消息编号，唯一标识这条消息，从1开始开始， 对端收到消息的反馈中携带相同id。如果收到对端反馈消息id为0，则表明请求端消息未携带该字段 |
|payload |json对象 |- |内容负载 |
|productKey |字符串|- |产品键值，云端创建产品时生成 |
|deviceName|字符串|- | 设备名称，云端创建设备时生成|
|properties|json数组 |[参考阿里云物模型](https://help.aliyun.com/document_detail/88250.html?spm=a2c4g.11186623.6.569.761af9bcpM8OJR) |设备属性 |
|identifier |字符串 |[参考阿里云物模型](https://help.aliyun.com/document_detail/88250.html?spm=a2c4g.11186623.6.569.761af9bcpM8OJR) |属性、事件、服务标识符 |
|inputData|json数组 |[参考阿里云物模型](https://help.aliyun.com/document_detail/88250.html?spm=a2c4g.11186623.6.569.761af9bcpM8OJR) |输入参数 |
|outputData|json数组 |[参考阿里云物模型](https://help.aliyun.com/document_detail/88250.html?spm=a2c4g.11186623.6.569.761af9bcpM8OJR) |输出参数 |
|type|字符串 |[参考阿里云物模型](https://help.aliyun.com/document_detail/88250.html?spm=a2c4g.11186623.6.569.761af9bcpM8OJR) |属性或参数的数据类型 |
|value|所有json数据类型 |- |属性或参数值 |

- Code值及其解释

| 取值 | 含义 |
| :--: | :-- |
|0 |请求成功 |
|100000 |不能被识别的错误，用户不应该看到的错误 |
|100001 |传入参数为NULL或无效 |
|100002 |指定的内存分配失败 |
|100003 |创建mutex失败 |
|100004 |写入文件失败 |
|100005 |读取文件失败 |
|100006 |超时 |
|100007 |参数范围越界 |
|100008 |服务不可达 |
|100009 |文件不存在 |
|109000 |设备未注册 |
|109001 |设备已下线 |
|109002 |属性不存在 |
|109003 |属性只读 |
|109004 |属性只写 |
|109005 |服务不存在 |
|109006 |服务的输入参数不正确错误码 |
|109007 |JSON格式错误 |
|109008 |参数类型错误 |

## 设备上线

- 消息方向： client->server
- 设备注册成功之后，可以执行设备上线命令，设备上线成功之后，才能被操作
- 消息体格式

```
请求
{
    "version": "1.0",
    "method": "onlineDevice",
    "messageId": 1,
    "payload": {
        "productKey": "product_key",
        "deviceName": "device_name"
    }
}

应答
{
    "code": 0,
    "message": "Success",
    "messageId": 1,
    "payload": {}
}
```

## 设备下线

- 消息方向：client->server
- 设备离线后，调用设备下线命令，通知边缘网关设备离线，设备重新上线后，无需再次注册，只需要上报设备上线即可

- 消息体格式

```
请求
{
    "version": "1.0",
    "method": "offlineDevice",
    "messageId": 2,
    "payload": {
        "productKey": "product_key",
        "deviceName": "device_name"  
    }
}

应答
{
    "code": 0,
    "message":"Success",
    "messageId": 2,
    "payload": {}
}
```

## 上报属性

- 消息方向： client->server
- 设备上线成功之后，上报全量设备属性，其他时候根据设备实际运行状况上报
- 消息体格式

```
请求
{
    "version": "1.0",
    "method": "reportProperty",
    "messageId": 0,
    "payload": {
        "productKey": "product_key",
        "deviceName": "device_name",  
        "properties": [
            {
                "identifier": "name_int",
                "type": "int",
                "value": 123
            }
        ]
    }
}

应答
{
    "code": 0,
    "message": "Success",
    "messageId": 0,
    "payload": {}
}
```
* 说明："name_int"为具体的设备属性名称，下文"name_bool"也是

## 上报事件

- 消息方向：client->server
- 事件触发时上报
- 消息体格式

```
请求
{
    "version": "1.0",
    "method": "reportEvent",
    "messageId": 1,
    "payload": {
        "productKey": "product_key",
        "deviceName": "device_name",
        "identifier": "event_name",
        "outputData": [
            {
                "identifier": "name_int",
                "type": "int",
                "value": 123
            }
        ]
    }
}

应答
{
    "code": 0,
    "message": "Success",
    "messageId": 1,
    "payload": {}
}
```

## 获取属性

- 消息方向：server->client
- 边缘网关获取设备属性
- 消息体格式

```
请求
{
    "version": "1.0",
    "method": "getProperty",
    "messageId": 3,
    "payload": {
        "productKey": "product_key",
        "deviceName": "device_name",
        "properties": ["name_int", "name_bool"]
    }
}

应答
{
    "code": 0,
    "messageId": 3,
    "payload": {
        "properties": [
            {
                "identifier": "name_int",
                "type": "int",
                "value": 123
            },
            {
                "identifier": "name_bool",
                "type": "bool",
                "value": 0
            }        
        ]
    }
}
```

## 设置属性

- 消息方向：server->client
- 边缘网关对设备属性设置
- 消息体格式

```
请求
{
    "version": "1.0",
    "method": "setProperty",
    "messageId": 2,
    "payload": {
        "productKey": "product_key",
        "deviceName": "device_name",
        "properties": [
            {
                "identifier": "name_int",
                "type": "int",
                "value": 123
            },
            {
                "identifier": "name_bool",
                "type": "bool",
                "value": 0
            }        
        ]
    }
}

应答
{
    "code": 0,
    "messageId": 2,
    "payload": {}
}
```

## 方法调用

- 消息方向：server->client
- 边缘网关操作设备方法
- 消息体格式

```
请求
{
    "version": "1.0",
    "method": "callService",
    "messageId": 4,
    "payload": {
        "productKey": "product_key",
        "deviceName": "device_name",
        "identifier": "service_name",
        "inputData": [
            {
                "identifier": "name_int",
                "type": "int",
                "value": 123
            }
        ]
    }
}

应答
{
    "code": 0,
    "messageId": 4,
    "payload": {
        "outputData": []
    }
}
```
