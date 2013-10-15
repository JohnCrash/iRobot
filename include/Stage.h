/*
	Stage的派生类包括Factory
*/
#ifndef _STAGE_H_
#define _STAGE_H_
#include "Input.h"

class Stage
{
public:
	virtual bool openStage() = 0; //展示Stage物体包括界面
	virtual void closeStage() = 0; //仅仅从Stage上撤下物体，并不真正删除Stage本身
};

#endif