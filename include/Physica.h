#ifndef _PHYSICA_H_
#define _PHYSICA_H_

class PhysicaEngine:
	public Ogre::Singleton<PhysicaEngine>
{
public:
	PhysicaEngine();
	virtual ~PhysicaEngine();
protected:
	dWorldID mWorld;
	dSpaceID mSpace;
};

#endif
