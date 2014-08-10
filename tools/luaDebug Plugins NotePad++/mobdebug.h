#ifndef _LuaMobDebug_H_
#define _LuaMobDebug_H_

//#define __STDC_WANT_SECURE_LIB__ 1

#include "boost/thread.hpp"
#include <boost/smart_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include "boost/algorithm/string.hpp"
#include "boost/xpressive/xpressive_dynamic.hpp"
#include "boost/asio.hpp"
#include <boost/format.hpp>

using namespace boost;
using namespace boost::asio;

typedef shared_ptr<ip::tcp::socket> SocketPtr;
typedef shared_ptr<std::wstring> StringPtr;
typedef std::vector<std::pair<int,std::wstring>> BPS;

typedef function<void (std::wstring,int) > BreakFunction;
typedef function<void () > NewDebugFunction;
typedef function<void (std::string)> GetInfoFunction;

class LuaMobDebug
{
public:
	LuaMobDebug(std::string addr,int port);
	virtual ~LuaMobDebug();
	void run();
	void stop();
	void doContinue();
	void getStack();
	void parseStack( const std::string& s );
	void doStep();
	void doStepIn();
	void doReset();
	void doTraceback();
	void doTracefront();
	void doGetVariable(const std::string& name);
	void addBreakpoint( std::wstring source,int line );
	void removeBreakpoint( std::wstring source,int line );
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
	void set_lua_source_root(std::wstring root);
	void enableGetV()
	{
		isGetV = !isGetV;
	}
protected:
	void async_accept();
	void accept_handler(const system::error_code& e,SocketPtr sock);
	void write_handler(const system::error_code& e,std::size_t bytes_transferred);
	void readline_handler( const system::error_code& e,std::size_t size );
	void listener();
	std::wstring mapRemotToLocal(std::wstring str,bool &b);
	void search_dir(std::wstring dir,std::wstring pat);
	std::wstring mapLocalToRemot(std::wstring local);

	io_service mIos;
	SocketPtr mSocket;
	ip::tcp::acceptor mAcceptor;
	thread* mThread;
	boost::asio::streambuf mInbuf;
	BPS bps;
	std::string mContinue;
	std::string mStep;
	std::string mStepIn;
	BreakFunction Break;
	BreakFunction Error;
	NewDebugFunction NewDebug;
	GetInfoFunction GetInfoNotify;
	GetInfoFunction ErrorNotify;

	std::wstring mLocalRoot; //本地代码根
	std::wstring mRemoteRoot; //远程根
	std::map<std::wstring,std::wstring> mLocalFileMap;
	int lastBreakLine;
	std::wstring lastBreakSource;
	int doStepCount;
	bool isRunning;
	bool isGetV;
	std::string lastGetV;
	std::string lastErrorMsg;
};
#endif