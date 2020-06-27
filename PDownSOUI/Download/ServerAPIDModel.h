#pragma once
#include <string>
#include <vector>
using namespace std;

/*һ����Ƭ�����ص�ַ(jsoncmd ���԰��������������)*/
struct SerDownFileDownUrl {
public:
	//�������� downurl
	wstring jsoncmd;
	//���������url
	wstring d_url;
	//���������header
	wstring d_header;
	//���������method
	wstring d_method;

	SerDownFileDownUrl() {}
	SerDownFileDownUrl(wstring jsoncmd, wstring d_method, wstring d_url, wstring d_header)
		:jsoncmd(std::move(jsoncmd)), d_url(std::move(d_url)), d_header(std::move(d_header)), d_method(std::move(d_method)) {
	}
};
/*һ����Ƭ���ļ���Ϣ���������ص�ַ��*/
struct SerDownFileItem
{
public:
	//��Ƭ���
	int pt_index;
	//��Ƭλ��
	int64_t pt_pos;
	//��Ƭ�������Ϊ��Ƭ����̫��int�Ϳ����ˣ�
	int64_t pt_size;
	//��Ƭ��crc32
	uint32_t pt_crc32;

	vector<SerDownFileDownUrl> downurls;

	SerDownFileItem() {}
	SerDownFileItem(int pt_index, int64_t pt_size, uint32_t pt_crc32, vector<SerDownFileDownUrl>& downurls)
		: pt_index(pt_index), pt_size(pt_size), pt_crc32(pt_crc32), downurls(downurls)
	{
	}
};