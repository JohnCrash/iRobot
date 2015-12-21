#ifndef _LUADEBUG_H_
#define _LUADEBUG_H_

//#define __STDC_WANT_SECURE_LIB__ 1
#define __STDC_WANT_SECURE_LIB__ 1
//#define _CRT_NONSTDC_NO_DEPRECATE 1

#include "boost/thread.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/xpressive/xpressive_dynamic.hpp"
#include "boost/asio.hpp"
#include "boost/format.hpp"

using namespace std;
using namespace boost;
using namespace boost::asio;

typedef boost::shared_ptr<ip::tcp::socket> SocketPtr;
typedef boost::shared_ptr<string> StringPtr;
typedef vector<pair<int,string>> BPS;
#define nullptr 0
typedef boost::function<void(string, int) > BreakFunction;
typedef boost::function<void() > NewDebugFunction;
typedef boost::function<void(string)> GetInfoFunction;

class LuaDebug
{
public:
	LuaDebug();
	virtual ~LuaDebug();
	void run();
	void stop();
	void doContinue();
	void doStep();
	void doStepIn();
	void doReset();
	void doTraceback();
	void doTracefront();
	void doGetVariable(const string& name);
	void addBreakpoint( string source,int line );
	void removeBreakpoint( string source,int line );
	void removeAll();
	void setGetInfoNotify( GetInfoFunction gif )
	{
		GetInfoNotify = gif;
	}
	void setBreakNotify( BreakFunction bf )
	{
		Break=bf;
	}
	void setErrorPointNotify( BreakFunction bf )
	{
		Error=bf;
	}
	void setNewDebug( NewDebugFunction ndf )
	{
		NewDebug = ndf;
	}
	void setErrorNotify( GetInfoFunction ndf )
	{
		ErrorNotify = ndf;
	}

	static wstring toUnicode( const string& s );
	static string toUTF8( const wstring& wstr );
	static wstring utf8ToUnicode( const string& str );
protected:
	void async_accept();
	void accept_handler(const system::error_code& e,SocketPtr sock);
	void write_handler(const system::error_code& e,std::size_t bytes_transferred);
	void readline_handler( const system::error_code& e,std::size_t size );
	void listener();

	io_service mIos;
	SocketPtr mSocket;
	ip::tcp::acceptor mAcceptor;
	thread* mThread;
	boost::asio::streambuf mInbuf;
	BPS bps;
	string mContinue;
	string mStep;
	string mStepIn;
	BreakFunction Break;
	BreakFunction Error;
	NewDebugFunction NewDebug;
	GetInfoFunction GetInfoNotify;
	GetInfoFunction ErrorNotify;
};

#endif