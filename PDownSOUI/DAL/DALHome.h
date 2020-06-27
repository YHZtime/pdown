#pragma once
#include <string>
#include "../jsoncpp/include/json/json.h"
#include "../Utils/CprHelper.h"
#include "../Utils/S2W.h"
#include "ServerAPI_Home.h"
#include "../UIPage/UIHome.h"
#include "event/SNotifyCenter.h"
#include "../Utils/GzipHelper.h"
class DALHome {
public:
	static void SubmiteSearch(std::wstring key) {
		vector<HomeItem> ChildList = vector<HomeItem>();
		wstring ret = ServerAPI_Home::SubmiteSearch(key, ChildList);
		if (ret != L"success") {//����
			ChildList.clear();
			auto item = HomeItem();
			item.FileName = L"����[" + key + L"]ʱ�����������,������";
			item.Link = L"";
			item.UserID = L"";
			item.UserName = L"";
			ChildList.push_back(item);
		}
		else if (ChildList.size() <= 0) {
			ChildList.clear();
			auto item = HomeItem();
			item.FileName = L"��������[" + key + L"]��ص��ļ����뻻���ؼ�������";
			item.Link = L"";
			item.UserID = L"";
			item.UserName = L"";
			ChildList.push_back(item);
		}
		SNotifyCenter::getSingletonPtr()->RunOnUIAsync([ChildList] {
			UIHome::GetI()->ShowSearchResult(ChildList);
			});
	}

	static void CheckUP() {
		wstring newver = ServerAPI_Home::CheckUP();
		if (newver != L"error") {//���ذ汾��
			if (APP_VER != newver) {
				//�汾��һ�£���ʾ����
				SNotifyCenter::getSingletonPtr()->RunOnUIAsync([newver] {
					UISetting::GetI()->EvtLinkVerClick(newver);
					});
			}
		}
	}

	static void PostLog() {
		std::wstring logdir = PathHelper::GetLocalAppDataDir(APP_NAMEL);
		std::wstring log7z = logdir + L"log.txt";
		size_t glen = 0;
		wstring IsOK = L"";
		char* buffer_tmp = (char*)GzipHelper::GzipFileData(log7z, glen);
		if (nullptr != buffer_tmp) {
			string postdata = string(buffer_tmp, glen);
			IsOK = ServerAPI_Home::PostLog(postdata, glen);
			delete[] buffer_tmp;
			buffer_tmp = nullptr;
		}
		if (IsOK == L"success") {
			_wremove(log7z.c_str());
		}
	}
};