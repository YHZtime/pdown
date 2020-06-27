#pragma once
#include <cstdio>
#include <cstdint>
#include "../AppEvent.h"
#include "../Utils/AppThread.h"
#include "../DAL/DBHelper.h"

#define BUFFSIZE_CDH 1024*1024*2
class DownloadResp {
private:
	/*���������*/
	wstring share_fsid;
	/*��������� �ϴα���ʱ��*/
	clock_t ReportTime = 0;
	/*��������� �����ٶ�*/
	int64_t ReportSpeed = 0;
	/*��������� ������*/
	int64_t ReportDownedSize = 0;
	/*��������� �������ؽ����õ�TotalSize*/
	int64_t ReportProgTotalSize = 0;
	/*�ļ����ؽ���*/
	int ReportProg = 0;
	int ReportUseTime = 0;
public:
	/*�ж�Ұָ����*/
	long long value = 0x123456789ABCDE0;
	void ReportDowning() {
		if (errormsg != L"") return;
		ReportUseTime = GetTickCount() - ReportTime;//���� ʱ���
		if (ReportUseTime < 1500) return;
		ReportTime = GetTickCount();
		//fw_tdpos  ��ǰ�����ص����ֽ���
		ReportSpeed = (ReportSpeed + (fw_tdpos - ReportDownedSize) * 1000 / ReportUseTime + 1) / 2;
		if (ReportSpeed < 0) ReportSpeed = 0;
		ReportUseTime = (int)((fw_size - fw_tdpos) / (ReportSpeed + 1));//ʣ��ʱ��s
		ReportDownedSize = fw_tdpos;//������ɺ���� ������
		//�������ʾ�õ� �����ٶȣ�ʣ��ʱ�䣬���ؽ���		

		int Prog = (int)(fw_tdpos / ReportProgTotalSize) - ReportProg;
		if (Prog < -5 || Prog>5) {//���ȸı�>5ʱ���������ݿ�Ľ���
			AppThread::DBPool.enqueue([](wstring share_fsid, int Prog) { DBHelper::GetI()->UpdateDowning(share_fsid, Prog, L"waiting"); }, share_fsid, Prog);
			ReportProg = Prog;
		}
		wstring cmd = share_fsid + L",downing," + FormatSpeed(ReportSpeed) + L"," + to_wstring(fw_tdpos / ReportProgTotalSize) + L"," + to_wstring(fw_tdpos) + L"," + FormatTime(ReportUseTime);
		AppEvent::SendUI(UICmd::UIDowning_UpdateDownItemState, 0, cmd);
	}
private:
	std::wstring FormatSpeed(int64_t downsize) {
		if (downsize < 0) return L"0B/s";
		wchar_t szText[64];
		double bytes = (double)downsize;
		DWORD cIter = 0;
		std::wstring pszUnits[] = { (L"B"), (L"KB"), (L"MB"), (L"GB"), (L"TB") };
		DWORD cUnits = 5;
		// move from bytes to KB, to MB, to GB and so on diving by 1024
		while (bytes >= 1000 && cIter < (cUnits - 1))
		{
			bytes /= 1024;
			cIter++;
		}
		int slen = swprintf_s(szText, 64, (L"%.2f"), bytes);
		/*
		0.00
		10.00
		100.00
		*/
		if (slen > 5) slen = 5;
		std::wstring ret = std::wstring(szText, slen) + pszUnits[cIter] + L"/s";
		return ret;
	}
	std::wstring FormatTime(int64_t usetime) {
		if (usetime < 0) return L"00:00:00";
		if (usetime / 3600 > 99) return L"99:99:99";
		wstring ret = L"";
		if (usetime > 36000) ret += to_wstring(usetime / 3600) + L":";
		else if (usetime > 3600) ret += L"0" + to_wstring(usetime / 3600) + L":";
		else ret += L"00:";
		usetime = usetime % 3600;
		if (usetime > 600) ret += to_wstring(usetime / 60) + L":";
		else if (usetime > 60) ret += L"0" + to_wstring(usetime / 60) + L":";
		else ret += L"00:";
		usetime = usetime % 60;
		if (usetime > 10) ret += to_wstring(usetime);
		else if (usetime > 0) ret += L"0" + to_wstring(usetime);
		else ret += L"00";

		return ret;
	}
public:
	FILE* fw_tdptr = nullptr;
	/*td�ļ���ǰ��С*/
	int64_t fw_tdsize = 0;
	/*д���ļ���λ��*/
	int64_t fw_tdpos = 0;
	/*�ļ��ܴ�С*/
	int64_t fw_size = 0;
public:
	DownloadResp(wstring ishare_fsid) {
		share_fsid = ishare_fsid;
		buffPos = 0;
		if (NULL == buff) buff = (unsigned char*)malloc(BUFFSIZE_CDH + 1);
		if (NULL == buff)
		{
			errormsg = L"BuffMemError";
		}
	}
	~DownloadResp() {
		if (fw_tdptr) {
			fclose(fw_tdptr);
			fw_tdptr = nullptr;
		}
		if (NULL != buff)free(buff);
		buff = NULL;
		buffPos = 0;
	}
	/*ֹͣ����*/
	bool StopDownload = false;
	/*�����ļ��������������td�ļ�ʱ����һ��*/
	void SetFilePtr(FILE* fileptr, uint64_t filetdsize, uint64_t filesize) {
		ReportTime = GetTickCount() - 2000;
		ReportSpeed = 0;
		ReportProgTotalSize = (size_t)(filesize / 100 + 1);//���������

		fw_tdptr = fileptr;
		fw_tdsize = filetdsize;
		fw_size = filesize;

		//SLOGFMTI("SetFilePtr filetdsize=%I64d filesize=%I64d", filetdsize, filesize);
	}
private:
	uint64_t down_start = 0;
	uint64_t down_total = 0;
public:
	/*���÷�Ƭ������λ�ã�ÿ�����ط�Ƭʱ�������*/
	void SetDownPos(int64_t pos, int64_t start, int64_t total) {
		fw_tdpos = pos + start;
		ReportDownedSize = fw_tdpos;
		_fseeki64(fw_tdptr, fw_tdpos, SEEK_SET);
		down_start = start;
		down_total = total;
		status_code = "";
		errormsg = L"";

		//SLOGFMTI("SetDownPos pos=%I64d start=%I64d total=%I64d ", pos, start, total);
	}

	string GetRequestRange() {
		return to_string(down_start) + "-" + to_string(down_total - 1);
	}

	string status_code = "";
	wstring errormsg = L"";
private:
	unsigned char* buff = NULL;
	size_t buffPos = 0;
public:
	/*�����ص�����д���ļ���ʵ������д�뻺�棩*/
	size_t WriteToFile(char* ptr, size_t size, size_t nmemb) {
		try {
			if (errormsg != L"") {
				return  size * nmemb;//��ֹ����
			}

			size_t totalcount = nmemb;//��Ҫд�������
			int canwritecount = 0;//һ����д�����
			size_t nowpos = 0;//��ǰ��ȡλ��	
			int lastcount = 0;//ʣ����Ҫд��ĸ���
			while (nowpos < totalcount) {
				canwritecount = BUFFSIZE_CDH - buffPos;//����������д��n��
				lastcount = totalcount - nowpos;
				if (lastcount < canwritecount) canwritecount = lastcount;//ʵ����д��(��� or ʣ��)
				memcpy((buff + buffPos), (ptr + nowpos), canwritecount);
				buffPos += canwritecount;//�ƶ�����λ��
				nowpos += canwritecount;
				if (buffPos >= BUFFSIZE_CDH) {
					if (!FlushToFile()) break;
				}
			}
			fw_tdpos += totalcount;
		}
		catch (...) {
			errormsg = L"WriteToFileError";//��ֹ����
		}
		return  size * nmemb;
	}

	/** ����������ˢ�µ��ļ�*/
	bool FlushToFile() {
		if (fw_tdptr) {
			if (buffPos > 0) {
				size_t numwritten = fwrite(buff, sizeof(unsigned char), buffPos, fw_tdptr);
				if (numwritten != (sizeof(unsigned char) * buffPos)) {
					//д���ļ�ʧ��
					buffPos = 0;
					errormsg = L"FlushToFileError";//��ֹ����
					return false;
				}
				else {
					buffPos = 0;
					return true;
				}
			}
			return true;
		}
		else return false;
	}


	void CloseFilePtr() {
		if (fw_tdptr) {
			fclose(fw_tdptr);
			fw_tdptr = nullptr;
		}
	}
};