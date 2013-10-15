#include "stdheader.h"
#include "InputFilter.h"

SINGLETON( InputFilter )


void InputFilter::addControlet( Controlet* pf )
{
	BOOST_AUTO( it,std::find( mControlet.begin(),mControlet.end(),pf ) );

	if( it == mControlet.end() ){
		mControlet.push_back( pf );
		mHasChange = true;
	}else{
		WARNING_LOG( "addControlet same Controlet interface pointer" );
	}
}

InputFilter::InputFilter():
	mControlet(),
	mCapture(false)
{
	mHasChange = false;
}

void InputFilter::removeControlet( Controlet* pf )
{
	BOOST_AUTO( it,std::find( mControlet.begin(),mControlet.end(),pf ) );

	if( it != mControlet.end() ){
		mControlet.erase(it);
		mHasChange = true;
	}
}

//同步静态表和主表
void InputFilter::synchStaticVector()
{
	if( mHasChange ){
		mStaticVector.resize( mControlet.size() );
		copy( mControlet.begin(),mControlet.end(),mStaticVector.begin() );
		mHasChange = false;
	}
}

void InputFilter::setCapture(){
	mCapture = true;
}

void InputFilter::releaseCapture(){
	mCapture = false;
}

/*这里我写一个宏用来完成一系列的类似循环结构
	这里主要用来解决一个问题，就是如果循环内部增加和删除数据时
	导致循环出现BUG，我的办法是:
	使用一个临时的表mStaticVector它是主表的拷贝，保证在循环中
	它的内容不会改变。增加的数据不会在这个循环生效。删除的数据
	必须在本次循环中终止对其进行调用。
*/
#define FOREACH_CONTROLET( function )\
{\
	synchStaticVector();\
	for( BOOST_AUTO(it,mStaticVector.begin());it!=mStaticVector.end();++it ){\
		if( mHasChange ){\
			BOOST_AUTO( i,std::find( mControlet.begin(),mControlet.end(),(*it) ) );\
			if( i==mControlet.end() )\
				continue;\
		}\
		if( (*it)->isEnable() )\
		{\
			(*it)->function;\
		}\
	}\
}\

void InputFilter::injectMouseMove(int _absx, int _absy, int _absz)
{
	bool b = MyGUI::InputManager::getInstance().injectMouseMove(_absx, _absy, _absz);

	if( !b || mCapture ){
		FOREACH_CONTROLET( injectMouseMove(_absx, _absy, _absz) )
	}
}

void InputFilter::injectMousePress(int _absx, int _absy, MyGUI::MouseButton _id)
{
	bool b = MyGUI::InputManager::getInstance().injectMousePress(_absx, _absy, _id);

	if( !b || mCapture ){
		FOREACH_CONTROLET( injectMousePress(_absx, _absy, _id) )
	}
}

void InputFilter::injectMouseRelease(int _absx, int _absy, MyGUI::MouseButton _id)
{
	bool b = MyGUI::InputManager::getInstance().injectMouseRelease(_absx, _absy, _id);

	if( !b || mCapture ){
		FOREACH_CONTROLET( injectMouseRelease(_absx, _absy, _id) )
	}
}

/*
	对一些Controlet可以不进行IsKeyFocus过滤，既为全局的
*/
#define FOREACH_CONTROLET2( function )\
{\
	synchStaticVector();\
	for( BOOST_AUTO(it,mStaticVector.begin());it!=mStaticVector.end();++it ){\
		if( mHasChange ){\
			BOOST_AUTO( i,std::find( mControlet.begin(),mControlet.end(),(*it) ) );\
			if( i==mControlet.end() )\
				continue;\
		}\
		if( (*it)->isEnable() )\
		{\
			if( (*it)->isGlobal() || !IsKeyFocus() )\
				(*it)->function;\
		}\
	}\
}\

void InputFilter::injectKeyPress(MyGUI::KeyCode _key, MyGUI::Char _text)
{
	if( MyGUI::InputManager::getInstance().injectKeyPress(_key, _text) )
		return;

	FOREACH_CONTROLET2( injectKeyPress( _key,_text) )
}

void InputFilter::injectKeyRelease(MyGUI::KeyCode _key)
{
	if( MyGUI::InputManager::getInstance().injectKeyRelease(_key) )
		return;

	FOREACH_CONTROLET2( injectKeyRelease( _key ) )
}

bool InputFilter::frameStarted(const Ogre::FrameEvent& evt)
{
	FOREACH_CONTROLET( frameStarted( evt ) )
	return true;
}

bool InputFilter::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	FOREACH_CONTROLET( frameRenderingQueued( evt ) )
	return true;
}

bool InputFilter::frameEnded(const Ogre::FrameEvent& evt)
{
	FOREACH_CONTROLET( frameEnded( evt ) )
	return true;
}

bool InputFilter::IsKeyFocus(){
	return MyGUI::InputManager::getInstance().isFocusKey();
}

bool InputFilter::IsMouseFocus(){
	return MyGUI::InputManager::getInstance().isFocusMouse();
}

/*
	将ScanCode映射为解释文本
*/
static const char* scanText[] = 
{
	"unassigned",
	"escape",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"0",
	"-",
	"=",
	"backspace",
	"tab",
	"q",
	"w",
	"e",
	"r",
	"t",
	"y",
	"u",
	"i",
	"o",
	"p",
	"(",
	")",
	"return",
	"ctrl",
	"a",
	"s",
	"d",
	"f",
	"g",
	"h",
	"j",
	"k",
	"l",
	";",
	"'",
	"~",
	"shift",
	"backslash",
	"z",
	"x",
	"c",
	"v",
	"b",
	"n",
	"m",
	",",
	".",
	"slash",
	"shift",
	"*",
	"alt",
	"space",
	"capital",
	"f1",
	"f2",
	"f3",
	"f4",
	"f5",
	"f6",
	"f7",
	"f8",
	"f9",
	"f10",
	"numlock",
	"scroll",
	"7", //numpad
	"8",
	"9",
	"-",
	"4",
	"5",
	"6",
	"+",
	"1",
	"2",
	"3",
	"numpad0",//numpad
	".",
	"",//54
	"",//55
	"OEM_102",
	"f11",
	"f12", //58
	"", //59
	"", //5A
	"", //5B
	"", //5C
	"", //5D
	"", //5E
	"", //5F
	"", //60
	"", //61
	"", //62
	"", //63
	"f13", //64
	"f14", //65
	"f15", //66
	"COMMAND",//67 Apple command
	"",//68
	"",//69
	"",//6A
	"",//6B
	"",//6C
	"",//6D
	"",//6E
	"",//6F
	"KANA",
	"",//71
	"",//72
	"ABNT_C1",//73
	"",//74
	"",//75
	"",//76
	"",//77
	"",//78
	"CONVERT", //79
	"",//7A
	"NOCONVERT", //7B
	"",//7C
	"YEN", //7D
	"ABNT_C2", //7E
	"",//7F
	"",//80
	"",//81
	"",//82
	"",//83
	"",//84
	"",//85
	"",//86
	"",//87
	"",//88
	"",//89
	"",//8A
	"",//8B
	"",//8C
	"NUMPADEQUALS", //8D
	"",//8E
	"",//8F
	"prevtrack", //90
	"AT", //91
	"COLON", //92
	"UNDERLINE", //93
	"KANJI", //94
	"stop", //95
	"ax", //96
	"unlabeled", //97
	"",//98
	"nexttrack", //99
	"",//9A
	"",//9B
	"enter", //9C
	"rcontrol",//9D
	"",//9E
	"",//9F
	"mut", //A0
	"calculator", //A1
	"play", //A2
	"",//A3
	"stop", //A4
	"",//A5
	"",//A6
	"",//A7
	"",//A8
	"",//A9
	"",//AA
	"",//AB
	"",//AC
	"",//AD
	"volumedown", //AE
	"",//AF
	"volumeup", //B0
	"",//B1
	"webhome",//B2
	"COMMA", //B3
	"",//B4
	"/", //B5
	"",//B6
	"SYSRQ", //B7
	"RMENU", //B8
	"",//B9
	"",//BA
	"",//BB
	"",//BC
	"",//BD
	"",//BE
	"",//BF
	"",//C0
	"",//C1
	"",//C2
	"",//C3
	"",//C4
	"PAUSE", //C5
	"",//C6
	"home", //C7
	"up",
	"pgup", //C9
	"",//CA
	"left", //CB
	"",//CC
	"right", //CD
	"",//CE
	"end", //CF
	"down",
	"pgdown",
	"insert",
	"delete", //D3
	"LWIN",
	"RWIN",
	"APPS",
	"power",
	"wake",
	"WEBSEARCH",
	"WEBFAVORITES",
	"WEBREFRESH",
	"WEBSTOP",
	"WEBFORWARD",
	"WEBBACK",
	"MYCOMPUTER",
	"MAIL",
	"MEDIASELECT",
	"MAX",
	NULL
};

const char* ScanCodeToText( int code )
{
	int size = sizeof(scanText)/sizeof(const char*);
	if( code >= 0 && code < size )
		return scanText[code];
	return "";
}