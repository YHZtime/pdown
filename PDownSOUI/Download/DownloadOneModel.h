#pragma once

#include "../AppEvent.h"
#include "../UIAdapter/DownItem.h"
#include "DownloadHttpModel.h"
#include "ServerAPIDModel.h"



/*���ؽ���*/
class DownloadProg
{
	std::wstring share_fsid = L"";

public:
	DownloadProg() {
	}
	DownloadProg(std::wstring ishare_fsid) {
		share_fsid = ishare_fsid;
	}
	~DownloadProg() {
	}
	void ReportDowning(wstring msg) {
		Report(L"downing", msg);
	}
	void ReportDowning(wstring speed, int64_t downed, int64_t total) {
		wstring info = to_wstring(downed * 100 / (total + 1)) + L"," + to_wstring(downed) + L",00:00:00";
		Report(L"downing", speed, info);
	}
	void ReportStop() {
		Report(L"stop", L"����ͣ");
	}
	void ReportDowned() {
		Report(L"downed", L"�����");
	}
	void ReportWaiting() {
		Report(L"waiting", L"��ȡ��");
	}
	void ReportError(wstring errormsg) {
		Report(L"error", errormsg);
		SLOGFMTE(L"ReportError=%ws", errormsg.c_str());
	}
private:
	void Report(wstring state, wstring msg, wstring info = L"0,0,00:00:00") {
		wstring cmd = share_fsid + L"," + state + L"," + msg + L"," + info;
		AppEvent::SendUI(UICmd::UIDowning_UpdateDownItemState, 0, cmd);
	}
};
/*�ļ���Ϣ*/
class DownloadFile {
public:
	//������������ȡ�����ļ���Ϣ
	wstring fs_state;
	uint32_t fs_crc32;
	int64_t fs_size;
	vector<SerDownFileItem> DownFileList;

	//��������������ʱ�õ�����ʱ����
	wstring fw_file;
	wstring fw_filetd;
	wstring fw_file0;


	DownloadFile() {
		fs_state = L"";
		fs_crc32 = 0;
		fs_size = 0;
		DownFileList = vector<SerDownFileItem>();
	}
	~DownloadFile() {
	}

	/*�������ձ�����ļ�·��fw_file0*/
	void SetDownPath(wstring file) {
		size_t pos = file.find_last_of(L'.');
		if (pos == wstring::npos) 	file = file + L".ext";
		fw_file = file;
		fw_file0 = file;
	}
	/*��ʱ����Ҫ����_c1 ... _c9*/
	void SetDownPath(int i) {
		size_t pos = fw_file0.find_last_of(L'.');
		wstring name = fw_file0.substr(0, pos);
		wstring ext = fw_file0.substr(pos);

		if (i > 0)	fw_file = name + L"_c" + to_wstring(i) + ext;////filefull_c1.ext
		else fw_file = name + ext;//filefull.ext
	}

	void SetDownPathTD(int i) {
		size_t pos = fw_file0.find_last_of(L'.');
		wstring name = fw_file0.substr(0, pos);
		wstring ext = fw_file0.substr(pos) + L".td";
		if (i > 0)	fw_filetd = name + L"_c" + to_wstring(i) + ext;////filefull_c1.ext.td
		else fw_filetd = name + ext;//filefull.ext.td
	}

};
/*һ���������Ϣ(��װDownItem��DownloadProg�Ϳ��Ʊ�־)*/
class DownloadOneModel {
public:
	/*�ж�Ұָ����*/
	long long value = 0x123456789ABCDE0;

	DowningItem downitem;
	/*���������*/
	DownloadProg downprog;
	/*����������*/
	DownloadResp* downresp = nullptr;
	/*�����ļ���*/
	DownloadFile downfile;

	DownloadOneModel(DowningItem item) {
		downitem = item;
		share_fsid = item.share_fsid;
		downprog = DownloadProg(item.share_fsid);
		downfile = DownloadFile();
		downresp = new DownloadResp(item.share_fsid);
	}
	~DownloadOneModel() {
		delete downresp;
		downresp = nullptr;
	}
	wstring share_fsid;

	void Stop() {
		downresp->StopDownload = true;
	}


};