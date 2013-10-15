#include "stdheader.h"
#include "Geometry.h"
#include "ObjectFactory.h"

SINGLETON(ObjectFactory)

ObjectFactory::ObjectFactory()
{
}

ObjectFactory::~ObjectFactory()
{
}

void ObjectFactory::registerFactory( const string& type,CreateObjectFunc func )
{
	mTypes[type] = func;
}

void ObjectFactory::unregisterFactory( const string& type )
{
	if( mTypes.find(type)!=mTypes.end() )
		mTypes.erase(type);
}

bool ObjectFactory::isExist( const string& type) const
{
	return mTypes.find(type)!=mTypes.end();
}

ObjectPtr ObjectFactory::createObject( const string& type )
{
	if( mTypes.find(type)!=mTypes.end() )
	{
		return ObjectPtr(mTypes[type]());
	}
	else
	{
		ERROR_LOG("ObjectFactory can't create new object of type " << type);
		return ObjectPtr();
	}
}