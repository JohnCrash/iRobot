#include "stdheader.h"
#include "StringUtility.h"

string vec3toString( const Ogre::Vector3& v )
{
	std::stringstream ss(std::stringstream::out);
	ss << v.x << " " << v.y << " " << v.z;
	return ss.str();
}

Ogre::Vector3 toVec3( const string& str,const Ogre::Vector3& def  )
{
	std::stringstream ss(str,std::stringstream::in);
	Ogre::Vector3 v;
	if(ss >> v.x >> v.y >> v.z)
    {
        return v;
    }
	return def;
}

Ogre::Quaternion toQua( const string& str,const Ogre::Quaternion& def  )
{
	std::stringstream ss(str,std::stringstream::in);
	Ogre::Quaternion q;
	if( ss >> q.x >> q.y >> q.z >> q.w )
    {
        return q;
    }
	return def;
}

string quatoString( const Ogre::Quaternion& q )
{
	std::stringstream ss(std::stringstream::out);
	ss << q.x << " " << q.y << " " << q.z << " " <<q.w;
	return ss.str();
}