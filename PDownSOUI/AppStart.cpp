#include "stdafx.h"
#include "AppStart.h"
#include "Utils/CprHelper.h"
#include "Utils/PathHelper.h"
#include "Utils/AppThread.h"
#include "AppEvent.h"
#include "Utils/ZlibFileHelper.h"
#include "DAL/DBHelper.h"
#include "Utils/unzip.h"
#include "AppInfo.h"
#include "Utils/Blink.h"


//�Զ���ĳ�ʼ��
CURLSH* CprHelper::DNSShared;
ThreadPool AppThread::HttpPool(5);
ThreadPool AppThread::DBPool(1);
ThreadPool AppThread::BlinkPool(1);
SNotifyCenter* AppEvent::pNotifyCenter;

//�Զ���ĳ�ʼ������


HANDLE AppStart::hMutex = NULL;
bool AppStart::checkMyselfExist()
{
	hMutex = CreateMutex(NULL, TRUE, APP_NAMEL);
	DWORD dwRet = GetLastError();
	if (dwRet == ERROR_ALREADY_EXISTS)
	{
		if (hMutex) CloseHandle(hMutex);
		hMutex = nullptr;


		HWND  win = ::FindWindow(APP_NAMEL, APP_NAMEL);
		if (win) {
			while (true) {//ѭ�����ң��ҵ��������
				long hwl2p = ::GetWindowLong(win, GWL_HWNDPARENT);
				if (hwl2p != 0) {//����0 ˵��ʧ�ܣ�û�и�����  ��0���Ǹ����ھ��
					win = (HWND)hwl2p;
				}
				else {
					break;
				}
			}
			::SendMessage(win, WM_COMMAND, 1, NULL);//����1 �Ƕ�ӦOnCommand(0,1,null)
			//ShowWindow(win, SW_SHOW);
		}
		return true;
	}
	else {
		return false;
	}
}


/*����Ƿ��Ǵ�localĿ¼����*/
bool AppStart::checkRunFromLocal() {

#ifndef _DEBUG	
	std::wstring AppDir = AppInfo::GetI()->AppDir;
	std::wstring exe_local = AppDir + APP_NAMEL + L".exe";
	std::wstring exe_localnew = AppDir + APP_NAMEL + L"_new.exe";
	std::wstring exe_start = PathHelper::GetAppStartDir();
	if (_waccess(exe_localnew.c_str(), 0) == 0) {//��������°汾
		_wremove(exe_local.c_str());//ɾ���ɵ�
		MoveFile(exe_localnew.c_str(), exe_local.c_str());//�����µ�
	}
	if (exe_local != exe_start) {
		if (_waccess(exe_local.c_str(), 0) != 0) {//���������local
			CopyFileW(exe_start.c_str(), exe_local.c_str(), true);//����start��local
		}

		if (_waccess(exe_local.c_str(), 0) == 0) {//����жϣ��������local������local
			exe_local = L"\"" + exe_local + L"\"";
			ShellExecuteW(NULL, L"open", exe_local.c_str(), NULL, NULL, SW_SHOWDEFAULT);
			return true;//ֻ��������Ҫ��local�������ŷ���true�����³���ֱ���˳�
		}
	}
#endif
	return false;
}

/*������־�ļ�*/
bool AppStart::checkBackUpLog() {

	std::wstring AppDir = AppInfo::GetI()->AppDir;
	std::wstring log7z = AppDir + L"log.txt";
	ZlibFileHelper ZLib = ZlibFileHelper();
	if (!ZLib.CreatZlibFile(log7z)) return false;

	bool IsAddFile = false;
	for (int i = 0; i < 10; i++) {
		std::wstring logfile = AppDir + APP_NAMEL + L"-" + to_wstring(i) + L".log";
		if (_waccess(logfile.c_str(), 0) == 0) {//���������־�ļ�
			if (ZLib.PackOneFile(logfile)) {
				IsAddFile = true;
				_wremove(logfile.c_str());
			}
		}
	}
	ZLib.CloseZlibFile();

	return true;

}




void AppStart::AppStartInit(SApplication* m_theApp)
{
	DBHelper::GetI()->OpenDBInit(AppInfo::GetI()->AppDir + L"data.db");
	CprHelper::GobalInit();
	AppEvent::InitAppEvent();


	//�����Ҽ��˵�
	wstring DPI = DBHelper::GetI()->GetConfigData("DPI");
	SStringT editmenuxml = _T("LAYOUT:XML_EDITMENU");
	if (DPI == L"150") {
		editmenuxml = _T("LAYOUT:XML_EDITMENU_150");
	}
	pugi::xml_document xmlDoc;
	if (m_theApp->LoadXmlDocment(xmlDoc, editmenuxml))
	{
		SRicheditMenuDef::getSingleton().SetMenuXml(xmlDoc.child(L"editmenu"));
	}
	Blink::GetI()->Run_OpenNode();
}

void AppStart::AppCloseClean()
{
	Blink::GetI()->Run_CloseNode();
	AppThread::WaitUntilStop();
	CprHelper::GobalClean();
	AppEvent::DestroyAppEvent();
	DBHelper::GetI()->CloseDBFile();

}
