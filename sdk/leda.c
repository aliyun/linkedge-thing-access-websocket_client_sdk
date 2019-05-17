#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <semaphore.h>

#include "libwebsockets.h"
#include "ws_client.h"
#include "log.h"
#include "base-utils.h"
#include "cJSON.h"
#include "os.h"
#include "threadpool.h"
#include "le_error.h"
#include "leda.h"


#define METHOD_ONLINE                       "onlineDevice"
#define METHOD_OFFLINE                      "offlineDevice"
#define EMTHOD_REPORT_PROPERTY              "reportProperty"
#define METHOD_REPORT_EVENT                 "reportEvent"
#define METHOD_CALL_SERVICE                  "callService"
#define METHOD_GET_PROPERTY                 "getProperty"
#define METHOD_SET_PROPERTY                 "setProperty"

#define CONN_PROTOCOL   "alibaba-iot-linkedge-protocol"
#define LOG_TAG "leda"
#define PROTOCOL_VERSION "v0.2"



typedef struct wsa_reply {
    struct list_head list_node;
    int msg_id;
    int code;
    sem_t sem;
    char *payload;
} wsa_reply_t;


typedef enum {
    MSG_INVALID = -1,
    MSG_RSP = 0,
    MSG_METHOD
} MSG_TYPE_E;

static ws_conn_cb_t g_conn_cb = {0};
static leda_device_callback_t g_devs_cb = {0}; 
extern struct lws_context *context;
extern volatile int force_exit;

LIST_HEAD(g_dev_list_head);
pthread_mutex_t g_dev_list_locker;
extern list_head_t g_dev_list_head;

static int g_is_init = 0;
static int g_conn_state = -1;
LIST_HEAD(wsa_replay_head);
pthread_mutex_t wsa_replay_lock;

static unsigned int     g_msg_id = 0;
static pthread_mutex_t  g_msg_locker = PTHREAD_MUTEX_INITIALIZER;
threadpool_t *g_threadpool_recv_proc = NULL;


static char *g_type_map[10] = {
    "int", //LEDA_TYPE_INT
    "bool", //LEDA_TYPE_BOOL
    "float", //LEDA_TYPE_FLOAT
    "text", //LEDA_TYPE_TEXT
    "date", //LEDA_TYPE_DATE
    "enum", //LEDA_TYPE_ENUM
    "struct", //LEDA_TYPE_STRUCT
    "array", //LEDA_TYPE_ARRAY
    "double", //LEDA_TYPE_DOUBLE
    "invalid" 
};

const char *type_number_to_string(int type)
{
    if (type < 0 || type > 8) {
        return g_type_map[9];
    }
    return g_type_map[type];
}

int type_string_to_number(char *type)
{
    if (0 == strcmp("int", type)) {
        return LEDA_TYPE_INT;
    } else if (0 == strcmp("bool", type)) {
        return LEDA_TYPE_BOOL;
    } else if (0 == strcmp("float", type)) {
        return LEDA_TYPE_FLOAT;
    } else if (0 == strcmp("text", type)) {
        return LEDA_TYPE_TEXT;
    } else if (0 == strcmp("date", type)) {
        return LEDA_TYPE_DATE;
    } else if (0 == strcmp("enum", type)) {
        return LEDA_TYPE_ENUM;
    } else if (0 == strcmp("struct", type)) {
        return LEDA_TYPE_STRUCT;
    } else if (0 == strcmp("array", type)) {
        return LEDA_TYPE_ARRAY;
    } else if (0 == strcmp("double", type)) {
        return LEDA_TYPE_DOUBLE;
    } else {
        return -1;
    }
    
}

cJSON *struct_data_to_json_data(const leda_device_data_t *struct_data, int data_cnt)
{
    int i;
    cJSON *root, *item, *sub_item;
    
    root = cJSON_CreateArray();
    if (!root) {
        return NULL;
    }

    for (i = 0; i < data_cnt; i++) {
        item = cJSON_CreateObject();
        if (!item) {
            cJSON_Delete(root);
            return NULL;
        }
        cJSON_AddStringToObject(item, "identifier", struct_data[i].key);
        cJSON_AddStringToObject(item, "type",type_number_to_string(struct_data[i].type));
        switch (struct_data[i].type) {
            case LEDA_TYPE_INT:
            case LEDA_TYPE_BOOL:
            case LEDA_TYPE_ENUM:
                cJSON_AddNumberToObject(item, "value", atoi(struct_data[i].value));
                break;
            case LEDA_TYPE_DOUBLE:
            case LEDA_TYPE_FLOAT:
                cJSON_AddNumberToObject(item, "value", atof(struct_data[i].value));
                break;
            case LEDA_TYPE_DATE:
            case LEDA_TYPE_TEXT:
                cJSON_AddStringToObject(item, "value", struct_data[i].value);
                break;
            case LEDA_TYPE_ARRAY:
            case LEDA_TYPE_STRUCT:
                sub_item = cJSON_Parse(struct_data[i].value);
                if (!sub_item) {
                    log_e(LOG_TAG, "value format is error!\n");
                    cJSON_Delete(root);
                    return NULL;
                }
                cJSON_AddItemToObject(item, "value", sub_item);
                break;
            default:
                cJSON_AddStringToObject(item, "value", "invalid");
                break;
        }
        cJSON_AddItemToArray(root, item);
    }

    return root;
}


int json_data_to_struct_data(cJSON *json_data, leda_device_data_t **struct_data, int *data_cnt)
{
    int size, i = 0;
    cJSON *item, *sub_item;
    char *buff;
    leda_device_data_t *data;
    

    size = cJSON_GetArraySize(json_data);
    if (size <= 0) {
        *data_cnt = 0;
        return LE_SUCCESS;
    }
    *data_cnt = size;
    data = malloc(sizeof(leda_device_data_t) * size);
    if (!data) {
        log_e(LOG_TAG, "no memory!");
        return LE_ERROR_ALLOCATING_MEM;
    }
    cJSON_ArrayForEach(item, json_data) {
        if (item->type == cJSON_String) {
            snprintf(data[i++].key, MAX_PARAM_NAME_LENGTH, "%s", item->valuestring);
            continue;
        }
        sub_item = cJSON_GetObjectItem(item, "identifier");
        if (!sub_item) {
            free(data);
            return LE_ERROR_INVAILD_PARAM;
        }
        snprintf(data[i].key, MAX_PARAM_NAME_LENGTH, "%s", sub_item->valuestring);
        sub_item = cJSON_GetObjectItem(item, "type");
        if (!sub_item | sub_item->type != cJSON_String) {
            free(data);
            return LE_ERROR_INVAILD_PARAM;
        }
        data[i].type = type_string_to_number(sub_item->valuestring);
        sub_item = cJSON_GetObjectItem(item, "value");
        if (!sub_item) {
            free(data);
            return LE_ERROR_INVAILD_PARAM;
        }
        switch (data[i].type) {
            case LEDA_TYPE_INT:
            case LEDA_TYPE_BOOL:
            case LEDA_TYPE_ENUM:
                snprintf(data[i].value, MAX_PARAM_VALUE_LENGTH, "%d", sub_item->valueint);
                break;
            case LEDA_TYPE_FLOAT:
            case LEDA_TYPE_DOUBLE:
                snprintf(data[i].value, MAX_PARAM_VALUE_LENGTH, "%lf", sub_item->valuedouble);
                break;
            case LEDA_TYPE_TEXT:
            case LEDA_TYPE_DATE:
                snprintf(data[i].value, MAX_PARAM_VALUE_LENGTH, "%s", sub_item->valuestring);
                break;
            case LEDA_TYPE_STRUCT:
            case LEDA_TYPE_ARRAY:
                buff = cJSON_Print(sub_item);
                if (!buff) {
                    log_e(LOG_TAG, "no memory!");
                    free(data);
                    return LE_ERROR_ALLOCATING_MEM;
                }
                snprintf(data[i].value, MAX_PARAM_VALUE_LENGTH, "%s", buff);
                free(buff);
                break;
            default:
                log_e(LOG_TAG, "invalid type for %s !\n", data[i].key);
                break;
        }
        ++i;
    }
    *struct_data = data;

    return LE_SUCCESS;
}


unsigned int get_msg_id()
{
    int ret = 0;

    pthread_mutex_lock(&g_msg_locker);
    ret = ++g_msg_id;
    pthread_mutex_unlock(&g_msg_locker);

    return ret;
}


static wsa_reply_t *_wsa_get_reply_by_msg_id(int msg_id)
{
    wsa_reply_t *pos, *next, *reply = NULL;

    pthread_mutex_lock(&wsa_replay_lock);
    list_for_each_entry_safe(pos, next, &wsa_replay_head, list_node) {
        if (pos->msg_id == msg_id) {
            reply = pos;
            break;
        }
    }
    pthread_mutex_unlock(&wsa_replay_lock);
    return reply;
}

int wsa_insert_reply(int msg_id)
{
    wsa_reply_t *reply;

    pthread_mutex_lock(&wsa_replay_lock);
    reply = (wsa_reply_t *)malloc(sizeof(wsa_reply_t));
    if (NULL == reply) {
        log_e(LOG_TAG, "no memory\r\n");
        pthread_mutex_unlock(&wsa_replay_lock);
        return LE_ERROR_ALLOCATING_MEM;
    }
    reply->msg_id = msg_id;
    reply->code = LE_ERROR_UNKNOWN;
    if (0 != sem_init(&reply->sem, 0, 0)) {
        free(reply);
        pthread_mutex_unlock(&wsa_replay_lock);
        log_e(LOG_TAG, "semphore init error!");
        return LE_ERROR_UNKNOWN;
    }
    list_add(&reply->list_node, &wsa_replay_head);
    pthread_mutex_unlock(&wsa_replay_lock);
    return LE_SUCCESS;
}

void wsa_remove_reply(int msg_id)
{
    wsa_reply_t *reply;

    reply = _wsa_get_reply_by_msg_id(msg_id);
    if (NULL == reply) {
        return;
    }

    pthread_mutex_lock(&wsa_replay_lock);
    if (reply->payload) {
        free(reply->payload);
        reply->payload = NULL;
    }
    sem_destroy(&reply->sem);
    list_del(&reply->list_node);
    free(reply);
    pthread_mutex_unlock(&wsa_replay_lock);
    return;
}

int wsa_set_reply_result(int msg_id, int code, char *payload)
{
    wsa_reply_t *reply;

    reply = _wsa_get_reply_by_msg_id(msg_id);
    if (NULL == reply) {
        return -1;
    }

    pthread_mutex_lock(&wsa_replay_lock);
    reply->code = code;
    if (payload) {
        reply->payload = strdup(payload);
    }
    pthread_mutex_unlock(&wsa_replay_lock);
    sem_post(&reply->sem);
    return 0;
}

int wsa_get_reply_result(int msg_id, int timeout_ms, int *code, char **payload)
{
    int time_count = 0, s;
    wsa_reply_t *reply;
    struct timespec ts = {0};

    reply = _wsa_get_reply_by_msg_id(msg_id);
    if (NULL == reply) {
        log_d(LOG_TAG, "get reply err:%d\r\n", msg_id);
        return LE_ERROR_INVAILD_PARAM;
    }
    if (-1 == clock_gettime(CLOCK_REALTIME, &ts)) {
        wsa_remove_reply(msg_id);
        return LE_ERROR_UNKNOWN;
    }
    ts.tv_nsec = ts.tv_nsec + (timeout_ms % 1000) * 1000000000;
    ts.tv_sec = ts.tv_sec + timeout_ms / 1000;
    while ((s = sem_timedwait(&reply->sem, &ts)) == -1 && errno == EINTR) {
        continue;
    }

    if (-1 == s) {
        wsa_remove_reply(msg_id);
        log_e(LOG_TAG, "get reply time_out:%s", strerror(errno));
        return LE_ERROR_TIMEOUT;
    }
    
    if (code) {
        *code = reply->code;
    }
    if (payload) {
        *payload = strdup(reply->payload);
    }
    wsa_remove_reply(msg_id);

    return LE_SUCCESS;
}


int leda_parse_received_msg(cJSON *root, int *msg_id, int *code, char **method, cJSON **payload)
{
    cJSON *item = NULL;

    item = cJSON_GetObjectItem(root, "messageId");
    if ((NULL == item) || (cJSON_Number != item->type)) {
        log_e(LOG_TAG, "msg id type is not integer\n");
        return MSG_INVALID;
    }
    *msg_id = item->valueint;
    *payload = cJSON_GetObjectItem(root, "payload");
    item = cJSON_GetObjectItem(root, "code");    
    if (item) {
        if (cJSON_Number != item->type) {
            log_e(LOG_TAG, "code type is not integer\n");
            return MSG_INVALID;
        }
        *code = item->valueint;
        return MSG_RSP;
    }
    item = cJSON_GetObjectItem(root, "method");
    if ((NULL == item) || (cJSON_String != item->type)) {
        return MSG_INVALID;
    }
    *method = item->valuestring;

    return MSG_METHOD;
}


int leda_send_method_sync(const char *pk, const char *dn, const char *method, const char *event_name,
                     const leda_device_data_t *data, int data_cnt)
{
    cJSON *root, *payload, *params, *item, *sub_item;
    unsigned int msg_id;
    char *msg;
    size_t len;
    int ret, code = 0, i;

    if (!pk || !dn || !method) {
        return LE_ERROR_INVAILD_PARAM;
    }

    msg_id = get_msg_id();

    root = cJSON_CreateObject();
    if (!root) {
        return LE_ERROR_ALLOCATING_MEM;
    }
    cJSON_AddStringToObject(root, "version", PROTOCOL_VERSION);
    cJSON_AddNumberToObject(root, "messageId", msg_id);
    cJSON_AddStringToObject(root, "method", method);
    payload = cJSON_CreateObject();
    if (!payload) {
        return LE_ERROR_ALLOCATING_MEM;
    }
    cJSON_AddItemToObject(root, "payload", payload);
    cJSON_AddStringToObject(payload, "productKey", pk);
    cJSON_AddStringToObject(payload, "deviceName", dn);

    if (0 == strcmp(method, METHOD_ONLINE) || 0 == strcmp(method, METHOD_OFFLINE)) {
        goto next_step;
    }
    params = struct_data_to_json_data(data, data_cnt);
    if (!params) {
        cJSON_Delete(root);
        goto next_step;
    }
    if (event_name) {
        cJSON_AddStringToObject(payload, "identifier", event_name);
        cJSON_AddItemToObject(payload, "outputData", params);
    } else {
        cJSON_AddItemToObject(payload, "properties", params);
    }

next_step:
    msg = cJSON_Print(root);
    cJSON_Delete(root);
    if (!msg) {
        return LE_ERROR_ALLOCATING_MEM;
    }
    len = strlen(msg);

    ret = wsa_insert_reply(msg_id);
    if (ret != LE_SUCCESS) {
        free(msg);
        return ret;
    }
    log_i(LOG_TAG, "send messege to server: %s", msg);
    ret = wsc_add_msg(msg, len, 0);
    free(msg);
    if (ret != LE_SUCCESS) {
        wsa_remove_reply(msg_id);
        return ret;
    }
    
    ret = wsa_get_reply_result(msg_id, 10000, &code, NULL);
    if (ret != LE_SUCCESS) {
        log_e(LOG_TAG, "reply code:%d\n", code);
        return ret;
    }

    return code;
}

int leda_send_method(const char *pk, const char *dn, const char *method, const char *event_name,
const leda_device_data_t *data, int data_cnt)
{
    cJSON *root, *payload, *params, *item, *sub_item;
    unsigned int msg_id;
    char *msg;
    size_t len;
    int ret, code = 0, i;
    
    if (!pk || !dn || !method) {
        log_e(LOG_TAG, "invalid param!");
        return -LE_ERROR_INVAILD_PARAM;
    }

    msg_id = get_msg_id();

    root = cJSON_CreateObject();
    if (!root) {
        return -LE_ERROR_ALLOCATING_MEM;
    }
    cJSON_AddStringToObject(root, "version", PROTOCOL_VERSION);
    cJSON_AddNumberToObject(root, "messageId", msg_id);
    cJSON_AddStringToObject(root, "method", method);
    payload = cJSON_CreateObject();
    if (!payload) {
        return -LE_ERROR_ALLOCATING_MEM;
    }
    cJSON_AddItemToObject(root, "payload", payload);
    cJSON_AddStringToObject(payload, "productKey", pk);
    cJSON_AddStringToObject(payload, "deviceName", dn);

    if (0 == strcmp(method, METHOD_ONLINE) || 0 == strcmp(method, METHOD_OFFLINE)) {
        goto next_step;
    }
    params = struct_data_to_json_data(data, data_cnt);
    if (!params) {
        cJSON_Delete(root);
        goto next_step;
    }
    if (event_name) {
        cJSON_AddStringToObject(payload, "identifier", event_name);
        cJSON_AddItemToObject(payload, "outputData", params);
    } else {
        cJSON_AddItemToObject(payload, "properties", params);
    }

next_step:
    msg = cJSON_Print(root);
    cJSON_Delete(root);
    if (!msg) {
        return -LE_ERROR_ALLOCATING_MEM;
    }
    len = strlen(msg);

    log_i(LOG_TAG, "send messege to server: %s", msg);
    ret = wsc_add_msg(msg, len, 0);
    free(msg);
    if (ret != LE_SUCCESS) {
        return -ret;
    }

    return msg_id;
}



int leda_send_rsp(int code, int msg_id, cJSON *payload)
{
    cJSON *root;
    char *msg;
    int ret;

    root = cJSON_CreateObject();
    if (!root) {
        return LE_ERROR_ALLOCATING_MEM;
    }
    cJSON_AddNumberToObject(root, "code", code);
    cJSON_AddNumberToObject(root, "messageId", msg_id);
    if (!payload) {
        payload  = cJSON_CreateObject();
        if (!payload) {
            cJSON_Delete(root);
            return LE_ERROR_ALLOCATING_MEM;
        }
    }
    cJSON_AddItemToObject(root, "payload", payload);

    msg = cJSON_Print(root);
    cJSON_Delete(root);
    if (!msg) {
        return LE_ERROR_ALLOCATING_MEM;
    }
    log_i(LOG_TAG, "send messege to server: %s", msg);
    ret = wsc_add_msg(msg, strlen(msg) + 1, 0);
    free(msg);

    return ret;
}

int leda_rsp_get_properties(char *pk, char *dn, int msg_id, leda_device_data_t *data, int data_cnt)
{
    int ret = LE_ERROR_UNKNOWN;
    cJSON *root, *payload, *properties;
    char *msg;

    payload = cJSON_CreateObject();
    if (!payload) {
        return LE_ERROR_ALLOCATING_MEM;
    }

    if (g_devs_cb.get_properties_cb) {
        ret = g_devs_cb.get_properties_cb(pk, dn, data, data_cnt, g_devs_cb.usr_data_get_property);
    } else {
        log_e(LOG_TAG, "callback get_properties_cb unregistered!\n");
    }
    
    if (ret == LE_SUCCESS) {
        properties = struct_data_to_json_data(data, data_cnt);
        cJSON_AddItemToObject(payload, "properties", properties);
    }

    return leda_send_rsp(ret, msg_id, payload);
}


int leda_rsp_set_properties(char *pk, char *dn, int msg_id, leda_device_data_t *data, int data_cnt)
{
    int ret = LE_ERROR_UNKNOWN;

    if (g_devs_cb.set_properties_cb) {
        ret = g_devs_cb.set_properties_cb(pk, dn, data, data_cnt, g_devs_cb.set_properties_cb);
    } else {
        log_e(LOG_TAG, "callback set_properties_cb unregistered!\n");
    }

    return leda_send_rsp(ret, msg_id, NULL);
}


int leda_rsp_call_service(char *pk, char *dn, int msg_id, const char *service_name, leda_device_data_t *input_params,
                          int params_cnt)
{
    int ret = LE_ERROR_UNKNOWN, i;
    leda_device_data_t *output_params = NULL;
    cJSON *payload = NULL, *params;

    output_params = malloc(sizeof(leda_device_data_t) * g_devs_cb.service_output_max_count);
    if (!output_params) {
        return LE_ERROR_ALLOCATING_MEM;
    }
    memset(output_params, 0, sizeof(leda_device_data_t) * g_devs_cb.service_output_max_count);
    if (g_devs_cb.call_service_cb) {
        ret = g_devs_cb.call_service_cb(pk, dn, service_name, input_params, params_cnt, output_params, g_devs_cb.usr_data_call_service);
    } else {
        log_e(LOG_TAG, "callback call_service_cb unregistered!\n");
    }

    if (ret != LE_SUCCESS) {
        goto end;
    }
    params_cnt = 0;
    for (i = 0; i < g_devs_cb.service_output_max_count; i++) {
        if (strlen(output_params[i].key) == 0) {
            break;
        }
        ++params_cnt;
    }
    payload = cJSON_CreateObject();
    if (!payload) {
        goto end;
    }
    params = struct_data_to_json_data(output_params, params_cnt);
    if (params) {
        cJSON_AddItemToObject(payload, "outputData", params);
    }

end:

    if (output_params) {
        free(output_params);
    }

    return leda_send_rsp(ret, msg_id, payload);
}

typedef struct {
    int msg_type;
    int msg_id;
    int code;
    char method[16];
    cJSON *payload;
} parsed_msg_t;


void threadpool_recv_proc(void *arg)
{
    parsed_msg_t *parsed_msg;
    char *payload_str = NULL, *pk, *dn;
    cJSON *item, *service_name;
    leda_device_data_t *data = NULL;
    int data_cnt = 0;

    if (!arg) {
        return;
    }
    parsed_msg = (parsed_msg_t *)arg;

    if (parsed_msg->msg_type == MSG_RSP) {
        payload_str = cJSON_Print(parsed_msg->payload);
        if (wsa_set_reply_result(parsed_msg->msg_id, parsed_msg->code, payload_str) < 0) {
            if (g_devs_cb.report_reply_cb) {
                g_devs_cb.report_reply_cb(parsed_msg->msg_id, parsed_msg->code, g_devs_cb.usr_data_report_reply);
            }
        }
        if (payload_str) {
            free(payload_str);
        }
        goto end;
    } else if (parsed_msg->msg_type == MSG_METHOD) {
        item = cJSON_GetObjectItem(parsed_msg->payload, "productKey");
        if (!item) {
            goto end;
        }
        pk = item->valuestring;
        item = cJSON_GetObjectItem(parsed_msg->payload, "deviceName");
        if (!item) {
            goto end;
        }
        dn = item->valuestring;
        item = cJSON_GetObjectItem(parsed_msg->payload, "properties");
        if (!item) {
            item = cJSON_GetObjectItem(parsed_msg->payload, "inputData");
        }
        if (!item) {
            goto end;
        }
        
        if (LE_SUCCESS != json_data_to_struct_data(item, &data, &data_cnt)) {
            goto end;
        }
        if (0 == strcmp(parsed_msg->method, METHOD_GET_PROPERTY)) {
            leda_rsp_get_properties(pk, dn, parsed_msg->msg_id, data, data_cnt);
        } else if (0 == strcmp(parsed_msg->method, METHOD_SET_PROPERTY)) {
            leda_rsp_set_properties(pk, dn, parsed_msg->msg_id, data, data_cnt);
        } else if (0 == strcmp(parsed_msg->method, METHOD_CALL_SERVICE)) {
            service_name = cJSON_GetObjectItem(parsed_msg->payload, "identifier");
            if (!service_name) {
                goto end;
            }
            leda_rsp_call_service(pk, dn, parsed_msg->msg_id, service_name->valuestring, data, data_cnt);
        } else {
            goto end;
        }
    } else {
        goto end;
    }

end:
    if (parsed_msg) {
        if (parsed_msg->payload) {
            cJSON_Delete(parsed_msg->payload);
        }
        free(parsed_msg);
    }
    if (data) {
        free(data);
    }
    return;
}

static void cb_ws_recv(const char *msg, size_t len, void *user)
{
    cJSON *root, *payload = NULL;
    int msg_type, msg_id = 0;
    char *method = NULL;
    parsed_msg_t *parsed_msg;
    
    if (!msg) {
        return;
    }
    log_i(LOG_TAG, "received msg: %s", msg);

    root = cJSON_Parse(msg);
    if (!root) {
        return;
    }
    parsed_msg = malloc(sizeof(parsed_msg_t));
    if (!parsed_msg) {
        cJSON_Delete(root);
        log_e(LOG_TAG, "no memory!\n");
        return;
    }
    memset(parsed_msg, 0, sizeof(parsed_msg_t));
    
    parsed_msg->msg_type = leda_parse_received_msg(root, &parsed_msg->msg_id, &parsed_msg->code, &method, &payload);
    if (method) {
        snprintf(parsed_msg->method, sizeof(parsed_msg->method), "%s", method);
    }
    if (payload) {
        parsed_msg->payload = cJSON_Duplicate(payload, 1);
        if (!parsed_msg->payload) {
            log_e(LOG_TAG, "no memory!");
            free(parsed_msg);
            cJSON_Delete(root);
            return;
        }
    }
    cJSON_Delete(root);

    threadpool_add(g_threadpool_recv_proc, threadpool_recv_proc, (void *)parsed_msg, 0);     

    return;
}


static void cb_ws_close(void *user)
{
    g_conn_state = LEDA_WS_DISCONNECTED;
    log_i(LOG_TAG, "connection closed.\n");

    if (g_conn_cb.conn_state_change_cb) {
        g_conn_cb.conn_state_change_cb(LEDA_WS_DISCONNECTED, g_conn_cb.usr_data);
    }
}

static void cb_ws_estab(void *user)
{
    g_conn_state = LEDA_WS_CONNECTED;
    log_i(LOG_TAG, "connection establish.\n");

    if (g_conn_cb.conn_state_change_cb) {
        g_conn_cb.conn_state_change_cb(LEDA_WS_CONNECTED, g_conn_cb.usr_data);
    }
}


int leda_online(const char *pk, const char *dn)
{
    return leda_send_method_sync(pk, dn, METHOD_ONLINE, NULL, NULL, 0);
}


int leda_offline(const char *pk, const char *dn)
{
    return leda_send_method_sync(pk, dn, METHOD_OFFLINE, NULL, NULL, 0);
}


int leda_report_properties(const char *pk, const char *dn, const leda_device_data_t properties[], int properties_count)
{
    return leda_send_method(pk, dn, EMTHOD_REPORT_PROPERTY, NULL, properties, properties_count);
}


int leda_report_event(const char *pk, const char *dn, const char *event_name, const leda_device_data_t data[],
                      int data_count)
{
    return leda_send_method(pk, dn, METHOD_REPORT_EVENT, event_name, data, data_count);
}


int leda_report_properties_sync(const char *pk, const char *dn, const leda_device_data_t properties[], int properties_count)
{
  return leda_send_method_sync(pk, dn, EMTHOD_REPORT_PROPERTY, NULL, properties, properties_count);
}


int leda_report_event_sync(const char *pk, const char *dn, const char *event_name, const leda_device_data_t data[],
                    int data_count)
{
  return leda_send_method_sync(pk, dn, METHOD_REPORT_EVENT, event_name, data, data_count);
}


int leda_wsc_init(const leda_conn_info_t *info)
{
    int ret = 0;
    int timeout;
    wsc_param_conn param_conn = {0};
    wsc_param_cb   param_cbs = {0};

    if (g_is_init == 1) {
        return 0;
    }

    g_threadpool_recv_proc = threadpool_create(5, 5*1024, 0);

    g_conn_cb.conn_state_change_cb = info->ws_conn_cb.conn_state_change_cb;
    g_conn_cb.usr_data = info->ws_conn_cb.usr_data;
    g_devs_cb.get_properties_cb = info->conn_devices_cb.get_properties_cb;
    g_devs_cb.usr_data_get_property = info->conn_devices_cb.usr_data_get_property;
    g_devs_cb.set_properties_cb = info->conn_devices_cb.set_properties_cb;
    g_devs_cb.usr_data_set_property = info->conn_devices_cb.usr_data_set_property;
    g_devs_cb.call_service_cb = info->conn_devices_cb.call_service_cb;
    g_devs_cb.usr_data_call_service = info->conn_devices_cb.usr_data_call_service;
    g_devs_cb.service_output_max_count = info->conn_devices_cb.service_output_max_count;
    
    g_conn_state = -1;
    param_conn.url = info->url;
    param_conn.ca_path = info->ca_path;
    param_conn.cert_path = info->cert_path;
    param_conn.key_path = info->key_path;

    param_conn.timeout = info->timeout;
    param_conn.protocol = CONN_PROTOCOL;

    param_cbs.p_cb_establish = cb_ws_estab;
    param_cbs.p_cb_close = cb_ws_close;
    param_cbs.p_cb_recv = cb_ws_recv;

    ret = wsc_init(&param_conn, &param_cbs);

    if (ret != 0) {
        return ret;
    }
    timeout = info->timeout;
    while (--timeout > 0) {
        os_sleep(1);
        if (g_conn_state == LEDA_WS_CONNECTED) {
            break;
        } else if (g_conn_state == LEDA_WS_DISCONNECTED) {
            return LE_ERROR_UNKNOWN;
        } else {
            log_i(LOG_TAG, "connecting to remote server\n");
        }
    }

    pthread_mutex_init(&wsa_replay_lock, NULL);

    if (g_conn_state < 0) {
        ws_client_destroy();
        return LE_ERROR_TIMEOUT;
    }
    
    g_is_init = 1;
    return LE_SUCCESS;
}

int leda_wsc_exit()
{
    g_is_init = 0;
    force_exit = 1;
    
    if (g_threadpool_recv_proc) {
        threadpool_destroy(g_threadpool_recv_proc, threadpool_graceful);
    }
    if (context) {
        lws_cancel_service(context);
        context = NULL;
    }

    return ws_client_destroy();

}


