#include <stdarg.h>
#include <time.h>
#include "base-utils.h"
#include "log.h"

#define MODULE_NAME_LEN 64
#define MAX_MSG_LEN     512*2

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

//date [module] level <tag> file-func:line content
#define LOG_FMT_VRB  "%s %s <%s> %s-%s:%d "

static const char *g_log_desc[] = 
{ 
    "DBG", 
    "INF",
    "WRN", 
    "ERR",
    "FTL" 
};

static int g_log_lvl = LOG_LEVEL_ERR;

static char *get_timestamp(char *buf, int len, time_t cur_time)
{
    struct tm tm_time;
#ifdef _WIN32
    localtime_s(&tm_time, &cur_time);
#else
    localtime_r(&cur_time, &tm_time);
#endif
    snprintf(buf, len, "%d-%d-%d %d:%d:%d",
             1900 + tm_time.tm_year, 1 + tm_time.tm_mon,
             tm_time.tm_mday, tm_time.tm_hour,
             tm_time.tm_min, tm_time.tm_sec);
    return buf;
}
#if 0
void set_log_level(int lvl)
{
    if(lvl < LOG_LEVEL_DEBUG || lvl > LOG_LEVEL_NONE)
        g_log_lvl = LOG_LEVEL_ERR; 
    else 
        g_log_lvl = lvl;
    printf("set log level :  %d\n", lvl);
}

#define color_len_fin strlen(COL_DEF)
#define color_len_start strlen(COL_RED)
void log_print(LOG_LEVEL lvl, cchar *color, cchar *t, cchar *f, cchar *func, int l, cchar *fmt, ...)
{
    if(lvl < g_log_lvl)
        return;

    va_list ap;
    va_start(ap, fmt);
    char *buf = NULL;

    char *tmp = NULL;
    size_t len = 0;
    char buf_date[20] = {0};
    time_t cur_time = time(NULL);

    t = !t ? "\b" : t;
    f = !f ? "\b" : f;
    func = !func ? "\b" : func;

    tmp = strrchr(f, '/');
    if(tmp)
        f = tmp + 1;

    buf = malloc(MAX_MSG_LEN + 1);
    if(NULL == buf){
        printf("failed to malloc memory .\n");
        return;
    }

    memset(buf, 0, MAX_MSG_LEN + 1);
    //add color support
    if (color) 
        strcat(buf, color);

    snprintf(buf + strlen(buf), MAX_MSG_LEN - strlen(buf), LOG_FMT_VRB,
                 get_timestamp(buf_date, 20, cur_time),
                 g_log_desc[lvl], t, f, func, l);

    len = MAX_MSG_LEN - strlen(buf) - color_len_fin - 5;
    len = len <= 0 ? 0 : len;
    tmp = buf + strlen(buf);
    if (vsnprintf(tmp, len, fmt, ap) > len) 
        strcat(buf, "...\n");

    if (color) 
        strcat(buf, COL_DEF);

    fprintf(stderr, "%s", buf);
    if (color) 
        buf[strlen(buf) - color_len_fin] = '\0';

    va_end(ap);

    free(buf);
}
#endif
#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
}
#endif