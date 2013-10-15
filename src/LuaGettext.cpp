#include "stdheader.h"
#include "LuaManager.h"

static int lua_gettext( lua_State* L )
{  
	size_t len;
	const char* s = luaL_checklstring(L,1,&len);
	const char* m = GettextManager::getSingleton().gettext(s);
	lua_pushlstring(L,m,strlen(m));
	return 1;
}

void luaopen_gettext( lua_State* L )
{
	LuaManager::getSingleton().registerCFunction("L",lua_gettext);
}