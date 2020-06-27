#pragma once
#include "ThreadPool.h"
class AppThread {
public:
	static ThreadPool HttpPool;
	static ThreadPool BlinkPool;
	static ThreadPool DBPool;
	static void WaitUntilStop() {
		DBPool.~ThreadPool();
		HttpPool.~ThreadPool();
		BlinkPool.~ThreadPool();
	}

	/*

	DWORD dwCurThreadID = GetCurrentThreadId();
	DWORD dwProcID=GetCurrentProcessId();
	*/
	/*
// �� �հ� ���ݵ��� UI�߳�
// ���� ���� ���� �� ��ͬ���͵� ���� �ŵ�һ�� ִ��  �����Ƿֿ����á�

// SendMessage [&] �е� & ��ָ fn����õı��� ���� ���ÿ�����
#define SRUNONUISYNC(fn)		SNotifyCenter::getSingletonPtr()->RunOnUISync([&](){fn})

// PostMessage [=] �е� �Ⱥ� ��ָ fn����õı��� ���� ֵ������
#define SRUNONUI(fn)		SNotifyCenter::getSingletonPtr()->RunOnUIAsync([=](){fn})

	*/

	/*
	��ʾ��
	RunOnUISync RunOnUIAsync


	����RunOnUISync����fn��װ��event msg��
					 Window��ӦUM_RUNONUISYNC��
					 ����SNotifyReceiver��MESSAGE_HANDLER_EX(UM_RUNONUISYNC, OnRunOnUISync)��
					 ֱ��ִ��fn
	����˵ͨ��������Ϣ����ֱ��ִ��


	����RunOnUIAsyncʱ,��fn��װ��event msg��
					 ��msg���뵽SNotifyCenter�ڲ���m_asyncFuns���У�
					 Window��Ӧtimer��TIMERID_ASYNC����
					 ����SNotifyReceiver��MSG_WM_TIMER(OnTimer)��
					 ִ�ж����е�����fn
	����˵ͨ�����ڵ�timer����ֱ��ִ��


	�����Ծ��ǣ�
	���ǳ�ʱ������Ῠ��UI
	RunOnUISync�Ῠ��UI+������������ӽ���
	RunOnUIAsync�Ῠ��UI   ���Ῠ����������ӽ���
	*/

	/*
	FireEventSync ֱ��ִ��event

	FireEventAsync ��event��װ��msg�����뵽m_ayncEvent���У�ͨ��timer����ִ��

	ͬ���棬FireEventSync���ӽ��̣�FireEventAsync����
	*/


};