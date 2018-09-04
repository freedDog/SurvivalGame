#ifndef __FIGHT_SCENE_H__
#define __FIGHT_SCENE_H__
#include "cocos2d.h"
/**
	战斗场景
*/

class FightGame:public cocos2d::Scene
{
public:
	//初始化场景
	virtual bool init();

	static Scene* createScene();

	//手动实现 "static create()" 方法
	CREATE_FUNC(FightGame);

private:

};
#endif // !__FIGHT_SCENE_H__

