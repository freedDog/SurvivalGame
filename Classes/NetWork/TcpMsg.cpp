#include "TcpMsg.h"

std::mutex TcpMsg::mutex;

static TcpMsg _sharedContext;

TcpMsg* TcpMsg::shareTcpMsg() 
{
	static bool s_bFirstUse = true;
	if (s_bFirstUse) 
	{
		s_bFirstUse = false;
	}
	return &_sharedContext;
}

TcpMsg::TcpMsg() :
	m_isRuning(false), m_recvQueue(NULL), m_sendQueue(NULL)
{
	m_recvQueue = new Queue();
	m_sendQueue = new Queue();
}

TcpMsg::~TcpMsg() 
{
	CC_SAFE_DELETE(m_recvQueue);
	CC_SAFE_DELETE(m_sendQueue);
}

void TcpMsg::tcp_start() 
{
	m_isRuning = true;
	m_socket.Connect(SERVER_HOST, SERVER_PORT);

	std::thread tRecv(&TcpMsg::th_recv, this, this);
	std::thread tSend(&TcpMsg::th_send, this, this);
	tRecv.detach();
	tSend.detach();

	CCLOG("tcp_start");
}

void TcpMsg::tcp_stop()
{
	m_isRuning = false;
	CCLOG("tcp_stop");
}

bool TcpMsg::isRuning()
{
	return m_isRuning;
}

Pocket* TcpMsg::MakePocket(const char* msg, unsigned int msgid)
{
	Pocket* pk = new Pocket();

	pk->length = strlen(msg);
	pk->id = msgid;
	pk->version = 1;
	pk->msg = new char[pk->length + 1];
	memset(pk->msg, 0, pk->length + 1);
	memcpy(pk->msg, msg, pk->length);

	return pk;
}

Pocket* TcpMsg::MakePocketFromData(const char* data)
{
	Pocket* pk = new Pocket;
	return pk;
}

Queue* TcpMsg::getRecvQueue()
{
	return m_recvQueue;
}

Queue* TcpMsg::getSendQueue()
{
	return m_sendQueue;
}

void TcpMsg::pushSendQueue(std::string str, unsigned int msgid)
{
	Pocket* pk = TcpMsg::MakePocket(str.c_str(), msgid);
	if (m_sendQueue!=NULL) 
	{
		m_sendQueue->PushPocket(pk);
	}
	CCLOG("m_sendQueue have %d", m_sendQueue->getSize());
}

void TcpMsg::pushRecvQueue(std::string str, unsigned int msgid)
{
	Pocket* pk = TcpMsg::MakePocket(str.c_str(),msgid);
	if (m_recvQueue!=NULL) 
	{
		m_recvQueue->PushPocket(pk);
	}
	CCLOG("m_recvQueue have %d", m_recvQueue->getSize());
}

void TcpMsg::tcpCheck(void)
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

//���������߼�
void TcpMsg::sendFunc(void)
{
	//������
	if (m_sendQueue != NULL&&m_sendQueue->getSize() > 0)
	{
		Pocket* pk = m_sendQueue->PullPocket();
		if (pk->id>0) 
		{
			char* sendData = new char[pk->length + 8];
			memset(sendData, 0, pk->length + 8);

			sendData[0] = ((pk->length + 6) >> 8) & 0xff;
			sendData[1] = (pk->length + 6) & 0xff;

			sendData[2] = (pk->version >> 8) & 0xff;
			sendData[3] = pk->version & 0xff;

			sendData[4] = (pk->id >> 24) & 0xff;
			sendData[5] = (pk->id >> 16) & 0xff;
			sendData[6] = (pk->id >> 8) & 0xff;
			sendData[7] = pk->id & 0xff;
			memcpy(sendData + 8, pk->msg, pk->length);

			int s = m_socket.Send(sendData, pk->length + 8, 0);

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

//���������߼�;
void TcpMsg::recvFunc(void)
{
	//������;
	tcpCheck();

	//ȡ��Ϣͷ���汾��Э��;
	//char recvHeadBuf[8] = "\0";
	char* recvHeadBuf = new char[8];
	memset(recvHeadBuf, 0, 8);
	int recvLen = m_socket.Recv(recvHeadBuf, 8, 0);
	if (recvLen != 8 && recvLen != -1)
	{
		while (recvLen < 8)
		{
			int tLen = m_socket.Recv(recvHeadBuf + recvLen, sizeof(recvHeadBuf - recvLen), 0);//һֱ��������Ϣͷ������;
			if (tLen != -1)
			{
				recvLen += tLen;
			}
		}
	}

	if (recvLen != -1)
	{
		CCLOG("recvFunc");

		//�����ϢժҪ��Ϣ
		unsigned short bodyLen, version;
		unsigned int protocol;
		bodyLen = recvHeadBuf[0] << 8 | recvHeadBuf[1];
		version = recvHeadBuf[2] << 8 | recvHeadBuf[3];
		protocol = (recvHeadBuf[4] << 24) | (recvHeadBuf[5] << 16) | (recvHeadBuf[6] << 8) | (recvHeadBuf[7] & 0xff);

		CCLOG("bodyLen:%d,version:%d,protocol:%d,{%02x,%02x,%02x,%02x}", bodyLen, version, protocol, recvHeadBuf[4], recvHeadBuf[5], recvHeadBuf[6], recvHeadBuf[7] & 0xff);

		//������Ϣ��;
		int readSize = 0; //��ǰ��ȡ��Ϣ�ĳ���;
		std::string rec_msg;
		//�����Ϣ�峤�ȴ���0;
		if (bodyLen > 6)
		{
			int msgLen = bodyLen - 6;
			char* msgBody = new char[msgLen + 1];
			memset(msgBody, 0, msgLen + 1);
			while (readSize < msgLen)
			{
				int recvLen = m_socket.Recv(msgBody + readSize, msgLen - readSize, 0);//һֱ��������Ϣ�������;
				if (recvLen != -1)
				{
					readSize += recvLen;
					if (readSize == msgLen)
					{
						rec_msg.assign((const char *)msgBody, msgLen);
						CCLOG("RECV: %d", msgLen);
						pushRecvQueue(rec_msg, protocol);//����Ϣ��ӵ�������;
						break;
					}
				}
			}
			CC_SAFE_DELETE_ARRAY(msgBody);
		}
		CC_SAFE_DELETE_ARRAY(recvHeadBuf);
	}
}

void TcpMsg::th_send(TcpMsg* TcpMsg)
{
	CCLOG("th_send start");
	//TcpMsg* TcpMsg = ((TcpMsg*) r);
	while (TcpMsg->m_isRuning)
	{
		//CCLOG("th_send");
		//TcpMsg::mutex.lock();
		TcpMsg->sendFunc();
		//TcpMsg::mutex.unlock();
#ifdef WIN32
		Sleep(SLEEP_TIME);
#else
		sleep(SLEEP_TIME / 1000.0f);
#endif
	}
	CCLOG("th_send end");
}

void TcpMsg::th_recv(TcpMsg* TcpMsg)
{
	CCLOG("th_recv start");
	//TcpMsg* TcpMsg = ((TcpMsg*) r);
	while (TcpMsg->m_isRuning)
	{
		//CCLOG("th_recv");
		//TcpMsg::mutex.lock();
		TcpMsg->recvFunc();
		//TcpMsg::mutex.unlock();
#ifdef WIN32
		Sleep(SLEEP_TIME);
#else
		sleep(SLEEP_TIME / 1000.0f);
#endif
	}
	CCLOG("th_recv end");
}