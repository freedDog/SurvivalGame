#ifndef __LOGIN_SCENE_H__
#define __LOGIN_SCENE_H__
#include "cocos2d.h"

#if _DEBUG
#pragma comment(lib, "libprotobufd.lib")
#pragma comment(lib, "libprotocd.lib")
#else
#pragma comment(lib, "libprotobuf.lib")
#pragma comment(lib, "libprotoc.lib")
#endif

USING_NS_CC;

/**
	登录场景
*/

class LoginGame:public cocos2d::Scene
{
public:
	//初始化场景
	virtual bool init();

	static Scene* createScene();
	//登录游戏
	void onLogin(Ref* pSender);
	//创建角色
	void createRole(Ref* pSender);
	//删除角色
	void deleteRole(Ref* pSender);
	//退出游戏
	void quitGame(Ref* pSender);
	//手动实现 "static create()" 方法
	CREATE_FUNC(LoginGame);

private:

};

#endif // !__LOGIN_SCENE_H__
