#ifndef _LUAMANAGER_H_
#define _LUAMANAGER_H_

struct LuaBind;

class LuaManager : 
	public MyGUI::IUnlinkWidget,
	public Ogre::Singleton<LuaManager>
{
public:
	LuaManager();
	virtual ~LuaManager();

	void doFile( string file ); //执行脚本文件
	
	/*
		registerClass用来注册原表
	*/
	void registerClass( string name,const luaL_Reg* lib );
	void registerClass( string name,const luaL_Reg* base,const luaL_Reg* derived );
	void registerClass( string name,const luaL_Reg* base,const luaL_Reg* derived,
		const luaL_Reg* derived2 );
	void registerClass( string name,const luaL_Reg* base,const luaL_Reg* derived,
		const luaL_Reg* derived2,const luaL_Reg* derived3 );

	void registerGlobal( const char* n,const luaL_Reg* lib );
	
	void registerCFunction( const char* name,lua_CFunction func );
	/*
		C++调用Lua函数的接口
	*/
	void callVoid( const char* f ); //调用一个无参数函数
	void callString( const char* f,const char* s ); //调用一个字符串的函数
	void callTableVoid( const char* t,const char* f ); //调用表t中的函数f
	void callRegVoid( const char* t,const char* f ); //调用注册表t中的函数f

	void callBindEventVoid( void* p,const char* f );
	void callBindEvent3Int( void* p,const char* f,int i,int j,int k ); //fun(int,int,int)
	void callBindEvent2Int( void* p,const char* f,int i,int j ); //fun(int,int)
	void callBindEvent1Int( void* p,const char* f,int i ); //fun(int,int)
	void callBindEvent1Obj( void* p,const char* f,void* pp );
	void callBindEvent1Bool( void* p,const char* f,bool b );
	void callBindEvent1String(void* p,const char* f,const char* s);
	void callBindEvent2String( void* p,const char* f,const char* s1,const char* s2);
	bool callBindEventDDContainer( void* p,const char* f,
		MyGUI::Widget* _s,int _si,MyGUI::Widget* _r,int _ri );
	void callBindEvent4Int1Bool( void* p,const char* f,int i1,int i2,int i3,int i4,bool b );
	void callBindEvent1Obj1IBDrawItemInfo( void* p,const char* f,
		MyGUI::Widget* _item,const MyGUI::IBDrawItemInfo& _info );
	void callBindEvent1IBDrawItemInfo( void* p,const char* f,
		const MyGUI::IBNotifyItemData& _info );

	/*
		lua可以注册一系列的函数共C++调用
	*/
	int SetScript( lua_State* L );
	void callEventVoid( const char* f );
	void callEventString( const char* f,string& s );
	void callEventInt( const char* f,int k );
	void callEvent2Int( const char* f,int x,int y );
	void callEvent3Int( const char* f,int x,int y,int z );
	void callEventNumber( const char* f,double d );
	/*
		bind C++对象
	*/
	void newLuaBind( void* pointer,LuaBind* pp );
	void deleteLuaBind( LuaBind* b );
	LuaBind* getLuaBind( void* pointer );
	//返回一个在lua执行期间不变的类名称字串
	const char* getClassString( const char* s );

	void bind( const char* meta,void* p );
	void bindWidget( MyGUI::Widget* w );

	/*
		设置lua脚本的位置
	*/
	void setLuaLocation( const string& loc );
	string getLuaLocation();
	void addLuaLocation( const string& loc );
	
	typedef vector<string> LocationVector;

	const LocationVector& getLuaLocations() const;
	/*
		LuaBind的C++类型是否可以转换
		如果derived可以转换成base返回true
	*/
//	bool isCast( const char* base,const char* derived );
	void unbind( void* p );
	//仅仅用在line断点钩子被调用
	void _lineHook(lua_State* L,lua_Debug* ar);

	bool _luaErrorNotify(lua_State* L); //lua在执行过程中发生错误
protected:
	void pcall( int nargs,int nresults );
	bool pushEventFunction( void* p,const char* f );
	bool pushEventFunction( const char* f );
	void registerClassNotPop( string name,const luaL_Reg* lib );
	//如果全局表没建立就建立，并且把全局表压入堆栈
	void _pushGlobal( lua_State* L );
	string getClassName( const luaL_Reg* lib );
	string getInheritName( const string& name );
	string _dotFront( const string& name );
	string _dotBack( const string& name );

	/*
		Lua Debug 函数
	*/
	void initializeDebug();
	void releaseDebug();
	void enableDebug(bool b);
	void setBreakpoint( const string& source,int line );
	void clearBreakpoint( const string& source,int line );
	void clearAllBreakpoint();
	void CommandLoop();
	void async_readHandler( const boost::system::error_code& e,size_t size );
	void async_writeHandler( const boost::system::error_code& e,size_t size );
	void notifyBreakpoint(const string& source,int line );
	void notifyErrorpoint(const string& source,int line );
	void notifyError(const string& msg);
	void debugMsgLoop();
	string getVarInfo( lua_State* L,const string& name,bool b );
	string getLocal(const string& name);
	string getUpvalue(const string& name);
	string getGlobal(const string& name);
	bool pushVarByName( lua_State* L,const string& name,bool b );
	bool pushLocalByName( lua_State* L,const string& name );
	bool pushUpvalueByName( lua_State* L,const string& name );
	bool pushGlobalByName( lua_State* L,const string& name );
	void sendDebugInfoByName( const string& name );
	void GetLuaSource(lua_State* L,lua_Debug* ar);
	void LuaWait(lua_State* L,lua_Debug* ar,int level,bool b);
	void TraceBack(bool b);
	void FillStep(lua_State* L,lua_Debug* ar);
	string lowerFileName( const char* s );

	typedef vector<pair<int,string> > BPVector;
	//boost::asio::ip::tcp::iostream mIO;
	boost::asio::io_service mIos;
	boost::asio::ip::tcp::socket mSocket;
	boost::asio::streambuf mInBuf;
	BPVector mBreakpoints; //断点
	boost::thread* mDebugThread; //调试线程
	boost::mutex mMutBPS; //断点访问控制
	boost::mutex mMu; //调度信号
	boost::condition_variable_any mContinueCondition;
	bool mIsDebug; //在调试
	enum ContinueType
	{
		CONTINUE,
		STEP,
		STEPIN,
		RUN
	};
	ContinueType mCType;
	lua_State* mDL; //仅仅在中断中有效
	lua_Debug* mAR; //仅仅在中断中有效
	int mStackLevel;
	bool mIsBreaking; //在中断中
	string mStepCurrentFunctionSource;
	string mStepLastFunctionSource;
	int mStepCurrentFunctionBeginLine;
	int mStepCurrentFunctionEndLine;
	int mStepLastFunctionBeginLine;
	int mStepLastFunctionEndLine;
	/*
		当一个Widget对象被删除时，将调用_unlinkWidget
		然后在该函数中完成解引用，和标记指针不可用
	*/	
	virtual void _unlinkWidget(MyGUI::Widget* _widget);

	lua_State * mL;

	map<void*,LuaBind*> mBinds;
	vector<char*> mStaticString;

	/*
		lua注册的函数被保持在一个表中,该表的引用值放入
		mGlobalRef
	*/
	int mGlobalRef;
	typedef pair<const luaL_Reg*,vector<string> > Inherit;
	map<string,Inherit > mInherit; //保存继承关系

	//防止错误处理函数被重入
	bool mIsCallError;

	LocationVector mLocations;
};

#include "LuaBind.h"

#endif