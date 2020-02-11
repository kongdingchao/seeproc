#include "ProcInfo.h"
#include "setting.h"
#include <unistd.h>
#include <stdlib.h>
#include<sys/time.h>
#include <ctype.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>

TProcShowStyle ProcShowStyle = ALLPROC;
TStartStatus StartStatus = START_NO;

bool operator<(const TProcKey &left, const TProcKey &right)
{
	return left.m_num < right.m_num;
}

bool operator==(const TProcKey &left, const TProcKey &right)
{
	return left.m_num == right.m_num;
}

//供命令行查找
std::map<TProcKey, TProcInfo>::iterator FindProc(std::map<TProcKey, TProcInfo> &promap, const std::string &cmdline)
{
	std::map<TProcKey, TProcInfo>::iterator iter = promap.begin();
	for (; iter != promap.end(); iter++)
	{
		const TProcKey &key = iter->first;
		if (key.m_cmdline == cmdline)
			break;
	}

	return iter;
}

std::map<TProcKey, TProcInfo> GetProcessListBySh(string sfile)
{
	std::map<TProcKey, TProcInfo> proclist;
	// 读取文件
	if (sfile != "")
	{
		int p_num = 1;
		std::string buffer;
		std::ifstream infFile((char *)sfile.c_str());
		if (infFile.fail())
		{
			printf("打开文件[%s]失败\n", sfile.c_str());
			exit(-1);
		}
		infFile.seekg(std::ios::beg);
		while (!infFile.eof())
		{
			buffer = "";
			getline(infFile, buffer, '\n');
			trim_more(buffer);
			if (buffer.length() > 0 && buffer[0] != '#' && buffer.substr(0,6) != "sleep ")
			{
				if (FindProc(proclist, buffer) == proclist.end())
				{
					TProcKey key(buffer, p_num);
					TProcInfo pi(0,p_num++);
					proclist.insert(std::map<TProcKey, TProcInfo>::value_type(key, pi));
				}
			}
		}
		infFile.close();
	}
	return proclist;
}


/* 补齐字符串 */
void _copy(std::string &ptr, Title_Format *format, const std::string &_value)
{
	int del = format->len - _value.length();
	if (del == 0)
		ptr.append(_value);
	else if (del < 0)
		ptr.append(_value.substr(0, format->len));
	else {
		if (format->direct == 'L')
			ptr.append(_value).append(del, ' ');
		else if (format->direct == 'R')
			ptr.append(del, ' ').append(_value);
		else
			ptr.append(del / 2, ' ').append(_value).append(del - del / 2, ' ');
	}
}

/* 补齐字符串 */
//void _copy2(std::string &ptr, Title_Format *format, char* _c)
//{
//	std::string	_value(_c);
//	//sprintf(_value.c_str(), "%s", _c);
//	int del = format->len - _value.length();
//	if (del == 0)
//		ptr.append(_value);
//	else if (del < 0)
//		ptr.append(_value.substr(0, format->len));
//	else {
//		if (format->direct == 'L')
//			ptr.append(_value).append(del, ' ');
//		else if (format->direct == 'R')
//			ptr.append(del, ' ').append(_value);
//		else
//			ptr.append(del / 2, ' ').append(_value).append(del - del / 2, ' ');
//	}
//	ptr.append(std::string("|"));
//}


/* 对数字进行补齐 */
void _copy(std::string &ptr, Title_Format *format, int _value)
{
	char _p[16];
	memset(_p, 0, 16);
	sprintf(_p, "%ld", _value);
	return _copy(ptr, format, _p);
}

/* 进程编号是否存在 */
bool existprocRtnCMD(std::map<TProcKey, TProcInfo> &processList, char *s_num,std::string &cmd)
{
	char tmp[50];
	for (MapIter iter = processList.begin(); iter != processList.end(); iter ++)
	{
		sprintf(tmp,"%d",iter->second.p_num);
		if (strcmp(tmp,s_num) == 0 && iter->second.p_count <= 0)
		{
			TProcKey key = iter->first;
			cmd = key.m_cmdline;
			return true;
		}
	}
	return false;
}

/* 进程编号是否存在 */
bool existprocRtnPID(std::map<TProcKey, TProcInfo> &processList, char *s_num,int &pid)
{
	char tmp[50];
	for (MapIter iter = processList.begin(); iter != processList.end(); iter ++)
	{
		sprintf(tmp,"%d",iter->second.p_num);
		if (strcmp(tmp,s_num) == 0 && iter->second.p_count > 0)
		{
			pid = iter->second.p_procid;
			return true;
		}
	}
	return false;
}
/* 启动进程 */
bool startprocess(std::map<TProcKey, TProcInfo> &processList, char *s_num)
{
	std::string cmd;
	if (!existprocRtnCMD(processList,s_num,cmd))
		return false;
	char sshell[1024];
	memset(sshell,0,1024);
	if (G_Settings.localPath != "")
	{
		sprintf(sshell,"cd %s\n%s\n",G_Settings.localPath.c_str(),cmd.c_str());
		popen(sshell,"r");
	}
	else
	{
		sprintf(sshell,"%s\n",cmd.c_str());
		popen(sshell,"r");
	}
	return true;
}

/* 关闭进程 */
bool stopprocess(std::map<TProcKey, TProcInfo> &processList, char *s_num)
{
	int procid;
	if (!existprocRtnPID(processList,s_num,procid))
		return false;
	char sshell[1024];
	memset(sshell,0,1024);
	if (G_Settings.localPath != "")
	{
		sprintf(sshell,"cd %s\nkill -9 %s\n",G_Settings.localPath.c_str(),procid);
		popen(sshell,"r");
	}
	else
	{
		sprintf(sshell,"kill -9 %d\n",procid);
		popen(sshell,"r");
	}
	return true;
}

void SetProcinfos(struct proc_info *procs, int userID, std::map<TProcKey, TProcInfo> &processList) //proc下的所有进程逐条赋值给procs，传入此函数，判断是否需要加入list
{
	static const int fixline = 5;//固定开始的行
	struct proc_info *curproc = NULL;
	char state[STATE_LENGTH];
	char cmdline[ARG_MAX];
	char pctcpu[PCT_LENGTH];
	char s_num[12];

	long utime, stime, cutime, cstime;
	int index = 0;

	curproc = procs;
	//过滤非当前用户下的进程
	if (userID != curproc->uid)
		return;

	// command args
	std::string strCmdLine = curproc->name;  //将启动命令赋值给strCmdLine
	MapIter pallitem = FindProc(processList, strCmdLine);
	if (pallitem == processList.end())
	{
		return;
	}
	TProcInfo *pProc = &(pallitem->second);
	pProc->p_procid = curproc->pid;
	//pProc->Status = PIS_STARTED;
	pProc->p_count = pProc->p_count + 1;

	memset(state, 0, STATE_LENGTH);
	std::string outstr;
	// presume we are not a zombie 
	int zombie = 0;
	switch (curproc->state) 
	{
	case 'R':
		strncpy(state, "RUN", 3); break; //可执行状态TASK_RUNNING
	case 'S':
		strncpy(state, "SLEEP", 5); break; //可中断的睡眠状态TASK_INTERRUPTIBLE。含义：可中断的睡眠状态的进程会睡眠直到某个条件变为真
	case 'D':
		strncpy(state, "TASK_UNINTERRUPTIBLE", 4); break; //不可中断的睡眠状态
	case 'T':
		strncpy(state, "STOP", 4); break; //暂停状态或跟踪状态TASK_STOPPED 
	case 'Z':
		strncpy(state, "ZOMBIE", 6); zombie = 1; break; //退出状态，进程成为僵尸进程TASK_DEAD – EXIT_ZOMBIE
	case 'X':
		strncpy(state, "EXIT_DEAD", 3); break; //退出状态，进程即将被销毁TASK_DEAD
	default:
		strncpy(state, "NON", 3); break;
		break;
	}

	if (zombie) {
		utime = curproc->utime;
		stime = curproc->stime;
	}
	else {
		struct timeval now_tval;
		gettimeofday(&now_tval, 0);
		G_Settings.nowTime = TVALU_TO_SEC(now_tval);
		utime = TVALN_TO_SEC(now_tval);
		stime = TVALN_TO_SEC(now_tval);
		cutime = TVALN_TO_SEC(now_tval);
		cstime = TVALN_TO_SEC(now_tval);
	}

	//calc pctcpu
	memset(pctcpu, 0, PCT_LENGTH);
	snprintf(pctcpu, sizeof(pctcpu),"%3.2f", ((utime + stime) * 100 / (G_Settings.nowTime - curproc->utime)) / G_Settings.ncpus); //记录CPU使用情况, 此行写法待考证

	//calc pctmem
	//这里curproc->rss表示实际物理内存占用了几页，g_pagesize页大小的单位是Byte
	int rss_size = curproc->vss * G_Settings.pageSize / 1024; //虚拟内存大小(kb)  这个值取的偏小
	int mem_size = curproc->rss * G_Settings.pageSize / 1024; //内存大小(kb)

	// 判断内存情况
	if (pProc->mem_last == 0)
	{
		pProc->mem_init = mem_size;
		pProc->mem_last = mem_size;
	}
	else
	{
		// 记录mem变动历史
		int cur_mem_delta = mem_size - pProc->mem_last;
		char cur_delta_buff[20] = {0};
		snprintf(cur_delta_buff,sizeof(cur_delta_buff),"%d;",cur_mem_delta);

		if(cur_mem_delta > 0)
		{
			if (strlen(pProc->mem_his) + strlen(cur_delta_buff)  >= MAX_MEM_HIS)
			{
				memset(pProc->mem_his,0,MAX_MEM_HIS);
				strcpy(pProc->mem_his, cur_delta_buff);
			}
			else
			{
				strcat(pProc->mem_his,cur_delta_buff);
			}
		}
		pProc->mem_last = mem_size;
		pProc->mem_glow = mem_size - pProc->mem_init;
	}
	if (pProc->p_count > 1)
		sprintf(s_num,"%d*%d", pProc->p_num, pProc->p_count);
	else
		sprintf(s_num,"%d", pProc->p_num);

	//当前界面该命令所在的位置
	int curPos = distance(processList.begin(), pallitem);
	int curformline = fixline + curPos;

	_copy(outstr, &TitleList[0], curproc->pid);
	_copy(outstr, &TitleList[1], s_num);
	//_copy3(outstr, &TitleList[2], curproc->pi_thcount);
	_copy(outstr, &TitleList[2], state);
	_copy(outstr, &TitleList[3], mem_size);
	_copy(outstr, &TitleList[4], rss_size);
	_copy(outstr, &TitleList[5], pctcpu);
	_copy(outstr, &TitleList[6], strCmdLine);
	//_copy(outstr, &TitleList[8], std::string(pitem->second.begmonidt));
	//_copy(outstr, &TitleList[9], std::string(pitem->second.mem_his));
	_copy(outstr, &TitleList[7], pProc->begmonidt);
	_copy(outstr, &TitleList[8], pProc->mem_glow);
	_copy(outstr, &TitleList[9], pProc->mem_his);

	//暂时存储画面
	pProc->m_formline = curformline;
	pProc->m_content = outstr;
}


//先将processList里的每个进程状态置为ended，再去/proc里的cmdline是否在ProcessList里存在，存在的话，将该进程置为已启动
bool GetProcessStatus(std::map<TProcKey, TProcInfo> &processList) 
{
	try
	{
		if (0 == processList.size())
		{
			return false;
		}
		//预先进程数重置
		std::map<TProcKey, TProcInfo>::iterator iter = processList.begin();
		for (; iter != processList.end(); iter++)
		{
			TProcInfo &proc = iter->second;
			
			proc.p_count = 0;
		}

		//获取进程信息设置到processList中去
		DIR *proc_dir, *task_dir;
		struct dirent *pid_dir, *tid_dir;
		char filename[64];
		FILE *file;
		int proc_num;
		struct proc_info *procs = NULL;
		pid_t pid, tid;

		proc_dir = opendir("/proc");
		if (!proc_dir) die("Could not open /proc.\n");

		proc_num = 0;		
		while ((pid_dir = readdir(proc_dir))) //读取/proc目录里的文件
		{
			if (!isdigit(pid_dir->d_name[0]))
				continue;

			pid = atoi(pid_dir->d_name);
			if(procs == NULL)
			{
				procs = new(struct proc_info);
				if (!procs) 
					die("Could not allocate struct process_info.\n");
			}

			procs->pid = procs->tid = pid;

			//进程ID文件夹下还有进程的stat，里面有程序名，例如checkiosvr_o		
			memset(filename, 0, 64);
			snprintf (filename, sizeof(filename), "/proc/%d/stat", pid);
			ReadProcStat(filename, procs); //给procs的各个time、stat字段赋值

			memset(filename, 0, 64);
			snprintf (filename, sizeof(filename), "/proc/%d/cmdline", pid);
			ReadProcCmdline(filename, procs); //procs->name 赋值为query PSOQry_A 23113

			memset(filename, 0, 64);
			snprintf (filename, sizeof(filename), "/proc/%d/status", pid);
			ReadProcStatus(filename, procs);  //给procs->uid, procs->gid赋值

			procs->num_threads = 0;

			//将proc放到list里去
			SetProcinfos(procs, G_Settings.userId, processList);
		}
		closedir(proc_dir);

		if (NULL != procs)
		{
			delete procs;
			procs = NULL;
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}


int ReadProcStat(char *filename, struct proc_info *proc) //例如:filename = /proc/25971/task/25971/stat
{
	FILE *file;
	char buf[MAX_LINE], *open_paren, *close_paren;
	int res, idx;

	file = fopen(filename, "r");
	if (!file) return 1;
	fgets(buf, MAX_LINE, file);
	fclose(file);

	/* Split at first '(' and last ')' to get process name. */
	open_paren = strchr(buf, '(');
	close_paren = strrchr(buf, ')');
	if (!open_paren || !close_paren) return 1;

	*open_paren = *close_paren = '\0';
	strncpy(proc->tname, open_paren + 1, THREAD_NAME_LEN);
	proc->tname[THREAD_NAME_LEN - 1] = 0;

	/* Scan rest of string. */
	sscanf(close_paren + 1, " %c %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d "
		"%lu %lu %*d %*d %*d %*d %*d %*d %*d %lu %ld",
		&proc->state, &proc->utime, &proc->stime, &proc->vss, &proc->rss);

	return 0;
}

int ReadProcCmdline(char *filename, struct proc_info *proc) //例如:filename = /proc/25971/cmdline
{
	FILE *file;
	char line[MAX_LINE];
	line[0] = '\0';

	std::ifstream f1;
	std::string str;
	std::string cmd;
	memset(line, 0, MAX_LINE);
	line[0] = '\0';

	f1.open(filename, std::ios::in);
	if (!f1.is_open())
	{
		return -1;
	}

	try
	{
		while((f1.getline(line, MAX_LINE, '\0')))
		{
			if (cmd.empty()) //起始位置不能有空格
			{
				cmd = std::string(line);
			}
			else
			{
				cmd += " " + std::string(line);
			}
		}
	}
	catch(...)
	{}
	f1.close();

	if (strlen(cmd.c_str()) > 0) 
	{
		strncpy(proc->name, cmd.c_str(), PROC_NAME_LEN); //PROC_NAME_LEN = 64
		proc->name[PROC_NAME_LEN - 1] = 0;
	}
	else
		proc->name[0] = 0;
	return 0;
}

int ReadProcStatus(char *filename, struct proc_info *proc) //filename = /proc/25971/status
{
	FILE *file;
	char line[MAX_LINE];
	unsigned int uid, gid;

	file = fopen(filename, "r");
	if (!file) return 1;
	while (fgets(line, MAX_LINE, file)) 
	{
		sscanf(line, "Uid: %u", &uid);
		sscanf(line, "Gid: %u", &gid);
	}
	fclose(file);
	proc->uid = uid; proc->gid = gid;
	return 0;
}