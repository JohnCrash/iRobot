#ifndef __LOG0_H__
#define __LOG0_H__

#include <MyGUI_ILogListener.h>
/* 大量使用相同的字符串会被编译器合并成一个字符串(MSVC8.0下测试过)
因此下面的宏不会产生大量的垃圾字串
*/
//用在程序不能完成预定任务，但是不影响程序的稳定性
#define WARNING_LOG( msg )\
{\
	std::stringstream ss(std::stringstream::out);\
	ss << msg;\
	if( MyGUI::LogManager::getInstancePtr() )MYGUI_LOG( Warning,ss.str() );\
}

//用在程序不能正确继续执行或者继续执行有风险的时候
#define ERROR_LOG( msg )\
{\
	std::stringstream ss(std::stringstream::out);\
	ss << msg;\
	if( MyGUI::LogManager::getInstancePtr() )MYGUI_LOG( Error,ss.str() );\
}

//程序知道怎么处理出现的问题
#define MSG( msg )\
{\
	std::cout<<msg<<"#000000"<<std::endl;\
}

//同时监听MyGUI和Ogre日志
class CoutSource:
	public MyGUI::ILogListener,
	public Ogre::LogListener
{
public:
	//MyGUI虚函数
	virtual void log(const std::string& _section, MyGUI::LogLevel _level, const struct tm* _time, const std::string& _message, const char* _file, int _line){
		if( _level==MyGUI::LogLevel::Info )
			MSG( "#0000FFMyGUI:#000000"<<_message )
		else if( _level==MyGUI::LogLevel::Warning )
			MSG( "#0000FFMyGUI:#8F4F00"<<_message<<"("<<_file<<":"<<_line<<")" )
		else
			MSG( "#0000FFMyGUI:#FF0000"<<_message<<"("<<_file<<":"<<_line<<")" )

	}
	//Ogre虚函数
#if OGRE_VERSION_MINOR<8
	virtual void messageLogged(const Ogre::String &message, Ogre::LogMessageLevel lml, bool maskDebug, const Ogre::String &logName){
#else
	virtual void messageLogged(const Ogre::String &message, Ogre::LogMessageLevel lml, bool maskDebug, const Ogre::String &logName, bool& skipThisMessage){
#endif
		MSG( "#008000Ogre:#000000"<< message )
	}
};

#endif