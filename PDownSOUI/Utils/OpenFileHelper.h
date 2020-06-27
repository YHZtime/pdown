#pragma once
#include "../stdafx.h"
#include <string>
#include "shellapi.h"
#include "windows.h"
#include "S2W.h"
class OpenFileHelper {
public:
	static bool GetDir(std::wstring full_file, std::wstring& dirpath, std::wstring& filename) {
		size_t pos = full_file.find_last_of('\\');
		if (pos != wstring::npos) {
			dirpath = full_file.substr(0, pos + 1);
			filename = full_file.substr(pos + 1);
			return true;
		}
		return false;
	}

	static bool OpenDir(std::wstring dir, std::wstring file) {
		if (file == L"") {
			if (_waccess(dir.c_str(), 0) == 0) {
				dir = L"\"" + dir + L"\"";
				ShellExecuteW(NULL, L"explore", dir.c_str(), NULL, NULL, SW_SHOWDEFAULT);
				return true;
			}
		}
		else {
			///select,
			if (_waccess(file.c_str(), 0) == 0) {
				file = L"/select, \"" + file+L"\"";
				ShellExecuteW(NULL, L"open", L"explorer", file.c_str(), NULL, SW_SHOWDEFAULT);
				return true;
			}
		}

		return false;
	}
	static bool OpenFile(std::wstring file) {
		if (_waccess(file.c_str(), 0) == 0) {
			file = L"\"" + file + L"\"";
			ShellExecuteW(NULL, L"open", L"explorer", file.c_str(), NULL, SW_SHOWDEFAULT);
			return true;
		}

		return false;
	}


	static void CopyToClipboard(std::wstring wstr) {

		if (::OpenClipboard(NULL)) // �򿪼�����
		{
			auto str = S2W::WString2String(wstr);
			const char* a = str.data();
			DWORD dwLength = str.size(); // Ҫ���Ƶ��ִ�����

			HANDLE hGlobalMemory = GlobalAlloc(GHND, dwLength + 1); // ����ȫ���ڴ沢��ȡ���
			LPBYTE lpGlobalMemory = (LPBYTE)GlobalLock(hGlobalMemory); // ����ȫ���ڴ�
			memcpy(lpGlobalMemory, a, dwLength);
			lpGlobalMemory[dwLength] = '\0';
			GlobalUnlock(hGlobalMemory); // �����ڴ�����

			::EmptyClipboard(); // ��ռ�����
			::SetClipboardData(CF_TEXT, hGlobalMemory); // ���ڴ��е����ݷ��õ�������
			::CloseClipboard(); // �رռ�����
		}
	}

	static wstring CopyFromClipboard() {
		wstring outstr = L"";
		try {
			if (::IsClipboardFormatAvailable(CF_UNICODETEXT))
			{
				if (::OpenClipboard(NULL))
				{
					HGLOBAL hMem = GetClipboardData(CF_UNICODETEXT);
					if (hMem != NULL)
					{
						//��ȡUNICODE���ַ�����
						LPTSTR lpStr = (LPTSTR)::GlobalLock(hMem);
						if (lpStr != NULL)
						{
							//��ʾ�����
							outstr = wstring(lpStr);
							//�ͷ����ڴ档
							::GlobalUnlock(hMem);
						}
					}
					::CloseClipboard();
				}
			}
			else if (::IsClipboardFormatAvailable(CF_TEXT))
			{
				if (::OpenClipboard(NULL))
				{
					HGLOBAL hMem = GetClipboardData(CF_TEXT);
					if (hMem != NULL)
					{
						//��ȡUNICODE���ַ�����
						LPCSTR lpStr = (LPCSTR)::GlobalLock(hMem);
						if (lpStr != NULL)
						{
							//��ʾ�����
							string outstrs = string(lpStr);
							outstr = S2W::StringtoWString(outstrs, CP_UTF8);
							//�ͷ����ڴ档
							::GlobalUnlock(hMem);
						}
					}
					::CloseClipboard();
				}
			}
		}
		catch (...) {
			outstr = L"";
		}
		return outstr;
	}
};