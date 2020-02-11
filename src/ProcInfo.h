#ifndef _M_ProcInfo_H
#define _M_ProcInfo_H
#include <string.h>
#include <map>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include "func.h"

using namespace std;

//展示标题格式
struct Title_Format {
	int len;//长度
	char title[128];//标题名
	char direct;//对齐方式
};

static Title_Format TitleList[] = {
	{8, 	"PID", 		'L'},
	{6, 	"SN",		'L'},
	//{8, 	"TCNT",		'L'}, // 线程数
	{8, 	"State", 	'L'},
	{10,	"SZ(K)",		'L'},
	{10,	"RSS(K)",		'L'},
	{5, 	"CPU", 			'L'},
	{45, 	"CMDLine", 		'L'},
	{10, 	"BegMoniDT", 	'L'},	// 开始监控日期时间
	{12, 	"Mem Glow(K)", 	'L'},	// 内存总上涨量
	{40, 	"Mem His(K)", 	'L'},	// 内存上涨历史(只填最近5)
	{-1,	"",			'0'},
};
static const int MAX_DT_LENGTH = 40;
static const int MAX_MEM_HIS = 40;

/* 补齐字符串 */
void _copy(std::string &ptr, Title_Format *format, const std::string &_value);
/* 补齐字符串 */
//void _copy(std::string &ptr, Title_Format *format, char* _c);
/* 对数字进行补齐 */
void _copy(std::string &ptr, Title_Format *format, int _value);

/*进程展示风格*/
enum TProcShowStyle
{
	ALLPROC = 'A',	//全部进程
	DUPPROC,		//重复启动的进程
	CLOSEPROC		//未运行的进程
};
extern TProcShowStyle ProcShowStyle;

/*启动状态*/
enum TStartStatus
{
	START_NO = 'N',	//未开始
	START_RUN,		//进行中
	START_FINISHED	//已完成
};
extern TStartStatus StartStatus;
//进程信息
class TProcInfo
{
public:
	int p_count;			// 进程个数
	int p_num;				// 进程编号
	int p_procid;			// 进程号

	char begmonidt[MAX_DT_LENGTH];  // 开始监控日期时间
	char mem_his[MAX_MEM_HIS];		 // 内存变化的历史
	int mem_init;		// 初始的内存
	int mem_last;		// 上次的内存
	int mem_glow;		// 内存增长

	//画面
	int m_formline;			//行数
	std::string m_content;	//内容

	TProcInfo() : m_formline(0), m_content("")
	{
		p_count = 0;
		p_num = 1;
		p_procid = 0;

		memset(begmonidt, 0, MAX_DT_LENGTH);
		memset(mem_his, 0, MAX_MEM_HIS);
		mem_init = 0;
		mem_last = 0;
		mem_glow = 0;
	}
	TProcInfo(int count,int num) : m_formline(0), m_content("")
	{
		p_count = count;
		p_num = num;
		p_procid = 0;

		memset(mem_his, 0, MAX_MEM_HIS);
		memset(begmonidt, 0, MAX_DT_LENGTH);
		memcpy(begmonidt, todayStr(), MAX_DT_LENGTH);
		mem_init = 0;
		mem_last = 0;
		mem_glow = 0;
	}
};

//供map排序和查找的key值类型
class TProcKey
{
public:
	std::string m_cmdline;	//启动命令
	int m_num;				//进程编号
	TProcKey()
		:m_cmdline(""), m_num(0)
	{
	}

	TProcKey(int num)
		:m_cmdline(""), m_num(num)
	{
	}

	TProcKey(const std::string &cmdline, int num)
		:m_cmdline(cmdline), m_num(num)
	{
	}

	~TProcKey()
	{
	}

};

bool operator<(const TProcKey &left, const TProcKey &right);
bool operator==(const TProcKey &left, const TProcKey &right);

//供命令行查找
std::map<TProcKey, TProcInfo>::iterator FindProc(std::map<TProcKey, TProcInfo> &promap, const std::string &cmdline);

std::map<TProcKey, TProcInfo> GetProcessListBySh(string sfile);
/* 进程编号是否存在并返回cmd命令行 */
bool existprocRtnCMD(std::map<TProcKey, TProcInfo> &processList, char *s_num,std::string &cmd);
/* 进程编号是否存在并返回进程id */
bool existprocRtnPID(std::map<TProcKey, TProcInfo> &processList, char *s_num,int &pid);

/* 启动进程 */
bool startprocess(std::map<TProcKey, TProcInfo> &processList, char *s_num);
/* 关闭进程 */
bool stopprocess(std::map<TProcKey, TProcInfo> &processList, char *s_num);

typedef std::map<TProcKey, TProcInfo>::iterator MapIter;
#define STATE_LENGTH 		64
#define PCT_LENGTH   		64
#define ARG_MAX   		64
#define	TVALU_TO_SEC(x)		(x.tv_sec + ((double)x.tv_usec / 1000000.0))
#define	TVALN_TO_SEC(x)		(x.tv_sec + ((double)x.tv_usec / 1000000000.0))
#define PROC_NAME_LEN 64
#define THREAD_NAME_LEN 32
#define MAX_LINE 256
#define die(...) { fprintf(stderr, __VA_ARGS__); exit(EXIT_FAILURE); }
struct proc_info {
	struct proc_info *next;
	pid_t pid; // 进程(包括轻量级进程，即线程)号
	pid_t tid;
	uid_t uid;
	gid_t gid;
	char name[PROC_NAME_LEN]; //启动命令行，例如query PSOQry_A 23113
	char tname[THREAD_NAME_LEN];
	char state;
	long unsigned utime;
	long unsigned stime;
	long unsigned delta_utime;
	long unsigned delta_stime;
	long unsigned delta_time;
	long vss;
	long rss; //实际占用物理内存，以页为单位存放
	int num_threads;
	char policy[32];
};
/************************************************************************/
/* 通过进程信息procs设置进程信息列表processList							*/
/************************************************************************/
void SetProcinfos(struct proc_info *procs, int userID, std::map<TProcKey, TProcInfo> &processList); //proc下的所有进程逐条赋值给procs，传入此函数，判断是否需要加入list
/************************************************************************/
/* 获取进程信息列表processList											*/
/************************************************************************/
bool GetProcessStatus(std::map<TProcKey, TProcInfo> &processList);

//读取进程状态，例如:filename = /proc/25971/task/25971/stat
int ReadProcStat(char *filename, struct proc_info *proc); 
//读取进程命令，例如:filename = /proc/25971/cmdline
int ReadProcCmdline(char *filename, struct proc_info *proc); 
//读取进程status,//filename = /proc/25971/status
int ReadProcStatus(char *filename, struct proc_info *proc); 
//////////////////////////////////////////////////////////////////////////////////////////////
//void ShowProcInfo(const MapIter& begin, const MapIter& end)
//{
//	MapIter iter = begin;
//	for (; iter != end; iter++)
//	{
//		std::cout << iter->first.m_num << ":" << iter->first.m_cmdline << std::endl;
//	}
//}
#endif

