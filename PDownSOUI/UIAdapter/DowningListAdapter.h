#pragma once
#include "../stdafx.h"
#include <vector>
#include <string>
#include <set>
#include <helper/SAdapterBase.h>
#include "DownItem.h"
#include "../Download/Downloader.h"
#include "../Utils/AppThread.h"
#include "../DAL/DBHelper.h"


class DowningListAdapter : public SAdapterBase
{
public:
	set<wstring> KeySource = set<wstring>();
	std::vector<DowningItem> DataSource = std::vector<DowningItem>();
	SWindow* list_null = nullptr;
	DowningListAdapter() {
	}
	~DowningListAdapter() {
		list_null = nullptr;
		DataSource.clear();
		KeySource.clear();
	}
	virtual int getCount() {
		return (int)DataSource.size();
	}
	/*��ǰѡ����index*/
	int sel = -1;
	virtual void getView(int position, SWindow* pItem, pugi::xml_node xmlTemplate) {
		if (xmlTemplate.empty()) {
			return;
		}
		if (pItem->GetChildrenCount() == 0)
		{
			pItem->InitFromXml(xmlTemplate);
		}

		pItem->SetUserData(position);
		auto Item = &DataSource[DataSource.size() - position - 1];
		try {
			pItem->GetEventSet()->subscribeEvent(EVT_CMD, Subscriber(&DowningListAdapter::EvtSelectItemChanged, this));
			pItem->GetEventSet()->subscribeEvent(EVT_ITEMPANEL_DBCLICK, Subscriber(&DowningListAdapter::EvtDBclickItem, this));
			auto div = pItem->FindChildByName(L"listitem");
			ULONG divflag = div->GetUserData();
			DWORD state = div->GetState();
			if (position == sel) div->ModifyState(WndState_Check, state, false);
			else {
				div->ModifyState(0, WndState_Check, false);
				if (state == WndState_Check) pItem->Invalidate();
			}
			bool IsStateError = (Item->down_state == L"error");
			auto listitem_dot = div->FindChildByName(L"listitem_dot");
			if (listitem_dot)
			{
				state = listitem_dot->GetState();
				if (!IsStateError && state != WndState_Normal) listitem_dot->ModifyState(WndState_Normal, state, true);
				if (IsStateError && state != WndState_Hover)   listitem_dot->ModifyState(WndState_Hover, state, true);
			}
			auto listitem_prog = div->FindChildByName(L"listitem_prog");
			if (listitem_prog) {
				auto pos = L"0,0,%" + std::to_wstring(Item->down_prog) + L",@3";
				listitem_prog->SetAttribute(L"pos", pos.c_str());
				state = listitem_prog->GetState();
				if (!IsStateError && state != WndState_Normal)  listitem_prog->ModifyState(WndState_Normal, state, true);
				if (IsStateError && state != WndState_Hover) listitem_prog->ModifyState(WndState_Hover, state, true);
			}

			auto listitem_state = div->FindChildByName(L"listitem_state");
			if (listitem_state) {
				state = listitem_state->GetState();
				if (!IsStateError && state != WndState_Normal) listitem_state->ModifyState(WndState_Normal, state, true);
				if (IsStateError && state != WndState_Hover) listitem_state->ModifyState(WndState_Hover, state, true);
			}
			div->FindChildByName(L"listitem_filename")->SetWindowTextW(Item->server_filename.c_str());
			div->FindChildByName(L"listitem_filesize")->SetWindowTextW(Item->sizestr.c_str());
			div->FindChildByName(L"listitem_fileprog")->SetWindowTextW((to_wstring(Item->down_prog) + L"%").c_str());
			div->FindChildByName(L"listitem_filetime")->SetWindowTextW(Item->down_timestr.c_str());

			wstring down_state = Item->down_state;

			bool isDowning = (down_state == L"downing" || down_state == L"waiting");
			bool isStop = (down_state == L"stop" || down_state == L"error");
			bool isEnable = (down_state != L"disable");

			if (down_state == L"waiting") {
				if (Item->down_statestr.find(L"��ȡ��") != wstring::npos) down_state = Item->down_statestr;
				else down_state = L"�Ŷ���";
			}
			else if (down_state == L"stop") down_state = L"����ͣ";
			else if (down_state == L"disable") down_state = L"������";
			//else if (down_state == L"downed") down_state = L"���سɹ�";
			else down_state = Item->down_statestr;//downing(ʣ��ʱ��),error(������Ϣ),
			div->FindChildByName(L"listitem_state")->SetWindowTextW(down_state.c_str());

			//��ť
			SButton* btn_start = div->FindChildByName2<SButton>(L"downing_btn_start");
			btn_start->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&DowningListAdapter::EvtBtnStartClick, this));
			SButton* btn_stop = div->FindChildByName2<SButton>(L"downing_btn_stop");
			btn_stop->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&DowningListAdapter::EvtBtnStopClick, this));
			SButton* btn_delete = div->FindChildByName2<SButton>(L"downing_btn_delete");
			btn_delete->GetEventSet()->subscribeEvent(EVT_LBUTTONUP, Subscriber(&DowningListAdapter::EvtBtnDeleteClick, this));


			btn_start->EnableWindow(isEnable && isStop, true);//stop error ���Կ�ʼ(disable,downed,downing,waiting ������)
			btn_stop->EnableWindow(isEnable && isDowning, true);//downing waiting��������ͣ(disable,downed,stop,error ������)
			btn_delete->EnableWindow(isEnable && isStop, true);//stop error����ɾ��(disable,downed,downing,waiting ������)
		}
		catch (std::exception& e) {
			SLOGFMTE(L"downing getview error %hs", e.what());
		}
		catch (...) {
			SLOGFMTE(L"downing getview error %ws", L"δ�������");
		}
	}


	void AddNewList(std::vector<DowningItem>& List) {
		DataSource.reserve(DataSource.size() + List.size() + 10);
		std::vector<DowningItem> InsertList = std::vector<DowningItem>();
		pair<set<wstring>::iterator, bool> pr;
		int addnum = 0;
		for (auto iter = List.begin(); iter != List.end(); iter++)
		{
			pr = KeySource.insert(iter->share_fsid);
			if (pr.second) {//key ����ɹ���˵�����Բ���
				DataSource.push_back(*iter);
				InsertList.push_back(*iter);
				addnum++;
			}
		}
		List.swap(InsertList);//ֻ�����˳ɹ�����ģ�����ʧ�ܵ�˵���Ѿ����ڣ�ֱ��ɾ������
		if (sel >= 0) sel += addnum;
		notifyDataSetChanged();
		OnDataSetChanged();
	}

	void OnStartAll() {
		vector<wstring> list_downid = vector<wstring>();
		list_downid.reserve(1000);
		int i = 0;
		for (auto iter = DataSource.rbegin(); iter != DataSource.rend(); iter++)//ע���ǵ���
		{
			if (iter->down_state == L"stop" || iter->down_state == L"error") {//��ͣ������
				iter->down_state = L"waiting";
				iter->down_statestr = L"";
				list_downid.push_back(iter->share_fsid);
			}
			i++;
		}
		notifyDataSetChanged();
		AppThread::DBPool.enqueue([](vector<wstring> list) { DBHelper::GetI()->UpdateDowning(list, L"waiting"); }, list_downid);//����DB
	}
	void OnStopAll() {
		vector<wstring> list_downid = vector<wstring>();
		list_downid.reserve(1000);
		int i = 0;
		for (auto iter = DataSource.rbegin(); iter != DataSource.rend(); iter++)//ע���ǵ���
		{
			if (iter->down_state == L"waiting") {//�Ŷ���
				iter->down_state = L"stop";
				iter->down_statestr = L"";
				list_downid.push_back(iter->share_fsid);
			}
			else if (iter->down_state == L"downing") {
				iter->down_state = L"disable";
				Downloader::GetI()->StopOne(iter->share_fsid);//֮���UIEvent�и���db
			}
			i++;
		}
		notifyDataSetChanged();
		AppThread::DBPool.enqueue([](vector<wstring> list) { DBHelper::GetI()->UpdateDowning(list, L"stop"); }, list_downid);//����DB
	}
	void OnDeleteAll() {
		vector<wstring> list_downdirpath = vector<wstring>();
		list_downdirpath.reserve(1000);
		vector<wstring> list_downid = vector<wstring>();
		list_downid.reserve(1000);
		std::vector<DowningItem> List = std::vector<DowningItem>();
		List.reserve(DataSource.size());
		for (auto iter = DataSource.begin(); iter != DataSource.end(); iter++)
		{
			if (iter->down_state == L"stop" || iter->down_state == L"error") {
				iter->down_state = L"disable";//��Ҫɾ����
				list_downdirpath.push_back(iter->down_dir + iter->path);
				list_downid.push_back(iter->share_fsid);
				KeySource.erase(iter->share_fsid);
			}
			else {
				List.emplace_back(*iter);// ��Ҫ������
			}
		}
		if (list_downid.size() == 0) return;//����Ҫɾ���κ�һ��
		DataSource.swap(List);

		AppThread::DBPool.enqueue([](vector<wstring> list) { DBHelper::GetI()->DelDowning(list); }, list_downid);//ɾ��DB
		AppThread::HttpPool.enqueue([](vector<wstring> list) {//��Ϊɾ�����ļ����ܱȽ϶࣬�������httppool�첽ɾ��
			for (auto iter = list.begin(); iter != list.end(); iter++)
			{
				Downloader::GetI()->DelOne(*iter);//ɾ���ļ�
			}
			}, list_downdirpath);
		sel = -1;
		notifyDataSetChanged();
		OnDataSetChanged();
	}
private:
	void OnDataSetChanged() {
		if (list_null != nullptr) {
			bool isvis = list_null->IsVisible();//��ǰ״̬
			if (DataSource.size() == 0) {//Ӧ����ʾ
				if (isvis == false)list_null->SetVisible(true, true);//������أ�ˢ����ʾ����
			}
			else {//Ӧ������
				if (isvis == true)list_null->SetVisible(false, true);//�����ʾ��ˢ������
			}
		}
	}
	bool EvtSelectItemChanged(EventArgs* pEvt)
	{
		SWindow* root = sobj_cast<SWindow>(pEvt->sender);
		int old = sel;
		sel = root->GetUserData();
		//notifyDataSetChanged();
		notifyItemDataChanged(old);
		notifyItemDataChanged(sel);
		return true;
	}

	bool EvtBtnStartClick(EventArgs* pEvt)
	{
		SWindow* root = sobj_cast<SWindow>(pEvt->sender)->GetRoot();
		int position = root->GetUserData();
		auto Item = &DataSource[DataSource.size() - position - 1];
		Item->down_state = L"waiting";
		notifyItemDataChanged(position);
		return true;
	}
	bool EvtBtnStopClick(EventArgs* pEvt)
	{
		SWindow* root = sobj_cast<SWindow>(pEvt->sender)->GetRoot();
		int position = root->GetUserData();
		auto Item = &DataSource[DataSource.size() - position - 1];
		Item->down_state = L"disable";
		notifyItemDataChanged(position);
		Downloader::GetI()->StopOne(Item->share_fsid);
		return true;
	}
	bool EvtBtnDeleteClick(EventArgs* pEvt)
	{
		SWindow* root = sobj_cast<SWindow>(pEvt->sender)->GetRoot();
		int position = root->GetUserData();
		auto Item = &DataSource[DataSource.size() - position - 1];
		Item->down_state = L"disable";
		//ֻ��ֹͣ״̬����ɾ��

		AppThread::DBPool.enqueue([](wstring down_id) {
			DBHelper::GetI()->DelDowning(down_id);//ɾ��DB
			}, Item->share_fsid);

		wstring down_dir_path = Item->down_dir + Item->path;
		Downloader::GetI()->DelOne(down_dir_path);//ֱ��ɾ���ļ�
		DeleteDownItem(Item->share_fsid);

		return true;
	}
	bool EvtDBclickItem(EventArgs* pEvt) {
		SWindow* root = sobj_cast<SWindow>(pEvt->sender)->GetRoot();
		int position = root->GetUserData();
		auto Item = &DataSource[DataSource.size() - position - 1];
		auto down_state = Item->down_state;
		bool isDowning = (down_state == L"downing" || down_state == L"waiting");
		bool isStop = (down_state == L"stop" || down_state == L"error");
		bool isEnable = (down_state != L"disable");

		if (isEnable) {
			if (isDowning && isStop == false)EvtBtnStopClick(pEvt);
			if (isDowning == false && isStop)EvtBtnStartClick(pEvt);
		}
		return true;
	}
public:
	void UpdateDownItemState(wstring share_fsid, std::wstring down_state, std::wstring down_speed, size_t down_prog, uint64_t down_size, std::wstring down_timeleft) {
		int i = 0;
		for (auto iter = DataSource.rbegin(); iter != DataSource.rend(); iter++)//ע���ǵ���
		{
			if (iter->share_fsid == share_fsid) {
				//�ҵ���
				iter->down_state = down_state;
				if (down_state == L"downing") {
					iter->down_statestr = down_speed;
					if (down_speed.length() > 0 && down_speed[down_speed.length() - 1] == L's') {
						iter->down_prog = down_prog;
						iter->down_timestr = down_timeleft;
					}
				}
				else if (down_state == L"error") {
					iter->down_statestr = down_speed;
					iter->down_timestr = L"";
				}
				else if (down_state == L"waiting") {
					if (down_speed == L"��ȡ��") {
						iter->down_timewait = StringHelper::GetTimeNow() + 30;
						iter->down_statestr = L"��ȡ��(30s)";
					}
					else {
						iter->down_statestr = L"";
					}
				}
				else if (down_state == L"downed") {
					iter->down_timestr = L"";
					iter->down_statestr = L"";
				}
				else if (down_state == L"stop") {
					iter->down_timestr = L"";
					iter->down_statestr = L"";
				}
				notifyItemDataChanged(i);
				break;
			}
			i++;
		}
		if (down_state == L"error" || down_state == L"stop") {
			AppThread::DBPool.enqueue([](wstring down_id) { DBHelper::GetI()->UpdateDowning(down_id, 0, L"stop"); }, share_fsid);
		}
		if (down_state != L"downing") Downloader::GetI()->FinishOne(share_fsid);
	}
	/*�������ʱ����ɾ��UI�����Item����ΪҪת�Ƶ�Downed����.ע�ⲻɾ�����ݿ⣬��Ϊadddowned��ʱ��ɾ������*/
	void DeleteDownItem(std::wstring share_fsid) {
		int position = DataSource.size();
		for (auto iter = DataSource.begin(); iter != DataSource.end(); iter++)//ע���ǵ���
		{
			position--;
			if (iter->share_fsid == share_fsid) {
				//�ҵ���
				DataSource.erase(iter);

				if (sel >= 0) {
					if (position <= sel) sel--;
				}

				notifyDataSetChanged();
				break;
			}
		}
		KeySource.erase(share_fsid);
		OnDataSetChanged();
	}
	bool FireDownloader() {
		if (Downloader::GetI()->IsCanAdd()) {
			//��ʼ����
			int i = 0;
			size_t timenow = StringHelper::GetTimeNow();
			int waiting = 0;
			for (auto iter = DataSource.rbegin(); iter != DataSource.rend(); iter++)//ע���ǵ���
			{
				if (iter->down_state == L"waiting") {//�ȴ��У��������
					int timesub = iter->down_timewait - timenow;
					if (timesub > 0) {
						iter->down_statestr = L"��ȡ��(" + to_wstring(timesub) + L"s)";//һ��˲�����ʧ��
					}
					else {
						iter->down_statestr = L"";
						bool IsAdd = Downloader::GetI()->AddOne(*iter, waiting);
						if (IsAdd == false) break;//�������ˣ������˳�
						iter->down_state = L"downing";
						iter->down_statestr = L"������";//һ��˲�����ʧ��
						waiting += 700;//��ÿ�������߳��Ӻ�0.7�룬���ٷ���������ѹ��
					}
					notifyItemDataChanged(i);
				}
				i++;
			}
		}

		return Downloader::GetI()->IsDowning();
	}
};