#include "MobDebug.h"
#include "utf8.h"

void LuaMobDebug::search_dir(std::wstring dir,std::wstring pat)
{
	WIN32_FIND_DATA ff;
	std::wstring search_path;
	search_path.append(dir);
	search_path.append(TEXT("\\"));
	search_path.append(pat);
	HANDLE handle = FindFirstFile(search_path.c_str(),&ff);
	if(handle!=INVALID_HANDLE_VALUE)
	{
		while( FindNextFile(handle,&ff) )
		{
			std::wstring filename = ff.cFileName;
			if((ff.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)&&
				filename!=TEXT(".")&&filename!=TEXT(".."))
			{//目录
				search_dir( dir+TEXT("\\")+filename,TEXT("*"));
			}
			else if(ff.dwFileAttributes&FILE_ATTRIBUTE_ARCHIVE)
			{
				std::wstring filename = ff.cFileName;
				if( filename.length()>4 )
				{
					std::wstring ex = filename.substr(filename.length()-4);
					if (ex == TEXT(".lua") || ex == TEXT(".Lua") || ex == TEXT(".LUA"))
					{
						/*
						 * 文件的存放的目录的第一级当成键，与文件名用'/'链接
						 */
						int lp = dir.find_last_of('\\');
						if (lp != std::string::npos)
						{
							mLocalFileMap[dir.substr(lp+1) + TEXT("/") + filename] = dir + TEXT("\\") + filename;
						}
						else
						{ //没有发现路径？
							mLocalFileMap[filename] = dir + TEXT("\\") + filename;
						}
					}
				}
			}
		}
		FindClose(handle);
	}
}
/*
mAcceptor(mIos,ip::tcp::endpoint(
					ip::address::from_string("192.168.2.116"),8172
			)),
*/

LuaMobDebug::LuaMobDebug(std::string addr,int port):
	mAcceptor(mIos,ip::tcp::endpoint(ip::address::from_string(addr),8172)),
mThread(nullptr),
mContinue("RUN\n"),
mStep("OVER\n"),
mStepIn("STEP\n")
{
	isGetV = true;
	isRunning = true;
}

void LuaMobDebug::set_lua_source_root(std::wstring root)
{
	mLocalRoot = root;
	mLocalFileMap.clear();
	search_dir( mLocalRoot,TEXT("*"));
}

std::wstring LuaMobDebug::mapLocalToRemot(std::wstring local)
{
	/*
	if( local.length() > mLocalRoot.length()+1 )
	{
		std::wstring s = local.substr(mLocalRoot.length()+1);
		for(auto i=s.begin();i!=s.end();++i )
		{
			if( *i == '\\' )
				*i = '/';
		}
		return TEXT(".\\")+s;
	}*/
	std::vector<std::wstring> vs;
	split_path(local, vs);
	if (vs.size() > 1)
	{
		return vs[1] + TEXT("/") + vs[0];
	}
	else
		return local; 
}

std::wstring LuaMobDebug::mapRemotToLocal(const std::wstring r,bool &b)
{
	std::vector<std::wstring> vs;
	split_path(r,vs);
	if(vs.size()>1)
	{
		std::wstring key;
		if (vs[1] == TEXT(".") && vs.size()>2)
			key = vs[2] + TEXT("/") + vs[0];
		else
			key = vs[1]+TEXT("/")+vs[0];
		if( mLocalFileMap.find(key)!=mLocalFileMap.end())
		{
			b = true;
			return mLocalFileMap[key];
		}
	}
	//尝试绝对路径
	{
		std::string filename = toUTF8(r);
		FILE* fp = fopen(filename.c_str(),"r");
		if(fp)
		{
			fclose(fp);
			b = true;
			return r;
		}
	}
	b = false;
	return r;
}

LuaMobDebug::~LuaMobDebug()
{
	stop();
}
//
void LuaMobDebug::async_accept()
{
	SocketPtr s = SocketPtr(new ip::tcp::socket(mIos));
	mAcceptor.async_accept(*s,boost::bind(&LuaMobDebug::accept_handler,this,boost::asio::placeholders::error,s));
}

void LuaMobDebug::readline_handler( const system::error_code& e,std::size_t size )
{
	if( e )
	{
		mSocket.reset();
		return;
	}

	std::string str;
	std::istream is(&mInbuf);
	if( getline(is,str) )
	{
		/*
			200 OK
			202 Paused source line
			203 Paused source line watch_idx
			204 Output stream size
			401 Error in Execution size
		*/
		//xpressive::sregex setbp = xpressive::sregex::compile("break<([^\?<>|\"*]+)>@(\\d+)");
		xpressive::sregex bpbreak1 = xpressive::sregex::compile("202 Paused (\\S+) (\\d+)");
		xpressive::sregex bpbreak2 = xpressive::sregex::compile("203 Paused (\\S+) (\\d+) (\\d+)");
		xpressive::sregex infop = xpressive::sregex::compile("GETV (.*)");
		xpressive::sregex errorp = xpressive::sregex::compile("ERRMSG (.+)");
		xpressive::sregex errorpt = xpressive::sregex::compile("ERROR (\\S+) (\\d+)");
		xpressive::sregex stackp = xpressive::sregex::compile("200 OK (.+)");
		
		xpressive::smatch what;
		if( xpressive::regex_match(str,what,bpbreak2)||xpressive::regex_match(str,what,bpbreak1) )
		{
			if( Break )
			{
				std::wstring remoate = toUnicode(what[1]);
				int line = boost::lexical_cast<int>(what[2]);
				bool b;
				std::wstring source = mapRemotToLocal(remoate,b);
				if( b )
				{
					doStepCount = 0;
					lastBreakSource = remoate;
					lastBreakLine = line;
					Break( source,line );
					isRunning = false;
					lastErrorMsg.clear();
				}
				else
				{
					std::wstring msg = remoate;
					MessageBox(NULL,msg.c_str(),TEXT("File not found"),MB_OK);
				}
			}
		}
		else if( xpressive::regex_match(str,what,stackp) )
		{
			std::string s = what[1];
			parseStack( s );
		}
		else if( xpressive::regex_match(str,what,infop) )
		{
			if( GetInfoNotify )
			{
				std::wstring ss = toUnicode(lastGetV)+TEXT("\n")+toUnicode(what[1]);
				if (!ss.empty())
				{
					for (auto it = ss.begin(); it != ss.end(); ++it)
					{
						if (*it == '\t')
							*it = '\n';
					}
				}
				else
				{
					ss = TEXT("empty");
				}
				GetInfoNotify(toUTF8(ss));
			}
		}
		else if( xpressive::regex_match(str,what,errorpt) )
		{
			if( Error )
			{
				std::wstring remoate = toUnicode(what[1]);
				int line = boost::lexical_cast<int>(what[2]);
				bool b;
				std::wstring source = mapRemotToLocal(remoate,b);
				if(b)
					Error( source,line);
				else
				{
					std::wstring msg = remoate;
					MessageBox(NULL,msg.c_str(),TEXT("File not found"),MB_OK);
				}
			}
		}
		else if( xpressive::regex_match(str,what,errorp) )
		{
			if( ErrorNotify )
			{
				lastErrorMsg = what[1];
				ErrorNotify(lastErrorMsg);
			}
		}
	}
	boost::asio::async_read_until( 
		*mSocket,
		mInbuf,
		'\n',
		bind(&LuaMobDebug::readline_handler, this, boost::asio::placeholders::error, boost::asio::placeholders::signal_number));
}

void LuaMobDebug::parseStack( const std::string& s )
{
	auto start = s.find_first_of('{',0);
	auto end = s.rfind('}');
	if( start != std::string::npos && end != std::string::npos )
	{
		auto ss = s.substr(start,end-start+1);
		printf("%s",ss.c_str());
	}
}

void LuaMobDebug::getStack()
{
	if( mSocket )
	{
		std::string cmd("STACK\n");
		boost::asio::write( *mSocket,buffer(cmd,cmd.size()) );
	}
}

void LuaMobDebug::write_handler(const boost::system::error_code& e,std::size_t bytes_transferred)
{
	if( e )
	{
		mSocket.reset();
	}
}

void LuaMobDebug::doGetVariable(const std::string& name)
{
	if( mSocket && !isRunning && isGetV )
	{
		std::string cmd("GETV ");
		cmd += name;
		cmd += "\n";
		lastGetV = name;
		boost::asio::write( *mSocket,buffer(cmd,cmd.size()) );
	}else if(!lastErrorMsg.empty())
	{
		GetInfoNotify(lastErrorMsg);
	}
}

void LuaMobDebug::doTraceback()
{
	if( mSocket )
	{
		std::string cmd("TRACEBACK\n");
		boost::asio::write( *mSocket,buffer(cmd,cmd.size()) );
	}
}

void LuaMobDebug::doTracefront()
{
	if( mSocket )
	{
		std::string cmd("TRACEFRONT\n");
		boost::asio::write( *mSocket,buffer(cmd,cmd.size()) );
	}
}

void LuaMobDebug::doContinue()
{
	if( mSocket )
	{
		lastErrorMsg.clear();
		isRunning = true;
		boost::asio::async_write( *mSocket,
			buffer(mContinue,mContinue.size()),
			bind(&LuaMobDebug::write_handler, this, boost::asio::placeholders::error, boost::asio::placeholders::signal_number));
	}
}

void LuaMobDebug::doStep()
{
	if( mSocket )
	{
		isRunning = true;
		boost::asio::async_write( *mSocket,
			buffer(mStep,mStep.size()),
			bind(&LuaMobDebug::write_handler, this, boost::asio::placeholders::error, boost::asio::placeholders::signal_number));
	}
}

void LuaMobDebug::doReset()
{
	if( mSocket )
	{
		lastErrorMsg.clear();
		isRunning = true;
		std::string reset("EXIT\n");
		boost::asio::write( *mSocket,buffer(reset,reset.size()) );
	}
}

void LuaMobDebug::doStepIn()
{
	if( mSocket )
	{
		isRunning = true;
		boost::asio::async_write( *mSocket,
			buffer(mStepIn,mStepIn.size()),
			bind(&LuaMobDebug::write_handler, this, boost::asio::placeholders::error, boost::asio::placeholders::signal_number));
	}
}

void LuaMobDebug::accept_handler(const system::error_code& e,SocketPtr s)
{
	if( e )
	{
		mIos.stop();
		return;
	}

	async_accept();
	bps.clear(); //清除以前的断点
	if( NewDebug )
		NewDebug(); //通知产生了新的调试上下文
	mSocket = s;//该操作将关闭上一次的Socket

	for( BPS::iterator i = bps.begin();i!=bps.end();++i )
	{
		std::wstring remoat_source = mapLocalToRemot(i->second);
		std::stringstream ss(std::stringstream::out);
		ss<<"SETB "<< toUTF8(remoat_source) <<" "<<i->first<<"\n";
		
		boost::asio::write( *mSocket,boost::asio::buffer(ss.str()) );
	}

	doContinue();
	//doStep();

	boost::asio::async_read_until( *mSocket,
		mInbuf,
		'\n',
		bind(&LuaMobDebug::readline_handler, this, boost::asio::placeholders::error, boost::asio::placeholders::signal_number));
}

//加入断点
void LuaMobDebug::addBreakpoint( std::wstring source,int line )
{
	for( BPS::iterator i = bps.begin();i!=bps.end();++i )
	{
		if( i->first == line && i->second == source )
		{
			return;
		}
	}
	bps.push_back(std::pair<int,std::wstring>(line,source));
	if( mSocket && mSocket->is_open() )
	{
		std::stringstream ss(std::stringstream::out);
		std::wstring remoat_source = mapLocalToRemot(source);
		ss<<"SETB "<< toUTF8(remoat_source)<<" "<<line<<"\n";
		mSocket->write_some(boost::asio::buffer(ss.str()));
	}
}

void LuaMobDebug::removeAll()
{
	if( mSocket && mSocket->is_open() )
	{
		for( auto i =bps.begin();i!=bps.end();++i )
		{
			std::stringstream ss(std::stringstream::out);
			std::wstring remoat_source = mapLocalToRemot(i->second);
			ss<<"DELB "<< toUTF8(remoat_source) <<" "<<i->first<<"\n";
			mSocket->write_some(boost::asio::buffer(ss.str()));			
		}
	}
	bps.clear();
}

void LuaMobDebug::removeBreakpoint( std::wstring source,int line )
{
	for( BPS::iterator i = bps.begin();i!=bps.end();++i )
	{
		if (i->first == line && wcscmp(source.c_str(),i->second.c_str())==0)
		{
			bps.erase(i);
			if( mSocket && mSocket->is_open() )
			{
				std::stringstream ss(std::stringstream::out);
				std::wstring remoat_source = mapLocalToRemot(source);
				ss<<"DELB "<< toUTF8(remoat_source) <<" "<<line<<"\n";
				mSocket->write_some(boost::asio::buffer(ss.str()));
			}
			return;
		}
	}
}

void LuaMobDebug::listener()
{
	try
	{
		async_accept();

		mIos.run();

		mSocket.reset();
	}catch(std::exception& e)
	{
		return;
	}
}

void LuaMobDebug::run()
{
	if( !mThread )
	{
		bps.clear();
		mThread = new thread( std::bind(&LuaMobDebug::listener,this) );
	}
}

void LuaMobDebug::stop()
{
	try
	{
		mAcceptor.close();
	}catch(std::exception &e)
	{
	}

	if( mThread )
	{
		mThread->join();
		delete mThread;
	}
	mThread = nullptr;
}