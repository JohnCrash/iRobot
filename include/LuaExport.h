#ifndef _LUAEXPORT_H_
#define _LUAEXPORT_H_

//给类Game输出一个Lua界面
void luaopen_game( lua_State* L );
//对MyGUI给出一个Lua界面
void luaopen_gui( lua_State* L );

void luaopen_rigid( lua_State* L );

//Ogre数学库
void luaopen_mat( lua_State* L );

//热键HotkeyManager界面
void luaopen_hotkey( lua_State* L );

//翻译,在_G中注册一个L函数
void luaopen_gettext( lua_State* L );

//MyGUI::xml
void luaopen_xml( lua_State* L );

//注册表库
void luaopen_registry( lua_State* L );
#endif