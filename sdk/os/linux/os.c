#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void os_sleep(int sec)
{
    sleep(sec);
    return;
}

void os_usleep(int us)
{
    (void)usleep(us);
    return;
}


char *os_strdup(char *s)
{
    return strdup(s);
}

char *os_strndup(const char *s, size_t n)
{
    return strndup(s, n);
}


