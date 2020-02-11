#include "MainFrame.h"
#include "setting.h"
#include <pthread.h>
#include<sys/time.h>

MainFrame::MainFrame(void)
{
	//读取全量程序
	AllProcessList = GetProcessListBySh(G_Settings.shellName);

	//根据展示风格设置进程列表
	ProcessList.clear();
	for (MapIter iter = AllProcessList.begin(); iter != AllProcessList.end(); iter ++)
	{
		if (ProcessList.size() < G_Settings.pageCount)
		{
			if (ProcShowStyle == CLOSEPROC && iter->second.p_count != 0)
			{
				continue;
			}
			if (ProcShowStyle == DUPPROC && iter->second.p_count <= 1)
			{
				continue;
			}
			ProcessList.insert(*iter);
		}
	}

	G_Settings.posHint = COLS / 2 + 30;

	win1 = NULL;
	win2 = NULL;
	win3 = NULL;
}


MainFrame::~MainFrame(void)
{
}
/* 打印和移动光标到提醒位置 */
void mvaddstrAndMoveHint(const string &content)
{
	mvaddstr(7 + G_Settings.pageCount, G_Settings.posHint, content.c_str());
	move(7 + G_Settings.pageCount, G_Settings.posHint);
}

/* 画指定的窗口大小 */
void winBOX(WINDOW *win, int startx, int starty)
{
	int X = 0, Y = 0, i = 0, LEN = 0;
	char    Across[]    = "─";      //横线
	char    Erect[]     = "│";      //竖线
	char    Upleft[]    = "┌";      //上左
	char    Upright[]   = "┐";      //上右
	char    Downleft[]  = "└";      //下左
	char    Downright[] = "┘";      //下右

	LEN = win->_maxx / 2 - 1;          //去掉两边的横线
	//上方的表格线
	for (i = 0;i < LEN; i++)
		mvwprintw(win, 0, i * 2, "%s", Across);
	mvwprintw(win, 0, 0, "%s", Upleft);
	mvwprintw(win, 0, win->_maxx - 2, "%s", Upright);

	//中间的竖线
	for (i = 1;i < win->_maxy - 1; i++) {
		mvwprintw(win, i, 0, "%s", Erect);
		mvwprintw(win, i, win->_maxx - 2, "%s", Erect);
	}

	//下方的竖线
	for (i = 0;i < LEN; i++)
		mvwprintw(win, win->_maxy - 1, i * 2, "%s", Across);
	mvwprintw(win, win->_maxy - 1, 0, "%s", Downleft);
	mvwprintw(win, win->_maxy - 1, win->_maxx - 2, "%s", Downright);
	wrefresh(win);
}
void MainFrame::InitFrame()
{
	initscr();
	cbreak();
	nonl();
	noecho();
	intrflush(stdscr, false);
	keypad(stdscr, true);
	refresh();
}

void MainFrame::DestroyFrame()
{
	keypad(stdscr, false);
	endwin();
	exit(0);
}

void MainFrame::PrintFrame()
{
	win1 = newwin(3, COLS - 2, 2, 0);
	winBOX(win1, 0, 0);

	win2 = newwin(5 + G_Settings.pageCount, COLS - 2, 1, 0);
	winBOX(win2, 0, 0);

	win3 = newwin(3, COLS - 2, 6 + G_Settings.pageCount, 0);
	winBOX(win3, 0, 0);

	// 开启反白模式
	attron(A_REVERSE);
	mvaddstr(1, COLS / 2 - 20, "时 代 银 通 系 统 监 控 终 端V3.0");
	attroff(A_REVERSE);

	attron(A_UNDERLINE);

	mvprintw(2, 2, "版权所有:时代银通 本机内存: CPU:%d", G_Settings.ncpus);
	attroff(A_UNDERLINE);

	mvaddstr(7 + G_Settings.pageCount, 3, "help: h:help n:next b:back s:search o:open c:close tab:All/NoRun `:All/DupRun q(ESC):exit");

	mvprintw(2, COLS - 20, "ViewMode:%6s", " ");
	if (ProcShowStyle == DUPPROC)
		mvprintw(2, COLS - 20, "ViewMode:DupRun");
	else if (ProcShowStyle == CLOSEPROC)
		mvprintw(2, COLS - 20, "ViewMode:NoRun");
	else
		mvprintw(2, COLS - 20, "ViewMode:All");

	char head_buff[2048] = {0};
	for(int index = 0; ; index ++)
	{
		if(TitleList[index].len == -1)
		{
			break;
		}
		strcat(head_buff,TitleList[index].title);
		restrcat_word(head_buff,' ',TitleList[index].len - strlen(TitleList[index].title));
	}
	mvprintw(4, 2, "%s", head_buff);
	move(7 + G_Settings.pageCount, G_Settings.posHint);
	refresh();
}

void MainFrame::PrintProcInfo()
{
	//按照顺序打印
	MapIter iterProc = ProcessList.begin();
	for (; iterProc != ProcessList.end(); iterProc++)
	{
		int formline = iterProc->second.m_formline;
		std::string strcontent = iterProc->second.m_content;
		if (formline != 0 && iterProc->second.p_count != 0)
		{
			mvaddstr(formline, 2, strcontent.c_str());
		}
	}
}

void MainFrame::StartRun()
{
	pthread_t pt;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 3 * 1024 * 1024);
	if (pthread_create(&pt, &attr, &MainFrame::ExecuteSeeProcThread, (void *)this))
	{
		attron(A_BLINK);
		mvaddstr(4, 30, "创建监控线程失败,系统退出...");
		attroff(A_BLINK);
		exit(-1);
	}
	pthread_attr_destroy(&attr);
}

void GetStartShellTime(const std::string &filename, std::string &starttime, std::string &endtime)
{
	if (filename != "")
	{
		std::string buffer;
		std::ifstream infFile((char *)filename.c_str());
		if (infFile.fail() == false)
		{
			infFile.seekg(std::ios::beg);
			while (!infFile.eof())
			{
				buffer = "";
				getline(infFile, buffer, '\n');
				trim_more(buffer);
				int pos_starttime = buffer.find("starttime=");
				int pos_endtime = buffer.find("endtime=");
				if (buffer.length() > 0 && pos_starttime != std::string::npos)
				{
					starttime = buffer.substr(strlen("starttime="), 8);
				}
				else if (buffer.length() > 0 && pos_endtime != std::string::npos)
				{
					endtime = buffer.substr(strlen("endtime="), 8);
				}
			}
			infFile.close();
		}
	}
}

void *MainFrame::ExecuteSeeProcThread(void *owner)
{
	int timeout = G_Settings.timeout;
	struct timeval now_tval;
	std::string titlestr, str;
	MainFrame &frame = *((MainFrame *)owner);
	while (1) {
		const int fixFormLine = 5;//固定开始的行
		int index = 0, fromline = fixFormLine;
		int totalp = 0,localp = 0;
		int totalp_dup = 0;
		int coutProcessList = 0;

		for (MapIter iter = frame.AllProcessList.begin(); iter != frame.AllProcessList.end(); iter ++)
			iter->second.p_count = 0;

		for (MapIter iter = frame.ProcessList.begin(); iter != frame.ProcessList.end(); iter ++)
		{
			iter->second.p_count = 0;
			iter->second.m_formline = 0;
			iter->second.m_content = "";
		}

		// 清空表单
		while (fromline < fixFormLine + G_Settings.pageCount)
		{
			char snull[255];
			//sprintf(snull,"%106s"," ");
			sprintf(snull,"%-*s", COLS - 8, " ");
			mvaddstr(fromline, 3, snull);
			fromline++;
		}

		coutProcessList = frame.ProcessList.size();

		// get current time of day
		gettimeofday(&now_tval, 0);
		G_Settings.nowTime = TVALU_TO_SEC(now_tval);
		int count = 0;

		GetProcessStatus(frame.ProcessList);

		if (ALLPROC == ProcShowStyle)
		{
			frame.PrintProcInfo();
		}

		for (MapIter iter = frame.AllProcessList.begin(); iter != frame.AllProcessList.end(); iter ++) {
			if (iter->second.p_count == 0)
				totalp++;
			else if (iter->second.p_count > 1)
				totalp_dup++;
		}

		//展示重复启动的进程
		if (DUPPROC == ProcShowStyle)
		{
			for (MapIter iter = frame.ProcessList.begin(); iter != frame.ProcessList.end(); iter ++)
			{
				if (iter->second.p_count > 1)
				{
					titlestr.clear();
					char s_num[12];
					memset(s_num,0,12);
					sprintf(s_num,"%d*%d",iter->second.p_num,iter->second.p_count);
					_copy(titlestr, &TitleList[0], "/");
					_copy(titlestr, &TitleList[1], s_num);
					_copy(titlestr, &TitleList[2], "NA");
					_copy(titlestr, &TitleList[3], "/");
					_copy(titlestr, &TitleList[4], "/");
					_copy(titlestr, &TitleList[5], "/");
					_copy(titlestr, &TitleList[6], (char *)iter->first.m_cmdline.c_str());

					//memset(iter->second.begmonidt, 0, MAX_DT_LENGTH);
					//iter->second.begmonidt[0] = '\0';
					_copy(titlestr, &TitleList[7], "/");

					iter->second.mem_last = 0;
					iter->second.mem_glow = 0;
					_copy(titlestr, &TitleList[8], iter->second.mem_glow);

					memset(iter->second.mem_his, 0, MAX_MEM_HIS);
					iter->second.mem_his[0] = '\0';
					_copy(titlestr, &TitleList[9], "/");

					int curline = fixFormLine + distance(frame.ProcessList.begin(), iter);
					mvaddstr(curline, 2, titlestr.c_str());
					localp++;
				}
			}
		}
		else
		{
			for (MapIter iter = frame.ProcessList.begin(); iter != frame.ProcessList.end(); iter ++)
			{
				if (iter->second.p_count == 0)
				{
					titlestr.clear();
					char s_num[12];
					memset(s_num,0,12);
					sprintf(s_num,"%d",iter->second.p_num);
					_copy(titlestr, &TitleList[0], "/");
					_copy(titlestr, &TitleList[1], s_num);
					_copy(titlestr, &TitleList[2], "NA");
					_copy(titlestr, &TitleList[3], "/");
					_copy(titlestr, &TitleList[4], "/");
					_copy(titlestr, &TitleList[5], "/");
					_copy(titlestr, &TitleList[6], (char *)iter->first.m_cmdline.c_str());

					//memset(iter->second.begmonidt, 0, MAX_DT_LENGTH);
					//iter->second.begmonidt[0] = '\0';
					_copy(titlestr, &TitleList[7], "/");

					iter->second.mem_last = 0;
					iter->second.mem_glow = 0;
					_copy(titlestr, &TitleList[8], iter->second.mem_glow);

					memset(iter->second.mem_his, 0, MAX_MEM_HIS);
					iter->second.mem_his[0] = '\0';
					_copy(titlestr, &TitleList[9], "/");

					int curline = fixFormLine + distance(frame.ProcessList.begin(), iter);
					mvaddstr(curline, 2, titlestr.c_str());
					localp++;
				}
			}
		}

		int countProc = frame.AllProcessList.size() - totalp;
		int countAllProc = frame.AllProcessList.size();

		//int countCurPageProc = ProcShowStyle == ALLPROC ? coutProcessList - localp : localp;
		G_Settings.totalPages  = frame.AllProcessList.size()/G_Settings.pageCount + 1;

		std::string strShowStyle = "Run";
		if (ProcShowStyle == CLOSEPROC)
		{
			countProc = totalp;
			G_Settings.totalPages = countProc/G_Settings.pageCount + 1;
			strShowStyle = "NoRun";
		}
		else if (ProcShowStyle == DUPPROC)
		{
			countProc = totalp_dup;
			G_Settings.totalPages = countProc/G_Settings.pageCount + 1;
			strShowStyle = "DupRun";
		}
		move(5 + G_Settings.pageCount, 2);
		printw("进程情况(%6s/All):%3d/%d 页码(%03d/%03d) 系统时间:%8s",
			strShowStyle.c_str(), countProc, countAllProc, G_Settings.localPage, G_Settings.totalPages, todayStr());

		/*printw("全汇总:%2d/%2d 当前页(%2d):%2d/%2d 总页数:%2d 每页数:%2d 系统时间:%s",
		countProc, countAllProc, localpage, countCurPageProc, coutProcessList,
		pages, page_count, todayStr());*/
		attroff(A_UNDERLINE);
		move(7 + G_Settings.pageCount, G_Settings.posHint);

		std::string starttime, endtime;
		//ProStartTime = "";
		GetStartShellTime(G_Settings.shellName + ".time", starttime, endtime);
		if (starttime == "")
			StartStatus = START_NO;
		else if (endtime == "")
		{
			StartStatus = START_RUN;
		}
		else// if (endtime == ProEndTime)
			StartStatus = START_FINISHED;

		if (START_NO == StartStatus)
			mvprintw(2, COLS - 60, "IsStarted:UnCerten  TimeStamp:%8s", starttime.c_str());
		else if (START_RUN == StartStatus)
			mvprintw(2, COLS - 60, "IsStarted:InProcess TimeStamp:%8s", starttime.c_str());
		else if (START_FINISHED == StartStatus)
			mvprintw(2, COLS - 60, "IsStarted:Finished  TimeStamp:%8s", endtime.c_str());
		move(7 + G_Settings.pageCount, G_Settings.posHint);
		refresh();

		sleep(timeout);
	}
	return NULL;
}

void MainFrame::HandleInput()
{
	G_Settings.posHint = COLS / 2 + 30;
	move(7 + G_Settings.pageCount, G_Settings.posHint);
	char s_num[50];
	char s_cmdline[50];
	char s_endtime[50];
	char s_tmp[1024];
	char s_tmp1[1024];
	do {
		// 等待自键盘输入字元
		int ch = getch();

		//提示栏清空
		memset(s_num,0,50);
		memset(s_cmdline,0,50);
		memset(s_endtime,0,50);
		memset(s_tmp,0,1024);
		memset(s_tmp1,0,1024);

		sprintf(s_tmp,"%-*s",COLS/2-30,"");
		mvaddstrAndMoveHint(s_tmp);

		switch (ch) {
		case 'q' 	:
		case 27		:
			endwin();
			exit(0);
		case 's'	:
		case 'S'	://命令匹配查找进程
			{
				sprintf(s_tmp,"%-*s",COLS/2-30,"输入进程命令(按ENTER结束)");
				mvaddstrAndMoveHint(s_tmp);
				refresh();
				scanf("%s",s_cmdline);
				sprintf(s_tmp1,"输入的进程命令为:%s 请确认(Y/n)",s_cmdline);
				sprintf(s_tmp,"%-*s",COLS/2-30,s_tmp1);
				mvaddstrAndMoveHint(s_tmp);
				refresh();
				ch = getch();
				if (ch == 'Y'|| ch == 'y')
				{
					//匹配进程
					ProcessList.clear();
					G_Settings.localPage = 1;//回到第一页
					for (MapIter iter = AllProcessList.begin(); iter != AllProcessList.end(); iter++)
					{
						if (ProcessList.size() < G_Settings.pageCount)
						{
							//展示没有运行的程序风格的时候，过滤已经运行的进程
							if (ProcShowStyle == CLOSEPROC && iter->second.p_count != 0)
							{
								continue;
							}

							if (ProcShowStyle == DUPPROC && iter->second.p_count <= 1)
							{
								continue;
							}

							if (std::string::npos != iter->first.m_cmdline.find(s_cmdline))
							{
								ProcessList.insert(*iter);
							}
						}
					}

					sprintf(s_tmp1,"进程:%s 搜索成功",s_cmdline);
					sprintf(s_tmp,"%-*s",COLS/2-30,s_tmp1);
					mvaddstrAndMoveHint(s_tmp);
				}
				else
				{
					sprintf(s_tmp,"%-*s",COLS/2-30," ");
					mvaddstrAndMoveHint(s_tmp);
				}
			}
			break;
		case 'o'	:
		case 'O'	:
			{
				sprintf(s_tmp,"%-*s",COLS/2-30,"输入开启的进程编号(按ENTER结束)");
				mvaddstrAndMoveHint(s_tmp);
				refresh();
				scanf("%s",s_num);
				sprintf(s_tmp1,"输入的进程编号为:%s 请确认(Y/n)",s_num);
				sprintf(s_tmp,"%-*s",COLS/2-30,s_tmp1);
				mvaddstrAndMoveHint(s_tmp);
				refresh();
				ch = getch();
				if (ch == 'Y'|| ch == 'y')
				{
					bool isNum = true;
					int n_num;
					try
					{
						n_num = atoi(s_num);
					}
					catch (...)
					{
						isNum = false;
					}

					// 启动进程
					if (isNum && startprocess(AllProcessList, s_num))
					{
						sprintf(s_tmp1,"进程编号:%s 启动完成",s_num);
						sprintf(s_tmp,"%-*s",COLS/2-30,s_tmp1);
						mvaddstrAndMoveHint(s_tmp);

						//更新启动时间
						TProcKey key(n_num);
						MapIter iterProc = AllProcessList.find(key);
						if (iterProc != AllProcessList.end())
						{
							memset(iterProc->second.begmonidt, 0, MAX_DT_LENGTH);
							memcpy(iterProc->second.begmonidt, todayStr(), MAX_DT_LENGTH);
						}

						iterProc = ProcessList.find(key);
						if (iterProc != ProcessList.end())
						{
							memset(iterProc->second.begmonidt, 0, MAX_DT_LENGTH);
							memcpy(iterProc->second.begmonidt, todayStr(), MAX_DT_LENGTH);
						}
					}
					// 进程已经存在
					else
					{
						sprintf(s_tmp1,"进程编号:%s 已经启动或不存在",s_num);
						sprintf(s_tmp,"%-*s",COLS/2-30,s_tmp1);	
						mvaddstrAndMoveHint(s_tmp);
					}
				}
				else
				{
					sprintf(s_tmp,"%-*s",COLS/2-30," ");
					mvaddstrAndMoveHint(s_tmp);
				}
			}
			break;
		case 'c'	:
		case 'C'	:
			{
				sprintf(s_tmp,"%-*s",COLS/2-30,"输入关闭的进程编号(按ENTER结束)");
				mvaddstrAndMoveHint(s_tmp);
				refresh();
				scanf("%s",s_num);
				sprintf(s_tmp1,"输入的进程编号为:%s 请确认(Y/n)",s_num);
				sprintf(s_tmp,"%-*s",COLS/2-30,s_tmp1);
				mvaddstrAndMoveHint(s_tmp);
				refresh();
				ch = getch();
				if (ch == 'Y'|| ch == 'y')
				{
					// 关闭进程
					if (stopprocess(AllProcessList, s_num))
					{
						sprintf(s_tmp1,"进程编号:%s 关闭完成",s_num);
						sprintf(s_tmp,"%-*s",COLS/2-30,s_tmp1);
						mvaddstrAndMoveHint(s_tmp);
					}
					// 进程已经存在
					else
					{
						sprintf(s_tmp1,"进程编号:%s 未启动或不存在",s_num);
						sprintf(s_tmp,"%-*s",COLS/2-30,s_tmp1);
						mvaddstrAndMoveHint(s_tmp);
					}
				}
				else
				{
					sprintf(s_tmp,"%-*s",COLS/2-30," ");
					mvaddstrAndMoveHint(s_tmp);
				}
			}
			break;
		case 'h'	:
			{
				sprintf(s_tmp,"%-*s",COLS/2-30,"-i 刷新间隔 -m 单页显示个数 -p 启动命令列表 -l 程序启动目录");
				mvaddstr(7 + G_Settings.pageCount, G_Settings.posHint, s_tmp);
			}	
			break;
		case 'n'	:
			{
				// 存在下一页
				if (G_Settings.localPage < G_Settings.totalPages)
				{
					int localpos = ProcessList.size() + (G_Settings.localPage-1)*G_Settings.pageCount;
					int pos = 0;
					ProcessList.clear();
					for (MapIter iter = AllProcessList.begin(); iter != AllProcessList.end(); iter++)
					{
						if (ProcShowStyle == CLOSEPROC && iter->second.p_count != 0)
						{
							continue;
						}
						if (ProcShowStyle == DUPPROC && iter->second.p_count <= 1)
						{
							continue;
						}

						if (pos > localpos - 1 && ProcessList.size() < G_Settings.pageCount)
						{
							ProcessList.insert(*iter);
						}
						pos++;
					}

					G_Settings.localPage++;
				}
				// 刷新第一页
				else if (G_Settings.localPage > 1)
				{
					G_Settings.localPage = 1;

					ProcessList.clear();
					for (MapIter iter = AllProcessList.begin(); iter != AllProcessList.end(); iter++)
					{
						if (ProcessList.size() < G_Settings.pageCount)
						{
							if (ProcShowStyle == CLOSEPROC && iter->second.p_count != 0)
							{
								continue;
							}
							if (ProcShowStyle == DUPPROC && iter->second.p_count <= 1)
							{
								continue;
							}
							ProcessList.insert(*iter);
						}
					}
				}
			}
			break;
		case 'b'	:
			// 存在上一页
			if (G_Settings.localPage > 1)
			{
				G_Settings.localPage--;
				int pos = 0;
				int localpos = (G_Settings.localPage-1)*G_Settings.pageCount;

				ProcessList.clear();
				for (MapIter iter = AllProcessList.begin(); iter != AllProcessList.end(); iter ++)
				{
					if (ProcShowStyle == CLOSEPROC && iter->second.p_count != 0)
					{
						continue;
					}
					if (ProcShowStyle == DUPPROC && iter->second.p_count <= 1)
					{
						continue;
					}

					if (pos > localpos - 1 && ProcessList.size() < G_Settings.pageCount)
					{
						ProcessList.insert(*iter);
					}
					pos++;
				}
			}
			// 刷新最后页
			else if (G_Settings.localPage == 1 && G_Settings.totalPages > 1)
			{
				G_Settings.localPage = G_Settings.totalPages;
				ProcessList.clear();
				int pos = 0;
				int localpos = (G_Settings.localPage-1)*G_Settings.pageCount;

				for (MapIter iter = AllProcessList.begin(); iter != AllProcessList.end(); iter ++)
				{
					if (ProcShowStyle == CLOSEPROC && iter->second.p_count != 0)
					{
						continue;
					}
					if (ProcShowStyle == DUPPROC && iter->second.p_count <= 1)
					{
						continue;
					}
					if (pos > localpos - 1 && ProcessList.size() < G_Settings.pageCount)
					{
						ProcessList.insert(*iter);
					}
					pos++;
				}
			}
			break;
		case '\t'	://切换显示模式（A-所有进程 C-未运行的进程）
			{
				ProcShowStyle = ProcShowStyle == ALLPROC ? CLOSEPROC : ALLPROC;

				ProcessList.clear();
				G_Settings.localPage = 1;//回到第一页
				for (MapIter iter = AllProcessList.begin(); iter != AllProcessList.end(); iter++)
				{
					if (ProcessList.size() < G_Settings.pageCount)
					{
						//展示没有运行的程序风格的时候，过滤已经运行的进程
						if (ProcShowStyle == CLOSEPROC && iter->second.p_count != 0)
						{
							continue;
						}
						ProcessList.insert(*iter);
					}
				}
			}
			break;
		case '`'	://切换显示模式（A-所有进程 B-重复启动的进程）
			{
				ProcShowStyle = ProcShowStyle == ALLPROC ? DUPPROC : ALLPROC;
				ProcessList.clear();
				G_Settings.localPage = 1;//回到第一页

				for (MapIter iter = AllProcessList.begin(); iter != AllProcessList.end(); iter++)
				{
					if (ProcessList.size() < G_Settings.pageCount)
					{
						//展示重复启动的程序风格的时候，过滤已经启动个数小于等于1的进程
						if (ProcShowStyle == DUPPROC && iter->second.p_count <= 1)
						{
							continue;
						}
						ProcessList.insert(*iter);
					}
				}
			}
			break;
		default:
			break;
		}

		// 重刷主框架
		PrintFrame();
		refresh();
	} while (1);
}