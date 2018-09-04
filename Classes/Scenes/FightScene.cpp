#include "FightScene.h"

USING_NS_CC;

Scene* FightGame::createScene()
{
	return FightGame::create();
}

bool FightGame::init()
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

		//ÉèÖÃ±³¾°
		Sprite* bg = Sprite::create("HelloWorld.png");
		bg->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));

		this->addChild(bg, 0);

		bRet = true;
	} while (0);
	return bRet;
}