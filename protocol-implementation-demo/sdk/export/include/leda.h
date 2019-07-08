/*
 * Copyright (c) 2014-2019 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef __LINKEDGE_DEVICE_ACCESS__
#define __LINKEDGE_DEVICE_ACCESS__

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif


#define MAX_PARAM_NAME_LENGTH                   64                  /* 属性或事件名的最大长度*/
#define MAX_PARAM_VALUE_LENGTH                  2048                /* 属性值或事件参数的最大长度*/


typedef enum leda_conn_state
{
    LEDA_WS_CONNECTED = 0,
    LEDA_WS_DISCONNECTED
} leda_conn_state_e;

typedef void (*conn_state_change_callback)(leda_conn_state_e state, void *usr_data);

typedef struct ws_conn_cb
{
    /* 连接状态的回调函数
     * 1. 连接成功, 上线设备
     * 2. 连接断开, 下线设备待重新连接成功再上线
    */
    conn_state_change_callback conn_state_change_cb;

    /*连接状态变更回调函数的用户数据*/
    void *usr_data;
} ws_conn_cb_t;

typedef enum leda_data_type
{
    LEDA_TYPE_INT = 0,                                              /* 整型 */
    LEDA_TYPE_BOOL,                                                 /* 布尔型 对应值为0 or 1*/
    LEDA_TYPE_FLOAT,                                                /* 浮点型 */
    LEDA_TYPE_TEXT,                                                 /* 字符串型 */
    LEDA_TYPE_DATE,                                                 /* 日期型 */
    LEDA_TYPE_ENUM,                                                 /* 枚举型 */
    LEDA_TYPE_STRUCT,                                               /* 结构型 */
    LEDA_TYPE_ARRAY,                                                /* 数组型 */
    LEDA_TYPE_DOUBLE,                                               /* 双精浮点型 */

    LEDA_TYPE_BUTT
} leda_data_type_e;

typedef struct leda_device_data
{
    leda_data_type_e    type;                                       /* 值类型, 需要跟设备 物模型 中保持一致 */  
    char                key[MAX_PARAM_NAME_LENGTH];                 /* 属性或事件名 */
    char                value[MAX_PARAM_VALUE_LENGTH];              /* 属性值 */
} leda_device_data_t;

/*
 * 获取属性的回调函数, LinkEdge 需要获取某个设备的属性时, SDK 会调用该接口间接获取到数据并封装成固定格式后回传给 LinkEdge.
 * 开发者需要根据设备id和属性名找到设备, 将属性值获取并以@device_data_t格式返回.
 *
 * @product_key:                 LinkEdge 需要获取属性的具体某个设备所属的ProductKey.
 * @device_name:                 LinkEdge 需要获取属性的具体某个设备的DeviceName.
 * @properties:         开发者需要将属性值更新到properties中.
 * @properties_count:   属性个数.
 * @usr_data:           客户端初始化时, 用户传递的私有数据.
 * 所有属性均获取成功则返回LE_SUCCESS, 其他则返回错误码(参考错误码宏定义).
 * 
 */
typedef int (*get_properties_callback)(const char *product_key,
                                       const char *device_name,
                                       leda_device_data_t properties[],
                                       int properties_count,
                                       void *usr_data);


/*
 * 设置属性的回调函数, LinkEdge 需要设置某个设备的属性时, SDK 会调用该接口将具体的属性值传递给应用程序, 开发者需要在本回调
 * 函数里将属性设置到设备.
 *
 * @product_key:                 LinkEdge 需要设置属性的具体某个设备所属的ProductKey.
 * @device_name:                 LinkEdge 需要设置属性的具体某个设备的DeviceName.
 * @properties:         LinkEdge 需要设置的设备的属性名和值.
 * @properties_count:   属性个数.
 * @usr_data:           客户端初始化时, 用户传递的私有数据.
 * 
 * 若获取成功则返回LE_SUCCESS, 失败则返回错误码(参考错误码宏定义).
 * 
 */
typedef int (*set_properties_callback)(const char *product_key,
                                       const char *device_name,
                                       const leda_device_data_t properties[],
                                       int properties_count,
                                       void *usr_data);


/*
 * 服务调用的回调函数, LinkEdge 需要调用某个设备的服务时, SDK 会调用该接口将具体的服务参数传递给应用程序, 开发者需要在本回调
 * 函数里调用具体的服务, 并将服务返回值按照设备 物模型 里指定的格式返回. 
 *
 * @product_key:           LinkEdge 需要调用服务的具体某个设备所属的ProductKey.
 * @device_name:           LinkEdge 需要调用服务的具体某个设备的DeviceName.
 * @service_name: LinkEdge 需要调用的设备的具体某个服务名.
 * @data:         LinkEdge 需要调用的设备的具体某个服务参数, 参数与设备 物模型 中保持一致.
 * @data_count:   LinkEdge 需要调用的设备的具体某个服务参数个数.
 * @output_data:  开发者需要将服务调用的返回值, 按照设备 物模型 中规定的服务格式返回到output中.
 * @usr_data:     客户端初始化时, 用户传递的私有数据.
 * 
 * 若获取成功则返回LE_SUCCESS, 失败则返回错误码(参考错误码宏定义).
 * */
typedef int (*call_service_callback)(const char *product_key,
                                     const char *device_name,
                                     const char *service_name,
                                     const leda_device_data_t data[],
                                     int data_count,
                                     leda_device_data_t output_data[],
                                     void *usr_data);

/*
 * 上报属性及事件的应答回调函数
 * 上报消息发送成功立马返回, 如果需要上报消息响应值, 需要注册该接口, msg_id对应上报接口的msg_id
 * 
 * @msg_id      消息id
 * @code        上报结果返回码
 * @usr_data    客户端初始化时, 用户传递的私有数据.
 * */
typedef int (*report_reply_callback)(unsigned int msg_id, int code, void *usr_data);

/*
 * 设备回调函数group
*/
typedef struct leda_device_callback
{
    get_properties_callback     get_properties_cb;          /* 设备属性获取回调 */
    void *usr_data_get_property;                            /* 获取属性回调函数的用户私有数据, 在接口被调用时, 该数据会传递过去 */

    set_properties_callback     set_properties_cb;          /* 设备属性设置回调 */
    void *usr_data_set_property;                            /* 设置属性回调函数的用户私有数据, 在接口被调用时, 该数据会传递过去*/

    call_service_callback       call_service_cb;            /* 设备服务回调 */
    int                         service_output_max_count;   /* 设备服务回调结果数组最大长度 */   
    void *usr_data_call_service;                            /* 服务回调函数的用户私有数据, 在接口被调用时, 该数据会传递过去*/

    report_reply_callback       report_reply_cb;            /* 异步上报属性及事件的应答回调函数*/
    void *usr_data_report_reply;                            /* 异步上报属性及事件的应答回调函数的私有数据，在接口被调用时，该数据会传递过去*/
} leda_device_callback_t;


typedef struct leda_conn_info
{
    const char                  *server_ip;         /* WebSocket驱动监听地址 */
    unsigned short int          server_port;        /* WebSocket驱动监听端口 */
    int                         use_tls;            /* 是否使用tls加密, 0不使用, 1使用 */

    const char                  *ca_path;           /* 根证书绝对路径 */
    const char                  *cert_path;         /* 公钥证书绝对路径 */
    const char                  *key_path;          /* 私钥证书绝对路径 */
    int                         timeout;            /* 连接超时时间，单位为秒. 如果设备和Linkedge之间, 在timeout时间内没有数据传输, 连接会被重置 */

    ws_conn_cb_t                ws_conn_cb;         /*websocket连接变更回调*/

    leda_device_callback_t      conn_devices_cb;    /*连接下所有设备的回调函数*/
} leda_conn_info_t;


/*
 * 初始化.
 *
 * @info: 连接信息.
 * 阻塞接口, 成功返回LE_SUCCESS, 失败返回错误码.
 */
int leda_init(const leda_conn_info_t *info);

/*
 * 退出.
 *
 * 退出前, 释放资源.
 *
 * 阻塞接口.
 */
void leda_exit(void);

/*
 * 上线设备, 设备只有上线后, 才能被 LinkEdge 识别.
 *
 * @product_key:          设备ProductKey.
 * @device_name:          设备DeviceName.
 *
 * 阻塞接口,成功返回LE_SUCCESS, 失败返回错误码.
 */
int leda_online(const char *product_key, const char *device_name);

/*
 * 下线设备, 假如设备工作在不正常的状态或设备退出前, 可以先下线设备, 这样LinkEdge就不会继续下发消息到设备侧.
 *
 * @product_key:          设备ProductKey.
 * @device_name:          设备DeviceName.
 *
 * 阻塞接口, 成功返回LE_SUCCESS,  失败返回错误码.
 *
 */
int leda_offline(const char *product_key, const char *device_name);

/*
 * 上报事件, 设备具有的事件上报能力在设备 物模型 里有约定.
 *
 * 
 * @product_key:          设备ProductKey.
 * @device_name:          设备DeviceName.
 * @event_name:  事件名称.
 * @data:        @leda_device_data_t, 事件参数数组.
 * @data_count:  事件参数数组长度.
 * @msg_id:      请求消息id, 消息发送成功后会生成消息id, 如果关心请求消息响应结果, 则保存该值, 否则只需要置空NULL即可
 *
 * 非阻塞接口, 等待服务端返回, 成功返回LE_SUCCESS,  失败返回错误码.
 *
 */
int leda_report_event(const char *product_key, const char *device_name, const char *event_name, const leda_device_data_t data[], int data_count, unsigned int *msg_id);

/*
 * 上报属性, 设备具有的属性在设备能力描述 物模型 里有规定.
 *
 * 上报属性, 可以上报一个, 也可以多个一起上报.
 *
 * @product_key:          设备ProductKey.
 * @device_name:          设备DeviceName.
 * @properties:           @leda_device_data_t, 属性数组.
 * @properties_count:     本次上报属性个数.
 * @msg_id:               请求消息id, 消息发送成功后会生成消息id, 如果关心请求消息响应结果, 则保存该值, 否则只需要置空NULL即可
 *
 * 非阻塞接口, 发送成功即返回, 成功返回LE_SUCCESS,  失败返回错误码.
 *
 */
int leda_report_properties(const char *product_key, const char *device_name, const leda_device_data_t properties[], int properties_count, unsigned int *msg_id);


#ifdef __cplusplus  /* If this is a C++ compiler, use C linkage */
}
#endif
#endif
