#ifndef _UTF8_H_
#define _UTF8_H_

//将本地字串转换为utf8
std::string toUTF8( const std::string& str );
//尝试产生一个
MyGUI::UString toUString( const std::string& str );

#endif