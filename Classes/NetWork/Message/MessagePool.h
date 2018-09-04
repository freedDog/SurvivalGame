#pragma once

#include <map>

using namespace std;
/**
	消息索引类，用来注册消息和消息处理
*/
class  MessagePool
{
public:
	 MessagePool();
	~ MessagePool();

	//初始化
	void init();
	//获取消息体
	void getMessage(int msgId);
	//获取消息处理器
	void getMessageHandler(int msgId);
public:

private:
	map<int, string> handlers;
};
