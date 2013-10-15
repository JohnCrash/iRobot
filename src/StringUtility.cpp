#include "stdheader.h"
#include "StringUtility.h"

string vec3toString( const Ogre::Vector3& v )
{
	std::stringstream ss(std::stringstream::out);
	ss << v.x << " " << v.y << " " << v.z;
	return ss.str();
}

Ogre::Vector3 toVec3( const string& str )
{
	std::stringstream ss(str,std::stringstream::in);
	Ogre::Vector3 v;
	ss >> v.x;
	ss >> v.y;
	ss >> v.z;
	return v;
}

Ogre::Quaternion toQua( const string& str )
{
	std::stringstream ss(str,std::stringstream::in);
	Ogre::Quaternion q;
	ss >> q.x;
	ss >> q.y;
	ss >> q.z;
	ss >> q.w;
	return q;
}

string quatoString( const Ogre::Quaternion& q )
{
	std::stringstream ss(std::stringstream::out);
	ss << q.x << " " << q.y << " " << q.z << " " <<q.w;
	return ss.str();
}