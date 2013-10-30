#include "stdheader.h"
#include "LuaManager.h"

/*
	绑定的对象，从lua层面不能施放。
	例如一个Button，如果Lua对应的userdata被建立那么它就不能
	被回收，直到Button被销毁后对应的userdata才能被回收。
	(这是因为该Lua对象有一个全局引用ref)
	从Lua角度，如果被回收不能同时销毁窗口
	从C++角度，窗口被销毁对应的Lua所指物被设置为nil，随后
	调用对象的方法将什么也不做。
	C++对象存在时，Lua对象可正常访问 
	C++对象删除时，Lua对象存在但是不能正常C++访问（保持LuaBind结构存在）
	C++对象被删除将解除Lua对象的自引用，如果这时Lua对象也停止引用。
	则删除LuaBind结构
	被绑定对象不需要实现__gc

	注意：该函数仅仅用于MyGUI::Widget的绑定，
	绑定其他类型需要在C++对象施放时调用LuaManager的施放函数
	参见LuaManager::_unlinkWidget
*/
void lua_bind( lua_State* L,const char* meta,void* obj )
{
	if( obj == NULL )
	{
		lua_pushnil(L);
		return;
	}

	LuaBind* plb = LuaManager::getSingleton().getLuaBind( obj );
	if( plb )
	{
		//已经存在它的绑定
		lua_rawgeti(L,LUA_REGISTRYINDEX,plb->ref);
	}
	else
	{ //建立一个新的绑定
		LuaBind* pp = (LuaBind*)lua_newuserdata(L,sizeof(LuaBind));
		/*
			建立对象的自身引用，这样在重复对该C++对象绑定时可以直接返回引用
		*/
		pp->matetable = LuaManager::getSingleton().getClassString(meta);
		
		lua_pushvalue(L,-1);
		pp->ref = luaL_ref(L,LUA_REGISTRYINDEX);
		pp->refUTable = LUA_REFNIL;
		pp->self = obj;

		LuaManager::getSingleton().newLuaBind( obj,pp );
		
		luaL_getmetatable(L,meta);
		lua_setmetatable(L,-2);
	}
}

/*
	完全绑定的对象
	例如一个UString，被创建的C++对象。在Lua停止引用时被Lua销毁。
	Lua完全用于该C++对象的生命期。需要为该对象实现__gc方法。
	该C++对象的生命期完全取决与Lua对象，Lua对象停止引用将删除对应
	的C++对象。
*/
void lua_bindComplete( lua_State* L,const char* meta,void* obj )
{
	if( obj == NULL )
	{
		lua_pushnil(L);
		return;
	}
	LuaBind* pp = (LuaBind*)lua_newuserdata(L,sizeof(LuaBind));

	pp->matetable = LuaManager::getSingleton().getClassString(meta);
	
	pp->ref = LUA_REFNIL;
	pp->refUTable = LUA_REFNIL;
	pp->self = obj;
	
	luaL_getmetatable(L,meta);
	lua_setmetatable(L,-2);
}

void lua_dumpStack(lua_State* L)
{
	int i;
	int top = lua_gettop(L);
	for(i =1;i<=top;i++ )
	{
		int t = lua_type(L,i);
		switch( t )
		{
		case LUA_TSTRING:
			{
				printf("'%s'",lua_tostring(L,i));
				break;
			}
		case LUA_TBOOLEAN:
			{
				printf(lua_toboolean(L,i)?"true":"false");
				break;
			}
		case LUA_TNUMBER:
			{
				printf("%g",lua_tonumber(L,i));
				break;
			}
		default:
			{
				printf("%s",lua_typename(L,t));
				break;
			}
		}
		printf("\n");
	}
}

/*
*/
void* lua_cast( lua_State* L,int n,const char* meta )
{
	LuaBind* plb = (LuaBind*)lua_touserdata(L,n);
	if( plb && plb->self )
	{
		if( strcmp(plb->matetable,meta)==0 )
			return plb->self;
		else if( strstr(plb->matetable,meta) )
			return plb->self;
		else
		{
			ostringstream os;
			os << "argument @"<<n<<" can't cast from '"<<plb->matetable<<"' to '"<<meta<<"'";
			lua_errorstring(L,os.str());
			return nullptr;
		}
	}
	else
	{
		//如果不是userdata数据
		if( !lua_isuserdata(L,n) )
		{
			ostringstream os;
			os << "argument @"<<n<<" can't cast from '"<<lua_typename(L,lua_type(L,n))<<"' to '"<<meta<<"'";
			lua_errorstring(L,os.str());
		}
		//失效的句柄，不显示错误
		return nullptr;
	}
}

void* lua_isa( lua_State* L,int n,const char* meta )
{
	LuaBind* plb = (LuaBind*)lua_touserdata(L,n);
	if( plb && plb->self )
	{
		if( strcmp(plb->matetable,meta)==0 )
			return plb->self;
		else if( strstr(plb->matetable,meta) )
			return plb->self;
		else
		{
			return nullptr;
		}
	}
	else
	{
		return nullptr;
	}
}

void lua_errorstring(lua_State* L,const string& msg )
{
	lua_pushstring(L,msg.c_str() );
	lua_error(L);
}