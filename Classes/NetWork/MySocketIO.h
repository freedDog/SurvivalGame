#pragma once

#include "cocos2d.h"
#include "network\SocketIO.h"

using namespace cocos2d;
using namespace cocos2d::network;

class MySocketIO :public Layer, SocketIO::SIODelegate
{
public:
	static Scene* createScene();
	CREATE_FUNC(MySocketIO);
	bool init();
	virtual void onConnect(SIOClient* client);
	virtual void onMessage(SIOClient* client, const std::string& data);

	virtual void onClose(SIOClient* client);
	virtual void onError(SIOClient* client, const std::string& data);

	void sendEvent();
	void closeSokcet();

	SIOClient* _socketlient;

};