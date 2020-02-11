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

//չʾ�����ʽ
struct Title_Format {
	int len;//����
	char title[128];//������
	char direct;//���뷽ʽ
};

static Title_Format TitleList[] = {
	{8, 	"PID", 		'L'},
	{6, 	"SN",		'L'},
	//{8, 	"TCNT",		'L'}, // �߳���
	{8, 	"State", 	'L'},
	{10,	"SZ(K)",		'L'},
	{10,	"RSS(K)",		'L'},
	{5, 	"CPU", 			'L'},
	{45, 	"CMDLine", 		'L'},
	{10, 	"BegMoniDT", 	'L'},	// ��ʼ�������ʱ��
	{12, 	"Mem Glow(K)", 	'L'},	// �ڴ���������
	{40, 	"Mem His(K)", 	'L'},	// �ڴ�������ʷ(ֻ�����5)
	{-1,	"",			'0'},
};
static const int MAX_DT_LENGTH = 40;
static const int MAX_MEM_HIS = 40;

/* �����ַ��� */
void _copy(std::string &ptr, Title_Format *format, const std::string &_value);
/* �����ַ��� */
//void _copy(std::string &ptr, Title_Format *format, char* _c);
/* �����ֽ��в��� */
void _copy(std::string &ptr, Title_Format *format, int _value);

/*����չʾ���*/
enum TProcShowStyle
{
	ALLPROC = 'A',	//ȫ������
	DUPPROC,		//�ظ������Ľ���
	CLOSEPROC		//δ���еĽ���
};
extern TProcShowStyle ProcShowStyle;

/*����״̬*/
enum TStartStatus
{
	START_NO = 'N',	//δ��ʼ
	START_RUN,		//������
	START_FINISHED	//�����
};
extern TStartStatus StartStatus;
//������Ϣ
class TProcInfo
{
public:
	int p_count;			// ���̸���
	int p_num;				// ���̱��
	int p_procid;			// ���̺�

	char begmonidt[MAX_DT_LENGTH];  // ��ʼ�������ʱ��
	char mem_his[MAX_MEM_HIS];		 // �ڴ�仯����ʷ
	int mem_init;		// ��ʼ���ڴ�
	int mem_last;		// �ϴε��ڴ�
	int mem_glow;		// �ڴ�����

	//����
	int m_formline;			//����
	std::string m_content;	//����

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

//��map����Ͳ��ҵ�keyֵ����
class TProcKey
{
public:
	std::string m_cmdline;	//��������
	int m_num;				//���̱��
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

//�������в���
std::map<TProcKey, TProcInfo>::iterator FindProc(std::map<TProcKey, TProcInfo> &promap, const std::string &cmdline);

std::map<TProcKey, TProcInfo> GetProcessListBySh(string sfile);
/* ���̱���Ƿ���ڲ�����cmd������ */
bool existprocRtnCMD(std::map<TProcKey, TProcInfo> &processList, char *s_num,std::string &cmd);
/* ���̱���Ƿ���ڲ����ؽ���id */
bool existprocRtnPID(std::map<TProcKey, TProcInfo> &processList, char *s_num,int &pid);

/* �������� */
bool startprocess(std::map<TProcKey, TProcInfo> &processList, char *s_num);
/* �رս��� */
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
	pid_t pid; // ����(�������������̣����߳�)��
	pid_t tid;
	uid_t uid;
	gid_t gid;
	char name[PROC_NAME_LEN]; //���������У�����query PSOQry_A 23113
	char tname[THREAD_NAME_LEN];
	char state;
	long unsigned utime;
	long unsigned stime;
	long unsigned delta_utime;
	long unsigned delta_stime;
	long unsigned delta_time;
	long vss;
	long rss; //ʵ��ռ�������ڴ棬��ҳΪ��λ���
	int num_threads;
	char policy[32];
};
/************************************************************************/
/* ͨ��������Ϣprocs���ý�����Ϣ�б�processList							*/
/************************************************************************/
void SetProcinfos(struct proc_info *procs, int userID, std::map<TProcKey, TProcInfo> &processList); //proc�µ����н���������ֵ��procs������˺������ж��Ƿ���Ҫ����list
/************************************************************************/
/* ��ȡ������Ϣ�б�processList											*/
/************************************************************************/
bool GetProcessStatus(std::map<TProcKey, TProcInfo> &processList);

//��ȡ����״̬������:filename = /proc/25971/task/25971/stat
int ReadProcStat(char *filename, struct proc_info *proc); 
//��ȡ�����������:filename = /proc/25971/cmdline
int ReadProcCmdline(char *filename, struct proc_info *proc); 
//��ȡ����status,//filename = /proc/25971/status
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

