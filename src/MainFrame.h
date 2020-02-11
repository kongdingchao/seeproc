#ifndef _M_MainFrame_H
#define _M_MainFrame_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <curses.h>
#include "ProcInfo.h"
#include "setting.h"

//主窗口
class MainFrame
{
public:
	MainFrame(void);
	~MainFrame(void);

	//初始化窗口
	void InitFrame();
	//销毁窗口
	void DestroyFrame();
	//打印窗口
	void PrintFrame();
	//打印进程信息
	void PrintProcInfo();
	//开启定时刷新窗口
	void StartRun();
	//处理用户输入
	void HandleInput();

	//监控线程运行
	static void *ExecuteSeeProcThread(void *owner);

	//进程列表
	std::map<TProcKey, TProcInfo> ProcessList;			// 当前页进程
	std::map<TProcKey, TProcInfo> AllProcessList;		// 全部进程

	WINDOW *win1;
	WINDOW *win2;
	WINDOW *win3;
};
#endif
