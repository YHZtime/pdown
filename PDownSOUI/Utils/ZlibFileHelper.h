#pragma once
#include <string>
#include <vector>
#include <fstream>
#include "windows.h"
#include "../third-part/zlib/zlib.h"
#include "PathHelper.h"

#define CHUNK 16384
class ZlibFileHelper {
public:
	ZlibFileHelper() {}
	~ZlibFileHelper() {}
private:
	FILE* file_zlib = nullptr;
	z_stream strm;
public:
	/*����zlib�ļ���д�룬�ɹ�����true*/
	bool CreatZlibFile(std::wstring savefile) {
		uint64_t filelen = 0;
		if (PathHelper::OpenFileAutoCreat(savefile, file_zlib, filelen))
		{
			return true;
		}
		return false;
	}

	bool PackOneFile(std::wstring file)
	{
		FILE* source = nullptr;
		source = _wfsopen(file.c_str(), L"rb", _SH_DENYWR);//bin��ʽ�����Ѵ��ڵ��ļ� ��д
		if (!source) return false;
		bool IsEndOfFile = false;
		bool IsSuccess = true;
		unsigned have;
		unsigned char in[CHUNK];
		_fseeki64(source, 0, SEEK_SET);
		try {
			/* compress until end of file */
			do {
				have = fread(in, 1, CHUNK, source);//��ȡ�����ļ�
				if (have <= 0) break;
				fwrite(in, 1, have, file_zlib);//д���ļ�
			} while (true);
		}
		catch (...) {
			IsSuccess = false;
		}
		fclose(source);
		in[0] = '\r';
		in[1] = '\n';
		fwrite(in, 1, 2, file_zlib);//д���ļ�
		source = nullptr;
		return IsSuccess;
	}

	void CloseZlibFile() {

		if (file_zlib != nullptr) {
			fclose(file_zlib);
			file_zlib = nullptr;
		}
	}
};