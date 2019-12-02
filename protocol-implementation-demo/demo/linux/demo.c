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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <limits.h>

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "leda.h"
#include "le_error.h"

typedef struct device
{
    char    pk[16];
    char    dn[32];

    int     online;

    int     power;
    int     brightness;
} device_t;

sem_t           g_conn_state_sem;
pthread_mutex_t g_conn_state_locker = PTHREAD_MUTEX_INITIALIZER;

pthread_t       g_thread_online;
pthread_t       g_thread_report;

device_t        *g_devices          = NULL;
int             g_devices_count     = 0;

int             g_is_connected      = 0;
int             g_is_running        = 1;

int get_device_index(const char *pk, const char *dn)
{
    int i = 0;

    for (i = 0; i < g_devices_count; i++)
    {
        if (strcmp(g_devices[i].dn, dn) == 0)
        {
            return i;
        }
    }
}

int set_led_power(const char *pk, const char *dn, int state)
{
    int index = 0;

    if (0 != state && 1 != state)
    {
        return LE_ERROR_PARAM_RANGE_OVERFLOW;
    }

    index = get_device_index(pk, dn);
    g_devices[index].power = state;
    return LE_SUCCESS;
}

int get_led_power(const char *pk, const char *dn)
{
    int index;
    index = get_device_index(pk, dn);
    return g_devices[index].power;
}

int set_led_brightness(const char *pk, const char *dn, int level)
{
    int index = 0;
    if (level < 0 || level > 5)
    {
        return LE_ERROR_PARAM_RANGE_OVERFLOW;
    }
    index = get_device_index(pk, dn);
    g_devices[index].brightness = level;
    return LE_SUCCESS;
}

int get_led_brightness(const char *pk, const char *dn)
{
    int index = 0;
    index = get_device_index(pk, dn);
    return g_devices[index].brightness;
}

static int cb_get_property(const char *pk, const char *dn,
                           leda_device_data_t properties[],
                           int properties_count,
                           void *usr_data)
{
    int i = 0;
    for (i = 0; i < properties_count; i++)
    {
        if (strcmp(properties[i].key, "power") == 0)
        {
            snprintf(properties[i].value, MAX_PARAM_VALUE_LENGTH, "%d", get_led_power(pk, dn));
            properties[i].type = LEDA_TYPE_BOOL;
        }
        else if (strcmp(properties[i].key, "brightness") == 0)
        {
            snprintf(properties[i].value, MAX_PARAM_VALUE_LENGTH, "%d", get_led_brightness(pk, dn));
            properties[i].type = LEDA_TYPE_INT;
        }
        else
        {
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
    int ret = LE_ERROR_UNKNOWN;
    int i = 0;

    for (i = 0; i < properties_count; i++)
    {
        if (0 == strcmp("brightness", properties[i].key))
        {
            ret = set_led_brightness(pk, dn, atoi(properties[i].value));
        }
        else if (0 == strcmp("power", properties[i].key))
        {
            ret = set_led_power(pk, dn, atoi(properties[i].value));
        }
        else
        {
            return LEDA_ERROR_PROPERTY_NOT_EXIST;
        }
        if (ret != LE_SUCCESS)
        {
            break;
        }
    }
    return ret;
}

static int cb_call_service(const char *pk, const char *dn, const char *method_name, const leda_device_data_t *data, int count,
                           leda_device_data_t *output, void *usr)
{
    if (!method_name || !output)
    {
        return LE_ERROR_INVAILD_PARAM;
    }
    if (0 == strcmp("blink", method_name))
    {
        printf("%s:%s blinking\n", pk, dn);
        return LE_SUCCESS;
    }
    else
    {
        return LEDA_ERROR_SERVICE_NOT_EXIST;
    }
}

static void cb_state_changed(leda_conn_state_e state, void *usr)
{
    printf("connection state: %s\n", !state ? "connected" : "disconnected");

    pthread_mutex_lock(&g_conn_state_locker);
    if (LEDA_WS_CONNECTED == state)
    {
        g_is_connected = 1;
    }
    else
    {
        g_is_connected = 0;
    }
    pthread_mutex_unlock(&g_conn_state_locker);

    return;
}

static int g_online_dev_num = 0;

void *thread_online(void *arg)
{
    int count = 0, i =0, ret = LE_SUCCESS;
    char *pk = NULL, *dn = NULL;

    pthread_detach(pthread_self());

    count = g_devices_count;
    while (g_is_running)
    {
        pthread_mutex_lock(&g_conn_state_locker);
        for (i = 0; i < count; i++)
        {
            if (g_is_connected)
            {
                if (!g_devices[i].online)
                {
                    ret = leda_online(g_devices[i].pk, g_devices[i].dn);
                    if (LE_SUCCESS == ret)
                    {
                        g_devices[i].online = 1;
                        g_online_dev_num += 1;
                        printf("online device(%s:%s) success\n", g_devices[i].pk, g_devices[i].dn);
                    }
                }
            }
            else
            {
                if (g_devices[i].online)
                {
                    g_devices[i].online = 0;
                    g_online_dev_num -= 1;
                    printf("online device(%s:%s) failed, will try online again\n", g_devices[i].pk, g_devices[i].dn);
                }
            }
        }

        printf("current online device number is %d\n", g_online_dev_num);
        pthread_mutex_unlock(&g_conn_state_locker);
        sleep(5);
    }
}

void *thread_report(void *arg)
{
    int count = 0, i = 0, ret = LE_SUCCESS;
    char *pk = NULL, *dn = NULL;
    leda_device_data_t property[2] = {0};

    pthread_detach(pthread_self());

    snprintf(property[0].key, MAX_PARAM_NAME_LENGTH, "brightness");
    property[0].type = LEDA_TYPE_INT;
    snprintf(property[1].key, MAX_PARAM_NAME_LENGTH, "power");
    property[1].type = LEDA_TYPE_BOOL;

    count = g_devices_count;
    while (g_is_running)
    {
        if (!g_is_connected)
        {
            printf("disconnect with websocket driver, stop send device data\n");
            sleep(1);
            continue;
        }

        for (i = 0; i < count; i++)
        {
            pk = g_devices[i].pk;
            dn = g_devices[i].dn;
            if (g_devices[i].online == 1)
            {
                snprintf(property[0].value, MAX_PARAM_VALUE_LENGTH, "%d", get_led_brightness(pk, dn));
                snprintf(property[1].value, MAX_PARAM_VALUE_LENGTH, "%d", get_led_power(pk, dn));
                ret = leda_report_properties(pk, dn, property, 2, NULL);
                ret |= leda_report_event(pk, dn, "ledBroken", NULL, 0, NULL);
            }
            usleep(10 * 1000000 / count);
        }

        sleep(1);
    }
}

static int load_device_list(const char *triad_path)
{
    FILE    *fp        = NULL;
    char    triad[128] = {0};

    int     line       = 0;
    int     i          = 0;
    

    fp = fopen(triad_path, "r+");
    if (NULL == fp)
    {
        printf("open file: %s error!\n", triad_path);
        return -1;
    }

    while (NULL != fgets(&triad[0], sizeof(triad), fp))
    {
        line++;
    }
    fseek(fp, 0, SEEK_SET);

    if (1 >= line)
    {
        return -1;
    }

    g_devices_count = line - 1;
    g_devices = malloc(g_devices_count * sizeof(device_t));
    if (NULL == g_devices)
    {
        printf("no memory can allocate\n");
        return -1;
    }

    memset(g_devices, 0, g_devices_count * sizeof(device_t));
    fscanf(fp, "%s\n", &triad[0]);
    for (i = 1; i < line; i++)
    {
        fscanf(fp, "%[^','],%s\n", &g_devices[i - 1].dn[0], &g_devices[i - 1].pk[0]);
    }

    fclose(fp);

    return 0;
}

static void unload_device_list()
{
    if (g_devices)
    {
        free(g_devices);
        g_devices = NULL;
    }
    g_devices_count = 0;
}

static void sigint_handler(int sig)
{
    if (!g_is_running)
    {
        printf("force exit!\n");
        exit(0);
    }

    printf("caught signal: %s, device will exit in 30s ...\n", strsignal(sig));
    g_is_running = 0;
}

int main(int argc, char **argv)
{
    struct sigaction sig_int = {0};
    leda_conn_info_t conn    = {0};
    char   *triad_path       = "./triad.csv";

    char ca_path[PATH_MAX]   = {0};
    int  i = 0;

    if (argc < 3 || argc > 4)
    {
        printf("usage: ./demo [$server_ip] [$server_port] <$use_tls>\n"
               "    $server_ip is the ip address of websocket driver\n"
               "    $server_port is the listen port of websocket driver\n"
               "    $use_tls is the tls switch, 0 close, 1 open, none is close for default\n");

        return -1;
    }

    if (0 != load_device_list(triad_path))
    {
        printf("load device list error!\n");
        return -1;
    }

    sigemptyset(&sig_int.sa_mask);
    sig_int.sa_handler = sigint_handler;
    sigaction(SIGINT, &sig_int, NULL);

    sem_init(&g_conn_state_sem, 0, 0);

    conn.server_ip = argv[1];
    conn.server_port = atoi(argv[2]);

    conn.use_tls = 0;
    conn.ca_path = NULL;
    if (argc == 4 && atoi(argv[3]) == 1)
    {
        conn.use_tls = 1;

        getcwd(ca_path, PATH_MAX);
        strcat(ca_path, "/../cert/CA.cer");
        conn.ca_path = ca_path;
    }

    conn.cert_path = NULL;
    conn.key_path = NULL;
    conn.timeout = 20 * 60;
    conn.ws_conn_cb.conn_state_change_cb = cb_state_changed;
    conn.ws_conn_cb.usr_data = NULL;

    conn.conn_devices_cb.get_properties_cb = cb_get_property;
    conn.conn_devices_cb.usr_data_get_property = NULL;
    conn.conn_devices_cb.set_properties_cb = cb_set_property;
    conn.conn_devices_cb.usr_data_set_property = NULL;
    conn.conn_devices_cb.call_service_cb = cb_call_service;
    conn.conn_devices_cb.usr_data_call_service = NULL;
    conn.conn_devices_cb.service_output_max_count = 0;
    while (leda_init(&conn) != LE_SUCCESS)
    {
        leda_exit();
        sleep(5);
        continue;
    }

    if (0 != pthread_create(&g_thread_online, NULL, thread_online, NULL))
    {
        goto end;
    }

    if (0 != pthread_create(&g_thread_report, NULL, thread_report, NULL))
    {
        goto end;
    }

    while (g_is_running)
    {
        sleep(5);
    }

end:
    for (i = 0; i < g_devices_count; i++)
    {
        if (g_is_connected && g_devices[i].online)
        {
            if (leda_offline(g_devices[i].pk, g_devices[i].dn) == LE_SUCCESS)
            {
                g_devices[i].online = 0;
            }
        }
    }

    leda_exit();
    sem_destroy(&g_conn_state_sem);
    unload_device_list();

    return 0;
}
