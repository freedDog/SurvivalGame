#ifndef __FIGHT_SCENE_H__
#define __FIGHT_SCENE_H__
#include "cocos2d.h"
/**
	ս������
*/

class FightGame:public cocos2d::Scene
{
public:
	//��ʼ������
	virtual bool init();

	static Scene* createScene();

	//�ֶ�ʵ�� "static create()" ����
	CREATE_FUNC(FightGame);

private:

};
#endif // !__FIGHT_SCENE_H__

