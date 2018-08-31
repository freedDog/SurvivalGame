#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__
#include "cocos2d.h"
#include "Classes\NetWork\Pocket.h"
#include "Classes\NetWork\TcpMsg.h"

#include "Utils/CursorTextField.h"
#include "Data\HallLoginMessage.pb.h"


#if _DEBUG
#pragma comment(lib, "libprotobufd.lib")
#pragma comment(lib, "libprotocd.lib")
#else
#pragma comment(lib, "libprotobuf.lib")
#pragma comment(lib, "libprotoc.lib")
#endif


USING_NS_CC;

class HelloWorld : public cocos2d::Layer
{
public:
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();

	// there's no 'id' in cpp, so we recommand to return the exactly class pointer
	static Scene* createScene();
	// a selector callback
	void menuCloseCallback(Ref* pSender);
	void menuLinkCallback(Ref* pSender);
	void menuLoginCallback(Ref* pSender);
	void menuSendCallback(Ref* pSender);
	void menuAnimateCallback(Ref* pSender);
	void userLogin(Ref* pSender);

	// implement the "static node()" method manually
	CREATE_FUNC(HelloWorld);

	void msgLogic(float dt);

private:
	Label *txtLabel;
	CursorTextField *txtInput;
	int userid;
};

#endif // __HELLOWORLD_SCENE_H__