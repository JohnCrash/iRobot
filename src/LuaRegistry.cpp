#include "stdheader.h"
#include "Registry.h"
#include "LuaManager.h"
#include "LuaExport.h"

static int lua_get( lua_State* L )
{
	size_t len;
	const char* s = luaL_tolstring(L,1,&len);
	if( s )
	{
		string value = Registry::getSingleton().get(s);
		if( value.empty() )
		{
			lua_pushvalue(L,2);
			return 1;
		}
		lua_pushstring(L,value.c_str());
		return 1;
	}
	lua_pushvalue(L,2);
	return 1;
}

static int lua_set( lua_State* L )
{
	size_t len;
	const char* key = luaL_tolstring(L,1,&len);
	const char* value = luaL_tolstring(L,2,&len);
	if( key && value )
		Registry::getSingleton().set(key,value);
	return 0;
}

static const struct luaL_Reg registryLibs[]=
{
	{"get",lua_get},
	{"set",lua_set},
	{nullptr,nullptr}
};

void luaopen_registry( lua_State* L )
{
	LuaManager::getSingleton().registerGlobal("registry",registryLibs);
}