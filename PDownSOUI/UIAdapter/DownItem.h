#pragma once
#include <string>
using namespace std;
struct DowningItem
{

	wstring share_fsid;
	/*�ٶȱ����ļ�����޸�ʱ��*/
	size_t local_mtime = 0;
	
	/*�ٶ��ļ�·��*/
	wstring path;
	/*UI �ٶ��ļ���*/
	wstring server_filename;
	
	/*�ٶ��ļ����int*/
	int64_t size = 0;
	/*UI �ļ���СxxMB*/
	wstring sizestr;
	
	/*���� �������ض��е�ʱ�䣬��������*/
	int64_t down_time = 0;
	/*UI ����ʱ��2020-01-01 03:03:00 ������ �ļ�����޸�ʱ��2020-01-01,��downingʱ����ʱ��ʾΪ00:00:00ʣ��ʱ��*/
	wstring down_timestr;
	/*���� �ļ�����Ŀ¼*/
	wstring down_dir;
	/*UI ���� ����%*/
	size_t down_prog = 0;
	
	/*UI ���� ״̬ downing,waiting,stop,error,downed,disable*/
	wstring down_state;

	/*UI ��ʱ ����״̬��˵������*/
	wstring down_statestr = L"";

	/*��ʱ ���صȴ�ʱ��(��������ȡ�У���Ҫ�ȵ�xxʱ�䣬���ܿ�ʼ���أ�һ��������ʱ��+30��)*/
	size_t down_timewait = 0;

};

struct DownedItem
{
	
	/*�ٶ��ļ�ID*/
	wstring down_id;
	/*�ٶȱ����ļ�����޸�ʱ��*/
	size_t local_mtime = 0;
	
	/*�ٶ��ļ�·��*/
	wstring path;
	/*UI �ٶ��ļ���*/
	wstring server_filename;
	
	/*�ٶ��ļ����int*/
	int64_t size = 0;
	/*UI �ļ���СxxMB*/
	wstring sizestr;

	/*���� �������ض��е�ʱ�䣬��������*/
	int64_t down_time = 0;
	/*UI ����ʱ��2020-01-01 03:03:00 ������ �ļ�����޸�ʱ��2020-01-01,��downingʱ����ʱ��ʾΪ00:00:00ʣ��ʱ��*/
	wstring down_timestr;
	/*���� �ļ�����Ŀ¼*/
	wstring down_dir;
	/*���� �ļ���������·��*/
	wstring down_file;
	
};