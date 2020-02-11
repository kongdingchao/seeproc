#include <iostream>
#include "setting.h"
#include "ProcInfo.h"
#include "MainFrame.h"
using namespace std;
static std::string VERSION = "01.20.02.10.01";
static std::string COPYRIGHT = "www.erayt.com";

static void printVersionFlag() {
	cout << "seeproc " << VERSION.c_str() << " - " <<  COPYRIGHT.c_str() <<  "\n"
		<< endl;
	exit(0);
}

static void printHelpFlag() {
	cout << "seeproc " << VERSION.c_str() << " - " <<  COPYRIGHT.c_str() <<  "\n"
		<< "-i --timeout							Setting timeout\n"
		<< "-m --page_count							Setting page count\n"
		<< "-f --shelltimefile						Setting shelltimefile\n"
		<< "-l --localpath        					Setting localpath\n"
		//<< "-p --proc								Monitor this proc\n"
		<< "-v --version          					Show verison\n"
		<< "-h --help          						Show help information\n"
		<< "Press another exit.\n"
		<< endl;

	exit(0);
}


static Settings parseArguments(int argc, char **argv)
{
	Settings flags;
	int c;
	int p_num = 1;
	while ((c = getopt(argc, argv, "hi:m:f:v:l:")) != -1)
	{
		switch(c)
		{
			case 'v' :
				printVersionFlag();
				return flags;
			break;
			case 'i' :
				flags.timeout = atoi(optarg);
				break;
			case 'm' :
				flags.pageCount = atoi(optarg);
				break;
			case 'f' :
				flags.shellName = std::string(optarg);
				break;
			case 'l' :
				flags.localPath = std::string(optarg);
				break;
			//case 'p' :
				//todo
				//break;
			case 'h' :
			case '?' :
			default :
				printHelpFlag();
				return flags;
				break;
		}
	}
	return flags;
}

//全局配置
Settings G_Settings;

/* 程序运行主函数 */
int main(int argc, char **argv)
{
	G_Settings = parseArguments(argc, argv);
	if (G_Settings.CheckValid())
	{
		MainFrame frame;
		frame.InitFrame();

		//打印主界面
		frame.PrintFrame();
		//另启线程监控
		frame.StartRun();
		//循环处理用户输入
		frame.HandleInput();

		frame.DestroyFrame();
		
	}
	else
	{
		return -1;
	}
	return 0;
}