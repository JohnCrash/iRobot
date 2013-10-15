#include "stdheader.h"
#include "LuaManager.h"
#include "gettext.h"
#include "game.h"

SINGLETON(GettextManager)

GettextManager::GettextManager()
{
}

GettextManager::~GettextManager()
{
}

const char* GettextManager::gettext( const char* msg )
{
	TextMap::iterator it = mTexts.find(msg);
	if( it != mTexts.end() )
		return it->second.c_str();
	return msg;
}


bool GettextManager::load( const string& file )
{
	MyGUI::xml::Document doc;
	string path = Game::getSingleton().getResourcePath() + file;
	if( !doc.open( path ) )
	{
		MyGUI::IDataStream* pdata = MyGUI::DataManager::getInstance().getData(file);
		if( pdata )
			doc.open( pdata );
		MyGUI::DataManager::getInstance().freeData(pdata);
	}

	MyGUI::xml::ElementPtr root = doc.getRoot();
	if( root && root->getName()=="Translate" )
	{
		MyGUI::xml::ElementEnumerator e = root->getElementEnumerator();
		while( e.next() )
		{
			MyGUI::xml::ElementPtr p = e.current();
			if( p && p->getName()=="Msg" )
			{
				string msgid,msgstr;
				if( p->findAttribute("msgid",msgid) &&
					p->findAttribute("msgstr",msgstr) )
				{
					if( !msgstr.empty() )
						mTexts[msgid] = msgstr;
				}
			}
		}
		return true;
	}

	return false;
}

static int gettext(lua_State* L)
{
	size_t len;
	const char* s = luaL_checklstring(L,1,&len);
	const char* m = GettextManager::getSingleton().gettext( s );
	lua_pushstring(L,m);
	return 1;
}

void luaopen_gettext( lua_State* L )
{
	LuaManager::getSingleton().registerCFunction("L",gettext);
}
