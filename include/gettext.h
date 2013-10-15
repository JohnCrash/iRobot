#ifndef _GETTEXT_H_
#define _GETTEXT_H_

class GettextManager:public Ogre::Singleton<GettextManager>
{
public:
	GettextManager();
	virtual ~GettextManager();
	const char* gettext( const char* msg );
	bool load( const string& file );
protected:
	typedef map<string,string> TextMap;
	TextMap mTexts;
};

#define L( msg ) GettextManager::getSingleton().gettext(msg)
#endif