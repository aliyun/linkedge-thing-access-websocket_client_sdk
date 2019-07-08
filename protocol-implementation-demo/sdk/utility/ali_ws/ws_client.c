#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include "libwebsockets.h"
#include "ws_client.h"
#include "wsc_buffer_mgmt.h"
#include "le_error.h"

extern void *thread_wsc_network(void *arg);
extern volatile int force_exit;
extern struct lws_context *context;

p_wsc_param_cb g_cbs = NULL;


int wsc_init(p_wsc_param_conn pc, p_wsc_param_cb cb)
{
    int ret = 0;
    if (!pc || !cb || !pc->url) {
        printf("wsc init failed, param should not be NULL.\n");
        return LE_ERROR_INVAILD_PARAM;
    }

    if (!strncmp(pc->url, "wss", 3) && !pc->ca_path) {
        printf("ca path must not be NULL when use ssl\n");
    }

    g_cbs = malloc(sizeof(wsc_param_cb));
    if (!g_cbs) {
        printf("malloc error.\n");
        return LE_ERROR_ALLOCATING_MEM;
    }

    memset(g_cbs, 0, sizeof(wsc_param_cb));
    memcpy(g_cbs, cb, sizeof(wsc_param_cb));
    
    force_exit = 0;

    ret = client_buf_mgmt_init(1024 * 2, 1024);
    if (ret != LE_SUCCESS) {
        free(g_cbs);
        g_cbs = NULL;
        return ret;
    }

    pthread_t id;
    ret = pthread_create(&id, NULL, thread_wsc_network, pc);
    if (ret == 0) {
        printf("wsc init ok\n");
    } else {
        free(g_cbs);
        g_cbs = NULL;
#ifndef _WIN32
        printf("wsc init faild, %s.\n", strerror(errno));
#else
        char buffer[64] = { 0 };
        strerror_s(buffer, sizeof(buffer), errno);
        printf("wsc init faild, %s.\n", buffer);
#endif
        client_buf_mgmt_destroy();

        return LE_ERROR_UNKNOWN;
    }
    pthread_detach(id);

    return LE_SUCCESS;
}

int wsc_add_msg(const char *msg, size_t len, int type)
{
    if (!msg || len <= 0 || type > 1 || type < 0) {
        return LE_ERROR_INVAILD_PARAM;
    }

    return client_buf_mgmt_push(msg, len, type);
}

int ws_client_destroy()
{
    force_exit = 1;
#ifndef _WIN32
    usleep(1000*1000);
#else
    Sleep(1000);
#endif
    if (g_cbs) {
        free(g_cbs);
        g_cbs = NULL;
    }

    return client_buf_mgmt_destroy();
}

