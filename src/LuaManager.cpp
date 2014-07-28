#include "stdheader.h"
#include "Registry.h"
#include "LuaManager.h"
#include "LuaExport.h"
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#undef nil
#endif
#include "boost/algorithm/string.hpp"
#include "boost/xpressive/xpressive_dynamic.hpp"
#include "boost/integer_traits.hpp"
#include "Game.h"

extern "C"
{
	//LuaFileSystem提供目录支持
	#include "lfs/lfs.h"
	//LuaSocket提供网络支持
	#include "socket/socket.h"
	#include "socket/inet.h"
	#include "socket/luasocket.h"
	#include "socket/mime.h"
}

SINGLETON(LuaManager)

struct readerStruct
{
	Ogre::DataStreamPtr ds;
	char buf[512];
};

static const char * reader(lua_State *L,
                                    void *data,
                                    size_t *size)
{
	readerStruct* pds = (readerStruct*)data;
	*size = pds->ds->read( pds->buf,sizeof(pds->buf) );
	return pds->buf;
}

//成功返回压入chunk函数，失败压入错误字符串
static bool loadFile( lua_State* L,string file )
{
	Ogre::DataStreamPtr ds;
	const LuaManager::LocationVector& locs = LuaManager::getSingleton().getLuaLocations();

	for( LuaManager::LocationVector::const_iterator i = locs.begin();i!=locs.end();++i )
	{
		try
		{
			ds = Ogre::ResourceGroupManager::getSingleton().openResource
						((*i)+file,Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,true);
			readerStruct rs;
			rs.ds = ds;
			/* Lua源代码的开头要求加入@
			*/
			string source = "@";
			source += MyGUI::DataManager::getInstance().getDataPath((*i)+file);
			int result = lua_load(L,reader,&rs,source.c_str(),NULL);
			if( result == LUA_OK )
			{
				return true;
			}
			else
			{
				ERROR_LOG( lua_tostring(L,-1) );
				return false;
			}
		}
		catch( const Ogre::FileNotFoundException& )
		{
		}
	}

	ERROR_LOG( "Can't load script file " << file );
	return false;
}

static void replace_dot( string& s,const char* dot,const char* seg )
{
	 string::size_type i = 0;
	 do
	 {
		 i = s.find(dot,i);
		 if( i != string::npos )
			s.replace(i,1,seg);
	 }while( i != string::npos );
}

static int lua_searchPak(lua_State* L)
{
	 const char *name = luaL_checkstring(L, 1);
	 string file = name;

	 replace_dot( file,".","/" );

	 file += ".lua";
	 if( loadFile(L,file) )
	 {
		 lua_pushstring(L,name);
		 return 2;
	 }
	 else
		 return luaL_error(L, "error loading module " LUA_QS
                         " from file " LUA_QS ":\n\t%s",
						 lua_tostring(L, 1), file.c_str(), lua_tostring(L, -1));
}

LuaManager::LuaManager():
mSocket(mIos)
{
	mIsCallError = false;
	mGlobalRef = LUA_REFNIL;
	mL = luaL_newstate();

	lua_setSearchEx(lua_searchPak);

	//打开标准库
	luaL_openlibs(mL);
	//lsf
	luaopen_lfs(mL);
	//socket
	luaopen_socket_core(mL);
	luaopen_mime_core(mL);

	luaopen_gettext(mL);
	luaopen_game(mL);
	luaopen_gui(mL);
	luaopen_xml(mL);
	luaopen_hotkey(mL);
	luaopen_rigid(mL);
	luaopen_mat(mL);
	luaopen_registry(mL);

	MyGUI::WidgetManager::getInstance().registerUnlinker(this);

	initializeDebug();
}

LuaManager::~LuaManager()
{
	MyGUI::WidgetManager::getInstance().unregisterUnlinker(this);

	releaseDebug();

	mLocations.clear();

	lua_close(mL);
	mL = nullptr;
    
    for( vector<char*>::iterator i=mStaticString.begin();i!=mStaticString.end();++i )
    {
        delete [] *i;
    }
}

//类似与?;?.lua;/user/local/lua/?/?.lua
string LuaManager::getLuaLocation()
{
	string loc;
	lua_getglobal(mL,"package");
	lua_getfield(mL,-1,"path");
	if( lua_isstring(mL,-1) )
		loc = lua_tostring(mL,-1);
	lua_pop(mL,3);
	return loc;
}

void LuaManager::setLuaLocation( const string& loc )
{
	lua_getglobal(mL,"package");
	lua_pushstring(mL,loc.c_str());
	lua_setfield(mL,-2,"path");
	lua_pop(mL,1);
}

const LuaManager::LocationVector& LuaManager::getLuaLocations() const
{
	return mLocations;
}

void LuaManager::addLuaLocation( const string& loc )
{
	if( find(mLocations.begin(),mLocations.end(),loc ) == mLocations.end() )
		mLocations.push_back(loc);
}

void LuaManager::doFile( string file )
{
	if( loadFile(mL,file) )
	{
		//如果是模块文件用文件名作为参数调用chunk
		string::size_type size = file.size();
		if( size > 4 )
		{
			string name = file.substr(0,size-4);
			replace_dot( name,"/","." );
			lua_pushstring(mL,name.c_str());
			pcall(1,LUA_MULTRET);
		}
		else
		{
			pcall(0,LUA_MULTRET);
		}
	}
	else
		lua_pop(mL,1);
}

void LuaManager::registerClassNotPop( string name,const luaL_Reg* lib )
{
	luaL_newmetatable(mL,name.c_str());
	lua_pushvalue(mL,-1);
	lua_setfield(mL,-2,"__index");//meta.__index = self;
	luaL_setfuncs(mL,lib,0);
}

void LuaManager::registerClass( string name,const luaL_Reg* lib )
{
	vector<string> vinher;
	mInherit[name] = Inherit(lib,vinher);

	registerClassNotPop( name,lib );
	lua_pop(mL,1);
}

string LuaManager::getClassName( const luaL_Reg* lib )
{
	for( BOOST_AUTO(i,mInherit.begin());i!=mInherit.end();++i )
	{
		if( i->second.first == lib )
		{
			return i->first;
		}
	}
	assert(false);
	return string();
}

void LuaManager::registerClass( string name,const luaL_Reg* base,const luaL_Reg* derived )
{
	vector<string> vinher;
	vinher.push_back( getClassName( base ) );
	mInherit[name] = Inherit(derived,vinher);

	registerClassNotPop(name,base);
	luaL_setfuncs(mL,derived,0);
	lua_pop(mL,1);
}

void LuaManager::registerClass( string name,const luaL_Reg* base,
							   const luaL_Reg* derived,const luaL_Reg* derived2 )
{
	vector<string> vinher;
	vinher.push_back( getClassName( base ) );
	vinher.push_back( getClassName( derived ) );
	mInherit[name] = Inherit(derived2,vinher);

	registerClassNotPop(name,base);
	luaL_setfuncs(mL,derived,0);
	luaL_setfuncs(mL,derived2,0);
	lua_pop(mL,1);
}

void LuaManager::registerClass( string name,const luaL_Reg* base,const luaL_Reg* derived,
	const luaL_Reg* derived2,const luaL_Reg* derived3 )
{
	vector<string> vinher;
	vinher.push_back( getClassName( base ) );
	vinher.push_back( getClassName( derived ) );
	vinher.push_back( getClassName( derived2 ) );
	mInherit[name] = Inherit(derived3,vinher);

	registerClassNotPop(name,base);
	luaL_setfuncs(mL,derived,0);
	luaL_setfuncs(mL,derived2,0);
	luaL_setfuncs(mL,derived3,0);
	lua_pop(mL,1);
}

void LuaManager::registerCFunction( const char* name,lua_CFunction func )
{
	lua_getglobal(mL,"_G");
	lua_pushcfunction(mL,func);
	lua_setfield(mL,-2,"L");
	lua_pop(mL,1);
}

void LuaManager::registerGlobal( const char* n,const luaL_Reg* lib )
{
	//lua_getglobal(mL,"_G");
	lua_newtable(mL);
	luaL_setfuncs(mL,lib,0);
	//因为和Lua是静态的连接形式,需要注册库名
	//下面代码实现：package.loaded[name] = libtable
	lua_getglobal(mL,"package");
	lua_getfield(mL,-1,"loaded");
	lua_pushvalue(mL,-3);
	lua_setfield(mL,-2,n);
	lua_pop(mL,3);
}

void LuaManager::callVoid(  const char* f  )
{
	lua_getglobal(mL,f);
	if( lua_isfunction(mL,-1) )
	{
		pcall(0,0);
	}
	else
	{
		lua_pop(mL,1); //不是期待的值
	}
}

void LuaManager::callString( const char* f,const char* s )
{
	lua_getglobal(mL,f);
	if( lua_isfunction(mL,-1) )
	{
		lua_pushstring(mL,s);
		pcall( 1,0 );
	}	
	else
	{
		lua_pop(mL,1);
	}
}

//static int lua_pcall_error_handle( lua_State* L )
//{
//}

static int traceback (lua_State *L) {
	if( LuaManager::getSingleton()._luaErrorNotify( L ) )
	{
	  const char *msg = lua_tostring(L, 1);
	  if (msg)
		luaL_traceback(L, L, msg, 1);
	  else if (!lua_isnoneornil(L, 1)) {  /* is there an error object? */
		if (!luaL_callmeta(L, 1, "__tostring"))  /* try its 'tostring' metamethod */
		  lua_pushliteral(L, "(no error message)");
	  }
	}
  return 1;
}

void LuaManager::pcall( int nargs,int nresults)
{
	int base = lua_gettop(mL) - nargs;  /* function index */
	lua_pushcfunction(mL,traceback);
	lua_insert(mL,base);
	if( lua_pcall(mL,nargs,nresults,base) != LUA_OK )
	{
		 const char *msg = lua_tostring(mL, -1);
		if (msg == NULL) msg = "(error object is not a string)";
		//尝试调用错误处理
		if( !mIsCallError )
		{
			mIsCallError = true;
			ERROR_LOG(string(msg));
            string smsg(msg);
			callEventString("eventErrorHandler",smsg);
			mIsCallError = false;
		}
		else
		{
			MSG("#FF0000"<<msg);
		}
		
		lua_pop(mL, 1);
		/* force a complete garbage collection in case of errors */
		lua_gc(mL, LUA_GCCOLLECT, 0);
	}
	lua_remove(mL,base);/* remove traceback function */
}

void LuaManager::callTableVoid( const char* t,const char* f )
{
	lua_getglobal(mL,t);
	if( lua_istable(mL,-1) )
	{
		lua_getfield(mL,-1,f);
		if( lua_isfunction(mL,-1) )
		{
			pcall(0,0);
			lua_pop(mL,1); //弹出表
		}
		else
		{
			lua_pop(mL,2);
		}
	}
	else
	{
		lua_pop(mL,1);
	}
}

/*
	调用注册表中的Key为t的表中的函数f
*/
void LuaManager::callRegVoid( const char* t,const char* f )
{
	lua_getfield(mL,LUA_REGISTRYINDEX,t);
	if( lua_istable(mL,-1) )
	{
		lua_getfield(mL,-1,f);
		if( lua_isfunction(mL,-1) )
		{
			pcall(0,0);
			lua_pop(mL,1); //弹出表
		}
		else
		{
			lua_pop(mL,2);
		}
	}
	else
	{
		lua_pop(mL,1);
	}
}

/*
	辅助函数
	将对象p的对应函数压入堆栈返回true。
	同时压入self作为第一个参数
	如果失败返回false
*/
bool LuaManager::pushEventFunction( void* p,const char* f )
{
	LuaBind* plb = getLuaBind(p);
	if( plb && plb->refUTable!=LUA_REFNIL )
	{
		lua_rawgeti(mL,LUA_REGISTRYINDEX,plb->refUTable);
		lua_getfield(mL,-1,f);
		if( lua_isfunction(mL,-1) )
		{
			//push self 作为第一个参数
			lua_rawgeti(mL,LUA_REGISTRYINDEX,plb->ref);
			return true;
		}
		else
		{
			lua_pop(mL,2);
			WARNING_LOG("pushEventFunction invaild evnet function ont Call("<<f<<")");
		}
	}
	else
	{
		WARNING_LOG("pushEventFunction LuaBind is NULL on Call("<<f<<")");
	}
	return false;
}

void LuaManager::callBindEventVoid( void* p,const char* f )
{
	if( pushEventFunction(p,f) )
	{
		pcall(1,0);
		lua_pop(mL,1); //弹出表
	}
}

void LuaManager::callBindEvent2String( void* p,const char* f,const char* s1,const char* s2)
{
	if( pushEventFunction(p,f) )
	{
		lua_pushstring(mL,s1);
		lua_pushstring(mL,s2);
		pcall(3,0);
		lua_pop(mL,1); //弹出表
	}
}

void LuaManager::callBindEvent3Int( void* p,const char* f,int i,int j,int k )
{
	if( pushEventFunction(p,f) )
	{
		lua_pushinteger(mL,i);
		lua_pushinteger(mL,j);
		lua_pushinteger(mL,k);
		pcall(4,0);
		lua_pop(mL,1); //弹出表
	}
}

void LuaManager::callBindEvent2Int( void* p,const char* f,int i,int j )
{
	if( pushEventFunction(p,f) )
	{
		lua_pushinteger(mL,i);
		lua_pushinteger(mL,j);
		pcall(3,0);
		lua_pop(mL,1); //弹出表
	}
}

void LuaManager::callBindEvent1Int(void* p,const char* f,int i)
{
	if( pushEventFunction(p,f) )
	{
		lua_pushinteger(mL,i);
		pcall(2,0);
		lua_pop(mL,1); //弹出表
	}
}

void LuaManager::callBindEvent1Obj( void* p,const char* f,void* pp )
{
	if( pushEventFunction(p,f) )
	{
		//注意第二个参数必须已经被绑定
		LuaBind* plb = getLuaBind(pp);
		if( plb && plb->ref!=LUA_REFNIL )
		{
			lua_rawgeti(mL,LUA_REGISTRYINDEX,plb->ref);
			pcall(2,0);
		}
		else if( pp == NULL ) //如果pp==NULL
		{
			lua_pushnil(mL);
			pcall(2,0);
		}
		else
		{
			lua_pop(mL,2); //弹出函数和self
		}
		lua_pop(mL,1); //弹出表
	}
}

void LuaManager::callBindEvent1Bool( void* p,const char* f,bool b )
{
	if( pushEventFunction(p,f) )
	{
		lua_pushboolean(mL,b);
		pcall(2,0);
		lua_pop(mL,1); //弹出表
	}
}

void LuaManager::callBindEvent1String(void* p,const char* f,const char* s)
{
	if( pushEventFunction(p,f) )
	{
		lua_pushstring(mL,s);
		pcall(2,0);
		lua_pop(mL,1);
	}
}

bool LuaManager::callBindEventDDContainer( void* p,const char* f,
	MyGUI::Widget* _s,int _si,MyGUI::Widget* _r,int _ri )
{
	bool ret = false;
	if( pushEventFunction(p,f) )
	{
		lua_bindWidget(mL,_s);
		lua_pushinteger(mL,_si);
		lua_bindWidget(mL,_r);
		lua_pushinteger(mL,_ri);
		pcall(5,1);
		ret = lua_toboolean(mL,-1)?true:false;
		lua_pop(mL,2);
	}
	return ret;
}

void LuaManager::callBindEvent4Int1Bool( void* p,const char* f,int i1,int i2,int i3,int i4,bool b )
{
	if( pushEventFunction(p,f) )
	{
		lua_pushinteger(mL,i1);
		lua_pushinteger(mL,i2);
		lua_pushinteger(mL,i3);
		lua_pushinteger(mL,i4);
		lua_pushboolean(mL,b);
		pcall(6,0);
		lua_pop(mL,1);
	}
}

void LuaManager::callBindEvent1Obj1IBDrawItemInfo( void* p,const char* f,
	MyGUI::Widget* _item,const MyGUI::IBDrawItemInfo& _info )
{
	if( pushEventFunction(p,f) )
	{
		lua_bindWidget(mL,_item);
		lua_pushinteger(mL,_info.index);
		lua_pushboolean(mL,_info.update);
		lua_pushboolean(mL,_info.select);
		lua_pushboolean(mL,_info.active);
		lua_pushboolean(mL,_info.drag);
		lua_pushboolean(mL,_info.drop_accept);
		lua_pushboolean(mL,_info.drop_refuse);
		pcall(9,0);
		lua_pop(mL,1);
	}
}

void LuaManager::callBindEvent1IBDrawItemInfo( void* p,const char* f,
	const MyGUI::IBNotifyItemData& _info )
{
	if( pushEventFunction(p,f) )
	{
		lua_pushinteger(mL,_info.index);
		if( _info.notify==MyGUI::IBNotifyItemData::MousePressed )
			lua_pushstring(mL,"MousePressed");
		else if( _info.notify==MyGUI::IBNotifyItemData::MouseReleased )
			lua_pushstring(mL,"MouseReleased");
		else if( _info.notify==MyGUI::IBNotifyItemData::KeyPressed )
			lua_pushstring(mL,"KeyPressed");
		else if( _info.notify==MyGUI::IBNotifyItemData::KeyReleased )
			lua_pushstring(mL,"KeyReleased");

		lua_pushboolean(mL,_info.x);
		lua_pushboolean(mL,_info.y);
		lua_pushinteger(mL,_info.id.getValue());
		lua_pushinteger(mL,_info.code.getValue());
		lua_pushinteger(mL,_info.key);
		pcall(8,0);
		lua_pop(mL,1);
	}
}

void LuaManager::bind( const char* meta,void* p )
{
	lua_bind(mL,meta,p);
	lua_pop(mL,1); //弹出该对象
}

string LuaManager::_dotFront( const string& name )
{
	string::size_type i = name.find(".");
	if( i==string::npos )
		return name;
	else
		return name.substr(0,i);
}

string LuaManager::_dotBack( const string& name )
{
	string::size_type i = name.find(".");
	if( i==string::npos )
		return name;
	else
		return name.substr(i+1,name.size()-i-1);
}

/*
	例如name="mygui.Button"返回"mygui.Widget.TextBox.Button"
*/
string LuaManager::getInheritName( const string& name )
{
	BOOST_AUTO(i,mInherit.find(name));
	if( i!=mInherit.end() )
	{
		string s(_dotFront(name));
		for( BOOST_AUTO(it,i->second.second.begin());it!=i->second.second.end();++it )
		{
			s += "."+_dotBack(*it);
		}
		return s += "." + _dotBack(name);
	}
	//没有发现继承关系
	ERROR_LOG("LuaManager::getInheritName('"<<name<<"') don't know inherit");
	return name;
}

void LuaManager::newLuaBind( void* pointer,LuaBind* pp )
{
	mBinds[pointer] = pp;
}

void LuaManager::deleteLuaBind( LuaBind* b )
{
	BOOST_AUTO(i,mBinds.find(b->self));
	if( i!=mBinds.end() )
		mBinds.erase(i);
}

LuaBind* LuaManager::getLuaBind( void* pointer )
{
	BOOST_AUTO(i,mBinds.find(pointer));
	if( i!=mBinds.end() )
		return i->second;
	else
		return nullptr;
}

const char* LuaManager::getClassString( const char* s )
{
    string meta = getInheritName(string(s));
    for( vector<char*>::iterator i=mStaticString.begin();i!=mStaticString.end();++i )
    {
        if( meta == *i )
            return *i;
    }
    char* cn = new char[meta.size()+sizeof(char)];
    strcpy(cn,meta.c_str());
    mStaticString.push_back(cn);
    return cn;
}

//解除绑定
void LuaManager::unbind( void* p )
{
	LuaBind* plb = LuaManager::getSingleton().getLuaBind(p);
	if( plb )
	{
		deleteLuaBind( plb );
		if( plb->ref != LUA_REFNIL )
		{
			luaL_unref(mL,LUA_REGISTRYINDEX,plb->ref); //表示该Lua对象可以被回收了
			plb->ref = LUA_REFNIL;
		}
		if( plb->refUTable != LUA_REFNIL )
		{
			luaL_unref(mL,LUA_REGISTRYINDEX,plb->refUTable);
			plb->refUTable = LUA_REFNIL;
		}
		plb->self = nullptr; //标记C++对象施放
	}
}

void LuaManager::_unlinkWidget(MyGUI::Widget* _widget)
{
	unbind(_widget);
}

void LuaManager::bindWidget( MyGUI::Widget* w )
{
	lua_bindWidget( mL,w );
}

void LuaManager::_pushGlobal( lua_State* L )
{
	if( mGlobalRef == LUA_REFNIL )
	{
		//初始化全局表
		lua_newtable(L);
		lua_pushvalue(L,-1);
		mGlobalRef = luaL_ref(mL,LUA_REGISTRYINDEX);
	}
	else
	{
		//直接引用全局表
		lua_rawgeti(mL,LUA_REGISTRYINDEX,mGlobalRef);
	}
}

/*
	注册lua函数共C++调用
*/
static const char* event_entry[]=
{
	"eventConsole",
	"eventErrorHandler",
	"eventMouseMove",
	"eventMousePress",
	"eventMouseRelease",
	"eventKeyPress",
	"eventKeyRelease",
	"eventFrameStarted",
	"eventFrameRenderingQueued",
	"eventFrameEnded",
	"eventShutdown"
};

int LuaManager::SetScript( lua_State* L )
{
	if( lua_isstring( L,1 ) )
	{
		if( lua_isfunction( L,2 ) )
		{
			const char* entry = lua_tostring(L,1);
			for( int i = 0; i < sizeof(event_entry)/sizeof(char*); i++ )
			{
				if( strcmp(event_entry[i],entry)== 0 )
				{
					_pushGlobal( L );
					lua_pushvalue(L,2);
					lua_setfield(L,-2,entry);
					return 0;
				}
			}
			WARNING_LOG("SetScript invaild event name :'"<<entry<<"'" );
		}
		else
		{
			WARNING_LOG("SetScript invaild paramer @argument2 not function");
		}
	}
	else
	{
		WARNING_LOG("SetScript invaild paramer @argument1 not string");
	}
	return 0;
}

bool LuaManager::pushEventFunction( const char* f )
{
	if( mGlobalRef != LUA_REFNIL )
	{
		lua_rawgeti(mL,LUA_REGISTRYINDEX,mGlobalRef);
		lua_getfield(mL,-1,f);
		if( lua_isfunction(mL,-1) )
		{
			return true;
		}
		else
		{
			lua_pop(mL,2);
		}
	}
	//这里不进行报错处理，默认为没有设置事件函数
	return false;
}

void LuaManager::callEventVoid( const char* f )
{
	if( pushEventFunction( f ) )
	{
		pcall(0,0);
		lua_pop(mL,1);
	}
}

void LuaManager::callEventString( const char* f,string& s )
{
	if( pushEventFunction( f ) )
	{
		lua_pushlstring(mL,s.data(),s.size());
		pcall(1,0);
		lua_pop(mL,1);
	}
}

void LuaManager::callEvent3Int( const char* f,int x,int y,int z )
{
	if( pushEventFunction( f ) )
	{
		lua_pushinteger(mL,x);
		lua_pushinteger(mL,y);
		lua_pushinteger(mL,z);
		pcall(3,0);
		lua_pop(mL,1);
	}
}

void LuaManager::callEventNumber( const char* f,double d )
{
	if( pushEventFunction( f ) )
	{
		lua_pushnumber(mL,d);
		pcall(1,0);
		lua_pop(mL,1);
	}
}

void LuaManager::callEventInt( const char* f,int k )
{
	if( pushEventFunction( f ) )
	{
		lua_pushinteger(mL,k);
		pcall(1,0);
		lua_pop(mL,1);
	}
}

void LuaManager::callEvent2Int( const char* f,int x,int y )
{
	if( pushEventFunction( f ) )
	{
		lua_pushinteger(mL,x);
		lua_pushinteger(mL,y);
		pcall(2,0);
		lua_pop(mL,1);
	}
}

static void lua_lineHook(lua_State* L,lua_Debug* ar)
{
	LuaManager::getSingleton()._lineHook( L,ar );
}

void LuaManager::debugMsgLoop()
{
	{
		boost::mutex::scoped_lock lock(mMu);
	}
	boost::system::error_code e;

	string debugserver = Registry::getSingleton().get("DebugServer",string("127.0.0.1"));
	int port = Registry::getSingleton().get<int>("DebugPort",3030);

	mSocket.connect( boost::asio::ip::tcp::endpoint(
		boost::asio::ip::address::from_string(debugserver),port
			),e );

	if( !e )
	{
		mIsDebug = true;
        mContinueCondition.notify_one();
		CommandLoop();
	}else
	{
		mSocket.close();
		mIsDebug = false;
		mContinueCondition.notify_one();
		WARNING_LOG(e.message());
	}
}

//
void LuaManager::initializeDebug()
{
	boost::mutex::scoped_lock lock(mMu);
	mIsDebug = false;
	mIsBreaking = false;
	mCType = CONTINUE;
	mDebugThread = new boost::thread(
		boost::bind(&LuaManager::debugMsgLoop,this)
		);
	
	mContinueCondition.wait(mMu);

	if( mIsDebug )
		enableDebug(true);
}

void LuaManager::async_readHandler( const boost::system::error_code& e,size_t size )
{
	if( e )
	{
		WARNING_LOG(e.message());
		mContinueCondition.notify_one();
		return;
	}

	boost::mutex::scoped_lock lock(mMu);

	string cmd;
	//文件名称被表述为不含<>/\|:"*的全部字符
	boost::xpressive::sregex setbp = boost::xpressive::sregex::compile("bp<([^?<>|\"*]+)>@(\\d+)");
	boost::xpressive::sregex clearbp = boost::xpressive::sregex::compile("clear<([^?<>|\"*]+)>@(\\d+)");
	boost::xpressive::sregex getxp = boost::xpressive::sregex::compile("get<([_a-zA-Z][_a-zA-Z0-9.]*)>");

	istream is(&mInBuf);
	if( getline(is,cmd) )
	{
		if( cmd == "continue" )
		{
			mCType = CONTINUE;
			mContinueCondition.notify_one();
		}
		else if( cmd == "step" )
		{
			mCType = STEP;
			mContinueCondition.notify_one();
		}
		else if( cmd == "stepin" )
		{
			mCType = STEPIN;
			mContinueCondition.notify_one();
		}
		else if( cmd == "reset" )
		{ //重启
			mCType = RUN;
			mContinueCondition.notify_one();
			Game::getSingleton().reset(Game::FAST);
		}
		else if( cmd == "traceback" )
		{
			TraceBack(true);
		}
		else if( cmd == "tracefront" )
		{
			TraceBack(false);
		}
        else if( cmd == "stack" )
        {
            StackInfo();
        }
		else
		{
			boost::xpressive::smatch what;
			if( cmd == "clearall" )
			{
				clearAllBreakpoint();
			}else if( boost::xpressive::regex_match(cmd,what,setbp) )
			{
				setBreakpoint(what[1],boost::lexical_cast<int>(what[2]));
			}else if( boost::xpressive::regex_match(cmd,what,clearbp) )
			{
				clearBreakpoint(what[1],boost::lexical_cast<int>(what[2]));
			}else if( boost::xpressive::regex_match(cmd,what,getxp) )
			{
				sendDebugInfoByName(what[1]);
			}else
			{
				MSG("Invlied debug command:"<<cmd)
			}
		}
	}
	//继续读
	boost::asio::async_read_until(mSocket,mInBuf,'\n',
		boost::bind(&LuaManager::async_readHandler,this,
		boost::asio::placeholders::error,
		boost::asio::placeholders::signal_number));	
}
/*
	调试器命令
	continue 立刻返回继续执行
	clearall 清除全部断点
	bp<file>@line 设置断点
	clear<file>@line 清除断点
*/
void LuaManager::CommandLoop()
{
	boost::asio::async_read_until(mSocket,mInBuf,'\n',
		boost::bind(&LuaManager::async_readHandler,this,
		boost::asio::placeholders::error,
		boost::asio::placeholders::signal_number));

	//异步事件循环
	mIos.run();
    //如果调试器主动关闭连接
    if( mSocket.is_open() )
        mSocket.close();
}

void LuaManager::releaseDebug()
{
	if( mSocket.is_open() )
		mSocket.close();
	if( mDebugThread )
		delete mDebugThread;
}

void LuaManager::enableDebug(bool b)
{
	if( b )
		lua_sethook(mL,lua_lineHook,LUA_MASKLINE,1);
	else
		lua_sethook(mL,nullptr,0,0);
}

//设置断点
void LuaManager::setBreakpoint( const string& source,int line )
{
	boost::mutex::scoped_lock lock(mMutBPS);

	string s = lowerFileName(source.c_str());

	for( BPVector::iterator i = mBreakpoints.begin();i!=mBreakpoints.end();++i )
	{
		if( i->first == line && i->second == s )
			return;
	}
	mBreakpoints.push_back(pair<int,string>(line,s));
}

//清除断点
void LuaManager::clearBreakpoint( const string& source,int line )
{
	boost::mutex::scoped_lock lock(mMutBPS);

	string s = lowerFileName(source.c_str());

	for( BPVector::iterator i = mBreakpoints.begin();i!=mBreakpoints.end();++i )
	{
		if( i->first == line && i->second == s )
		{
			mBreakpoints.erase(i);
			return;
		}
	}
}

//清除全部断点
void LuaManager::clearAllBreakpoint()
{
	boost::mutex::scoped_lock lock(mMutBPS);

	mBreakpoints.clear();
}

void LuaManager::async_writeHandler( const boost::system::error_code& e,size_t size )
{
}

//通知调试器中断
void LuaManager::notifyBreakpoint(const string& source,int line )
{
	if( mSocket.is_open() )
	{
		std::stringstream ss(std::stringstream::out);
		ss<<"break<"<<source<<">@"<<line<<std::endl;
		mSocket.write_some(boost::asio::buffer(ss.str()));
	}
}

void LuaManager::notifyErrorpoint(const string& source,int line )
{
	if( mSocket.is_open() )
	{
		std::stringstream ss(std::stringstream::out);
		ss<<"error<"<<source<<">@"<<line<<std::endl;
		mSocket.write_some(boost::asio::buffer(ss.str()));
	}
}

//使用\t来表示换行，因为发送命令用\n结尾
//取得栈顶变量的信息
string LuaManager::getVarInfo( lua_State* L,const string& name,bool b )
{
	string info;

	if( lua_isnumber(L,-1) )
	{
		lua_Number d = lua_tonumber(L,-1);
		info += boost::lexical_cast<string>(d);
		info += "(number)\t";
	}
	else if( lua_istable(L,-1) )
	{
		info += "(table)\t";
		if( b )
		{
			lua_pushnil(L); //push nil key
			int count = 0;
			while(lua_next(L,-2) !=0 )
			{
				string tn;
				//不扰乱next,复制一份key
				lua_pushvalue(L,-2);
				const char* c =lua_tostring(L,-1);
				if( c )
				{
					info += "[";
					info += c;
					info += "]=";
					tn = c;
				}
				lua_pop(L,1);

				info += getVarInfo(L,tn,false);

				lua_pop(L,1); //pop value
				if( ++count > 48 )
				{
					lua_pop(L,1);
					info += "...\t";
					break;
				}
			}
		}
	}
	else if( lua_isfunction(L,-1) )
	{
		info += boost::lexical_cast<string>(lua_topointer(L,-1));
		info += "(function)\t";
	}
	else if( lua_iscfunction(L,-1) )
	{
		info += boost::lexical_cast<string>(lua_topointer(L,-1));
		info += "(cfunction)\t";
	}
	else if( lua_isboolean(L,-1) )
	{
		info += lua_toboolean(L,-1)?"true":"false";
		info += "(boolean)\t";
	}
	else if( lua_isuserdata(L,-1) )
	{
		LuaBind* lb = (LuaBind*)lua_touserdata(L,-1);
		try
		{
			if( lb )
				info += lb->matetable;
		}catch(...){}
		info += "(userdata)\t";
	}
	else if( lua_isnil(L,-1) )
	{
		info += "(nil)\t";
	}
	else if( lua_isnone(L,-1) )
	{
		info += "(none)\t";
	}
	else if( lua_isthread(L,-1) )
	{
		info += "(thread)\t";
	}
	else if( lua_isnoneornil(L,-1) )
	{
		info += "(nonernil)\t";
	}
	else if( lua_islightuserdata(L,-1) )
	{
		info += "(lightuserdata)\t";
	}
	else if( lua_isstring(L,-1) )
	{
		const char* s = lua_tostring (L,-1);
		info += "\"";
		info += s;
		info += "\"";
		info += "(string)\t";
	}
	
	return info;
}

//调试中返回局部变量
string LuaManager::getLocal(const string& name)
{
	const char* na;
	int i = 1;

	do
	{
		na = lua_getlocal(mDL,mAR,i++);
		if( na )
		{
			if( name == na )
			{
				string info = getVarInfo(mDL,name,true);
				info += "local:";
				info += name;
				lua_pop(mDL,1);
				return info;
			}
			lua_pop(mDL,1);
		}
	}while( na );

	return string();
}

//调试中返回upvalue
string LuaManager::getUpvalue(const string& name)
{
	int funindex = lua_getinfo(mDL,"f",mAR);
	int i = 1;
	const char* na;
	if( lua_isfunction(mDL,-1) )
	{
		do
		{
			na = lua_getupvalue(mDL,-1,i++);
			if( na )
			{
				if( name == na )
				{
					string info = getVarInfo(mDL,name,true);
					info += "upvalue:";
					info += name;
					lua_pop(mDL,2);
					return info;
				}
				lua_pop(mDL,1);
			}
		}while( na );
		lua_pop(mDL,1);
	}
	return string();
}

//调试中返回全局变量
string LuaManager::getGlobal(const string& name)
{
	lua_getglobal(mDL,name.c_str());
	if( lua_isnil(mDL,-1) )
		return string();
	string value = getVarInfo(mDL,name,true);
	lua_pop(mDL,1);
	value += "global:";
	value += name;
	return value;
}

bool LuaManager::pushLocalByName( lua_State* L,const string& name )
{
	const char* na;
	int i = 1;

	do
	{
		na = lua_getlocal(L,mAR,i++);
		if( na )
		{
			if( name == na )
			{
				return true;
			}
			lua_pop(L,1);
		}
	}while( na );
	return false;
}

bool LuaManager::pushUpvalueByName( lua_State* L,const string& name )
{
	int funindex = lua_getinfo(L,"f",mAR);
	int i = 1;
	const char* na;
	if( lua_isfunction(L,-1) )
	{
		do
		{
			na = lua_getupvalue(L,-1,i++);
			if( na )
			{
				if( name == na )
				{
					lua_remove(L,-2); //remove function
					return true;
				}
				lua_pop(L,1);
			}
		}while( na );
		lua_pop(L,1); //pop function
	}
	return false;
}

bool LuaManager::pushGlobalByName( lua_State* L,const string& name )
{
	lua_getglobal(mDL,name.c_str());
	if( lua_isnil(L,-1) )
	{
		lua_pop(L,1);
		return false;
	}
	return true;
}

//函数将在查找local upvalue global
//并且将值push到栈顶同时返回true,否则返回false
//b==true,根查找.b==false假设栈顶是一个表格
bool LuaManager::pushVarByName( lua_State* L,const string& name,bool b )
{
	if( b )
	{
		if( pushLocalByName( L,name ) )
			return true;
		else if( pushUpvalueByName( L,name ) )
			return true;
		else if( pushGlobalByName( L,name ) )
			return true;
	}
	else
	{
		if( lua_istable(L,-1) )
		{
			lua_getfield(L,-1,name.c_str());
			return true;
		}
	}
	return false;
}

void LuaManager::notifyError(const string& msg)
{
	if( mSocket.is_open() )
	{
		std::stringstream ss(std::stringstream::out);
		ss<<"error<"<<msg<<">"<<std::endl;
		mSocket.write_some(boost::asio::buffer(ss.str()));
	}
}

bool LuaManager::_luaErrorNotify(lua_State* L)
{
	if( mSocket.is_open() )
	{
		lua_Debug ar;
		mAR = &ar;
		mDL = L;
		int level = 0;
		while( lua_getstack(mDL,level++,mAR) == 1 )
		{
			GetLuaSource(mDL,mAR);
			if( strcmp(mAR->what,"Lua") == 0 )
			{
				const char *msg = lua_tostring(L, 1);
				if( msg )
					notifyError(msg);
				LuaWait(mDL,mAR,level-1,false);
				return true;
			}
		}
	}
	return true; //让traceback做进一步处理
}

//返回给调试器堆栈信息
void LuaManager::StackInfo()
{
    if( mIsBreaking )
    {
        string msg = "info:";
        int level = 0;
        while( lua_getstack(mDL, level, mAR) == 1 )
        {
            lua_getinfo(mDL,"n",mAR);
            if( mAR->name )
            {
                msg += level==mStackLevel?'*':' ';
                msg += mAR->name;
                msg +='\t';
            }
            level++;
        }
        if( mSocket.is_open() )
        {
            msg += '\n';
            mSocket.write_some(boost::asio::buffer(msg));
        }
    }
}
void LuaManager::TraceBack(bool b)
{
	if( mIsBreaking )
	{
		int dt = b?1:-1;
		int level = mStackLevel+dt;
		while( lua_getstack(mDL,level,mAR) == 1 )
		{
			GetLuaSource(mDL,mAR);
			if( strcmp(mAR->what,"Lua") == 0  )
			{
				mStackLevel = level;
				notifyErrorpoint(mAR->short_src,mAR->currentline);
				return;
			}
			level += dt;
		}
	}
}

//按顺序找先看看是不是局部变量，然后看看是不是upvalue,然后全局变量，
//然后函数堆栈
void LuaManager::sendDebugInfoByName( const string& name )
{
	if( mIsBreaking )
	{
		string msg;

		boost::xpressive::sregex reg = boost::xpressive::sregex::compile("([^.]+)");
		boost::xpressive::sregex_iterator pos(name.begin(),name.end(),reg);
		boost::xpressive::sregex_iterator end;

		if( lua_getstack(mDL,mStackLevel,mAR) == 1 ) //取得当前栈层
		{
			if( name.find('.',0)!=string::npos )
			{
				//
				int n = 0;
				bool isok = true;
				while( pos!=end )
				{
					if( pushVarByName(mDL,(*pos)[0],n==0) )
					{
						n++;
					}
					else
					{
						isok = false;
						break;
					}
					pos++;
				}
				if( isok )
				{
					msg = getVarInfo(mDL,name,true);
					msg += "member:";
					msg += name;
				}
				if( n>0 )
					lua_pop(mDL,n);
			}
			else
			{
				msg = getLocal( name );
				if( !msg.empty() )
					goto return_msg;
				msg = getUpvalue( name );
				if( !msg.empty() )
					goto return_msg;
				msg = getGlobal( name );
				if( !msg.empty() )
					goto return_msg;
			}
		}
return_msg:
		if( !msg.empty() )
		{
			string info("info:");
			info+=msg;
			info+="\n";
			mSocket.write_some(boost::asio::buffer(info));
		}
	}
}

void LuaManager::GetLuaSource(lua_State* L,lua_Debug* ar)
{
	lua_getinfo(L,"Sln",ar);
}

//阻塞Lua执行
void LuaManager::LuaWait(lua_State* L,lua_Debug* ar,int level,bool b )
{
	mStackLevel = level;
	mDL = L;
	mAR = ar;
	mIsBreaking = true;
	if( b )
		notifyBreakpoint(ar->short_src,ar->currentline);
	else
		notifyErrorpoint(ar->short_src,ar->currentline);
	{
		boost::mutex::scoped_lock lock(mMu);
		mContinueCondition.wait(mMu);
	}
	mIsBreaking = false;
}

string LuaManager::lowerFileName( const char* s )
{
	string source = s;
	boost::to_lower(source);
	boost::replace_all(source,"\\","/");
	source += '\0';
	return source;
}

void LuaManager::FillStep(lua_State* L,lua_Debug* ar)
{
	int level = 0;
	if( lua_getstack(L,level++,ar) == 1 )
	{
		GetLuaSource(L,ar);
		if( strcmp(ar->what,"main")==0 )
		{//chunk块
			mStepCurrentFunctionBeginLine = 0;
			mStepCurrentFunctionEndLine = boost::integer_traits<int>::const_max;
		}
		else
		{
			mStepCurrentFunctionBeginLine = ar->linedefined;
			mStepCurrentFunctionEndLine = ar->lastlinedefined;
		}
		mStepCurrentFunctionSource = lowerFileName(ar->short_src);

		while( lua_getstack(L,level++,ar) == 1 )
		{
			GetLuaSource(L,ar);
			if( strcmp(ar->what,"Lua")==0 )
			{
				mStepLastFunctionBeginLine = ar->linedefined;
				mStepLastFunctionEndLine = ar->lastlinedefined;
				mStepLastFunctionSource = lowerFileName(ar->short_src);
				return;
			}
		}
	}
	else
	{
		mStepCurrentFunctionBeginLine = -1;
		mStepCurrentFunctionEndLine = -1;
		mStepCurrentFunctionSource.clear();
	}
	mStepLastFunctionBeginLine = -1;
	mStepLastFunctionEndLine = -1;
	mStepLastFunctionSource.clear();
}

//Lua的Line Hook 每一行Lua命令都会调用该函数
void LuaManager::_lineHook(lua_State* L,lua_Debug* ar)
{
	if( mCType == RUN )
		return;
	else if( mCType == STEPIN )
	{
		GetLuaSource(L,ar);
		LuaWait(L,ar,0,true); //等待调试器命令
		FillStep(L,ar);
		return;
	}
	else if( mCType == STEP )
	{
		GetLuaSource(L,ar);
		string source = lowerFileName(ar->short_src);
		//在当前函数中或者回到上级函数中
		if( (ar->currentline <= mStepCurrentFunctionEndLine &&
			ar->currentline >= mStepCurrentFunctionBeginLine &&
			mStepCurrentFunctionSource == source) ||
			(ar->currentline <= mStepLastFunctionEndLine &&
			ar->currentline >= mStepLastFunctionBeginLine &&
			mStepLastFunctionSource == source) 
			)
		{
			LuaWait(L,ar,0,true);
			FillStep(L,ar);
			return;
		}
		//一旦步入如别的源文件，它可能永远也不回来了。因此需要考虑已知断点
	}

	//这么写确保在STEP到别的文件不回去的情况，或者在步入别文件发生断点
	{ //CONTINUE 下一个断点
		bool isGetinfo = false;
		string source;
		mMutBPS.lock();

		for( BPVector::iterator i = mBreakpoints.begin();i!=mBreakpoints.end();++i )
		{
			if( ar->currentline==i->first )
			{
				if( !isGetinfo )
				{
					GetLuaSource(L,ar);
					source = lowerFileName(ar->short_src);
					isGetinfo = true;
				}
				if( i->second == source )
				{
					mMutBPS.unlock();
					LuaWait(L,ar,0,true);
					FillStep(L,ar);
					return;
				}
			}
		}
		mMutBPS.unlock();
	}
}