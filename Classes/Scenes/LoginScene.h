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
	��¼����
*/

class LoginGame:public cocos2d::Scene
{
public:
	//��ʼ������
	virtual bool init();

	static Scene* createScene();
	//��¼��Ϸ
	void onLogin(Ref* pSender);
	//������ɫ
	void createRole(Ref* pSender);
	//ɾ����ɫ
	void deleteRole(Ref* pSender);
	//�˳���Ϸ
	void quitGame(Ref* pSender);
	//�ֶ�ʵ�� "static create()" ����
	CREATE_FUNC(LoginGame);

private:

};

#endif // !__LOGIN_SCENE_H__
