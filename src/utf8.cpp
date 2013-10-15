#include "stdheader.h"
#include "utf8.h"
#include "Log.h"

#ifdef WIN32
//将本地字串转变为utf8
std::string toUTF8( const std::string& s ){
	std::wstring wstr;
	std::string str;

	int len = MultiByteToWideChar(CP_ACP,0,(char *)s.c_str(),-1,NULL,NULL);
	if( len == 0 )
        goto _error_;
    
	wstr.resize( len );
	len = MultiByteToWideChar(CP_ACP,0,(char *)s.c_str(),-1,&wstr[0],wstr.size()); 
	if( len == 0 )
        goto _error_;
    
	len = WideCharToMultiByte(CP_UTF8,0,wstr.c_str(),-1,NULL,NULL,NULL,NULL);
	if( len == 0 )
        goto _error_;
    
	str.resize( len );
	len = WideCharToMultiByte(CP_UTF8,0,wstr.c_str(),-1,&str[0],str.size(),NULL,NULL); 
	if( len == 0 )
        goto _error_;
	/*
		该函数总是返回0结尾字串
	*/
	str.resize( len-1 );
	return str;
_error_:
    ERROR_LOG( "toUTF8 MultiByteToWideChar ErrorCode="<<GetLastError() );
    return "";
}
#else
//其他系统
std::string toUTF8( const std::string& s )
{
    return s;
}
#endif

MyGUI::UString toUString( const std::string& str )
{
	MyGUI::UString us;
	try
	{
		us = str;
	}
	catch( MyGUI::UString::invalid_data& )
	{
		//非法的utf8字符序列，尝试进行转换
		us = toUTF8( str );
	}
	return us;
}