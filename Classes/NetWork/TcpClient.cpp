#include "TcpClient.h"

std::mutex TcpClient::mutex;

static TcpClient _sharedContext;

TcpClient* TcpClient::shareTcpClient()
{
	static bool s_bFirstUse = true;
	if (s_bFirstUse) 
	{
		s_bFirstUse = false;
	}
	return &_sharedContext;
}

TcpClient::TcpClient() :
	m_isRuning(false), m_recvQueue(NULL), m_sendQueue(NULL)
{
	m_recvQueue = new Queue();
	m_sendQueue = new Queue();
}

TcpClient::~TcpClient()
{
	CC_SAFE_DELETE(m_recvQueue);
	CC_SAFE_DELETE(m_sendQueue);
}

void TcpClient::tcp_start()
{
	m_isRuning = true;
	m_socket.Connect(SERVER_HOST, SERVER_PORT);

	std::thread tRecv(&TcpClient::th_recv, this, this);
	std::thread tSend(&TcpClient::th_send, this, this);
	tRecv.detach();
	tSend.detach();

	CCLOG("tcp_start");
}

void TcpClient::tcp_stop()
{
	m_isRuning = false;
	CCLOG("tcp_stop");
}

bool TcpClient::isRuning()
{
	return m_isRuning;
}

Pocket* TcpClient::MakePocket(const char* msg, unsigned int msgid)
{
	Pocket* pk = new Pocket();

	pk->length = strlen(msg);
	pk->id = msgid;
	pk->msg = new char[pk->length ];
	memset(pk->msg, 0, pk->length );
	memcpy(pk->msg, msg, pk->length);

	return pk;
}

Pocket* TcpClient::MakePocketFromData(const char* data)
{
	Pocket* pk = new Pocket;
	return pk;
}

Queue* TcpClient::getRecvQueue()
{
	return m_recvQueue;
}

Queue* TcpClient::getSendQueue()
{
	return m_sendQueue;
}

void TcpClient::pushSendQueue(const char* msg, unsigned int msgid)
{
	Pocket* pk = TcpClient::MakePocket(msg, msgid);
	if (m_sendQueue!=NULL) 
	{
		m_sendQueue->PushPocket(pk);
	}
	CCLOG("m_sendQueue have %d", m_sendQueue->getSize());
}

void TcpClient::pushRecvQueue(const char* msg, unsigned int msgid)
{
	Pocket* pk = TcpClient::MakePocket(msg,msgid);
	if (m_recvQueue!=NULL) 
	{
		m_recvQueue->PushPocket(pk);
	}
	CCLOG("m_recvQueue have %d", m_recvQueue->getSize());
}

void TcpClient::tcpCheck(void)
{
	if (!m_socket.Check()) 
	{
		CCLOG("Socket connection Error:%d", m_socket.GetError());

		bool status = false;
		while (!status)
		{
			if (m_socket.Connect(SERVER_HOST, SERVER_PORT))
			{
				status = true;
				CCLOG("Reconnection Success");
			}
			else
			{
				CCLOG("Reconnection Failure");
				return;
			}
		}
	}
}

//发送数据逻辑
void TcpClient::sendFunc(void)
{
	//网络检测
	if (m_sendQueue != NULL&&m_sendQueue->getSize() > 0)
	{
		Pocket* pk = m_sendQueue->PullPocket();
		if (pk->id>0) 
		{
			char* sendData = new char[pk->length + 10];
			memset(sendData, 0, pk->length + 10);
			//包长
			sendData[0] = ((pk->length + 8)) & 0xff;
			sendData[1] = (pk->length + 8) >> 8 & 0xff;
			//消息ID
			sendData[2] = (pk->id ) & 0xff;
			sendData[3] = (pk->id >> 8) & 0xff;
			sendData[4] = (pk->id >> 16) & 0xff;
			sendData[5] = (pk->id >> 24) & 0xff;
			//消息内容长度
			sendData[6] = (pk->length ) & 0xff;
			sendData[7] = (pk->length >> 8) & 0xff;
			sendData[8] = (pk->length >> 16) & 0xff;
			sendData[9] = (pk->length >> 24) & 0xff;


			memcpy(sendData + 10, pk->msg, pk->length);

			int s = m_socket.Send(sendData, pk->length + 10, 0);

			CC_SAFE_DELETE(sendData);
		}
		else 
		{
			int s = m_socket.Send(pk->msg, pk->length, 0);
		}
		CC_SAFE_DELETE(pk);
		CCLOG("sendFunc");
	}
}

//接收数据逻辑;
void TcpClient::recvFunc(void)
{
	//网络检测;
	tcpCheck();
	//取消息头跟版本和协议;
	//char recvHeadBuf[8] = "\0";
	int headLen = 10;
	char* recvHeadBuf = new char[headLen];
	memset(recvHeadBuf, 0, headLen);
	int recvLen = m_socket.Recv(recvHeadBuf, headLen, 0);
	if (recvLen != headLen && recvLen != -1)
	{
		while (recvLen < headLen)
		{
			int tLen = m_socket.Recv(recvHeadBuf + recvLen, sizeof(recvHeadBuf - recvLen), 0);//一直读到把消息头接收完;
			if (tLen != -1)
			{
				recvLen += tLen;
			}
		}
	}

	if (recvLen != -1)
	{
		CCLOG("recvFunc");

		//输出消息摘要信息
		unsigned short protobufLength;
		unsigned int msgId, msgLen;
		unsigned int protocol;
		protobufLength = recvHeadBuf[0]  | recvHeadBuf[1] << 8;
		msgId = (recvHeadBuf[2] & 0xff) | recvHeadBuf[3] << 8 | recvHeadBuf[4] << 16 | (recvHeadBuf[5] << 24 );
		protocol = (recvHeadBuf[6] & 0xff) | (recvHeadBuf[7] << 8) | (recvHeadBuf[8] << 16) | (recvHeadBuf[9] << 24);

		CCLOG("bodyLen:%d,msgId:%d,protocol:%d,{%02x,%02x,%02x,%02x}", protobufLength, msgId, protocol, recvHeadBuf[4], recvHeadBuf[5], recvHeadBuf[6], recvHeadBuf[7] & 0xff);

		//接收消息体;
		int readSize = 0; //当前读取消息的长度;
		//如果消息体长度大于0;
		if (protobufLength > 6)
		{
			int msgLen = protocol;
			char* msgBody = new char[msgLen + 1];
			memset(msgBody, 0, msgLen + 1);
			while (readSize < msgLen)
			{
				int recvLen = m_socket.Recv(msgBody + readSize, msgLen - readSize, 0);//一直读到把消息体接收完;
				if (recvLen != -1)
				{
					readSize += recvLen;
					if (readSize == msgLen)
					{
						CCLOG("RECV: %d", msgLen);
						pushRecvQueue(msgBody, msgId);//将消息添加到队列中;
						break;
					}
				}
			}
			CC_SAFE_DELETE_ARRAY(msgBody);
		}
		CC_SAFE_DELETE_ARRAY(recvHeadBuf);
	}
}

void TcpClient::th_send(TcpClient* TcpClient)
{
	CCLOG("th_send start");
	//TcpMsg* TcpMsg = ((TcpMsg*) r);
	while (TcpClient->m_isRuning)
	{
		//CCLOG("th_send");
		//TcpMsg::mutex.lock();
		TcpClient->sendFunc();
		//TcpMsg::mutex.unlock();
#ifdef WIN32
		Sleep(SLEEP_TIME);
#else
		sleep(SLEEP_TIME / 1000.0f);
#endif
	}
	CCLOG("th_send end");
}

void TcpClient::th_recv(TcpClient* TcpClient)
{
	CCLOG("th_recv start");
	//TcpMsg* TcpMsg = ((TcpMsg*) r);
	while (TcpClient->m_isRuning)
	{
		//CCLOG("th_recv");
		//TcpMsg::mutex.lock();
		TcpClient->recvFunc();
		//TcpMsg::mutex.unlock();
#ifdef WIN32
		Sleep(SLEEP_TIME);
#else
		sleep(SLEEP_TIME / 1000.0f);
#endif
	}
	CCLOG("th_recv end");
}