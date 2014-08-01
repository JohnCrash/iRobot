#ifndef _UTF8__H_
#define _UTF8__H_
#include <string>
#include <vector>

std::wstring toUnicode( const std::string& s );
std::wstring utf8ToUnicode( const std::string& s );
std::string toUTF8( const std::wstring& wstr );
void split_path(const std::string& r,std::vector<std::string>& vs);
std::string reversion(const std::string& s);

#endif