#include "LoginScene.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

Scene* LoginGame::createScene()
{

	return LoginGame::create();
}

bool LoginGame::init()
{
	bool bRet = false;
	do
	{
		if (!Scene::init())
		{
			return bRet;
		}
		auto visibleSize = Director::getInstance()->getVisibleSize();
		Vec2 origin = Director::getInstance()->getVisibleOrigin();

		auto closeItem = MenuItemImage::create("ui/CloseNormal.png", "ui/CloseSelected.png", CC_CALLBACK_1(LoginGame::quitGame, this));
		closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width / 2,
			origin.y + closeItem->getContentSize().height / 2));

		auto startItem = MenuItemImage::create("ui/CloseSelected.png", "ui/CloseNormal.png", CC_CALLBACK_1(LoginGame::onLogin, this));
		startItem->setPosition(Vec2(origin.x + visibleSize.width,
			origin.y + startItem->getContentSize().height));

		auto loginMenu = Menu::create();
		loginMenu->addChild(closeItem);
		loginMenu->addChild(startItem);
		loginMenu->setPosition(Vec2::ZERO);
		this->addChild(loginMenu, 1);

		//ÉèÖÃ±³¾°
		Sprite* bg = Sprite::create("HelloWorld.png");
		bg->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));

		this->addChild(bg, 0);

		bRet = true;
	} while (0);
	return bRet;
}

void LoginGame::onLogin(Ref* pSender)
{


}

void LoginGame::createRole(Ref* pSender)
{

}
void LoginGame::deleteRole(Ref* pSender)
{

}

void LoginGame::quitGame(Ref* pSender)
{
	Director::getInstance()->end();
#if (CC_TARGET_PLATFORM==CC_PLATFORM_IOS)
	exit(0);
#endif // (CC_TARGET_PLATFORM==CC_PLATFORM_IOS)

}


