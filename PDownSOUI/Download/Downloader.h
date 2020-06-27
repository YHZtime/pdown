#pragma once
#include <string>
#include "../Utils/singleton.h"
#include "DownloadOneModel.h"
#include "DownloadFlow.h"	
#include "../UIAdapter/DownItem.h"
#include "../AppEvent.h"
#include <thread>
#include <atomic>
#include "vector"


using namespace std;
/*���ض��п���*/
class Downloader
{
public:
	SINGLETON_DEFINE(Downloader);
	Downloader(void) {}
	~Downloader(void) {}
private:

	/*���������е��߳�����*/
	std::atomic<int> DowningCount = 0;
	/*���ͬʱ������*/
	int MaxCount = 3;

public:
	vector<std::shared_ptr<DownloadOneModel>> DownList = vector<std::shared_ptr<DownloadOneModel>>();
	std::mutex m;
	/*����ǰ�ж��Ƿ����� ��������*/
	bool IsCanAdd() {
		return DowningCount < MaxCount;
	}
	bool IsDowning() {
		return DowningCount > 0;
	}
	/*�������������أ��������� ֻ�����̲߳��� �������˷���false  ��������*/
	bool AddOne(DowningItem& Item, int waiting) {
		if (DowningCount >= MaxCount) return false;

		wstring share_fsid = Item.share_fsid;
		//std::lock_guard<std::mutex> lck(m);
		for (auto iter = DownList.begin(); iter != DownList.end(); iter++)
		{
			if ((*iter)->share_fsid == share_fsid) {
				return true;//�ҵ��ˣ�����������,ֱ���˳�(��Ӧ�ó���)
			}
		}
		DowningCount++;
		std::shared_ptr<DownloadOneModel> Add = std::make_shared<DownloadOneModel>(Item);
		//new DownloadOneModel(Item);
		DownList.push_back(Add);
		//�����������̣߳���ʼ����
		thread t1 = thread(DownloadFlow::RunOneDownload, Add, waiting);
		t1.detach();
		//�����߳��ڲ�����Ҫ����down_state( UI��ʱ��ʾdowning,������)  ��Ҫ�������ؽ�����Ϣ��UI�ĳ���ʾ�����ٶȵȣ�
		return true;
	}
	/*��ͣ  ֻ�����̲߳��� �����Ƿ��ҵ� ��������*/
	void StopOne(wstring share_fsid) {
		bool IsFind = false;
		//std::lock_guard<std::mutex> lck(m);
		for (auto iter = DownList.begin(); iter != DownList.end(); iter++)
		{
			if ((*iter)->share_fsid == share_fsid) {
				//�ҵ��ˣ���ʼ��ͣ
				(*iter)->Stop();//�����̻߳�ʱʱ��⣬�Զ��˳����ؽ����߳�	
				IsFind = true;
			}
		}
		//�����߳��ڲ���Ҫ����   down_state==stop(����ʱ,��UI��ʾ ����-->����ͣ )
		if (IsFind == false) {
			wstring cmd = share_fsid + L",stop,����ͣ,0,0,00:00:00";
			AppEvent::SendUI(UICmd::UIDowning_UpdateDownItemState, 0, cmd);
		}
	}

	/*��� ֻ�����̲߳��� ��������*/
	void FinishOne(wstring share_fsid) {
		//std::lock_guard<std::mutex> lck(m);
		for (auto iter = DownList.begin(); iter != DownList.end(); )
		{
			if ((*iter)->share_fsid == share_fsid) {
				//�ҵ��ˣ���ʼɾ��
				iter = DownList.erase(iter);//�����ض�����ɾ��(ʹ����shared_ptr������Ҫdelete��)
				DowningCount--;
			}
			else iter++;
		}
	}
	/*ɾ��TD�ļ� ֻ�����̲߳��� ��������*/
	void DelOne(wstring down_dir_path) {
		DownloadFlow::DelTDFile(down_dir_path);
	}

};