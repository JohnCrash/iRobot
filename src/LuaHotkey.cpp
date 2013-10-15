#include "stdheader.h"
#include "HotkeyManager.h"
#include "LuaManager.h"
#include "LuaExport.h"
#include "LuaHotkey.h"

/*	为HotkeyManager实现一个Lua界面
	hotkey.load( name ) 装入一个热键表
	hotkey.clear() 清除全部热键
	hotkey.save( name ) 存储当前热键表
	hotkey.remove( name ) 删除一个热键
	hotkey.add( name,caption,tip,hotkey ) 加入一个热键
	注意：切换场景时自动删除上一个场景的全部热键
*/
static HotkeyManager* lua_getHotkeyManager( lua_State* L )
{  
	HotkeyManager* p = HotkeyManager::getSingletonPtr();
	if( !p )
	{
		luaL_error( L,"invaild HotkeyManager instance" );
	}
	return p;
}

static int lua_loadHotkey( lua_State* L )
{
	HotkeyManager* p = lua_getHotkeyManager( L );
	if( p )
	{
		size_t len;
		const char* s = luaL_checklstring(L,1,&len);
		lua_pushboolean(L,p->load( s ));
		return 1;
	}
	return 0;
}

static int lua_clearHotkey( lua_State* L )
{
	HotkeyManager* p = lua_getHotkeyManager( L );
	if( p )
	{
		p->clear();
	}
	return 0;
}

static int lua_saveHotkey( lua_State* L )
{
	HotkeyManager* p = lua_getHotkeyManager( L );
	if( p )
	{
		size_t len;
		const char* s = luaL_checklstring(L,1,&len);
		lua_pushboolean(L,p->save( s ));
		return 1;
	}
	return 0;
}

static int lua_addHotkey( lua_State* L )
{
	HotkeyManager* p = lua_getHotkeyManager( L );
	if( p )
	{
		size_t len;
		const char* name = luaL_checklstring(L,1,&len);
		const char* caption = luaL_checklstring(L,2,&len);
		const char* tip = luaL_checklstring(L,3,&len);
		const char* key = luaL_checklstring(L,4,&len);
		p->addHotkey(name,caption,tip,key);
	}
	return 0;
}

static int lua_removeHotkey( lua_State* L )
{
	HotkeyManager* p = lua_getHotkeyManager( L );
	if( p )
	{
		size_t len;
		const char* name = luaL_checklstring(L,1,&len);
		p->removeHotkey( name );
	}
	return 0;
}

static int lua_setHotkey( lua_State* L )
{
	HotkeyManager* p = lua_getHotkeyManager( L );
	if( p )
	{
		size_t len;
		const char* name = luaL_checklstring(L,1,&len);
		const char* key = luaL_checklstring(L,2,&len);
		p->setHotkey( name,key );
	}
	return 0;
}

static int lua_getCount( lua_State* L )
{
	HotkeyManager* p = lua_getHotkeyManager( L );
	if( p )
	{
		lua_pushinteger(L,p->getHotkeyCount());
		return 1;
	}
	return 0;
}

static int lua_getHotkeyAt( lua_State* L )
{
	HotkeyManager* p = lua_getHotkeyManager( L );
	if( p )
	{
		int i = luaL_checkint(L,1);
		if( i < 0 || i >= p->getHotkeyCount() )
		{
			ostringstream os;
			os << "hotkey.at(" << i << ") out of rang";
			lua_errorstring(L,os.str());
			return 0;
		}
		HotkeyManager::hotkey hk = p->getHotkeyAt(i);
		lua_pushlstring(L,hk.mName.c_str(),hk.mName.size());
		lua_pushlstring(L,hk.mCaption.c_str(),hk.mCaption.size());
		lua_pushlstring(L,hk.mTip.c_str(),hk.mTip.size());
		lua_pushlstring(L,hk.mHotkey.c_str(),hk.mHotkey.size());
		return 4;
	}
	return 0;
}

static const struct luaL_Reg hotkeyLibs[]=
{
	{"load",lua_loadHotkey},
	{"clear",lua_clearHotkey},
	{"save",lua_saveHotkey},
	{"add",lua_addHotkey},
	{"remove",lua_removeHotkey},
	{"set",lua_setHotkey},
	{"count",lua_getCount},
	{"at",lua_getHotkeyAt},
	{nullptr,nullptr}
};

void luaopen_hotkey( lua_State* L )
{
	LuaManager::getSingleton().registerGlobal("hotkey",hotkeyLibs);
}