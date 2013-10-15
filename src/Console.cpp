#include "stdheader.h"
#include "Console.h"
#if OGRE_PLATFORM==OGRE_PLATFORM_WIN32
#include <io.h>
#else//APPLE,Linux
#include <unctrl.h>
#endif
#include <fcntl.h>
#include "LuaManager.h"
#include "utf8.h"

Console::Console():
	mOut(),
	mStdOut(0),
	mThread(0),
	mEdit(0),
	mInput(nullptr)
	{

	mPipe[0] = 0;
	mPipe[1] = 0;
	
	mOut.reserve(STDOUT_BUFFER_MAX_CAPS);
#if OGRE_PLATFORM==OGRE_PLATFORM_WIN32		
	if( _pipe(mPipe,0,_O_TEXT) == 0 ){
		mStdOut = _fdopen(mPipe[WRITE],"w");
#else
	if( pipe(mPipe)== 0 ){
		mStdOut = fdopen(mPipe[WRITE],"w");
#endif
		if( mStdOut ){
			//重定向标准输出到管道的写入端
			mOldStdout = *stdout;
			*stdout = *mStdOut;
			//stdout不使用缓存区
			setvbuf( stdout, NULL, _IONBF, 0 );
			//创建读取线程
			mThread = new boost::thread(boost::bind(&Console::read_stdout,this));
		}
	}
}
	
void Console::read_stdout(){
	int len;
	char buf[80];
	while( true ){
#if OGRE_PLATFORM==OGRE_PLATFORM_WIN32		
		len = _read(mPipe[READ],buf,80);
#else
		len = read(mPipe[READ],buf,80);
#endif
		if( len <= 0 )break; //句柄被关闭或者=0非法的句柄
		if( mOut.size()+len>=STDOUT_BUFFER_MAX_CAPS ){
			//丢弃[0,STDOUT_BUFFER_MAX_CAPS-STDOUT_BUFFER_MAX_SIZE]部分
			mOut.erase(0,STDOUT_BUFFER_MAX_CAPS-STDOUT_BUFFER_MAX_SIZE);
		}
		mOut += std::string(buf,len);

		/*
			这里存在一个非UTF8字符的问题
		*/
		if( mEdit )
		{
			mAppendStr.append( toUString( string(buf,len) ) );
		}
	}
}

void Console::updateConsole(){
	if( mEdit && !mAppendStr.empty() ){
		//如果加入的字符超过了编辑栏的上限
		if( mEdit->getTextLength()+mAppendStr.size() > STDOUT_BUFFER_MAX_CAPS ){
			mEdit->setCaption( mOut );
		}else
			mEdit->addText( mAppendStr );
		mAppendStr.clear();
	}
}

Console::~Console(){
	if( mStdOut ){
		//恢复标准输出
		*stdout = mOldStdout;
		fclose(mStdOut);
#if OGRE_PLATFORM==OGRE_PLATFORM_WIN32		
		_close(mPipe[READ]);
#else
		close(mPipe[READ]);
#endif
		//删除线程
		delete mThread;
	}else if( mPipe[READ] ){
		//mStdOut创建可能失败单独施放
#if OGRE_PLATFORM==OGRE_PLATFORM_WIN32	
		_close(mPipe[WRITE]);
		_close(mPipe[READ]);
#else
		close(mPipe[WRITE]);
		close(mPipe[READ]);
#endif
	}
	mPipe[0] = 0;
	mPipe[1] = 0;
	mStdOut = 0;
	mThread = 0;
}

//打开一个控制台窗口
void Console::openConsole(){
	MyGUI::Gui* gui = MyGUI::Gui::getInstancePtr();
	if( gui && !mEdit ){
		MyGUI::IntSize size = MyGUI::RenderManager::getInstancePtr()->getViewSize();
		mEdit = gui->createWidget<MyGUI::EditBox>("EditBoxStretch",
			MyGUI::IntCoord(0,0,size.width,size.height/2),
			MyGUI::Align::HFull|MyGUI::Align::Top,"Info","_console");
		mInput = gui->createWidget<MyGUI::EditBox>("EditBox",
			MyGUI::IntCoord(0,size.height/2-1,size.width,28),
			MyGUI::Align::HFull|MyGUI::Align::Top,"Info","_consoleInput");
		mEdit->setTextAlign(MyGUI::Align::Left|MyGUI::Align::Bottom);

	//	MyGUI::InputManager::getInstance().addWidgetModal(mInput);

		//mEdit->setEnabledSilent(false);
		//mEdit->setEnabled(false);
	//	mEdit->setEditStatic(true);
		mEdit->setAlpha(0.9f);
		mEdit->setEditReadOnly(true);
		mEdit->setEditMultiLine(true);
		mEdit->setMaxTextLength(STDOUT_BUFFER_MAX_CAPS);
		mEdit->setVisibleHScroll(false);
		mEdit->setVisibleVScroll(true);
		/*
			非UTF8字符可能导致问题
		*/
		mEdit->setCaption( toUString(mOut) );

	//	MyGUI::InputManager::getInstance().addWidgetModal(mEdit);
		MyGUI::InputManager::getInstance().setKeyFocusWidget(mInput);
		mInput->eventKeyButtonPressed += MyGUI::newDelegate(this,&Console::_eventKeyButtonPressed);
	}
}

//关闭打开的控制台窗口
void Console::closeConsole(){
	MyGUI::Gui* gui = MyGUI::Gui::getInstancePtr();
	if( gui && mEdit ){
	//	MyGUI::InputManager::getInstance().removeWidgetModal(mInput);
		gui->destroyWidget(mEdit);
		mEdit = nullptr;
		gui->destroyWidget(mInput);
		mInput=nullptr;
		mAppendStr.clear();
	}
}

void Console::_eventKeyButtonPressed(MyGUI::Widget* _sender, MyGUI::KeyCode _key, MyGUI::Char _char)
{
	if( _key == MyGUI::KeyCode::Return )
	{
		LuaManager* pl = LuaManager::getSingletonPtr();
		if( pl )
		{
			string cmd = mInput->getCaption().asUTF8();
			if( !cmd.empty() )
			{
				mInput->setCaption(""); //清空
				pl->callEventString("eventConsole",cmd);			
			}
		}
	}
}

bool Console::isOpen()
{
	return mEdit?true:false;
}
