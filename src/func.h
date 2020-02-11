#ifndef _M_func_H
#define _M_func_H
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

/* ��2�����߳���2�����滻Ϊ1���ո� */
static void trim_more(std::string &str)
{
	std::string::size_type pos = 0;
	while ((pos = str.find("  ", 0)) != std::string::npos)
		str.replace(pos, 2, " ");
	int _len = str.length();
	if (_len > 0 && isspace(str[0]))
		str = str.substr(1);
	_len = str.length();
	if (_len > 0 && isspace(str[_len-1]))
		str = str.substr(0, _len - 1);
}
//���num���ַ�word���ַ���src
static void restrcat_word(char * src, char word, int num)
{
	for(int i = 0; i < num; i ++)
	{
		src[strlen(src)] = word;
	}
	src[strlen(src)] = '|';
}
//�õ�ϵͳ��ǰ����
static const char *todayStr()
{
	static char szDateTime[24];
	struct tm *c_tm;
	time_t t1;
	memset(szDateTime, 0, 24);
	t1 = time(NULL);
	c_tm = localtime(&t1);
	sprintf(szDateTime,"%02d:%02d:%02d",
		c_tm->tm_hour, c_tm->tm_min, c_tm->tm_sec);
	return szDateTime;
}
#endif