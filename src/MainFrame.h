#ifndef _M_MainFrame_H
#define _M_MainFrame_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <curses.h>
#include "ProcInfo.h"
#include "setting.h"

//������
class MainFrame
{
public:
	MainFrame(void);
	~MainFrame(void);

	//��ʼ������
	void InitFrame();
	//���ٴ���
	void DestroyFrame();
	//��ӡ����
	void PrintFrame();
	//��ӡ������Ϣ
	void PrintProcInfo();
	//������ʱˢ�´���
	void StartRun();
	//�����û�����
	void HandleInput();

	//����߳�����
	static void *ExecuteSeeProcThread(void *owner);

	//�����б�
	std::map<TProcKey, TProcInfo> ProcessList;			// ��ǰҳ����
	std::map<TProcKey, TProcInfo> AllProcessList;		// ȫ������

	WINDOW *win1;
	WINDOW *win2;
	WINDOW *win3;
};
#endif
