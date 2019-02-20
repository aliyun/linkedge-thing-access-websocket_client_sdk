#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <limits.h>

#include "leda.h"
#include "le_error.h"
#include <unistd.h>


typedef struct {
    char pk[16];
    char dn[32];
    int online;
    int power;
    int brightness;
} device_t;

device_t g_devices[5] = {
    {"xxxxxxxxxx", "ledDev001", 0, 0, 0},
    {"xxxxxxxxxx", "ledDev002", 0, 0, 0},
    {"xxxxxxxxxx", "ledDev003", 0, 0, 0},
    {"xxxxxxxxxx", "ledDev004", 0, 0, 0},
    {"xxxxxxxxxx", "ledDev005", 0, 0, 0},
};

int g_is_connected = 0;
int g_running = 1;


int get_device_index(const char *pk, const char *dn)
{
    int i;
    
    for (i = 0; i < sizeof(g_devices) / sizeof(device_t); i++) {
        if (strcmp(g_devices[i].dn, dn) == 0) {
            return i;
        }
    }
}


int set_led_power(const char *pk, const char *dn, int state)
{
    int index;
    
    if (0 != state && 1 != state) {
        return LE_ERROR_PARAM_RANGE_OVERFLOW;
    }

    index = get_device_index(pk, dn);
    g_devices[index].power = state;
    return LE_SUCCESS;
}

int get_led_power(const char *pk,  const char *dn)
{
    int index;
    index = get_device_index(pk, dn);
    return g_devices[index].power;
}


int set_led_brightness(const char *pk, const char *dn, int level)
{
    int index;
    if (level < 0 || level > 5) {
        return LE_ERROR_PARAM_RANGE_OVERFLOW;
    }
    index = get_device_index(pk, dn);
    g_devices[index].brightness = level; 
    return LE_SUCCESS;
}

int get_led_brightness(const char *pk, const char *dn)
{
    int index;
    index = get_device_index(pk, dn);
    return g_devices[index].brightness;
}


static int cb_get_property(const char *pk, const char *dn,
                                       leda_device_data_t properties[], 
                                       int properties_count, 
                                       void *usr_data)
{
    int i;
    for (i = 0; i < properties_count; i++) {
        if (strcmp(properties[i].key, "power") == 0) {
            snprintf(properties[i].value, MAX_PARAM_VALUE_LENGTH, "%d", get_led_power(pk, dn));
            properties[i].type = LEDA_TYPE_BOOL;
        } else if (strcmp(properties[i].key, "brightness") == 0) {
            snprintf(properties[i].value, MAX_PARAM_VALUE_LENGTH, "%d", get_led_brightness(pk, dn));
            properties[i].type = LEDA_TYPE_INT;
        } else {
            return LEDA_ERROR_PROPERTY_NOT_EXIST;
        }
    }

    return LE_SUCCESS;
}


static int cb_set_property(const char *pk, const char *dn,
                                       const leda_device_data_t properties[], 
                                       int properties_count, 
                                       void *usr_data)
{
    int ret = LE_ERROR_UNKNOWN, i;

    for (i = 0; i < properties_count; i++) {
        if (0 == strcmp("brightness", properties[i].key)) {
            ret = set_led_brightness(pk, dn, atoi(properties[i].value));
        } else if (0 == strcmp("power", properties[i].key)) {
            ret = set_led_power(pk, dn, atoi(properties[i].value));
        } else {
            return LEDA_ERROR_PROPERTY_NOT_EXIST;
        }
    }
    return ret;
}

static int cb_call_service(const char *pk, const char *dn, const char *method_name, const leda_device_data_t *data, int count, 
                            leda_device_data_t *output, void *usr)
{
    if (!method_name || !output) {
        return LE_ERROR_INVAILD_PARAM;
    }
    if (0 == strcmp("blink", method_name)) {
        printf("%s:%s blinking\n", pk, dn);
        return LE_SUCCESS;
    } else {
        return LEDA_ERROR_SERVICE_NOT_EXIST;
    }
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
    return;
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




int main(int argc, char **argv)
{
    leda_conn_info_t conn = {0};
    struct sigaction sig_int = {0};
    char path[PATH_MAX] = {0};
    int i;
    char *url = NULL, *pk, *dn;
    
    if (argc < 2) {
        printf("usage: ./demo [ip] <tls>\n");
        return -1;
    }

    sigemptyset(&sig_int.sa_mask);
    sig_int.sa_handler = sigint_handler;
    sigaction(SIGINT, &sig_int, NULL);

    url = malloc(32);
    if (!url) {
        return -1;
    }
    memset(url, 0, 32);
    if (argc == 3 && atoi(argv[2]) == 1) {
        snprintf(url, 32, "wss://%s:17682", argv[1]);
    } else {
        snprintf(url, 32, "ws://%s:17682", argv[1]);
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
    conn.conn_devices_cb.set_properties_cb = NULL;
    conn.conn_devices_cb.call_service_cb = cb_call_service;
    conn.conn_devices_cb.usr_data_call_service = NULL;
    if (leda_wsc_init(&conn) != LE_SUCCESS) {
        sleep(1);
    }
    free(url);
    
    leda_device_data_t property[2];
    snprintf(property[0].key, MAX_PARAM_NAME_LENGTH, "brightness");
    property[0].type = LEDA_TYPE_INT;
    snprintf(property[1].key, MAX_PARAM_NAME_LENGTH, "power");
    property[1].type = LEDA_TYPE_BOOL;

    while (g_running) {
        for (i = 0; i < sizeof(g_devices) / sizeof(device_t); i++) {
            pk = g_devices[i].pk;
            dn = g_devices[i].dn;
            if (g_is_connected) {
                if (g_devices[i].online == 0) {
                    if (leda_online(pk, dn) == LE_SUCCESS) {
                        g_devices[i].online = 1;
                    }
                }
                if (g_devices[i].online == 1)
                {
                    snprintf(property[0].value, MAX_PARAM_VALUE_LENGTH, "%d", get_led_brightness(pk, dn));
                    snprintf(property[1].value, MAX_PARAM_VALUE_LENGTH, "%d", get_led_power(pk, dn));
                    leda_report_properties(pk, dn, property, 2);
                    leda_report_event(pk, dn, "ledBroken", NULL, 0);
                }
           } else {
                g_devices[i].online = 0;
            }
            sleep(1);
        }
        sleep(15);
    }
    for (i = 0; i < sizeof(g_devices) / sizeof(device_t); i++) {
        pk = g_devices[i].pk;
        dn = g_devices[i].dn;
        if (g_is_connected && g_devices[i].online) {
            if (leda_offline(pk, dn) == LE_SUCCESS) {
                g_devices[i].online = 0;
            }
        }
    }
    leda_wsc_exit();

    return 0;
}


