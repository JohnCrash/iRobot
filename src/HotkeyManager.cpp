#include "stdheader.h"
#include "Game.h"
#include "HotkeyManager.h"
#include "boost/algorithm/string.hpp"

SINGLETON(HotkeyManager)

HotkeyManager::HotkeyManager()
{
	/*
		枚举系统里面已经存在的Widget
	*/
	setGlobal(true);

	MyGUI::WidgetManager::getInstance().registerUnlinker(this);
}

HotkeyManager::~HotkeyManager()
{
	MyGUI::WidgetManager::getInstance().unregisterUnlinker(this);
}

void HotkeyManager::injectKeyPress(MyGUI::KeyCode _key, MyGUI::Char _text)
{
	/*
		被翻译成下面的字符串，不区分大小写
		Shift+Ctrl+Alt+X，不区分左右Shift和Ctrl,Alt
	*/
	Game* g = Game::getSingletonPtr();
	if( g )
	{
		string hotkey;

		//_key不能是Shift,Ctrl,Alt其中之一
		if( _key == MyGUI::KeyCode::Enum(MyGUI::KeyCode::RightShift) ||
			_key == MyGUI::KeyCode::Enum(MyGUI::KeyCode::LeftShift) ||
			_key == MyGUI::KeyCode::Enum(MyGUI::KeyCode::RightControl) ||
			_key == MyGUI::KeyCode::Enum(MyGUI::KeyCode::LeftControl) ||
			_key == MyGUI::KeyCode::Enum(MyGUI::KeyCode::RightAlt) ||
			_key == MyGUI::KeyCode::Enum(MyGUI::KeyCode::LeftAlt) )
			return;

		if( g->getKeyState(MyGUI::KeyCode::Enum(MyGUI::KeyCode::RightShift)) ||
			g->getKeyState(MyGUI::KeyCode::Enum(MyGUI::KeyCode::LeftShift))
			)
		{
			hotkey += "shift+";
		}
		if( g->getKeyState(MyGUI::KeyCode::Enum(MyGUI::KeyCode::RightControl)) ||
			g->getKeyState(MyGUI::KeyCode::Enum(MyGUI::KeyCode::LeftControl))
			)
		{
			hotkey += "ctrl+";
		}
		if( g->getKeyState(MyGUI::KeyCode::Enum(MyGUI::KeyCode::RightAlt)) ||
			g->getKeyState(MyGUI::KeyCode::Enum(MyGUI::KeyCode::LeftAlt))
			)
		{
			hotkey += "alt+";
		}
		//注意：正确的话，ScanCodeToText返回小写字传串
		hotkey += ScanCodeToText(_key.getValue());

		for( HotkeyTable::iterator it = mHotkeys.begin();it!=mHotkeys.end();++it )
		{
			if( it->mSHotkey == hotkey )
			{
				MyGUI::Widget* p = MyGUI::Gui::getInstance().findWidget<MyGUI::Widget>(it->mName,false);
				if( p )
					p->eventMouseButtonClick( p );
			}
		}
	}
}

string HotkeyManager::getStandardHotkeyName( const string hotkey ) const
{
	string ts,key;
	key = hotkey;
	if(!key.empty())
	{
		boost::to_lower(key);
		bool bshift,bctrl,balt;
		string k;
		bshift = bctrl = balt = false;
		/*
			考虑到热键的设置顺序可能不是shift,ctrl,alt。这里进行
			重新序列化
		*/
		list<boost::iterator_range<string::iterator> > L;
		split(L,key,boost::is_any_of("+"));
		for( BOOST_AUTO(pos,L.begin());pos!=L.end();++pos )
		{
			string str(pos->begin(),pos->end());
			if( str == "shift" )
			{
				bshift = true;
			}
			else if( str == "ctrl" )
			{
				bctrl = true;
			}
			else if( str == "alt" )
			{
				balt = true;
			}
			else
			{
				k = str;
			}
		}
		if( bshift )
			ts += "shift+";
		if( bctrl )
			ts += "ctrl+";
		if( balt )
			ts += "alt+";
		ts += k;
	}
	return ts;
}

void HotkeyManager::clear()
{
	for( HotkeyTable::iterator it = mHotkeys.begin();it!=mHotkeys.end();++it )
	{
		setWidgetHotkey( it->mName,"" );
	}
	mHotkeys.clear();
}

bool HotkeyManager::save( string xml )
{
	MyGUI::xml::Document doc;
	string path = Game::getSingleton().getResourcePath() + xml;
	MyGUI::xml::ElementPtr root = doc.createRoot("HKS");
	doc.createDeclaration();
	MyGUI::xml::ElementPtr node;

	for( HotkeyTable::iterator it = mHotkeys.begin();it!=mHotkeys.end();++it )
	{
		node = root->createChild("hotkey");
		node->addAttribute("name",it->mName);
		node->addAttribute("caption",it->mCaption);
		node->addAttribute("tip",it->mTip);
		node->addAttribute("key",it->mHotkey);
	}
	return doc.save( path );
}

void HotkeyManager::addHotkey( string name,string caption,string tip,string hotkey )
{
	HotkeyManager::hotkey hk;
	hk.mName = name;
	hk.mCaption = caption;
	hk.mTip = tip;
	hk.mHotkey = hotkey;
	hk.mSHotkey = getStandardHotkeyName(hk.mHotkey);
	mHotkeys.push_back( hk );
	setWidgetHotkey( hk.mName,hk.mHotkey );
}

void HotkeyManager::removeHotkey( string name )
{
	for( HotkeyTable::iterator it = mHotkeys.begin();it!=mHotkeys.end();++it )
	{
		if( it->mName == name )
		{
			setWidgetHotkey( it->mName,"" );
			mHotkeys.erase(it);
			return;
		}
	}
}

void HotkeyManager::setHotkey( string name,string hotkey )
{
	for( HotkeyTable::iterator it = mHotkeys.begin();it!=mHotkeys.end();++it )
	{
		if( it->mName == name )
		{
			it->mHotkey = hotkey;
			it->mSHotkey = getStandardHotkeyName(hotkey);
			setWidgetHotkey( it->mName,it->mHotkey );
			return;
		}
	}
}

int HotkeyManager::getHotkeyCount() const
{
	return mHotkeys.size();
}

HotkeyManager::hotkey HotkeyManager::getHotkeyAt( int i ) const
{
	assert(i>=0&&i<(int)mHotkeys.size());
	return mHotkeys[i];
}

/*从xml加载热键
	
*/
bool HotkeyManager::load( string xml )
{
	MyGUI::xml::Document doc;
	string path = Game::getSingleton().getResourcePath() + xml;
	if( !doc.open( path ) )
	{
		MyGUI::IDataStream* pdata = MyGUI::DataManager::getInstance().getData(xml);
		if( pdata )
			doc.open( pdata );
		MyGUI::DataManager::getInstance().freeData(pdata);
	}

	MyGUI::xml::ElementPtr root = doc.getRoot();
	if( root )
	{
		_load( root,xml );
		return true;
	}
	
	WARNING_LOG("HotkeyManager can't load "<<xml);
	return false;
}

void HotkeyManager::setWidgetHotkey( const string name,const string key )
{
	MyGUI::Gui& gui = MyGUI::Gui::getInstance();
	MyGUI::Widget* p = gui.findWidget<MyGUI::Widget>( name,false );
	if( p && (p->isType<MyGUI::Button>() || p->isType<MyGUI::MenuItem>() ) )
	{
		MyGUI::MenuItem* menuitem = p->castType<MyGUI::MenuItem>(false);
		if( menuitem )
			menuitem->setUserString( "hotkey",key );
		else
			p->setUserString( "hotkey",key );
	}
}

void HotkeyManager::_load( MyGUI::xml::ElementPtr root,const string xml )
{
	if( root == nullptr || root->getName() != "HKS" )
	{
		WARNING_LOG("bad Hotkey config file "<<xml);
		return;
	}
	MyGUI::xml::ElementEnumerator node = root->getElementEnumerator();
	while( node.next() )
	{
		if( node->getName() == "hotkey" )
		{
			hotkey hk;
			if( !node->findAttribute("name",hk.mName) )
			{
				WARNING_LOG("Hotkey config file "<<xml<<" invalid hotkey node");
				continue;
			}
			node->findAttribute("caption",hk.mCaption);
			node->findAttribute("tip",hk.mTip);
			node->findAttribute("key",hk.mHotkey);
			hk.mSHotkey = getStandardHotkeyName(hk.mHotkey);
			mHotkeys.push_back( hk );
			//设置对应Widget的User string，便于其显示出正常的热键
			setWidgetHotkey( hk.mName,hk.mHotkey );
		}
	}
}

void HotkeyManager::_unlinkWidget(MyGUI::Widget* _widget)
{
}

/*
	注意：_linkWidget被调用时，还没有设置属性和UserString
	这样addHotkey将不会成功
*/

void HotkeyManager::_linkWidget(MyGUI::Widget* _widget)
{
	if( _widget && 
		(_widget->isType<MyGUI::MenuItem>() || 
		_widget->isType<MyGUI::Button>() ) )
	{
		string name = _widget->getName();
		if( !name.empty() )
		{
			for( HotkeyTable::iterator it = mHotkeys.begin();it!=mHotkeys.end();++it )
			{
				if( it->mName == name )
				{
					MyGUI::MenuItem* menuitem = _widget->castType<MyGUI::MenuItem>(false);
					if( menuitem )
						menuitem->setUserString("hotkey",it->mHotkey );
					else
						_widget->setUserString( "hotkey",it->mHotkey );
					return;
				}
			}
		}
	}
}