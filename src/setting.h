#ifndef _M_setting_H
#define _M_setting_H
#include <iostream>
#include <unistd.h>
using namespace std;

//����
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
		pageSize = getpagesize(); //ϵͳ�������ṩ�������ڴ�ʱ����ҳΪ��λ�ṩ��һ�������ṩһҳ����ʵ�ڴ�ռ�

		nowTime = 0.0;
		posHint = 0;	//��ʾ��Yλ��
		localPage = 1;		//��ǰҳ
		totalPages = 1;		//��ҳ��
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

	int timeout;		//��ʱˢ��ʱ��
	int pageCount;		//ÿҳ���̸���
	string shellName;	//shell�ű�����
	string localPath;	//Ŀ�����·��

	uid_t userId;		//�û�ID
	int ncpus;			//cpu����
	int pageSize;		//ҳ��С

	double nowTime;		//��ǰʱ��
	int posHint;		//��ʾ��Yλ��
	int localPage;		//��ǰҳ
	int totalPages;		//��ҳ��
};

//������������
extern Settings G_Settings;

#endif