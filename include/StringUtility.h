#ifndef _UTILITY_H_
#define _UTILITY_H_

//将值转换为字符串
template<typename T> string toString( T value )
{
	std::stringstream ss(std::stringstream::out);
	ss << value;
	return ss.str();
}

//将字符串转换为值,如果不成功返回默认值
template<typename T> T toValue( const string& s,T def )
{
	T value;
	try
	{
		value = boost::lexical_cast<T>(s);
	}catch(boost::bad_lexical_cast& e )
	{
		ERROR_LOG(e.what());
		return def;
	}
	return value;
}

string vec3toString( const Ogre::Vector3& v );
string quatoString( const Ogre::Quaternion& q );
Ogre::Vector3 toVec3( const string& str );
Ogre::Quaternion toQua( const string& str );

#endif