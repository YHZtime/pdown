#pragma once
#include <string>
class AppStart
{
	
public:
	static int ScreenWidth;
	static int ScreenHeight;

	static HANDLE  hMutex;
	/*��������Ѿ���һ�������У��򷵻�true*/
	static bool  checkMyselfExist();

	/*����Ƿ��Ǵ�localĿ¼����*/
	static bool checkRunFromLocal();

	/*������־�ļ�*/
	static bool checkBackUpLog();

	static void AppStartInit(SApplication* m_theApp);
	static void AppCloseClean();
};

