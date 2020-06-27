#pragma once
#include <string>
#include "../Utils/JsonHelper.h"
#include "../Utils/CprHelper.h"
#include "../Utils/S2W.h"
#include "BaiDuYun.h"
#include "ServerAPI_Add.h"
#include "../UIPage/UIAddNew.h"
#include "event/SNotifyCenter.h"
class DALAddNew {
private:
	static void ShowUIMessage(bool IsEndError, std::wstring msg) {
		SNotifyCenter::getSingletonPtr()->RunOnUIAsync([IsEndError, msg] {
			UIAddNew::GetI()->ShowStepMsg(IsEndError, msg);
			});
	}
	static void ShowUIFileTree(vector<SerFileItem>& FileList) {
		SNotifyCenter::getSingletonPtr()->RunOnUIAsync([FileList] {
			UIAddNew::GetI()->ShowFileTree(FileList);
			});
	}



	/*�ݹ���������������ļ���*/
	static std::wstring GetSerDirFileList(SerAddNewLink& Link, JsonFileItem& DirItem) {
		std::wstring dirname = DirItem.server_filename;
		ShowUIMessage(false, L"��ʼ�����ļ���:" + dirname);
		std::wstring isget = L"";
		std::string outjson = "";
		DirItem.ZiList.clear();
		DirItem.ZiList.reserve(1000);
		for (int r = 1; r < 20; r++) {//���2000���ļ�
			if (r > 1) ShowUIMessage(false, L"��ʼ�����ļ���" + to_wstring(r) + L":" + dirname);
			isget = BaiDuYun::GetLinkDirFileList(Link.bdlink, Link.bdclnd, Link.shareid, Link.uk, DirItem.path, r, outjson);
			if (isget != L"success") {
				ShowUIMessage(false, L"�����ļ���ʧ��:" + dirname);
				return L"error";
			}
			Json::Value json_object;
			int iserrno = -1;
			isget = JsonHelper::GetJsonWith_errno(outjson, json_object, iserrno);
			if (isget != L"success") {
				if (iserrno == -21) ShowUIMessage(false, L"����ʧ��:����������ʧЧ��ɾ��");
				else if (iserrno == -9) ShowUIMessage(false, L"����ʧ��:�ļ������Ѿ���ɾ��");
				else if (iserrno == -3) ShowUIMessage(false, L"����ʧ��:�ٶȷ��ز���̫Ƶ��");
				else ShowUIMessage(false, L"����ʧ��:�ٶȷ��ش������ " + to_wstring(iserrno));
				//else ShowUIMessage(false, L"����jsonʧ��:" + dirname);
				return L"error";
			}

			try {
				json_object = json_object["list"];
				/*
				[{	"category": 2,
	"fs_id": 651684469246892,
	"isdir": 0,
	"local_ctime": 1569138544,
	"local_mtime": 1569138550,
	"md5": "666acae16304fa85aa914f19d5f1d50a",
	"path": "\/1-300\/300.mp3",
	"server_ctime": 1569155773,
	"server_filename": "300.mp3",
	"server_mtime": 1578372053,
	"size": 11016723 }]
				*/
				if (json_object) {
					size_t len = json_object.size();
					ServerAPI_Add::GetSerJsonFileItemList(json_object, DirItem.ZiList);
					if (len < 100) break;//ȫ���г�����
				}
			}
			catch (...) {
				SLOGFMTE(L"unkownError %d", 2);
				ShowUIMessage(false, L"δ֪����:" + dirname);
				return L"error";
			}
		}
		DirItem.ZiList.shrink_to_fit();
		ShowUIMessage(false, L"���������ļ���:" + dirname + L"  (" + to_wstring(DirItem.ZiList.size()) + L")");
		//�������ļ���
		bool iserror = false;
		for (auto iter = DirItem.ZiList.begin(); iter != DirItem.ZiList.end(); iter++)
		{
			if (iter->isdir) {
				std::wstring isok = GetSerDirFileList(Link, *iter);
				if (isok != L"success") iserror = true;
			}
		}
		return  (iserror ? L"error" : L"success");
	}

	static std::wstring GetLinkOnlyOneFileName(SerAddNewLink& Link, JsonFileItem& FileItem) {
		ShowUIMessage(false, L"��ʼ�����ļ���:" + FileItem.server_filename);
		std::wstring isget = L"";
		std::string outjson = "";

		isget = BaiDuYun::GetLinkOnlyOneFileName(Link.bdlink, Link.bdclnd, Link.shareid, Link.uk, outjson);
		if (isget != L"success") {
			ShowUIMessage(false, L"�����ļ���ʧ��:" + FileItem.server_filename);
			return L"error";
		}

		Json::Value json_object;
		int iserrno = -1;
		isget = JsonHelper::GetJsonWith_errno(outjson, json_object, iserrno);
		if (isget != L"success") {
			ShowUIMessage(false, L"����jsonʧ��:" + FileItem.server_filename);
			return L"error";
		}

		try {
			/*
			{

	"errno": 0,

	"list": [{
		"category": "6",
		"fs_id": "383946898409085",
		"isdir": "0",
		"local_ctime": "1560361280",
		"local_mtime": "1560361280",
		"md5": "8f7a2c9fedae7a135e081742ff0340ac",
		"path": "\/iPlaySoft.com_Feem_v4.3.0_beta_Win.zip",
		"server_ctime": "1560361281",
		"server_filename": "iPlaySoft.com_Feem_v4.3.0_beta_Win.zip",
		"server_mtime": "1560361281",
		"size": "35641663",
		"dlink": ""
	}]
}
			*/
			json_object = json_object["list"];

			size_t len = json_object.size();
			if (len <= 0) {
				SLOGFMTE(L"listlenError outjson=%hs", outjson.c_str());
				ShowUIMessage(false, L"�����ļ���ʧ��:" + FileItem.server_filename);
				return L"error";
			}

			Json::Value& current = json_object[0];//�����ǰٶȷ��صģ�fs_id���⣬���ַ���
			int64_t fs_id = stoll(current["fs_id"].asString());
			wstring md5 = S2W::StringtoWString(current["md5"].asString(), CP_UTF8);
			wstring path = S2W::StringtoWString(current["path"].asString(), CP_UTF8);
			wstring server_filename = S2W::StringtoWString(current["server_filename"].asString(), CP_UTF8);//��ʵֻ������ļ�������Ҫ��

			if (FileItem.fs_id != fs_id) {
				SLOGFMTE(L"fs_idError outjson=%hs", outjson.c_str());
				ShowUIMessage(false, L"�����ļ���ʧ��:" + FileItem.server_filename);
				return L"error";
			}
			FileItem.md5 = md5;
			FileItem.path = path;
			FileItem.server_filename = server_filename;
		}
		catch (...) {
			SLOGFMTE(L"unkownError %d", 2);
			ShowUIMessage(false, L"δ֪����:" + FileItem.server_filename);
			return L"error";
		}

		ShowUIMessage(false, L"���������ļ���:" + FileItem.server_filename);
		return   L"success";
	}
public:
	static void SubmiteLink(std::wstring linkstr, bool isrefresh) {
		ShowUIMessage(false, L"��ʼ��������");
		auto link = ServerAPI_Add::AddNewLink(linkstr);//1 �ӷ�����������bdlink
		if (link.state != L"success") return ShowUIMessage(true, link.state);
		ShowUIMessage(false, L"�ɹ��ύ���ӵ�������");
		if (isrefresh) link.fileliststate = L"notfound";
		wstring isget;
		if (link.fileliststate == L"notfound" || link.fileliststate == L"") {//��Ҫ�ӰٶȽ��� ��ҳ�ļ��б�			
			ShowUIMessage(false, L"�ӰٶȽ���������ҳ�������ļ�");
			wstring indexjson = L"";
			isget = BaiDuYun::GetLinkIndexFileList(link.bdlink, link.pwd, link.surl, link.bdclnd, indexjson);
			if (isget != L"success") return ShowUIMessage(true, L"����ʧ��," + isget);

			ShowUIMessage(false, L"��ȡ��:" + link.bdclnd == L"" ? L"����������Ҫ" : link.bdclnd);

			isget = ServerAPI_Add::PostLinkIndexFileList(link, indexjson);//���浽������
			if (isget != L"success") return ShowUIMessage(true, L"��������������ʧ�ܣ����Ժ�����");
			link.fileliststate = L"index";
		}


		if (link.fileliststate == L"index") {//��Ҫ����ȫ���ļ�
			ShowUIMessage(false, L"��ʽ��&&������ҳ�ļ��б�");
			vector<JsonFileItem> JsonList;
			isget = ServerAPI_Add::GetLinkIndexFileList(link, JsonList);//����Info��uk shareid
			if (isget != L"success") return ShowUIMessage(true, L"��������ȡ�������ļ�ʧ�ܣ����Ժ�����");
			if (JsonList.size() == 0) return ShowUIMessage(true, L"��������ʧ�ܣ������ڲ������κ��ļ�");//��������²����ܷ���,�����ǳ���

			ShowUIMessage(false, L"��ʼ�ݹ���������ļ����ڵ��ļ�");
			bool iserror = false;

			if (JsonList.size() == 1 && JsonList[0].isdir == false) {//ֻ��һ���ļ�������
				std::wstring isok = GetLinkOnlyOneFileName(link, JsonList[0]);
				if (isok != L"success") iserror = true;
			}
			else {
				for (auto iter = JsonList.begin(); iter != JsonList.end(); iter++)
				{
					if (iter->isdir) {//�ݹ���������������ļ���
						std::wstring isok = GetSerDirFileList(link, *iter);
						if (isok != L"success") iserror = true;
					}
				}
			}

			if (iserror) {
				return ShowUIMessage(true, L"��������,�����ļ���������");
			}
			else {
				ShowUIMessage(false, L"��������,ȫ�������ɹ�");
				ServerAPI_Add::PostLinkDirFileList(link.bdlink, JsonList);//���浽������
				link.fileliststate = L"cache";
			}
		}

		//����ˣ��ӷ�������ȡȫ���ļ�������Ű���share_fsid��
		vector<SerFileItem> FileList;
		ShowUIMessage(false, L"�ӷ�������ȡ������ļ��б�");
		isget = ServerAPI_Add::GetLinkDirFileList(link.bdlink, FileList);
		if (isget != L"success") return ShowUIMessage(true, L"��������ȡ����ʧ�ܣ����Ժ�����");

		//ShowUIMessage(false, L"ģ��������ʱ");
		//Sleep(2000);//ģ��������ʱ
		//��FileList�󶨵�UI,��ʾfiletree
		ShowUIMessage(false, L"�������");
		ShowUIFileTree(FileList);
	}
};