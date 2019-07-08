#ifndef _OS_H_
#define _OS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief 微秒级睡眠
* @param us 微秒
* @return void
*/
void os_usleep(int us);

/**
* @brief 秒级睡眠
* @param sec 秒
* @return void
*/
void os_sleep(int sec);


/**
* @brief malloc一块内存并将字符串s拷贝到这块内存
* @params
*		s 字符串
* @return
*		NULL 执行失败
*		> 0 内存地址
*/
char *os_strdup(const char *s);

/**
* @brief malloc一块内存并将字符串s拷贝到这块内存，字符串长度超过n-1时，被截断。字符串以'\0'结束
* @params 
*		s 字符串
*		n 分配的内存最大长度为n
* @return 
*		NULL 执行失败
*		> 0 内存地址
*/
char *os_strndup(const char *s, size_t n);

#ifdef __cplusplus
}
#endif

#endif