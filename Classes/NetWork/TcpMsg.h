#pragma once
#ifndef  __TCPMSG__
#define __TCPMSG__

#include <thread>
#include "cocos2d.h"
#include "Queue.h"
#include "ODSocket.h"

USING_NS_CC;

#define SLEEP_TIME 50
//TCP������IP&& �˿�
#define SERVER_HOST "127.0.0.1"
#define SERVER_PORT		8002

class TcpMsg
{
public:
	TcpMsg();
	~TcpMsg();

	static TcpMsg* shareTcpMsg();
	void tcp_start();
	void tcp_stop();
	bool isRuning();

	Pocket* MakePocket(const char* msg, unsigned int msgid);
	Pocket* MakePocketFromData(const char* msg);

	//��ȡ������Ϣ����
	Queue* getRecvQueue();
	Queue* getSendQueue();
	
	void pushSendQueue(const char* msg, unsigned int msgid);//���뷢�Ͷ���
	void pushRecvQueue(std::string str, unsigned int msgid);//������ն���

	void recvFunc(void);//������Ϣ����
	void sendFunc(void);//������Ϣ����
	static std::mutex mutex;

	void th_recv(TcpMsg* TcpMsg);//���������߳�
	void th_send(TcpMsg* TcpMsg);//���������߳�
protected:
	void tcpCheck(void);//TCP ״̬��ⷽ��
	//������Ϣ����
	Queue* m_recvQueue;
	//������Ϣ����
	Queue* m_sendQueue;
private:
	bool m_isRuning;//�߳����б�־
	ODSocket m_socket;


};

#endif // ! __TCPMSG__



