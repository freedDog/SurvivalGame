#pragma once

#include <map>

using namespace std;
/**
	��Ϣ�����࣬����ע����Ϣ����Ϣ����
*/
class  MessagePool
{
public:
	 MessagePool();
	~ MessagePool();

	//��ʼ��
	void init();
	//��ȡ��Ϣ��
	void getMessage(int msgId);
	//��ȡ��Ϣ������
	void getMessageHandler(int msgId);
public:

private:
	map<int, string> handlers;
};
