#ifndef _M_setting_H
#define _M_setting_H
#include <iostream>
#include <unistd.h>
using namespace std;

//配置
class Settings {
public:
	Settings()
	{
		timeout = 2;
		pageCount = 23;
		localPath = "";

		userId = getuid();
		/* get the number of processors online */
		ncpus = sysconf(_SC_NPROCESSORS_ONLN);
		if (ncpus == -1) { /* sysconf error */
			ncpus = 1;
		}

		/* get the page size in bytes */
		pageSize = getpagesize(); //系统给我们提供真正的内存时，用页为单位提供，一次最少提供一页的真实内存空间

		nowTime = 0.0;
		posHint = 0;	//提示的Y位置
		localPage = 1;		//当前页
		totalPages = 1;		//总页数
	}

	bool CheckValid()
	{
		if (timeout <= 0
			|| pageCount <= 0
			|| shellName.empty())
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	int timeout;		//定时刷新时间
	int pageCount;		//每页进程个数
	string shellName;	//shell脚本名称
	string localPath;	//目标程序路径

	uid_t userId;		//用户ID
	int ncpus;			//cpu个数
	int pageSize;		//页大小

	double nowTime;		//当前时间
	int posHint;		//提示的Y位置
	int localPage;		//当前页
	int totalPages;		//总页数
};

//程序启动设置
extern Settings G_Settings;

#endif