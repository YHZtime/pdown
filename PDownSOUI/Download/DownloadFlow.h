#pragma once
#include "Downloader.h"
#include "DownloadOneModel.h"
#include "DownloadHttp.h"
#include "DownloadBlink.h"
#include "ServerAPID.h"
#include <thread>
#include "../Utils/OpenFileHelper.h"
#include "../Utils/PathHelper.h"
#include "../Utils/CodeCrc32.h"

#ifndef CheckNull
#define CheckNull(One) \
if (nullptr == One || 0x123456789ABCDE0 != One->value) {\
	One = nullptr;\
}
#define CheckNull_R(One) \
if (nullptr == One || 0x123456789ABCDE0 != One->value) {\
	One = nullptr;\
	return;\
}
#define CheckNull_RF(One) \
if (nullptr == One || 0x123456789ABCDE0 != One->value) {\
	One = nullptr;\
	return false;\
}
#endif

/*һ���������������*/
class DownloadFlow {
public:
	/*�ⲿ���ã���ʼ����������*/
	static void RunOneDownload(std::shared_ptr<DownloadOneModel> One, int waiting) {
		if (waiting > 0) this_thread::sleep_for(std::chrono::milliseconds(waiting));

		CheckNull_R(One);//�Ҳ����������� ֱ���˳�
		if (!r1_GetDownLinkList(One)) return;//��ȡ���ص�ַʧ��
		if (!r2_CreatDownDir(One)) return;//�����ļ���ʧ��

		if (One->downfile.fs_size <= 0) {//�ļ����Ϊ0 ����
			if (r3_1_TryCreatDownFileSize0(One)) {
				//�����ɹ����������ݿ��UI,�˳�
				rf_SaveToDBAndUI(One);
				return;
			}
			else {
				return;//����ʧ��(�ڲ����������)��ֱ���˳�
			}
		}

		//�ļ������0

		if (r3_2_CheckHasDowned(One)) {
			//�Ѿ����ع����������ݿ��UI,�˳�
			rf_SaveToDBAndUI(One);
			return;
		}

		//������Ѿ�ȷ���ˣ���Ҫ�������� ��ʼѭ������(���������Զ�����)
		if (!r3_3_DownFileList(One)) return;//����ʧ�ܡ�ֹͣ���˳�
		if (!r4_SaveToFinalFile(One)) return;//�����ļ�ʧ�ܣ��˳�
		else {
			rf_SaveToDBAndUI(One);//����ɹ����������ݿ��UI���˳�
			return;
		}

	}
	/*�ⲿ���ã�ɾ��һ�������������ʱ�ļ�*/
	static void DelTDFile(wstring down_dir_path) {

		wstring finalfile = PathHelper::PathCleanup(down_dir_path);
		if (finalfile == L"") 	return;
		wstring dirpath = L"", filename = L"";
		bool IsDir = PathHelper::GetDir(finalfile, dirpath, filename);
		if (IsDir == false || _waccess(dirpath.c_str(), 0) != 0) {
			//�ļ��в�����
			return;
		}
		DownloadFile downfile = DownloadFile();
		downfile.SetDownPath(finalfile);
		for (int i = 0; i < 10; i++) {
			downfile.SetDownPathTD(i);
			_wremove(downfile.fw_filetd.c_str());
		}
		//ȫ��ɾ����
	}
private:

	/*У���ļ���crc32�Ƿ�Ϸ�
nofile  �ļ������ڣ�success ��֤�ɹ�
crcerror �ļ�����&&��֤ʧ�� sizeerror �ļ�����&&�ļ����Ȳ�һ��
*/
	static wstring r23_CheckCrc32(wstring filefull, uint64_t ck_filesize, uint32_t ck_filecrc32) {
		FILE* fileptr = nullptr;
		uint64_t filesize = 0;
		bool IsOpen = PathHelper::OpenFile(filefull, fileptr, filesize);
		if (IsOpen == false) return L"nofile";

		//�����ǿ��ļ�������ɾ��
		if (ck_filesize != filesize) {
			fclose(fileptr);
			fileptr = nullptr;
			if (filesize == 0) {//�����Ǹ����ļ�
				if (_wremove(filefull.c_str()) == 0) {
					return L"nofile";//�ȳ���ɾ����ɾ���˾ͷ���nofile���������ء�
				}
				else return L"sizeerror";//���ɾ�����ˣ��Ǿͷ���crcerror����One������
			}
			else return L"sizeerror";
		}
		else if (ck_filesize <= 0) {//����ck_filesize===filesize===0
			fclose(fileptr);
			fileptr = nullptr;
			return L"success";//Ҫ���ص��ļ�����Ϊ0 ��ȷ
		}


		//�ļ����ڣ���Сһ�£������ļ�crc,ʧ�ܳ���ɾ��
		CodeCrc32 crc = CodeCrc32();
		uint32_t filecrc32 = crc.getFileCrc32(fileptr);
		fclose(fileptr);
		fileptr = nullptr;

		if (ck_filecrc32 == filecrc32) {
			return L"success";
		}
		else {//�ļ���Сһ����ʱ��
			if (_wremove(filefull.c_str()) == 0) {
				return L"nofile";//�ȳ���ɾ����ɾ���˾ͷ���nofile���������ء�
			}
			else return L"crcerror";//���ɾ�����ˣ��Ǿͷ���crcerror����One������
		}
	}

private:
	/*[�ڲ�����]�ӷ�������ȡ���ص�ַ�б�
	1 ��ȡ����ʧ�� ReportError������false
	2 ��ȡ�� ReportWaiting������false
	3 �ɹ�������true*/
	static bool r1_GetDownLinkList(std::shared_ptr<DownloadOneModel> One) {
		//bdlink,fs_id,md5,path,server_filename,size
		wstring ret = ServerAPID::GetDownFileList(One);
		CheckNull_RF(One);
		if (ret != L"success") {
			One->downprog.ReportError(ret);
			return false;
		}
		else if (One->downfile.fs_state == L"waiting") {
			One->downprog.ReportWaiting();//��������ȡ�У�ֱ���˳�
			return false;
		}
		else {
			//227 210 130

			int64_t total = 0;
			vector<SerDownFileItem>& DownFileList = One->downfile.DownFileList;
			for (auto iter = DownFileList.begin(); iter != DownFileList.end(); iter++)
			{
				total += (*iter).pt_size;
			}
			if (One->downfile.fs_size < total) {
				SLOGFMTE(L"Error fs_size share_fsid=%ws fs_size=%I64u total=%I64u", One->share_fsid.c_str(), One->downfile.fs_size, total);
				One->downfile.fs_size = total;//������ȷ���ļ���С�����������Ȼ��õ�
			}

			return true;//�ɹ���ȡ�����ص�ַ��
		}
	}

	/*[�ڲ�����]����ǰ�ȴ����ļ���
	1 �����ɹ�����true��·�����浽fw_file0
	2 ����ʧ�� ReportError������false*/
	static bool r2_CreatDownDir(std::shared_ptr<DownloadOneModel> One) {
		CheckNull_RF(One);
		wstring ff = One->downitem.down_dir + One->downitem.path;
		wstring finalfile = PathHelper::PathCleanup(ff);
		if (finalfile == L"") {
			SLOGFMTE(L"Error Save Path ff=%ws", ff.c_str());
			One->downprog.ReportError(L"�����ļ�����·������");
			return false;
		}
		wstring dirpath = L"", filename = L"";
		bool IsDir = PathHelper::GetDir(finalfile, dirpath, filename);
		if (IsDir == false || PathHelper::CreatDir(dirpath) == false) {
			SLOGFMTE(L"Error Save Path dirpath=%ws", dirpath.c_str());
			One->downprog.ReportError(L"�����ļ���ʧ��");
			return false;
		}
		One->downfile.SetDownPath(finalfile);
		return true;
	}

	/*[�ڲ�����]����ļ���СΪ0���ļ���ֱ�Ӵ���,
	1 �����ɹ��ˣ�����true���ļ�·��������downfile.fw_file
	2 ����ʧ�ܣ�ReportError������false��fw_fileû��*/
	static bool r3_1_TryCreatDownFileSize0(std::shared_ptr<DownloadOneModel> One) {
		CheckNull_RF(One);
		FILE* fileptr = nullptr;
		uint64_t filesize = 0;
		bool IsOpen = false;
		auto& downfile = One->downfile;

		for (int i = 0; i < 10; i++) {
			downfile.SetDownPath(i);
			IsOpen = PathHelper::OpenFileAutoCreat(downfile.fw_file, fileptr, filesize);
			if (IsOpen) {
				fclose(fileptr);
				fileptr = nullptr;
				if (filesize == 0) return true;
			}
		}
		One->downprog.ReportError(L"�����ļ�ʧ��0");
		return false;
	}

	/*[����Ҫ����]���֮ǰ�Ƿ������ع�
	1 �ҵ��ˣ�����true,�ļ�·��������downfile.fw_file
	2 �Ҳ���������false,fw_fileû��*/
	static bool r3_2_CheckHasDowned(std::shared_ptr<DownloadOneModel> One) {
		CheckNull_RF(One);
		auto& downfile = One->downfile;

		for (int i = 0; i < 10; i++) {
			downfile.SetDownPath(i);
			wstring IsCrc = r23_CheckCrc32(downfile.fw_file, downfile.fs_size, downfile.fs_crc32);
			if (IsCrc == L"success") return true;//֮ǰ�Ѿ����ع���ֱ�ӷ���true			
			else if (IsCrc == L"crcerror" || IsCrc == L"sizeerror") continue;//ֻҪ���ǳɹ��ͼ������
			else if (IsCrc == L"nofile") continue;//ֻҪ���ǳɹ��ͼ������
		}
		return false;
	}


	/*[�ڲ�����]ѭ������ȫ���ķ�Ƭ
	1 td��ʧ�� ReportError������false
	2 StopDownload ReportStop������false
	3 ��γ��Ժ�����ʧ�� ReportError������false
	4 ���з�Ƭ�����سɹ� ReportDowning������true*/
	static bool r3_3_DownFileList(std::shared_ptr<DownloadOneModel> One) {
		if (!r3_3_1_TryOpenTDFile(One)) {
			One->downprog.ReportError(L"��д��ʱ�ļ�ʧ��");
			return false;//��TD�ļ�ʧ��
		}

		vector<SerDownFileItem>& DownFileList = One->downfile.DownFileList;
		int parti = 0;
		//��˳��ѭ���������з�Ƭ
		for (auto iter = DownFileList.begin(); iter != DownFileList.end(); iter++)
		{
			bool IsDown = false;
			parti++;
			for (int i = 0; i < 10; i++) {
				CheckNull_RF(One);
				//SLOGFMTI("r2_DownFileList parti=%d for=%d %ws", parti, i, One->share_fsid.c_str());
				if (r3_3_2_DownAndCheckOneFile(One, parti, *(iter))) {
					//SLOGFMTI("r2_DownFileList ���سɹ� parti=%d i=%d", parti, i);
					IsDown = true;//�ɹ�����
					break;
				}
				//����ʧ�ܻ����û�����Ҫ��ֹͣ
				//SLOGFMTI("r2_DownFileList ����ʧ�� parti=%d i=%d", parti, i);
				CheckNull_RF(One);
				CheckNull_RF(One->downresp);
				if (One->downresp->StopDownload) {
					One->downprog.ReportStop();
					return false;//������û�����Ҫ��ֹͣ��ֱ���˳�
				}
			}
			if (IsDown == false) {
				CheckNull_RF(One);
				One->downprog.ReportError(L"����ʧ�ܣ�������");
				return false;//��γ�����Ȼ����,ֱ���˳������������غ�����ļ�
			}
			CheckNull_RF(One);
		}
		//�������˵�������з�Ƭ���ɹ�������
		CheckNull_RF(One);
		One->downprog.ReportDowning(L"0B/s", One->downfile.fs_size, One->downfile.fs_size);
		One->downprog.ReportDowning(L"�����ļ���");
		return true;
	}

	/*[����Ҫ����]���Դ�TD�ļ�����ʼд��
	1 �ɹ��򿪡�����������true���ļ�·�����浽downfile.fw_filetd,�ļ����One->downresp->fw_tdptr
	2 ��ʧ�ܣ�����false*/
	static bool r3_3_1_TryOpenTDFile(std::shared_ptr<DownloadOneModel> One) {
		CheckNull_RF(One);
		auto& downfile = One->downfile;
		FILE* tdfileptr = nullptr;
		uint64_t tdfilesize = 0;
		bool IsOpen = false;
		for (int i = 0; i < 10; i++) {
			downfile.SetDownPathTD(i);
			IsOpen = PathHelper::OpenFileAutoCreat(downfile.fw_filetd, tdfileptr, tdfilesize);
			if (IsOpen) break;//�ɹ�����,ֱ���˳�ѭ��
			else {
				//td�ļ���ռ�ã���������
				SLOGFMTE(L"WriteToFileError=%ws", downfile.fw_filetd.c_str());
			}
		}
		if (IsOpen) {
			One->downresp->SetFilePtr(tdfileptr, tdfilesize, One->downfile.fs_size);
			//OpenFileHelper::OpenDir(L"", downfile.fw_filetd); //����ʱ�Զ�������Ŀ¼
			return true;
		}
		else {
			return false;
		}
	}

	/*[�ڲ�����]���غ�У�� һ���ļ�  �������
	1 �����Ƭ�Ѿ����ع�(crcУ��ͨ��) ReportDowning������true
	2 ��Ƭ���ؽ��� ReportDowning������true/false (crcУ����)
	3 ����������˳� ReportDowning������false
	4 �û������˳� ReportDowning������false
	ֻ��������⣬����true/false ��ReportDowning*/
	static bool r3_3_2_DownAndCheckOneFile(std::shared_ptr<DownloadOneModel> One, int index, SerDownFileItem DownFile) {
		CheckNull_RF(One);
		DownloadResp* downresp = One->downresp;
		CheckNull_RF(downresp);
		int64_t pt_start = 0;
		//У��td�ļ��Ƿ������������Ƭ
		if (downresp->fw_tdsize >= (DownFile.pt_pos + DownFile.pt_size)) {
			//�Ѿ����ع��ˣ�У��crc
			CodeCrc32 crc = CodeCrc32();
			uint32_t filecrc32 = crc.getFileCrc32(downresp->fw_tdptr, DownFile.pt_pos, DownFile.pt_size);
			if (filecrc32 == DownFile.pt_crc32) {
				//һ�£�����Ҫ������,ֱ���˳�����������һ��
				//SLOGFMTI("DownedBefor&&CheckCRC32pass filecrc32=%I32u", filecrc32);
				pt_start = DownFile.pt_size;
				One->downprog.ReportDowning(L"0B/s", DownFile.pt_pos + DownFile.pt_size, One->downfile.fs_size);
				return true;
			}
			else {
				//��һ�£���ͷ��������
				//SLOGFMTI("DownedBeforButCheckCRC32Error=%I32u pt_crc32=%I32u ", filecrc32, DownFile.pt_crc32);
				pt_start = 0;
				One->downprog.ReportDowning(L"0B/s", DownFile.pt_pos, One->downfile.fs_size);
			}
		}
		else if (downresp->fw_tdsize > DownFile.pt_pos) {//pt_pos << fw_tdsize << pt_pos + pt_size  ˵����Ҫ�ϵ�����
			pt_start = downresp->fw_tdsize - DownFile.pt_pos;//����
			One->downprog.ReportDowning(L"0B/s", DownFile.pt_pos + pt_start, One->downfile.fs_size);
			//SLOGFMTI("RangeDownload pt_start=%I64d pt_pos=%I64d ", pt_start, DownFile.pt_pos);
		}

		//�������Ѿ���ȷ�ˣ���Ҫ����pt_start----(pt_pos + pt_size)��һ��(Rang:pt_start-)�Ϳ�����

		bool IsDownSuccess = false;
		for (size_t i = 0; i < DownFile.downurls.size(); i++) {
			auto& downurl = DownFile.downurls[i];
			if (downurl.jsoncmd == L"downurl") {
				CheckNull(One);
				CheckNull_RF(downresp);
				//SLOGFMTI("StartToDownurls d_url=%ls pt_start=%I64d pt_pos=%I64d pt_size=%I64d ", downurl.d_url.c_str(), DownFile.pt_pos, pt_start, DownFile.pt_size);
				downresp->SetDownPos(DownFile.pt_pos, pt_start, DownFile.pt_size);
				IsDownSuccess = DownloadHttp::DownFile(downurl.d_method, downurl.d_url, downurl.d_header, downresp);
			}
			else if (downurl.jsoncmd == L"http") {
				AppThread::BlinkPool.enqueue([](wstring url, wstring useragent) {
					DownloadBlink::RunHttpCommand(url, useragent);
				}, downurl.d_url, downurl.d_header);
			}
		}
		//SLOGFMTI("FinishDown index=%d IsDownSuccess=%ws", index, IsDownSuccess ? L"true" : L"false");
		CheckNull_RF(One);
		//���ؽ���
		if (IsDownSuccess) {//���سɹ���У��crc
			One->downprog.ReportDowning(L"CheckPartCRC " + to_wstring(index));
			CodeCrc32 crc = CodeCrc32();
			uint32_t filecrc32 = crc.getFileCrc32(downresp->fw_tdptr, DownFile.pt_pos, DownFile.pt_size);
			bool IsCrc32 = (filecrc32 == DownFile.pt_crc32);//�����Ƿ�ɹ�
			if (IsCrc32) {
				//SLOGFMTI("PartCrcOK,RunNextPart %ws", L"");
				//One->downprog.ReportDowning(L"�ȴ�����");
			}
			else {
				//SLOGFMTI("PartCrcError,Redownload filecrc32=%I32d pt_crc32=%I32d", filecrc32, DownFile.pt_crc32);
				downresp->fw_tdsize = DownFile.pt_pos;
				One->downprog.ReportDowning(L"У��ʧ��׼������");
			}
			return IsCrc32;
		}
		else if (One->downresp->StopDownload) {
			//SLOGFMTI("���ؽ��� �û�����ȡ�� %d", index);
			//�û�����ȡ��&&û�����ؽ���
			return false;
		}
		else {
			//��������������������ж��˳�
			wstring msg = One->downresp->errormsg;
			//SLOGFMTI("DownFinaly Error index=%d msg=%ws", index, msg.c_str());
			One->downprog.ReportDowning(msg == L"" ? L"��·����" : msg);//��Ƭ�����з�����������󣬲����ϱ�Ϊ����ʱ�䣬�ᵼ��One����
			//One->downprog.ReportError(One->downresp->errormsg);
		}
		return false;//����ʧ�ܣ����������³���
	}

	/*[�ڲ�����]ȫ�����ؽ��������浽���յ��ļ�
	1 ����ʧ�� ReportError������false
	2 У��ʧ�� ReportError������false
	3 ������ʧ�� ReportError������false
	4 ����ɹ�����true*/
	static bool r4_SaveToFinalFile(std::shared_ptr<DownloadOneModel> One) {
		CheckNull_RF(One);
		//1 ����
		if (!r4_1_DecodeFile(One)) {
			//ɾ���ļ�
			One->downresp->CloseFilePtr();
			_wremove(One->downfile.fw_filetd.c_str());
			One->downprog.ReportError(L"�����ļ���дʧ��");
			return false;
		}
		//2 У��
		CodeCrc32 crc = CodeCrc32();
		uint32_t filecrc32 = crc.getFileCrc32(One->downresp->fw_tdptr);
		if (filecrc32 != One->downfile.fs_crc32) {
			//У��ʧ�ܣ�ɾ���ļ�
			One->downresp->CloseFilePtr();
			_wremove(One->downfile.fw_filetd.c_str());
			One->downprog.ReportError(L"�ļ�У��ʧ��");
			return false;
		}

		//3  ������td-->final
		if (!r4_3_TrySaveFile(One)) {
			One->downprog.ReportError(L"�����ļ�ʧ��");
			return false;
		}

		return true;
	}
	/*[����Ҫ����]����
	1 �ļ���дʧ�� ����false
	2 ���ܳɹ� ����true*/
	static bool r4_1_DecodeFile(std::shared_ptr<DownloadOneModel> One) {
		
		return true;//���ܳɹ�
	}
	/*[����Ҫ����]��td�ļ�����Ϊ�����ļ�
	1 ����ɹ� ����true
	2 ����ʧ�� ����false*/
	static bool r4_3_TrySaveFile(std::shared_ptr<DownloadOneModel> One) {
		CheckNull_RF(One);
		auto& downfile = One->downfile;
		One->downresp->CloseFilePtr();

		for (int i = 0; i < 15; i++) {
			downfile.SetDownPath(i);
			wstring IsCrc = r23_CheckCrc32(downfile.fw_file, downfile.fs_size, downfile.fs_crc32);
			if (IsCrc == L"success") {
				//�Ѿ����ˣ�ֱ��ɾ��td�ļ�
				_wremove(downfile.fw_filetd.c_str());
				return true;
			}
			else if (IsCrc == L"nofile") {
				//�ƶ�td�ļ�,�ɹ���ֱ���˳����ƶ�ʧ�ܸ�������
				if (0 == _wrename(downfile.fw_filetd.c_str(), downfile.fw_file.c_str())) return true;
			}
			else if (IsCrc == L"crcerror" || IsCrc == L"sizeerror") continue;//ֻҪ���ǳɹ��͸����������
		}
		return false;
	}
	/*��Downingɾ�������浽Downed��ˢ��UI*/
	static void rf_SaveToDBAndUI(std::shared_ptr<DownloadOneModel> One) {
		DownedItem downed = DownedItem();
		int64_t down_time = StringHelper::GetTimeNowWeiMiao();
		auto& downitem = One->downitem;
		downed.down_id = downitem.share_fsid + L"_" + to_wstring(down_time);
		downed.local_mtime = downitem.local_mtime;
		downed.path = downitem.path;
		downed.server_filename = downitem.server_filename;
		downed.size = One->downfile.fs_size;
		downed.sizestr = StringHelper::FormatFileSize(downed.size);
		wstring down_timestr = S2W::StringtoWString(StringHelper::FormatJsTimeStr(StringHelper::GetTimeNow(), false), CP_UTF8);

		downed.down_time = down_time;
		downed.down_timestr = down_timestr;
		downed.down_dir = downitem.down_dir;
		downed.down_file = One->downfile.fw_file;
		One->downprog.ReportDowned();

		SLOGFMTE(L"down success %ws %ws", downitem.share_fsid.c_str(), downed.down_file.c_str());
		//���浽���ݿ�
		AppThread::DBPool.enqueue([](wstring share_fsid, DownedItem downed) {
			DBHelper::GetI()->AddDowned(downed);
			DBHelper::GetI()->DelDowning(share_fsid);
			AppEvent::SendUI(UICmd::UIDowned_AddDownItem, 0, downed.down_id);
			}, downitem.share_fsid, downed);
		AppEvent::SendUI(UICmd::UIDowning_DeleteDownItem, 0, downitem.share_fsid);
	}

};