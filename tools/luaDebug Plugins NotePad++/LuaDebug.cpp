#include "LuaDebug.h"

wstring LuaDebug::toUnicode( const string& s )
{
	std::wstring wstr;
	std::string str;

	int len = MultiByteToWideChar(CP_ACP,0,(char *)s.c_str(),-1,NULL,NULL);
	if( len == 0 )
	{  //失败
		return TEXT("");
	}
	wstr.resize( len );
	len = MultiByteToWideChar(CP_ACP,0,(char *)s.c_str(),-1,&wstr[0],wstr.size()); 
	if( len == 0 )
	{ //失败
		return TEXT("");
	}
	return wstr;
}

wstring LuaDebug::utf8ToUnicode( const string& s )
{
	std::wstring wstr;
	std::string str;

	int len = MultiByteToWideChar(CP_UTF8,0,(char *)s.c_str(),-1,NULL,NULL);
	if( len == 0 )
	{  //失败
		return TEXT("");
	}
	wstr.resize( len );
	len = MultiByteToWideChar(CP_UTF8,0,(char *)s.c_str(),-1,&wstr[0],wstr.size()); 
	if( len == 0 )
	{ //失败
		return TEXT("");
	}
	return wstr;
}

string LuaDebug::toUTF8( const wstring& wstr )
{
	std::string str;
	int len = WideCharToMultiByte(CP_UTF8,0,wstr.c_str(),-1,NULL,NULL,NULL,NULL); 
	if( len == 0 )
	{
		return "";
	}
	str.resize( len );
	len = WideCharToMultiByte(CP_UTF8,0,wstr.c_str(),-1,&str[0],str.size(),NULL,NULL);
	return str;
}

LuaDebug::LuaDebug():
mAcceptor(mIos,ip::tcp::endpoint(
					ip::address::from_string("127.0.0.1"),3030
			)),
mThread(nullptr),
mContinue("continue\n"),
mStep("step\n"),
mStepIn("stepin\n")
{
}

LuaDebug::~LuaDebug()
{
}
//
void LuaDebug::async_accept()
{
	SocketPtr s = SocketPtr(new ip::tcp::socket(mIos));
	mAcceptor.async_accept(*s,boost::bind(&LuaDebug::accept_handler,this,boost::asio::placeholders::error,s));
}

void LuaDebug::readline_handler( const system::error_code& e,std::size_t size )
{
	if( e )
	{
		mSocket.reset();
		return;
	}

	string str;
	istream is(&mInbuf);
	if( getline(is,str) )
	{
		xpressive::sregex setbp = xpressive::sregex::compile("break<([^\?<>|\"*]+)>@(\\d+)");
		xpressive::sregex infop = xpressive::sregex::compile("info:([^\\n]+)");
		xpressive::sregex errorp = xpressive::sregex::compile("error<([^<^>]*)>");
		xpressive::sregex errorpt = xpressive::sregex::compile("error<([^\?<>|\"*]+)>@(\\d+)");
		xpressive::smatch what;
		if( xpressive::regex_match(str,what,setbp) )
		{
			if( Break )
				Break( what[1],boost::lexical_cast<int>(what[2]) );
		}
		else if( xpressive::regex_match(str,what,infop) )
		{
			if( GetInfoNotify )
				GetInfoNotify(what[1]);
		}
		else if( xpressive::regex_match(str,what,errorpt) )
		{
			if( Error )
				Error( what[1],boost::lexical_cast<int>(what[2]) );
		}
		else if( xpressive::regex_match(str,what,errorp) )
		{
			if( ErrorNotify )
				ErrorNotify(what[1]);
		}
	}
	boost::asio::async_read_until( 
		*mSocket,
		mInbuf,
		'\n',
		bind(&LuaDebug::readline_handler, this, boost::asio::placeholders::error, boost::asio::placeholders::signal_number));
}

void LuaDebug::write_handler(const boost::system::error_code& e,std::size_t bytes_transferred)
{
	if( e )
	{
		mSocket.reset();
	}
}

void LuaDebug::doGetVariable(const string& name)
{
	if( mSocket )
	{
		string cmd("get<");
		cmd += name;
		cmd += ">\n";
		boost::asio::write( *mSocket,buffer(cmd,cmd.size()) );
	}	
}

void LuaDebug::doTraceback()
{
	if( mSocket )
	{
		string cmd("traceback\n");
		boost::asio::write( *mSocket,buffer(cmd,cmd.size()) );
	}
}

void LuaDebug::doTracefront()
{
	if( mSocket )
	{
		string cmd("tracefront\n");
		boost::asio::write( *mSocket,buffer(cmd,cmd.size()) );
	}
}

void LuaDebug::doContinue()
{
	if( mSocket )
	{
		boost::asio::async_write( *mSocket,
			buffer(mContinue,mContinue.size()),
			bind(&LuaDebug::write_handler, this, boost::asio::placeholders::error, boost::asio::placeholders::signal_number));
	}
}

void LuaDebug::doStep()
{
	if( mSocket )
	{
		boost::asio::async_write( *mSocket,
			buffer(mStep,mStep.size()),
			bind(&LuaDebug::write_handler, this, boost::asio::placeholders::error, boost::asio::placeholders::signal_number));
	}
}

void LuaDebug::doReset()
{
	if( mSocket )
	{
		string reset("reset\n");
		boost::asio::write( *mSocket,buffer(reset,reset.size()) );
	}
}

void LuaDebug::doStepIn()
{
	if( mSocket )
	{
		boost::asio::async_write( *mSocket,
			buffer(mStepIn,mStepIn.size()),
			bind(&LuaDebug::write_handler, this, boost::asio::placeholders::error, boost::asio::placeholders::signal_number));
	}
}

void LuaDebug::accept_handler(const system::error_code& e,SocketPtr s)
{
	if( e )
	{
		return;
	}

	async_accept();
	bps.clear(); //清除以前的断点
	if( NewDebug )
		NewDebug(); //通知产生了新的调试上下文
	mSocket = s;//该操作将关闭上一次的Socket

	for( BPS::iterator i = bps.begin();i!=bps.end();++i )
	{
		std::stringstream ss(std::stringstream::out);
		ss<<"bp<"<<i->second<<">@"<<i->first<<"\n";
		
		boost::asio::write( *mSocket,boost::asio::buffer(ss.str()) );
	}

	doContinue();

	boost::asio::async_read_until( *mSocket,
		mInbuf,
		'\n',
		bind(&LuaDebug::readline_handler, this, boost::asio::placeholders::error, boost::asio::placeholders::signal_number));
}

//加入断点
void LuaDebug::addBreakpoint( string source,int line )
{
	for( BPS::iterator i = bps.begin();i!=bps.end();++i )
	{
		if( i->first == line && i->second == source )
		{
			return;
		}
	}
	bps.push_back(std::pair<int,std::string>(line,source));
	if( mSocket && mSocket->is_open() )
	{
		std::stringstream ss(std::stringstream::out);
		ss<<"bp<"<<source<<">@"<<line<<"\n";
		mSocket->write_some(boost::asio::buffer(ss.str()));
	}
}

void LuaDebug::removeAll()
{
	bps.clear();
	if( mSocket && mSocket->is_open() )
	{
		mSocket->write_some(boost::asio::buffer("clearall\n"));
	}
}

void LuaDebug::removeBreakpoint( string source,int line )
{
	for( BPS::iterator i = bps.begin();i!=bps.end();++i )
	{
		if( i->first == line && i->second == source )
		{
			bps.erase(i);
			if( mSocket && mSocket->is_open() )
			{
				std::stringstream ss(std::stringstream::out);
				ss<<"clear<"<<source<<">@"<<line<<"\n";
				mSocket->write_some(boost::asio::buffer(ss.str()));
			}
			return;
		}
	}
}

void LuaDebug::listener()
{
	async_accept();

	mIos.run();
}

void LuaDebug::run()
{
	if( !mThread )
	{
		bps.clear();
		mThread = new boost::thread( bind(&LuaDebug::listener,this) );
	}
}

void LuaDebug::stop()
{
	if( mSocket )
		mSocket.reset();
	mIos.stop();

	if( mThread )
		delete mThread;
	mThread = nullptr;
}