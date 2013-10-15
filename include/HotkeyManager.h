#ifndef _HOTKEY_MANAGER_H_
#define _HOTKEY_MANAGER_H_

#include "Controlet.h"
/*	管理程序中的自定义热键
*/
class HotkeyManager:
	public Ogre::Singleton<HotkeyManager>,
	public Controlet,
	public MyGUI::IUnlinkWidget
{
public:
	struct hotkey
	{
		string mName; //Widget名称
		string mCaption; //显示名称
		string mTip; //长解释
		string mHotkey; //热键
		string mSHotkey; //标准热键
	};

	HotkeyManager();
	virtual ~HotkeyManager();
	void clear();
	bool load( string xml ); //装入自定义热键文件
	void _load( MyGUI::xml::ElementPtr root,const string xml );
	bool save( string xml );
	void addHotkey( string name,string caption,string tip,string hotkey );
	void removeHotkey( string name );
	void setHotkey( string name,string hotkey );
	int getHotkeyCount() const;
	hotkey getHotkeyAt( int i ) const;
protected:
	typedef vector<hotkey> HotkeyTable;
	HotkeyTable mHotkeys;

	virtual void injectKeyPress(MyGUI::KeyCode _key, MyGUI::Char _text);
	
	virtual void _linkWidget(MyGUI::Widget* _widget);
	virtual void _unlinkWidget(MyGUI::Widget* _widget);

	void setWidgetHotkey( const string name,const string key );
	//标准热键名有固定的shift alt ctrl顺序与大小写
	string getStandardHotkeyName( const string key ) const;
};

#endif