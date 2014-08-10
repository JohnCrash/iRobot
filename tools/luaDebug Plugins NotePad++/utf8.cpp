#include "utf8.h"
#include <Windows.h>
std::wstring reversion(const std::wstring& s)
{
	std::wstring r;
	for(auto i=s.rbegin();i!=s.rend();++i)
		r += *i;
	return r;
}

void split_path(const std::wstring& r,std::vector<std::wstring>& vs)
{
	std::wstring s;
	if( !r.empty() )
	{
		for( int i=r.length()-1;i>=0;i-- )
		{
			if( r.at(i)!='/'&&r.at(i)!='\\' )
			{
				s += r.at(i);
			}else
			{
				vs.push_back(reversion(s));
				s.clear();
			}
		}
		if( !s.empty() )
		{
			vs.push_back(reversion(s));
			s.clear();
		}
	}
}

std::wstring toUnicode( const std::string& s )
{
	std::wstring wstr;
	std::string str;

	int len = MultiByteToWideChar(CP_ACP,0,(char *)s.c_str(),-1,NULL,NULL);
	if( len == 0 )
	{  //Ê§°Ü
		return TEXT("");
	}
	wstr.resize( len );
	len = MultiByteToWideChar(CP_ACP,0,(char *)s.c_str(),-1,&wstr[0],wstr.size()); 
	if( len == 0 )
	{ //Ê§°Ü
		return TEXT("");
	}
	if( wstr.back()==0 )
		wstr.pop_back();
	return wstr;
}

std::wstring utf8ToUnicode( const std::string& s )
{
	std::wstring wstr;
	std::string str;

	int len = MultiByteToWideChar(CP_UTF8,0,(char *)s.c_str(),-1,NULL,NULL);
	if( len == 0 )
	{  //Ê§°Ü
		return TEXT("");
	}
	wstr.resize( len );
	len = MultiByteToWideChar(CP_UTF8,0,(char *)s.c_str(),-1,&wstr[0],wstr.size()); 
	if( len == 0 )
	{ //Ê§°Ü
		return TEXT("");
	}
	if( wstr.back()==0 )
		wstr.pop_back();
	return wstr;
}

std::string toUTF8( const std::wstring& wstr )
{
	std::string str;
	int len = WideCharToMultiByte(CP_UTF8,0,wstr.c_str(),-1,NULL,NULL,NULL,NULL); 
	if( len == 0 )
	{
		return "";
	}
	str.resize( len );
	len = WideCharToMultiByte(CP_UTF8,0,wstr.c_str(),-1,&str[0],str.size(),NULL,NULL);
	if( str.back()==0 )
		str.pop_back();
	return str;
}