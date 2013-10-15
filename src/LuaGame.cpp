#include "stdheader.h"
#include "Game.h"
#include "LuaManager.h"
#include "LuaExport.h"

/*为Game类实现一个Lua界面
	game.getStages() 返回一个场景命中表
	game.openStage( name ) 打开一个场景
	game.currentStage() 返回当前场景名称
*/
static Game* lua_getGame( lua_State* L )
{  
	Game* g = Game::getSingletonPtr();
	if( !g )
	{
		luaL_error( L,"invaild game instance" );
	}
	return g;
}

/*
	game.quit() 退出游戏
*/

static int lua_quitGame( lua_State* L )
{
	Game* g = lua_getGame(L);
	if( g )
	{
		g->quit();
	}
	return 0;
}

static int lua_resetGame( lua_State* L )
{
	Game* g = lua_getGame(L);
	if( g )
	{
		size_t len;
		const char* s = luaL_tolstring(L,1,&len);
		if( s )
		{
			if( strcmp(s,"NORMAL")==0 )
			{
				g->reset(Game::NORMAL);
				return 0;
			}
			else if( strcmp(s,"FAST")==0 )
			{
				g->reset(Game::FAST);
				return 0;
			}
		}
		luaL_error(L,"game.reset invaild parameter");
	}
	return 0;
}

/*
	注册C++调用Lua函数的界面
*/
static int lua_SetScript( lua_State* L )
{
	LuaManager* pl = LuaManager::getSingletonPtr();
	if( pl )
	{
		return pl->SetScript( L );
	}
	return 0;
}

/*
	设置一个视角控制器
*/
static int lua_setCameraControlet( lua_State* L )
{
	Game::getSingleton().setCameraControlet(tostring(1));
	return 0;
}

static int lua_applyTranslate( lua_State* L )
{
	pbool(GettextManager::getSingleton().load( luaL_checkstring(L,1) ));
	return 1;
}

static int lua_execute( lua_State* L )
{
	Game* g = lua_getGame(L);
	if( g )
	{
		size_t len;

		const char* s = luaL_tolstring(L,1,&len);
		if( s )
		{
			g->execute( s,Game::FAST );
		}
	}
	return 0;
}

static const struct luaL_Reg gameLibs[]=
{
	{"quit",lua_quitGame},
	{"reset",lua_resetGame},
	{"setScript",lua_SetScript},
	{"setCameraControlet",lua_setCameraControlet},
	{"applyTranslate",lua_applyTranslate},
	{"execute",lua_execute},
	{nullptr,nullptr}
};

void luaopen_game( lua_State* L )
{
	LuaManager::getSingleton().registerGlobal("game",gameLibs);
}
