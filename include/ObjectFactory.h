#ifndef _OBJECTFACTORY_H_
#define _OBJECTFACTORY_H_
#include "ClassInfo.h"

class ObjectFactory: public Ogre::Singleton<ObjectFactory>
{
public:
	ObjectFactory();
	virtual ~ObjectFactory();
	
	void registerFactory( const string& type,CreateObjectFunc func );
	
	template <typename Type> void registerFactory( const string& _type )
	{
		registerFactory(_type, GenericFactory<Type>::getFactory());
	}

	void unregisterFactory( const string& type );

	ObjectPtr createObject( const string& type );

	bool isExist( const string& type) const;
private:
	typedef map<string,CreateObjectFunc> TypeMap;
	TypeMap mTypes;
};

#endif