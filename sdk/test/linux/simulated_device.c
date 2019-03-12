#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/prctl.h>

#include "leda.h"
#include "le_error.h"

#define PROPERTY_INT32_RW   "int32_rw"
#define PROPERTY_INT32_R    "int32_r"
#define PROPERTY_FLOAT_RW   "float_rw"
#define PROPERTY_FLOAT_R    "float_r"
#define PROPERTY_DOUBLE_RW  "double_rw"
#define PROPERTY_DOUBLE_R   "double_r"
#define PROPERTY_ENUM_RW    "enum_rw"
#define PROPERTY_ENUM_R     "enum_r"
#define PROPERTY_BOOL_RW    "bool_rw"
#define PROPERTY_BOOL_R     "bool_r"
#define PROPERTY_STRING_RW  "string_rw"
#define PROPERTY_STRING_R   "string_r"
#define PROPERTY_DATE_RW    "date_rw"
#define PROPERTY_DATE_R     "date_r"
#define PROPERTY_INVALID    "invalid"

typedef struct {
    int property_enum;
    char property_string[16];
}property_string_enum_map_t;


enum enum_property {
    E_PROPERTY_INT32_RW = 0,   
    E_PROPERTY_INT32_R,
    E_PROPERTY_FLOAT_RW,  
    E_PROPERTY_FLOAT_R,    
    E_PROPERTY_DOUBLE_RW,  
    E_PROPERTY_DOUBLE_R,   
    E_PROPERTY_ENUM_RW,  
    E_PROPERTY_ENUM_R,   
    E_PROPERTY_BOOL_RW,    
    E_PROPERTY_BOOL_R,   
    E_PROPERTY_STRING_RW,
    E_PROPERTY_STRING_R, 
    E_PROPERTY_DATE_RW,  
    E_PROPERTY_DATE_R,

    E_PROPERTY_INVALID
};


#define SERVICE_WRITE "service_write"

#define EVENT_INT32 "event_int32"
#define EVENT_FLOAT "event_float"
#define EVENT_STRING "event_string"


typedef enum {
    ENUM_VALUE0 = 0,
    ENUM_VALUE1,
    ENUM_VALUE2,
    ENUM_VALUE3,
    ENUM_VALUE4
} enum_e;

typedef struct {
    char pk[16];
    char dn[32];
    int online;
    int int32_rw;
    int int32_r;
    float float_rw;
    float float_r;
    double double_rw;
    double double_r;
    enum_e enum_rw;
    enum_e enum_r;
    bool bool_rw;
    bool bool_r;
    char string_rw[2048];
    char string_r[2048];
    char date_rw[32];
    char date_r[32];
} device_t;


device_t *g_devices = NULL;
int g_devices_count = 0;

property_string_enum_map_t g_property_string_enum_map[15] = {
    {E_PROPERTY_INT32_RW, PROPERTY_INT32_RW},
    {E_PROPERTY_INT32_R, PROPERTY_INT32_R},
    {E_PROPERTY_FLOAT_RW, PROPERTY_FLOAT_RW}, 
    {E_PROPERTY_FLOAT_R, PROPERTY_FLOAT_R},
    {E_PROPERTY_DOUBLE_RW, PROPERTY_DOUBLE_RW},
    {E_PROPERTY_DOUBLE_R, PROPERTY_DOUBLE_R}, 
    {E_PROPERTY_ENUM_RW, PROPERTY_ENUM_RW},
    {E_PROPERTY_ENUM_R, PROPERTY_DOUBLE_R},
    {E_PROPERTY_BOOL_RW, PROPERTY_BOOL_RW},
    {E_PROPERTY_BOOL_R, PROPERTY_BOOL_R},
    {E_PROPERTY_STRING_RW, PROPERTY_STRING_RW},
    {E_PROPERTY_STRING_R, PROPERTY_STRING_R},
    {E_PROPERTY_DATE_RW, PROPERTY_DATE_RW},
    {E_PROPERTY_DATE_R, PROPERTY_DATE_R},
    {E_PROPERTY_INVALID, PROPERTY_INVALID}
};


pthread_t g_thread_property_monitor;
pthread_t g_thread_online;
pthread_t g_thread_report;

pthread_mutex_t g_property_monitor_locker = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_mtx_online = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_cond_property_changed = PTHREAD_COND_INITIALIZER;
pthread_cond_t g_cond_conn_state_changed = PTHREAD_COND_INITIALIZER;

char g_property_changed_pk[16] = {0};
char g_property_changed_dn[32] = {0};

int g_is_connected = 0;
int g_running = 1;


int get_device_index(const char *pk, const char *dn)
{
    int i;
    
    for (i = 0; i < g_devices_count; i++) {
        if (strcmp(g_devices[i].dn, dn) == 0) {
            return i;
        }
    }
}


int get_dev_int32_rw(const char *pk,  const char *dn)
{
    int index;
    index = get_device_index(pk, dn);
    return g_devices[index].int32_rw;
}


int set_dev_int32_rw(const char *pk, const char *dn, int value)
{
    int index;
    
    if (value < -1000 || value > 1000) {
        return LE_ERROR_PARAM_RANGE_OVERFLOW;
    }

    index = get_device_index(pk, dn);
    g_devices[index].int32_rw = value;
    return LE_SUCCESS;
}


int get_dev_int32_r(const char *pk,  const char *dn)
{
    int index;
    index = get_device_index(pk, dn);
    return g_devices[index].int32_r;
}


float get_dev_float_rw(const char *pk,  const char *dn)
{
    int index;
    index = get_device_index(pk, dn);
    return g_devices[index].float_rw;
}


int set_dev_float_rw(const char *pk, const char *dn, float value)
{
    int index;
    
    if (value < -1000 || value > 1000) {
        return LE_ERROR_PARAM_RANGE_OVERFLOW;
    }

    index = get_device_index(pk, dn);
    g_devices[index].float_rw = value;
    return LE_SUCCESS;
}


float get_dev_float_r(const char *pk,  const char *dn)
{
    int index;
    index = get_device_index(pk, dn);
    return g_devices[index].float_r;
}


double get_dev_double_rw(const char *pk,  const char *dn)
{
    int index;
    index = get_device_index(pk, dn);
    return g_devices[index].double_rw;
}


int set_dev_double_rw(const char *pk, const char *dn, double value)
{
    int index;
    
    if (value < -1000 || value > 1000) {
        return LE_ERROR_PARAM_RANGE_OVERFLOW;
    }

    index = get_device_index(pk, dn);
    g_devices[index].double_rw = value;
    return LE_SUCCESS;
}


double get_dev_double_r(const char *pk,  const char *dn)
{
    int index;
    index = get_device_index(pk, dn);
    return g_devices[index].double_r;
}


enum_e get_dev_enum_rw(const char *pk,  const char *dn)
{
    int index;
    index = get_device_index(pk, dn);
    return g_devices[index].enum_rw;
}


int set_dev_enum_rw(const char *pk, const char *dn, int value)
{
    int index;
    
    if (value < ENUM_VALUE0 || value > ENUM_VALUE4) {
        return LE_ERROR_PARAM_RANGE_OVERFLOW;
    }

    index = get_device_index(pk, dn);
    g_devices[index].enum_rw = value;
    return LE_SUCCESS;
}


enum_e get_dev_enum_r(const char *pk,  const char *dn)
{
    int index;
    index = get_device_index(pk, dn);
    return g_devices[index].enum_r;
}


bool get_dev_bool_rw(const char *pk,  const char *dn)
{
    int index;
    index = get_device_index(pk, dn);
    return g_devices[index].bool_rw;
}


int set_dev_bool_rw(const char *pk, const char *dn, int value)
{
    int index;
    
    if (value != 0 && value != 1) {
        return LE_ERROR_PARAM_RANGE_OVERFLOW;
    }

    index = get_device_index(pk, dn);
    g_devices[index].bool_rw = value;
    return LE_SUCCESS;
}


bool get_dev_bool_r(const char *pk,  const char *dn)
{
    int index;
    index = get_device_index(pk, dn);
    return g_devices[index].bool_r;
}


char * get_dev_string_rw(const char *pk,  const char *dn)
{
    int index;
    index = get_device_index(pk, dn);
    return g_devices[index].string_rw;
}


int set_dev_string_rw(const char *pk, const char *dn, const char *value)
{
    int index;
    
    if (strlen(value) > 2048) {
        return LE_ERROR_PARAM_RANGE_OVERFLOW;
    }

    index = get_device_index(pk, dn);
    snprintf(g_devices[index].string_rw, sizeof(g_devices[index].string_rw), "%s", value);
    return LE_SUCCESS;
}


char * get_dev_string_r(const char *pk,  const char *dn)
{
    int index;
    index = get_device_index(pk, dn);
    snprintf(g_devices[index].string_r, sizeof(g_devices[index].string_r), "%s", "MlRVZq" );
    return g_devices[index].string_r;
}


char * get_dev_date_rw(const char *pk,  const char *dn)
{
    int index;
    index = get_device_index(pk, dn);
    return g_devices[index].date_rw;
}


int set_dev_date_rw(const char *pk, const char *dn, const char *value)
{
    int index;
    
    if (strlen(value) > 32) {
        return LE_ERROR_PARAM_RANGE_OVERFLOW;
    }

    index = get_device_index(pk, dn);
    snprintf(g_devices[index].date_rw, sizeof(g_devices[index].date_rw), "%s", value);
    return LE_SUCCESS;
}


char * get_dev_date_r(const char *pk,  const char *dn)
{    
    int index;
    struct timespec ts = {0};
    clock_gettime(CLOCK_REALTIME, &ts);

    index = get_device_index(pk, dn);
    snprintf(g_devices[index].date_r, sizeof(g_devices[index].date_r), "%ld%03ld", ts.tv_sec, ts.tv_nsec/1000000);

    return g_devices[index].date_r;
}


static int cb_get_property(const char *pk, const char *dn,
                                       leda_device_data_t properties[], 
                                       int properties_count, 
                                       void *usr_data)
{
    int i;
    for (i = 0; i < properties_count; i++) {
        if (strcmp(properties[i].key, PROPERTY_INT32_RW) == 0) {
            snprintf(properties[i].value, MAX_PARAM_VALUE_LENGTH, "%d", get_dev_int32_rw(pk, dn));
            properties[i].type = LEDA_TYPE_INT;
        } else if (strcmp(properties[i].key, PROPERTY_INT32_R) == 0) {
            snprintf(properties[i].value, MAX_PARAM_VALUE_LENGTH, "%d", get_dev_int32_r(pk, dn));
            properties[i].type = LEDA_TYPE_INT;
        } else if (strcmp(properties[i].key, PROPERTY_FLOAT_RW) == 0) {
            snprintf(properties[i].value, MAX_PARAM_VALUE_LENGTH, "%f", get_dev_float_rw(pk, dn));
            properties[i].type = LEDA_TYPE_FLOAT;
        } else if (strcmp(properties[i].key, PROPERTY_FLOAT_R) == 0) {
            snprintf(properties[i].value, MAX_PARAM_VALUE_LENGTH, "%f", get_dev_float_r(pk, dn));
            properties[i].type = LEDA_TYPE_FLOAT;
        } else if (strcmp(properties[i].key, PROPERTY_DOUBLE_RW) == 0) {
            snprintf(properties[i].value, MAX_PARAM_VALUE_LENGTH, "%lf", get_dev_double_rw(pk, dn));
            properties[i].type = LEDA_TYPE_DOUBLE;
        } else if (strcmp(properties[i].key, PROPERTY_DOUBLE_R) == 0) {
            snprintf(properties[i].value, MAX_PARAM_VALUE_LENGTH, "%lf", get_dev_double_r(pk, dn));
            properties[i].type = LEDA_TYPE_DOUBLE;
        } else if (strcmp(properties[i].key, PROPERTY_ENUM_RW) == 0) {
            snprintf(properties[i].value, MAX_PARAM_VALUE_LENGTH, "%d", get_dev_enum_rw(pk, dn));
            properties[i].type = LEDA_TYPE_ENUM;
        } else if (strcmp(properties[i].key, PROPERTY_ENUM_R) == 0) {
            snprintf(properties[i].value, MAX_PARAM_VALUE_LENGTH, "%d", get_dev_enum_r(pk, dn));
            properties[i].type = LEDA_TYPE_ENUM;
        }  else if (strcmp(properties[i].key, PROPERTY_BOOL_RW) == 0) {
            snprintf(properties[i].value, MAX_PARAM_VALUE_LENGTH, "%d", get_dev_bool_rw(pk, dn));
            properties[i].type = LEDA_TYPE_BOOL;
        } else if (strcmp(properties[i].key, PROPERTY_BOOL_R) == 0) {
            snprintf(properties[i].value, MAX_PARAM_VALUE_LENGTH, "%d", get_dev_bool_r(pk, dn));
            properties[i].type = LEDA_TYPE_BOOL;
        } else if (strcmp(properties[i].key, PROPERTY_STRING_RW) == 0) {
            snprintf(properties[i].value, MAX_PARAM_VALUE_LENGTH, "%s", get_dev_string_rw(pk, dn));
            properties[i].type = LEDA_TYPE_TEXT;
        } else if (strcmp(properties[i].key, PROPERTY_STRING_R) == 0) {
            snprintf(properties[i].value, MAX_PARAM_VALUE_LENGTH, "%s", get_dev_string_r(pk, dn));
            properties[i].type = LEDA_TYPE_TEXT;
        }  else if (strcmp(properties[i].key, PROPERTY_DATE_RW) == 0) {
            snprintf(properties[i].value, MAX_PARAM_VALUE_LENGTH, "%s", get_dev_date_rw(pk, dn));
            properties[i].type = LEDA_TYPE_DATE;
        }   else if (strcmp(properties[i].key, PROPERTY_DATE_R) == 0) {
            snprintf(properties[i].value, MAX_PARAM_VALUE_LENGTH, "%s", get_dev_date_r(pk, dn));
            properties[i].type = LEDA_TYPE_DATE;
        } else {
            return LEDA_ERROR_PROPERTY_NOT_EXIST;
        }
    }

    return LE_SUCCESS;
}


int get_property_enum(const char *name)
{
   int i = E_PROPERTY_INVALID, count;

   count = sizeof(g_property_string_enum_map) / sizeof(property_string_enum_map_t);

   for(i = 0; i < count; i++) {
       if (0 == strcmp(g_property_string_enum_map[i].property_string, name)) {
           break;
       }
   }
   return i;
}


void send_signal_property_changed(const char *pk, const char *dn)
{
   int i;
   
   snprintf(g_property_changed_pk, sizeof(g_property_changed_pk), "%s", pk);
   snprintf(g_property_changed_dn, sizeof(g_property_changed_dn), "%s", dn);
   pthread_mutex_lock(&g_property_monitor_locker);
   pthread_cond_signal(&g_cond_property_changed);
   pthread_mutex_unlock(&g_property_monitor_locker);
}

static int cb_set_property(const char *pk, const char *dn,
                                       const leda_device_data_t properties[], 
                                       int properties_count, 
                                       void *usr_data)
{
    int ret = LE_ERROR_UNKNOWN, i;

    for (i = 0; i < properties_count; i++) {
        if (strcmp(properties[i].key, PROPERTY_INT32_RW) == 0) {
            ret = set_dev_int32_rw(pk, dn, atoi(properties[i].value));
        } else if (strcmp(properties[i].key, PROPERTY_FLOAT_RW) == 0) {
            ret = set_dev_float_rw(pk, dn, atof(properties[i].value));
        }else if (strcmp(properties[i].key, PROPERTY_DOUBLE_RW) == 0) {
            ret = set_dev_double_rw(pk, dn, atof(properties[i].value));
        } else if (strcmp(properties[i].key, PROPERTY_ENUM_RW) == 0) {
            ret = set_dev_enum_rw(pk, dn, atoi(properties[i].value));
        } else if (strcmp(properties[i].key, PROPERTY_BOOL_RW) == 0) {
            ret = set_dev_bool_rw(pk, dn, atoi(properties[i].value));
        } else if (strcmp(properties[i].key, PROPERTY_STRING_RW) == 0) {
            ret = set_dev_string_rw(pk, dn, properties[i].value);
        } else if (strcmp(properties[i].key, PROPERTY_DATE_RW) == 0) {
            ret = set_dev_date_rw(pk, dn, properties[i].value);
        } else {
            printf("property %s is not exist!\n", properties[i].value);
            return LEDA_ERROR_PROPERTY_NOT_EXIST;
        }
        if (ret != LE_SUCCESS) {
            break;
        }
    }
    if (ret == LE_SUCCESS) {
        //send_signal_property_changed(pk, dn);
    }
    return ret;
}

static int cb_call_service(const char *pk, const char *dn, const char *method_name, const leda_device_data_t *input, int count, 
                            leda_device_data_t *output, void *usr)
{
    int i, ret = LE_ERROR_UNKNOWN;
    
    if (!method_name || !output) {
        return LE_ERROR_INVAILD_PARAM;
    }
    if (0 == strcmp(SERVICE_WRITE, method_name)) {
        ret = cb_set_property(pk, dn, input, count, NULL);
    } else {
        ret = LEDA_ERROR_SERVICE_NOT_EXIST;
    }

    return ret;
}

static void cb_state_changed(leda_conn_state_e state, void *usr)
{
    int ret = -1;
    printf("connection state: %s\n", !state ? "connected" : "disconnected");
    if (LEDA_WS_CONNECTED == state) {
        g_is_connected = 1;
    } else {
        g_is_connected = 0;
    }
    pthread_mutex_lock(&g_mtx_online);
    pthread_cond_signal(&g_cond_conn_state_changed);
    pthread_mutex_unlock(&g_mtx_online);
    return;
}


void report_all_event(const char *pk, const char *dn)
{
    leda_device_data_t event_output[1] = {0};

    snprintf(event_output[0].key, MAX_PARAM_NAME_LENGTH, PROPERTY_INT32_RW);
    event_output[0].type = LEDA_TYPE_INT;
    snprintf(event_output[0].value, MAX_PARAM_VALUE_LENGTH, "%d", get_dev_int32_rw(pk, dn));
    leda_report_event(pk, dn, EVENT_INT32, event_output, 1);
    
    snprintf(event_output[0].key, MAX_PARAM_NAME_LENGTH, PROPERTY_FLOAT_RW);
    event_output[0].type = LEDA_TYPE_FLOAT;
    snprintf(event_output[0].value, MAX_PARAM_VALUE_LENGTH, "%f", get_dev_float_rw(pk, dn));
    leda_report_event(pk, dn, EVENT_FLOAT, event_output, 1);
    
    snprintf(event_output[0].key, MAX_PARAM_NAME_LENGTH, PROPERTY_STRING_RW);
    event_output[0].type = LEDA_TYPE_TEXT;
    snprintf(event_output[0].value, MAX_PARAM_VALUE_LENGTH, "%s", get_dev_string_rw(pk, dn));
    leda_report_event(pk, dn, EVENT_STRING, event_output, 1);
}


void report_all_property(const char *pk, const char *dn)
{
    leda_device_data_t property[14];
    
    snprintf(property[0].key, MAX_PARAM_NAME_LENGTH, PROPERTY_INT32_RW);
    property[0].type = LEDA_TYPE_INT;
    snprintf(property[0].value, MAX_PARAM_VALUE_LENGTH, "%d", get_dev_int32_rw(pk, dn));
    
    snprintf(property[1].key, MAX_PARAM_NAME_LENGTH, PROPERTY_INT32_R);
    property[1].type = LEDA_TYPE_INT;
    snprintf(property[1].value, MAX_PARAM_VALUE_LENGTH, "%d", get_dev_int32_r(pk, dn));
    
    snprintf(property[2].key, MAX_PARAM_NAME_LENGTH, PROPERTY_FLOAT_RW);
    property[2].type = LEDA_TYPE_FLOAT;
    snprintf(property[2].value, MAX_PARAM_VALUE_LENGTH, "%f", get_dev_float_rw(pk, dn));
    
    snprintf(property[3].key, MAX_PARAM_NAME_LENGTH, PROPERTY_FLOAT_R);
    property[3].type = LEDA_TYPE_FLOAT;
    snprintf(property[3].value, MAX_PARAM_VALUE_LENGTH, "%f", get_dev_float_r(pk, dn));
    
    snprintf(property[4].key, MAX_PARAM_NAME_LENGTH, PROPERTY_DOUBLE_RW);
    property[4].type = LEDA_TYPE_DOUBLE;
    snprintf(property[4].value, MAX_PARAM_VALUE_LENGTH, "%lf", get_dev_double_rw(pk, dn));
    
    snprintf(property[5].key, MAX_PARAM_NAME_LENGTH, PROPERTY_DOUBLE_R);
    property[5].type = LEDA_TYPE_DOUBLE;
    snprintf(property[5].value, MAX_PARAM_VALUE_LENGTH, "%lf", get_dev_double_r(pk, dn));
    
    snprintf(property[6].key, MAX_PARAM_NAME_LENGTH, PROPERTY_ENUM_RW);
    property[6].type = LEDA_TYPE_ENUM;
    snprintf(property[6].value, MAX_PARAM_VALUE_LENGTH, "%d", get_dev_enum_rw(pk, dn));
    
    snprintf(property[7].key, MAX_PARAM_NAME_LENGTH, PROPERTY_ENUM_R);
    property[7].type = LEDA_TYPE_ENUM;
    snprintf(property[7].value, MAX_PARAM_VALUE_LENGTH, "%d", get_dev_enum_r(pk, dn));
    
    snprintf(property[8].key, MAX_PARAM_NAME_LENGTH, PROPERTY_BOOL_RW);
    property[8].type = LEDA_TYPE_BOOL;
    snprintf(property[8].value, MAX_PARAM_VALUE_LENGTH, "%d", get_dev_bool_rw(pk,dn));
    
    snprintf(property[9].key, MAX_PARAM_NAME_LENGTH, PROPERTY_BOOL_R);
    property[9].type = LEDA_TYPE_BOOL;
    snprintf(property[9].value, MAX_PARAM_VALUE_LENGTH, "%d", get_dev_bool_r(pk, dn));
    
    snprintf(property[10].key, MAX_PARAM_NAME_LENGTH, PROPERTY_STRING_RW);
    property[10].type = LEDA_TYPE_TEXT;
    snprintf(property[10].value, MAX_PARAM_VALUE_LENGTH, "%s", get_dev_string_rw(pk, dn));
    
    snprintf(property[11].key, MAX_PARAM_NAME_LENGTH, PROPERTY_STRING_R);
    property[11].type = LEDA_TYPE_TEXT;
    snprintf(property[11].value, MAX_PARAM_VALUE_LENGTH, "%s", get_dev_string_r(pk, dn));
    
    snprintf(property[12].key, MAX_PARAM_NAME_LENGTH, PROPERTY_DATE_RW);
    property[12].type = LEDA_TYPE_DATE;
    snprintf(property[12].value, MAX_PARAM_VALUE_LENGTH, "%s", get_dev_date_rw(pk, dn));
    
    snprintf(property[13].key, MAX_PARAM_NAME_LENGTH, PROPERTY_DATE_R);
    property[13].type = LEDA_TYPE_DATE;
    snprintf(property[13].value, MAX_PARAM_VALUE_LENGTH, "%s", get_dev_date_r(pk, dn));
    
    leda_report_properties(pk, dn, property, sizeof(property) / sizeof(leda_device_data_t));

    return;
}


void *property_monitor(void *arg)
{
    leda_device_data_t property[1] = {0};

    pthread_detach(pthread_self());
    
    while (g_running) {
        pthread_mutex_lock(&g_property_monitor_locker);
        pthread_cond_wait(&g_cond_property_changed, &g_property_monitor_locker);
        report_all_property(g_property_changed_pk, g_property_changed_dn);
        memset(g_property_changed_pk, 0, sizeof(g_property_changed_pk));
        memset(g_property_changed_dn, 0, sizeof(g_property_changed_dn));
        pthread_mutex_unlock(&g_property_monitor_locker);
    }

    return NULL;
}

void sigint_handler(int sig)
{
    if (!g_running) {
        printf("force exit!\n");
        exit(0);
    }
    printf("caught signal: %s, device will exit in 30s ...\n", strsignal(sig));
    g_running = 0;
}


void* thread_online(void *arg)
{
    int count, i, ret;
    char *pk, *dn;
    
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "online", NULL, NULL, NULL);

    count = g_devices_count;
    
    while (g_running) {
        pthread_mutex_lock(&g_mtx_online);
        pthread_cond_wait(&g_cond_conn_state_changed, &g_mtx_online);
        for (i = 0; i < count; i++) {
            if (g_is_connected) {
                ret = leda_online(g_devices[i].pk, g_devices[i].dn);
                if (LE_SUCCESS == ret) {
                    g_devices[i].online = 1;
                }
            } else {
                g_devices[i].online = 0; 
            }
        }
        pthread_mutex_unlock(&g_mtx_online);
    }
}


void *thread_report(void *arg) {
    int count, i;
    char *pk, *dn;

    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "report", NULL, NULL, NULL);
    
    count = g_devices_count;
    
    while (g_running) {
        for (i = 0; i < count; i++) {
            pk = g_devices[i].pk;
            dn = g_devices[i].dn;
            if (g_devices[i].online == 1) {
                report_all_event(pk, dn);
                report_all_property(pk, dn);
            }
            usleep(5*1000000/count);
        }
    }
}


void unload_device_list()
{
    if (g_devices) {
        free(g_devices);
        g_devices = NULL;
    }
    g_devices_count = 0;
}


int load_device_from_csv(char *path)
{
    FILE *fp;
    char ch;
    int line = 0, i;
    device_t invalid = {0};
    char tmp[128] = {0};
    
    fp = fopen(path, "r+");
    if (!fp) {
        printf("open file:%s error!\n", path);
        return -1;
    }
    
    while (NULL != fgets(&tmp[0], sizeof(tmp), fp)) {
       line++;
    }
    fseek(fp, 0, SEEK_SET);
    
    g_devices_count = line - 1;
    g_devices = malloc(g_devices_count * sizeof(device_t));
    memset(g_devices, 0, g_devices_count * sizeof(device_t));
    for (i = 0; i < line; i++) {
        if (i == 0) {
            fscanf(fp, "%s\n", &tmp[0]);
        } else {
            fscanf(fp, "%[^','],%[^','],%s\n", &g_devices[i - 1].dn[0], &tmp[0], &g_devices[i - 1].pk[0]);
        }        
    }
    fclose(fp);

    return 0;
}


int main(int argc, char **argv)
{
    leda_conn_info_t conn = {0};
    struct sigaction sig_int = {0};
    char path[PATH_MAX] = {0};
    char url[32] = {0};
    int i;
    char *pk, *dn;
    
    leda_device_data_t event_output[1] = {0};
    
    if (argc < 3) {
        printf("usage: ./simulated_device [$csv_path] [$ip] <$tls_switch>\n"\
                "    $csv_path is the path of csv file downloaded from IoT platform\n"\
                "    $ip is the ip address of Link IoT Edge\n"\
                "    $tls_switch is the tls switch, 0 close, 1 open, none is close for default\n");
        return -1;
    }

    if (0 != load_device_from_csv(argv[1])) {
        printf("load device list error!\n");
        return -1;
    }

    sigemptyset(&sig_int.sa_mask);
    sig_int.sa_handler = sigint_handler;
    sigaction(SIGINT, &sig_int, NULL);

    if (argc == 4 && atoi(argv[3]) == 1) {
        snprintf(url, sizeof(url), "wss://%s:17682", argv[2]);
    } else {
        snprintf(url, sizeof(url), "ws://%s:17682", argv[2]);
    }

    getcwd(path, PATH_MAX);
    strcat(path, "../cert/CA.cer");
    conn.url = url;
    conn.timeout = 20 * 60;
    conn.ca_path = path;
    conn.cert_path = NULL;
    conn.key_path = NULL;
    conn.ws_conn_cb.conn_state_change_cb = cb_state_changed;
    conn.ws_conn_cb.usr_data = NULL;
    conn.conn_devices_cb.get_properties_cb = cb_get_property;
    conn.conn_devices_cb.usr_data_get_property = NULL;
    conn.conn_devices_cb.set_properties_cb = cb_set_property;
    conn.conn_devices_cb.usr_data_set_property = NULL;
    conn.conn_devices_cb.call_service_cb = cb_call_service;
    conn.conn_devices_cb.usr_data_call_service = NULL;
    if (0 != pthread_create(&g_thread_online, NULL, thread_online, NULL)) {
        goto end;
    }

    while (leda_wsc_init(&conn) != LE_SUCCESS && g_running) {
        leda_wsc_exit();
        sleep(5);
        continue;
    }
#if 1
    if (0 != pthread_create(&g_thread_report, NULL, thread_report, NULL)) {
        goto end;
    }
#endif    
#if 0
    if (0 != pthread_create(&g_thread_property_monitor, NULL, property_monitor, NULL)) {
        goto end;
    }
#endif
    while (g_running) {
         sleep(3);
    }

end:    
    for (i = 0; i < g_devices_count; i++) {
        if (g_is_connected && g_devices[i].online) {
            if (leda_offline(g_devices[i].pk, g_devices[i].dn) == LE_SUCCESS) {
                g_devices[i].online = 0;
            }
        }
    }
    leda_wsc_exit();
    unload_device_list();

    return 0;
}


