#ifndef __STDHEADER_H__
#define __STDHEADER_H__
//lua
#include "lua.hpp"
//ode
#include <ode/ode.h>
//std
#include <string>
#include <map>
#include <vector>
#include <algorithm>
//boost
//#include <boost/typeof/typeof.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/any.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/asio.hpp>

//ois
#include <OIS.h>
//ogre
#include <Ogre.h>
//mygui
#include <MyGUI.h>
#include <MyGUI_OgrePlatform.h>

using namespace std;

//#if OGRE_VERSION_MINOR<8
//#define SINGLETON( T ) template<> T* Ogre::Singleton<T>::ms_Singleton = 0;
//#else
#define SINGLETON( T ) template<> T* Ogre::Singleton<T>::msSingleton = 0;
//#endif

#include "gettext.h"
#include "Log.h"

#endif