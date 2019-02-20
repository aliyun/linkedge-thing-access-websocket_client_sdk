#ifndef _OS_H_
#define _OS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief ΢�뼶˯��
* @param us ΢��
* @return void
*/
void os_usleep(int us);

/**
* @brief �뼶˯��
* @param sec ��
* @return void
*/
void os_sleep(int sec);


/**
* @brief mallocһ���ڴ沢���ַ���s����������ڴ�
* @params
*		s �ַ���
* @return
*		NULL ִ��ʧ��
*		> 0 �ڴ��ַ
*/
char *os_strdup(const char *s);

/**
* @brief mallocһ���ڴ沢���ַ���s����������ڴ棬�ַ������ȳ���n-1ʱ�����ضϡ��ַ�����'\0'����
* @params 
*		s �ַ���
*		n ������ڴ���󳤶�Ϊn
* @return 
*		NULL ִ��ʧ��
*		> 0 �ڴ��ַ
*/
char *os_strndup(const char *s, size_t n);

#ifdef __cplusplus
}
#endif

#endif