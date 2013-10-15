#ifndef _LUA_BIND_H_
#define _LUA_BIND_H_

/*
	LuaBind连接Lua对象和C++对象
	matetable 是原表名称
	ref Lua对象只身的引用
	pointer C++对象指针
*/
struct LuaBind
{
	const char* matetable; //原表名
	int ref; //该绑定对象对应的引用
	int refUTable; //该对象的Lua附属表
	/*
	一个指针如果null表示不可用
	否则*self就是具体对应的C++对象指针
	*/
	void* self; 

	LuaBind():
		matetable(nullptr),
        ref(LUA_REFNIL),
		refUTable(LUA_REFNIL),
		self(nullptr)
		{}
};

void lua_errorstring(lua_State* L,const string& msg );

/*
	成员函数调用时用于取出对应的C++对象指针
	这里假设所有的userdata都是使用LuaBind来绑定的

	将堆栈n，的值转换为对象。
	为了加速转换这里采用一种静态加速技术。
	例如
	meta="Button"用这种方式类明确表示继承关系
	(*plb)->matetable="mygui.Widget.Button"
*/
void* lua_cast( lua_State* L,int n,const char* meta );

//用来判断能不能转换
void* lua_isa( lua_State* L,int n,const char* meta );

/*
	将一个C++对象绑定为一个Lua userdata类型
	类型名称为meta。函数将向L中压入创建好的对象
	如果obj==NULL，函数会压入一个nil进入L
	具体细节看实现说明
*/
void lua_bind( lua_State* L,const char* meta,void* obj );

/*
	将一个C++对象绑定为一个Lua userdata类型
	类型名称为meta。函数将向L中压入创建好的对象
	如果obj==NULL，函数会压入一个nil进入L
	具体细节看实现说明
*/
void lua_bindComplete( lua_State* L,const char* meta,void* obj );

/*
	绑定一个boost::shared_ptr
	C++掌握自己的生命期，Lua仅仅是引用C++对象(观察者)。
	需要实现__gc用来删除weak_ptr指针
*/
template <typename T> void lua_bindWeakPtr( lua_State* L,
											 const char* meta,
											 const boost::shared_ptr<T>& ptr )
{
	if( !ptr )
	{
		lua_pushnil(L);
		return;
	}
	LuaBind* pp = LuaManager::getSingletonPtr()->getLuaBind(ptr.get());
	if( pp )
	{
		lua_rawgeti(L,LUA_REGISTRYINDEX,pp->ref);
	}
	else
	{
		LuaBind* pp = (LuaBind*)lua_newuserdata(L,sizeof(LuaBind));

		pp->matetable = LuaManager::getSingleton().getClassString(meta);
		
		pp->ref = LUA_REFNIL;
		pp->refUTable = LUA_REFNIL;
		pp->self = new boost::weak_ptr<T>(ptr);

		LuaManager::getSingleton().newLuaBind( ptr.get(),pp );

		luaL_getmetatable(L,meta);
		lua_setmetatable(L,-2);
	}
}

template<typename T> boost::shared_ptr<T> cast_weak_ptr(lua_State*L,int n,const char* meta)
{
	boost::weak_ptr<T>* wptr = (boost::weak_ptr<T>*)lua_cast( L,1,meta );
	if( wptr == nullptr || wptr->expired() )
	{
		string msg = "expired shared_ptr or invalid class ";
		msg += meta;
		lua_errorstring(L, msg);
		return boost::shared_ptr<T>();
	}
	return wptr->lock();
}

/*
	绑定一个boost::shared_ptr
	拥有该对象一个shared_ptr实例，需要__gc来施放shared_ptr指针
*/
template <typename T> void lua_bindSharedPtr( lua_State* L,
											 const char* meta,
											 const boost::shared_ptr<T>& ptr )
{
	if( !ptr )
	{
		lua_pushnil(L);
		return;
	}
	LuaBind* pp = LuaManager::getSingletonPtr()->getLuaBind(ptr.get());
	if( pp )
	{
		lua_rawgeti(L,LUA_REGISTRYINDEX,pp->ref);
	}
	else
	{
		pp = (LuaBind*)lua_newuserdata(L,sizeof(LuaBind));

		pp->matetable = LuaManager::getSingleton().getClassString(meta);
		
		lua_pushvalue(L,-1);
		pp->ref = luaL_ref(L,LUA_REGISTRYINDEX);
		pp->refUTable = LUA_REFNIL;
		pp->self = new boost::shared_ptr<T>(ptr);
        
		LuaManager::getSingleton().newLuaBind( ptr.get(),pp );

		luaL_getmetatable(L,meta);
		lua_setmetatable(L,-2);
	}
}

template<typename T> boost::shared_ptr<T> cast_shared_ptr(lua_State*L,int n,const char* meta)
{
	boost::shared_ptr<T>* ptr = (boost::shared_ptr<T>*)lua_cast( L,1,meta );

	if( !ptr )
	{
		string msg = "expired shared_ptr or invalid class ";
		msg += meta;
		lua_errorstring(L, msg);
		return boost::shared_ptr<T>();
	}
	return *ptr;
}

/*
	默认的Lua userdata清除函数
*/
int lua_defaultGC( lua_State* L );

/*
	调试函数，打印当前堆栈
*/
void lua_dumpStack(lua_State* L);

void lua_bindWidget( lua_State* L,MyGUI::Widget* p );

/*
	简化宏
*/
#define s_len( x ) \
size_t len;\
const char* s = luaL_checklstring(L,x,&len);\

#define toint( x ) luaL_checkinteger(L,x)
#define tobool( x ) lua_toboolean(L,x)?true:false
#define tostring( x ) luaL_checkstring(L,x)
#define tonumber( x ) (float)luaL_checknumber(L,x)

#define pint( x ) lua_pushinteger(L,x)
#define pstring( x ) lua_pushstring(L,x)
#define pbool( x ) lua_pushboolean(L,x)
#define pnumber( x ) lua_pushnumber(L,x)

#endif