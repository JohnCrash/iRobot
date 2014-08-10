//this file is part of notepad++
//Copyright (C)2003 Don HO <donho@altern.org>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef PLUGINDEFINITION_H
#define PLUGINDEFINITION_H

//
// All difinitions of plugin interface
//
#include "PluginInterface.h"
#include <vector>
#include <string>

//-------------------------------------//
//-- STEP 1. DEFINE YOUR PLUGIN NAME --//
//-------------------------------------//
// Here define your plugin name
//
const TCHAR NPP_PLUGIN_NAME[] = TEXT("Lua debuger");

//-----------------------------------------------//
//-- STEP 2. DEFINE YOUR PLUGIN COMMAND NUMBER --//
//-----------------------------------------------//
//
// Here define the number of your plugin commands
//
const int nbFunc = 10;


//
// Initialization of your plugin data
// It will be called while plugin loading
//
void pluginInit(HANDLE hModule);

//
// Cleaning of your plugin
// It will be called while plugin unloading
//
void pluginCleanUp();

//
//Initialization of your plugin commands
//
void commandMenuInit();

//
//Clean up your plugin commands allocation (if any)
//
void commandMenuCleanUp();

//
// Function which sets your command 
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk = NULL, bool check0nInit = false);


//切换到文档
void switchDocument( std::wstring source);

//清理标记
void clearMakeLine( std::wstring source, int line );
//标记
void makeLine( std::wstring source,int line,int type );
//调到指定的行
bool gotoLine( std::wstring source,int line );
//切换到指定文档
void SwitchTo(std::wstring filename );
//确定当前编辑的文件名
std::wstring getCurrentFile();
//取得当前编辑窗口光标所在行
int getCurrentLine();
//取当前编辑窗句柄
HWND getCurrentSCI();
//保存全部文档
void saveAll();
void setCurrentBookLine(int line);
void clearCurrentBookLine(int line);
//当前行是一个BookLine?
bool isCurrentBookLine( int line );
int CurrentBookNext( int line );
std::vector<std::wstring> GetAllFileName();
int GetNextMark( int line );
bool GetMark( int line );

//
//
// Your plugin command functions
//
void hello();
void helloDlg();

#endif //PLUGINDEFINITION_H
