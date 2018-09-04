#pragma once
#ifndef  __TCPMSG__
#define __TCPMSG__

#include <thread>
#include "cocos2d.h"
#include "Queue.h"
#include "ODSocket.h"

USING_NS_CC;

#define SLEEP_TIME 50
//TCP服务器IP&& 端口
#define SERVER_HOST "127.0.0.1"
#define SERVER_PORT		8002

class TcpClient
{
public:
	TcpClient();
	~TcpClient();

	static TcpClient* shareTcpClient();
	void tcp_start();
	void tcp_stop();
	bool isRuning();

	Pocket* MakePocket(const char* msg, unsigned int msgid);
	Pocket* MakePocketFromData(const char* msg);

	//获取接收消息队列
	Queue* getRecvQueue();
	Queue* getSendQueue();
	
	void pushSendQueue(const char* msg, unsigned int msgid);//加入发送队列
	void pushRecvQueue(const char* msg, unsigned int msgid);//加入接收队列

	void recvFunc(void);//接收消息方法
	void sendFunc(void);//发送消息方法
	static std::mutex mutex;

	void th_recv(TcpClient* TcpMsg);//接收数据线程
	void th_send(TcpClient* TcpMsg);//发送数据线程
protected:
	void tcpCheck(void);//TCP 状态检测方法
	//接收消息队列
	Queue* m_recvQueue;
	//发送消息队列
	Queue* m_sendQueue;
private:
	bool m_isRuning;//线程运行标志
	ODSocket m_socket;


};

#endif // ! __TCPMSG__



