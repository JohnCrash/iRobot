#include "stdheader.h"
#include "Physica.h"

SINGLETON( PhysicaEngine )

PhysicaEngine::PhysicaEngine(){
	dInitODE2(0);
	mWorld = dWorldCreate();
	mSpace = dHashSpaceCreate(0);
	//设置重力加速度
	dWorldSetGravity(mWorld,0,-9.81,0);
}

PhysicaEngine::~PhysicaEngine(){
	dSpaceDestroy(mSpace);
	dWorldDestroy(mWorld);
	dCloseODE();
}