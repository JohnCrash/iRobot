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
#include "mobDebug.h"
//#include "MobDebug.h"
#include "utf8.h"
//#define WIN32_LEAN_AND_MEAN
#include "PluginDefinition.h"
#include "menuCmdID.h"
#include "commctrl.h"
//#define BOOST_ASIO_NO_WIN32_LEAN_AND_MEAN
//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;
//const char *ipaddr = "192.168.2.116";
LuaMobDebug *dbg;//("192.168.2.116",8172);

int LastLine = -1;
std::wstring LastSource;
HINSTANCE ghThisInstance;
HWND getCurrentSCI();
std::wstring get_lua_source_root();

//确定当前编辑的文件名
std::wstring getCurrentFile()
{
	std::wstring name;
	name.resize(256);
	LRESULT BufferID = ::SendMessage(nppData._nppHandle,NPPM_GETCURRENTBUFFERID,0,0);
	::SendMessage(nppData._nppHandle,NPPM_GETFULLPATHFROMBUFFERID,BufferID,(LPARAM)name.c_str());
	return name;
}

//取得当前编辑窗口光标所在行
int getCurrentLine()
{
	int pos = ::SendMessage(getCurrentSCI(),SCI_GETCURRENTPOS,0,0);
	int line = ::SendMessage(getCurrentSCI(),SCI_LINEFROMPOSITION,pos,0);
	return line+1;
}

#define MARK_BOOKMARK 24

//保存全部文档
void saveAll()
{
	SendMessage(nppData._nppHandle,NPPM_SAVEALLFILES,0,0);
}

void setCurrentBookLine(int line)
{
	if( line >= 1 )
		::SendMessage(getCurrentSCI(),SCI_MARKERADD,line-1,MARK_BOOKMARK);
}

void clearCurrentBookLine(int line)
{
	if( line >= 1 )
		::SendMessage(getCurrentSCI(),SCI_MARKERDELETE,line-1,MARK_BOOKMARK);
}

//当前行是一个BookLine?
bool isCurrentBookLine( int line )
{
	if( line >= 1 )
	{
		if( ::SendMessage(getCurrentSCI(),SCI_MARKERGET,line-1,0) == MARK_BOOKMARK )
			return true;
	}
	return false;
}

int CurrentBookNext( int line )
{
	if( line >= 1 )
	{
		int next = (int)SendMessage(getCurrentSCI(),SCI_MARKERGET,line-1,0);
		if( next >= 0 )
			return next+1;
		else
			return -1;
	}
}

bool gotoLine( std::wstring source,int line )
{
	std::wstring us = source;
	SwitchTo( us );
	::SendMessage(getCurrentSCI(),SCI_GOTOLINE,(WPARAM)line-1,0);
	return true;
}

#define SCE_UNIVERSAL_FOUND_STYLE_INC 28
#define SCE_UNIVERSAL_FOUND_STYLE 31

int lastMakeLineType;

void makeLine( std::wstring source,int line,int type )
{
	int start,len;
	std::wstring us = source;
	SwitchTo( source );

	start = ::SendMessage(getCurrentSCI(),SCI_POSITIONFROMLINE,line-1,0);
	if( start != -1 )
	{
		len = ::SendMessage(getCurrentSCI(),SCI_LINELENGTH,line-1,0);
		if( len > 0 )
		{
			::SendMessage(getCurrentSCI(),SCI_SETINDICATORCURRENT,type,0);
			::SendMessage(getCurrentSCI(),SCI_INDICATORFILLRANGE,start,len);
			lastMakeLineType = type;
		}
		else
		{
			MessageBox(getCurrentSCI(),TEXT("len < 0"),TEXT("makeLine"),MB_OK);
		}
	}
	else
	{
		MessageBox(getCurrentSCI(),TEXT("start = -1"),TEXT("makeLine"),MB_OK);
	}
}

HWND getCurrentSCI()
{
	int r;
	SendMessage(nppData._nppHandle,NPPM_GETCURRENTSCINTILLA,0,(LPARAM)&r);
	if( r == MAIN_VIEW )
		return nppData._scintillaMainHandle;
	else if( r == SUB_VIEW )
		return nppData._scintillaSecondHandle;
	else 
		return nppData._scintillaMainHandle;
}

void clearMakeLine( std::wstring source, int line )
{
	int start,len;
	std::wstring us = source;
	SwitchTo(source);
	start = ::SendMessage(getCurrentSCI(),SCI_POSITIONFROMLINE,line-1,0);
	if( start != -1 )
	{
		len = ::SendMessage(getCurrentSCI(),SCI_LINELENGTH,line-1,0);
		::SendMessage(getCurrentSCI(),SCI_SETINDICATORCURRENT,lastMakeLineType,0);
		::SendMessage(getCurrentSCI(),SCI_INDICATORCLEARRANGE,start,len);
	}
}

//NPPM_GETNBOPENFILES
//NPPM_GETCURRENTSCINTILLA
//NPPM_GETOPENFILENAMES
std::vector<std::wstring> GetAllFileName()
{
	std::vector<std::wstring> vps;
	std::vector<TCHAR*> wps;
	int n = SendMessage(nppData._nppHandle,NPPM_GETNBOPENFILES,0,ALL_OPEN_FILES);
	if( n > 0 && n <128 )
	{
		wps.resize(n);
		for( int i = 0;i < n;i++ )
		{
			std::wstring ws;
			ws.resize(512);
			vps.push_back(ws);
			wps[i] = (TCHAR*)vps[i].c_str();
		}
		SendMessage(nppData._nppHandle,NPPM_GETOPENFILENAMES,(LPARAM)&wps[0],n);
	}
	return vps;
}

int GetNextMark( int line )
{
	return (int)SendMessage(getCurrentSCI(),SCI_MARKERNEXT,line,0x1000000);
}

bool GetMark( int line )
{
	return (int)SendMessage(getCurrentSCI(),SCI_MARKERGET,line-1,0)==0x1000000?true:false;
}

std::wstring lastBreakSource;
int lastBreakline;

void breakPoint( std::wstring source,int line )
{
	SetActiveWindow(nppData._nppHandle);
	SetForegroundWindow(nppData._nppHandle);

	lastBreakSource = source;
	lastBreakline = line;
	if( LastLine != -1 )
	{
		clearMakeLine(LastSource,LastLine);
		LastSource.clear();
		LastLine = -1;
	}

	if( gotoLine( source,line ) )
	{
		makeLine( source,line,SCE_UNIVERSAL_FOUND_STYLE_INC );
		LastSource = source;
		LastLine = line;
	}
}

void Error(std::wstring source,int line)
{
	SetActiveWindow(nppData._nppHandle);
	SetForegroundWindow(nppData._nppHandle);

	lastBreakSource = source;
	lastBreakline = line;
	if( LastLine != -1 )
	{
		clearMakeLine(LastSource,LastLine);
		LastSource.clear();
		LastLine = -1;
	}

	if( gotoLine( source,line ) )
	{
		makeLine( source,line,SCE_UNIVERSAL_FOUND_STYLE );
		LastSource = source;
		LastLine = line;
	}
}

void SwitchTo(std::wstring filename )
{
	if( !SendMessage(nppData._nppHandle,NPPM_SWITCHTOFILE,0,(LPARAM)filename.c_str()) )
	{
		std::string fn = toUTF8(filename);
		FILE *fp = fopen( fn.c_str(),"r" );
		if( fp )
		{
			fclose( fp );
			//没有切换好
			SendMessage(nppData._nppHandle,NPPM_DOOPEN,0,(LPARAM)filename.c_str());
		}
		else
		{
			MessageBox(nppData._nppHandle,filename.c_str(),TEXT("Not exist file"),MB_OK );
		}
	}
}

void newDebug()
{
	if(dbg )
	{
		//同步当前文档中的全部BookMark为断点
		std::vector<std::wstring> wsv = GetAllFileName();
		for( std::vector<std::wstring>::iterator i = wsv.begin();i!=wsv.end();++i )
		{
			SwitchTo(*i);
			int line = 0;
			do
			{
				line = GetNextMark(line);
				if( line==-1 )break;
				line++;
				dbg->addBreakpoint(*i,line);
			}while( line>=0 );
		}
	}
}

#define SHOW_INFO WM_USER+101
std::string lastInfo;
std::string errorMsg;
int  errorLine;
std::wstring errorSource;

void showInfo(std::string info)
{
	lastInfo = info;
	PostMessage(nppData._scintillaMainHandle,SHOW_INFO,0,0);
}

void ErrorInfo( std::string msg)
{
	//格式大概是这样的H:/s/s.lua:1:msg
	xpressive::sregex setbp = xpressive::sregex::compile("([^|]+):(\\d+):([^|]+)");
	xpressive::smatch what;
	if( xpressive::regex_match(msg,what,setbp) )
	{
		errorSource = toUnicode(what[1]);
		boost::to_lower(errorSource);
		errorLine = lexical_cast<int>(what[2]);
		errorMsg = what[3];
	}
	else
	{
		errorSource = lastBreakSource;
		errorLine = lastBreakline;
		errorMsg = msg;
	}
	lastInfo = errorMsg;
	PostMessage(nppData._scintillaMainHandle,SHOW_INFO,0,0);
}

std::string get_local_ip()
{
	char hostname[255];
	if( !gethostname(hostname,sizeof(hostname)) )
	{
		hostent *ht = gethostbyname(hostname);
		if( ht&&ht->h_addr_list)
		{
			return std::string(inet_ntoa(*(in_addr *)ht->h_addr_list[0]));
		}
	}
	return std::string("127.0.0.0");
}
//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE hModule)
{
	if( !dbg )
	{
		std::string ip = get_local_ip();
		try
		{
			dbg=new LuaMobDebug(ip,8172);
			if( dbg )
			{
				dbg->set_lua_source_root(get_lua_source_root());
			}
		}catch(std::exception &e)
		{
			dbg = nullptr;
			MessageBox(NULL,toUnicode(e.what()).c_str(),toUnicode(ip).c_str(),MB_OK);
		}
	}
	if(dbg)
	{
		dbg->run();
		dbg->setBreakNotify(breakPoint);
		dbg->setNewDebug(newDebug);
		dbg->setGetInfoNotify(showInfo);
		dbg->setErrorNotify(ErrorInfo);
		dbg->setErrorPointNotify(Error);
	}
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
	if(dbg )
	dbg->stop();
}

void run()
{
	if(dbg )
	{
		if( LastLine != -1 )
		{
			clearMakeLine(LastSource,LastLine);
			LastSource.clear();
			LastLine = -1;
		}
		dbg->doContinue();
	}
}

void setBreakpoint()
{
	if(dbg )
	{
		std::wstring file = getCurrentFile();
		int line = getCurrentLine();
		if( GetMark(line) )
		{
			//有删除
			clearCurrentBookLine(line);
			dbg->removeBreakpoint(file,line);
		}
		else
		{
			dbg->addBreakpoint( file,line );
			setCurrentBookLine( line );
		}
	}
}

void step()
{
	if(dbg )
	{
		if( LastLine != -1 )
		{
			clearMakeLine(LastSource,LastLine);
			LastSource.clear();
			LastLine = -1;
		}
		dbg->doStep();
	}
}

bool flag = false;
void stepin()
{
	if(dbg )
	{
		if( LastLine != -1 )
		{
			clearMakeLine(LastSource,LastLine);
			LastSource.clear();
			LastLine = -1;
		}
		dbg->doStepIn();
	}
}

void clearAll()
{
	if(dbg )
	{
		std::vector<std::wstring> wsv = GetAllFileName();
		for( std::vector<std::wstring>::iterator i = wsv.begin();i!=wsv.end();++i )
		{
			SwitchTo(*i);
			int line = 0;
			do
			{
				line = GetNextMark(line);
				if( line==-1 )break;
				line++;
				clearCurrentBookLine(line);
			}while( line>=0 );
		}
		dbg->removeAll();
	}
}

void reset()
{
	if( dbg )
	{
		if( LastLine != -1 )
		{
			clearMakeLine(LastSource,LastLine);
			LastSource.clear();
			LastLine = -1;
		}
		dbg->doReset();
	}
}

WNDPROC oldProc;
int oldx,oldy;
int x,y;
int lastx,lasty;
HWND mouseWnd;
std::string lastStr;
std::map<HWND,HWND> wndMap;

/* CREATE A TOOLTIP CONTROL OVER THE ENTIRE WINDOW AREA */
void CreateMyTooltip (HWND hwnd)
{
                 // struct specifying control classes to register
    INITCOMMONCONTROLSEX iccex; 

          // struct specifying info about tool in ToolTip control
    TOOLINFO ti;
    unsigned int uid = 0;       // for ti initialization
	HWND hwndTT = NULL;
    RECT rect;                  // for client area coordinates

	if( wndMap.find(hwnd) != wndMap.end() )
	{
		hwndTT = wndMap[hwnd];
	}
	if( !hwndTT && !IsWindow(hwndTT) )
	{
		/* INITIALIZE COMMON CONTROLS */
		iccex.dwICC = ICC_WIN95_CLASSES;
		iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		InitCommonControlsEx(&iccex);

		/* CREATE A TOOLTIP WINDOW */
		hwndTT = CreateWindowEx(WS_EX_TOPMOST,
			TOOLTIPS_CLASS,
			NULL,
			WS_POPUP | TTS_NOPREFIX |TTS_BALLOON,// TTS_ALWAYSTIP,		
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			hwnd,
			NULL,
			ghThisInstance,
			NULL
			);
		SetWindowPos(hwndTT,
			HWND_TOPMOST,
			0,
			0,
			0,
			0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		/* GET COORDINATES OF THE MAIN CLIENT AREA */
		GetClientRect (hwnd, &rect);

		/* INITIALIZE MEMBERS OF THE TOOLINFO STRUCTURE */
		ti.cbSize = sizeof(TOOLINFO);
		ti.uFlags = TTF_SUBCLASS|TTF_TRANSPARENT ;
		ti.hwnd = hwnd;
		ti.hinst = ghThisInstance;
		ti.uId = uid;
		ti.lpszText = LPSTR_TEXTCALLBACK;//lptstr;
			// ToolTip control will cover the whole window
		ti.rect.left = rect.left;    
		ti.rect.top = rect.top;
		ti.rect.right = rect.right;
		ti.rect.bottom = rect.bottom;

		/* SEND AN ADDTOOL MESSAGE TO THE TOOLTIP CONTROL WINDOW */
		SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);	
		wndMap[hwnd] = hwndTT;
		SendMessage(hwndTT, TTM_ACTIVATE, (WPARAM)TRUE, 0);	
		//设置延迟
		SendMessage(hwndTT, TTM_SETDELAYTIME, (WPARAM)TTDT_AUTOMATIC, 999*1000);	
	}
	else
	{
		GetClientRect (hwnd, &rect);

		ti.cbSize = sizeof(TOOLINFO);
		ti.hinst = ghThisInstance;
		ti.lpszText = LPSTR_TEXTCALLBACK;//lptstr;
		ti.uId = uid;
		ti.hwnd = hwnd;
			// ToolTip control will cover the whole window
		ti.rect.left = rect.left;    
		ti.rect.top = rect.top;
		ti.rect.right = rect.right;
		ti.rect.bottom = rect.bottom;

		SendMessage(hwndTT, TTM_UPDATETIPTEXT, 0, (LPARAM) (LPTOOLINFO) &ti);	
		SendMessage(hwndTT, TTM_NEWTOOLRECT, (WPARAM)0, (LPARAM)&ti);	
	}
} 

bool isNameChar( char c )
{
	if( ( c >= 'a' && c <= 'z' ) ||
		( c >= 'A' && c <= 'Z' ) ||
		( c >= '0' && c <= '9' ) ||
		c == '_' || c == '.' )
		return true;
	return false;
}

bool isNameCharRight( char c )
{
	if( ( c >= 'a' && c <= 'z' ) ||
		( c >= 'A' && c <= 'Z' ) ||
		( c >= '0' && c <= '9' ) ||
		c == '_' )
		return true;
	return false;
}

void lineHandle(std::string line,int pos,std::string source,int l)
{
	if(dbg )
	{
		if( pos >= 0 && pos < line.size() )
		{
			std::string left,right;
			if( pos + 1 < line.size() && pos > 0 )
			{
				for( int i = pos+1;i<line.size();++i )
				{
					if( isNameCharRight(line.at(i)) )
					{
						right += line.at(i);
					}else
					{
						break;
					}
				}
			}
			for( int j = pos;j>=0;j-- )
			{
				if( isNameChar(line.at(j)) )
				{
					left.insert(left.begin(),line.at(j));
				}else
				{
					break;
				}
			}
			lastStr = left + right;
			if( !lastStr.empty() )
				dbg->doGetVariable(lastStr);
		}
		//如果有错误显示错误信息
		if( !errorMsg.empty() && 
			errorLine == l
			)
		{
			lastInfo = errorMsg;
			PostMessage(nppData._scintillaMainHandle,SHOW_INFO,0,0);
		}
	}
}

VOID CALLBACK TimerProc( HWND hWnd,UINT uMsg,UINT_PTR id,DWORD dwTime)
{
	if( oldx == x && oldy == y )
	{
		if( lastx != oldx || lasty != oldy )
		{
			int pos = SendMessage(mouseWnd,SCI_POSITIONFROMPOINT,x,y);
			if( pos >= 0 )
			{
				int line = SendMessage(mouseWnd,SCI_LINEFROMPOSITION,pos,0);
				if( line >= 0 )
				{
					int len = SendMessage(mouseWnd,SCI_GETLINE,line,0);
					if( len > 0 )
					{
						std::string str;
						str.resize(len);
						SendMessage(mouseWnd,SCI_GETLINE,line,(LPARAM)str.data());
						int startpos = SendMessage(mouseWnd,SCI_POSITIONFROMLINE,line,0);
						if( startpos>= 0 )
						{
							std::wstring source = getCurrentFile();
							boost::to_lower(source);
							lineHandle(str,pos-startpos,toUTF8(source),line);
						}
					}
				}
			}
		}
		lastx = x;
		lasty = y;
	}
	else
	{
		oldx = x;
		oldy = y;
	}
}

std::wstring wStr;

LRESULT PASCAL SCWndProc(
						 HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) 
{
	switch( iMessage )
	{
	case WM_MOUSEMOVE:
		{
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			mouseWnd = hWnd;
		}
		break;
	case SHOW_INFO:
		CreateMyTooltip(mouseWnd);
		break;
	case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR)lParam;
			if( lpnmhdr->code == TTN_GETDISPINFO )
			{
				LPNMTTDISPINFO lpttd = (LPNMTTDISPINFO)lpnmhdr;
				SendMessage(lpnmhdr->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 1024);
				wStr = utf8ToUnicode(lastInfo);
				lpttd->lpszText = (LPTSTR)wStr.c_str();
				return 0;
			}
		}
		break;
	}
	return CallWindowProc(oldProc,hWnd,iMessage,wParam,lParam);
}

//子类化
void SubClass(HWND hWnd)
{
	//oldProc = GetWindowLongPtr(hWnd,GWLP_WNDPROC);
	oldProc = (WNDPROC)SetWindowLongPtr(hWnd,GWLP_WNDPROC,(LONG_PTR)SCWndProc);
}

//回溯一级调用堆栈
void traceback()
{
	if(dbg )
		dbg->doTraceback();
}

//向前一级
void tracefront()
{
	if(dbg )
		dbg->doTracefront();
}
#include "Shlobj.h"

std::wstring get_lua_source_root()
{
	HKEY Root,NotePad,Key;
	std::wstring s;
	s.reserve(255);
	if( RegCreateKey(HKEY_CURRENT_USER,TEXT("Software"),&Root)==ERROR_SUCCESS)
	{
		if( RegCreateKey(Root,TEXT("NotePad++"),&NotePad)==ERROR_SUCCESS)
		{
			TCHAR path[255];
			LONG len;
			if( RegQueryValue(NotePad,TEXT("luaDebugRoot"),path,&len) == ERROR_SUCCESS )
			{
				s = path;
			}
			RegCloseKey(NotePad);
		}
		RegCloseKey(Root);
	}
	return s;
}
void set_lua_source_root(std::wstring n)
{
	HKEY Root,NotePad,Key;
	if( RegCreateKey(HKEY_CURRENT_USER,TEXT("Software"),&Root)==ERROR_SUCCESS)
	{
		if( RegCreateKey(Root,TEXT("NotePad++"),&NotePad)==ERROR_SUCCESS)
		{
			if( RegCreateKey(NotePad,TEXT("luaDebugRoot"),&Key)==ERROR_SUCCESS)
			{
				RegSetValue(Key,NULL,REG_SZ,n.c_str(),n.length());
				RegCloseKey(Key);
			}
			RegCloseKey(NotePad);
		}
		RegCloseKey(Root);
	}
}
//打开关闭取变量
void enableGetV()
{
	if( dbg )
	{
		dbg->enableGetV();
	}
}
//打开配置对话框
void configureDialog()
{
	wchar_t szPath[MAX_PATH];
	std::string str;
	BROWSEINFO bi;
	bi.hwndOwner = getCurrentSCI();
	bi.pidlRoot = NULL;
	bi.pszDisplayName = szPath;
	bi.lpszTitle = TEXT("Select lua source root");
	bi.ulFlags = 0;
	bi.lpfn = NULL;
	bi.iImage = 0;
	LPITEMIDLIST lp = SHBrowseForFolder(&bi);
	if(lp && SHGetPathFromIDList(lp, szPath))
	{
		set_lua_source_root(std::wstring(szPath));
		if( dbg )
		{
			dbg->set_lua_source_root( std::wstring(szPath) );
		}
	}
}

ShortcutKey mySK[nbFunc] = 
{
	{false,false,false,VK_F5}, //0
	{false,false,false,VK_F10}, //1
	{false,false,false,VK_F11},//2
	{false,false,false,VK_F9},//3
	{false,false,false,'0'},//4
	{false,false,false,VK_F12},//5
	{false,false,false,VK_F8},//6
	{false,false,false,VK_F7}//7
};
//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{
	ghThisInstance = GetModuleHandle(NULL);
    //--------------------------------------------//
    //-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
    //--------------------------------------------//
    // with function :
    // setCommand(int index,                      // zero based number to indicate the order of command
    //            TCHAR *commandName,             // the command name that you want to see in plugin menu
    //            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
    //            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
    //            bool check0nInit                // optional. Make this menu item be checked visually
    //            );
    //setCommand(0, TEXT("Hello Notepad++"), hello, NULL, false);
    //setCommand(1, TEXT("Hello (with dialog)"), helloDlg, NULL, false);
	setCommand(0,TEXT("Run"),run,&mySK[0],false);
	setCommand(1,TEXT("Step"),step,&mySK[1],false);
	setCommand(2,TEXT("Step in"),stepin,&mySK[2],false);
	setCommand(3,TEXT("Set Breakpoint"),setBreakpoint,&mySK[3],false);
	setCommand(4,TEXT("Clear all Breakpoint"),clearAll,NULL,false);
	setCommand(5,TEXT("Reset"),reset,&mySK[5],false);
	setCommand(6,TEXT("Trace back"),traceback,&mySK[6],false);
	setCommand(7,TEXT("Trace front"),tracefront,&mySK[7],false);
	setCommand(8,TEXT("Tooltips Enable or Disable"),enableGetV,NULL,false);
	setCommand(9,TEXT("Configure..."),configureDialog,NULL,false);

	SubClass(nppData._scintillaMainHandle);
	SubClass(nppData._scintillaSecondHandle);
	SetTimer(nppData._nppHandle,1002,100,TimerProc);
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
	if(dbg)
	{
		delete dbg;
		dbg = nullptr;
	}
	KillTimer(nppData._nppHandle,1002);
	//destory tooltip
	for( std::map<HWND,HWND>::iterator i = wndMap.begin();i!=wndMap.end();++i )
		DestroyWindow(i->second);
	wndMap.clear();
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
    if (index >= nbFunc)
        return false;

    if (!pFunc)
        return false;

    lstrcpy(funcItem[index]._itemName, cmdName);
    funcItem[index]._pFunc = pFunc;
    funcItem[index]._init2Check = check0nInit;
    funcItem[index]._pShKey = sk;

    return true;
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//
void hello()
{
    // Open a new document
    ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);

    // Get the current scintilla
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return;
    HWND curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;

    // Say hello now :
    // Scintilla control has no Unicode mode, so we use (char *) here
    ::SendMessage(curScintilla, SCI_SETTEXT, 0, (LPARAM)"Hello, Notepad++!");
}

void helloDlg()
{
    ::MessageBox(NULL, TEXT("Hello, Notepad++!"), TEXT("Notepad++ Plugin Template"), MB_OK);
}
