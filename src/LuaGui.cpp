#include "stdheader.h"
#include "LuaManager.h"
#include "LuaExport.h"
#include "utf8.h"
#include "Game.h"

#ifdef _MSC_VER
//不让事件支持多处理
//#pragma warning(disable:4996)
#endif

//meta元表名称
#define METHOD( fun,T ) \
static int fun( lua_State* L )\
{\
	MyGUI::T* self = (MyGUI::T*)lua_cast( L,1,#T );\
	if( self == nullptr )return 0;\

#define METHOD_END( ret ) \
	return ret; }\

#define WIDGET_METHOD( fun ) METHOD( fun,Widget )

//检测ListBox ComboBox等的范围
#define assert_rang(n) \
if( (size_t)toint(n)>=self->getItemCount() || (size_t)toint(n) < 0 ){\
	string msg;\
	msg = self->getTypeName();\
	msg += " out of rang";\
	lua_errorstring(L,msg);\
	return 0;\
}

/*
	多处出现
*/
#define INSERT_ITEM_AT() \
assert_rang(2);\
s_len(3);\
if( lua_isstring(L,4) )\
{\
self->insertItemAt( toint(2),toUString(string(s,len)),string(tostring(4)) );\
}\
else\
{\
self->insertItemAt( toint(2),toUString(string(s,len)));\
}

#define ADD_ITEM() \
s_len(2);\
if( lua_isstring(L,3) )\
{\
self->addItem(toUString(string(s,len)),string(tostring(3)));\
}\
else\
{\
self->addItem(toUString(string(s,len)));\
}

#define SET_ITEM_DATA_AT() \
assert_rang(2);\
self->setItemDataAt(toint(2),string(tostring(3)));

#define GET_ITEM_DATA_AT() \
assert_rang(2);\
string* ps = self->getItemDataAt<string>(toint(2),false);\
if(ps){\
pstring(ps->c_str());\
}else{\
lua_pushnil(L);\
}

void lua_bindWidget( lua_State* L,MyGUI::Widget* p )
{
	string type("mygui.");
	if( p )
		type += p->getTypeName();
	lua_bind(L,type.c_str(),p);
}

static void TranslateWidget( MyGUI::Widget* _widget )
{
	if( !_widget )return;

	MyGUI::TextBox* tb = _widget->castType <MyGUI::TextBox>(false);
	if( tb )
	{
		MyGUI::UString msg = tb->getCaption();
		tb->setCaption(L(msg.asUTF8_c_str()));
	}
	MyGUI::EnumeratorWidgetPtr e = _widget->getEnumerator();
	while( e.next() )
	{
		MyGUI::Widget* p = e.current();
		//递归
		TranslateWidget(p);
	}
}

/*
	Global
	为MyGUI提供一个Lua界面
	LoadLayout( layoutFile ) 加载一个mygui界面文件
*/
static int lua_LoadLayout( lua_State* L )
{
	const char* s = tostring(1);

	MyGUI::VectorWidgetPtr vwp = 
		MyGUI::LayoutManager::getInstance().loadLayout(s);

	MyGUI::IntSize size = MyGUI::RenderManager::getInstance().getViewSize();
	MyGUI::IntSize newSize = size;

	for( BOOST_AUTO(i,vwp.begin());i!=vwp.end();++i )
	{
		//重新定位
		(*i)->_setAlign(size,newSize);
		//对界面进行翻译
		TranslateWidget( (*i) );
		//绑定为lua对象
		lua_bindWidget(L,(*i));
	}
	
	return vwp.size();
}

//返回ITEM_NONE
static int lua_itemNone( lua_State* L )
{
	lua_pushinteger(L,MyGUI::ITEM_NONE);
	return 1;
}

/*
	创建一个窗口原件
	lua 函数例子
	createWidget(type,skin,layer,align,name)
	createWidget( "Window","WindowCX","Overlapped","Center" )
*/
static int lua_createWidget( lua_State* L )
{
	string _type = tostring(1);
	string _skin = tostring(2);
	string _layer = tostring(3);
	MyGUI::Align _align = MyGUI::Align::parse2(tostring(4));
	string _name;
	
	if( lua_isstring(L,5) )
		_name = lua_tostring(L,5);

	MyGUI::IntCoord _coord;
	MyGUI::Widget* pret = MyGUI::Gui::getInstance().createWidgetT(
		_type,_skin,_coord,_align,_layer,_name );
	if( pret )
	{
		MyGUI::IntSize size = MyGUI::RenderManager::getInstance().getViewSize();
		MyGUI::IntSize newSize = size;
		pret->_setAlign(size,newSize);
		lua_bindWidget(L,pret);
	}
	else
	{
		lua_pushnil(L);
	}
	return 1;
}

static int lua_getWidgetFromPoint( lua_State* L )
{
	lua_bindWidget(L,
		MyGUI::LayerManager::getInstance().getWidgetFromPoint(
		toint(1),toint(2)
			)
		);
	return 1;
}

static int lua_getDefaultSkin( lua_State* L )
{
	pstring(MyGUI::SkinManager::getInstance().getDefaultSkin().c_str());
	return 1;
}

static int lua_setDefaultSkin( lua_State* L )
{
	MyGUI::SkinManager::getInstance().setDefaultSkin(tostring(1));
	return 0;
}

static int lua_loadResource( lua_State* L )
{
	MyGUI::ResourceManager& rm = MyGUI::ResourceManager::getInstance();
	string filename = tostring(1);
	if( !rm.isExist(filename) )
		rm.load(filename);
	return 0;
}

static int lua_screenSize( lua_State* L )
{
	MyGUI::IntSize size = MyGUI::RenderManager::getInstance().getViewSize();
	pint(size.width);
	pint(size.height);
	return 2;
}

static int lua_setWindowText( lua_State* L )
{
	Game::getSingleton().setWindowCaption(toUString(tostring(1)).asWStr());
	return 0;
}

static int lua_getWindowText( lua_State* L )
{
	wstring _text = Game::getSingleton().getWindowCaption();
	MyGUI::UString us(_text);
	string utf8 = us.asUTF8();
	lua_pushlstring(L,utf8.data(),utf8.size());
	return 1;
}

static int lua_getKeyState( lua_State* L )
{
	Game* g = Game::getSingletonPtr();
	if( g )
	{
		if( lua_isnumber(L,1) )
		{
			int key = toint(1);
			if( key >= 0 && key < 256 )
				return g->getKeyState(MyGUI::KeyCode::Enum(key))==base::InputManager::DOWN;
			else
				return false;
		}
	}
	return 0;
}

static int lua_setKeyFocusWidget( lua_State* L )
{
	MyGUI::Widget* widget = (MyGUI::Widget*)lua_cast(L,1,"Widget");
	if( widget )
	{
		MyGUI::InputManager::getInstance().setKeyFocusWidget(widget);
	}
	return 0;
}

static int lua_getKeyFocusWidget( lua_State* L )
{
	lua_bindWidget(L,MyGUI::InputManager::getInstance().getKeyFocusWidget());
	return 1;
}

static int lua_getMouseFocusWidget( lua_State* L )
{
	lua_bindWidget(L,MyGUI::InputManager::getInstance().getMouseFocusWidget());
	return 1;
}

static int lua_getMousePosition( lua_State* L )
{
	const MyGUI::IntPoint& pt = MyGUI::InputManager::getInstance().getMousePosition();
	pint(pt.top);
	pint(pt.left);
	return 2;
}

static int lua_getLastPressedPosition( lua_State* L )
{
	const MyGUI::IntPoint& pt = 
		MyGUI::InputManager::getInstance().getLastPressedPosition(
		MyGUI::MouseButton((MyGUI::MouseButton::Enum)toint(1))
		);
	pint(pt.top);
	pint(pt.left);
	return 2;
}

//在对话框关闭时，不用担心删除。InputManager会做相关操作
static int lua_addWidgetModal( lua_State* L )
{
	MyGUI::Widget* widget = (MyGUI::Widget*)lua_cast(L,1,"Widget");
	if( widget )
		MyGUI::InputManager::getInstance().addWidgetModal(widget);
	return 0;
}

static int lua_removeWidgetModal( lua_State* L )
{
	MyGUI::Widget* widget = (MyGUI::Widget*)lua_cast(L,1,"Widget");
	if( widget )
		MyGUI::InputManager::getInstance().removeWidgetModal(widget);
	return 0;
}

static int lua_isFocusMouse( lua_State* L )
{
	pbool(MyGUI::InputManager::getInstance().isFocusMouse());
	return 1;
}

static int lua_isFocusKey( lua_State* L )
{
	pbool(MyGUI::InputManager::getInstance().isFocusKey());
	return 1;
}

static int lua_isCaptureMouse( lua_State* L )
{
	pbool(MyGUI::InputManager::getInstance().isCaptureMouse());
	return 1;
}

static int lua_isModalAny( lua_State* L )
{
	pbool(MyGUI::InputManager::getInstance().isModalAny());
	return 1;
}

static int lua_newUString( lua_State* L );


static const struct luaL_Reg guiLibs[]=
{
	{"loadLayout",lua_LoadLayout},
	{"itemNone",lua_itemNone},//返回ITEM_NONE
	{"createWidget",lua_createWidget},
	{"getWidgetFromPoint",lua_getWidgetFromPoint},
	{"getDefaultSkin",lua_getDefaultSkin},
	{"setDefaultSkin",lua_setDefaultSkin},
	{"loadResource",lua_loadResource},
	{"newUString",lua_newUString},
	{"getScreenSize",lua_screenSize},
	{"getWindowText",lua_getWindowText},
	{"setWindowText",lua_setWindowText},
	{"getKeyState",lua_getKeyState},
	{"setKeyFocusWidget",lua_setKeyFocusWidget},
	{"getKeyFocusWidget",lua_getKeyFocusWidget},
	{"getMouseFocusWidget",lua_getMouseFocusWidget},
	{"getMousePosition",lua_getMousePosition},
	{"getLastPressedPosition",lua_getLastPressedPosition},
	{"addWidgetModal",lua_addWidgetModal},
	{"removeWidgetModal",lua_removeWidgetModal},
	{"isFocusMouse",lua_isFocusMouse},
	{"isFocusKey",lua_isFocusKey},
	{"isCaptureMouse",lua_isCaptureMouse},
	{"isModalAny",lua_isModalAny},
	{nullptr,nullptr}
};

/*
	Widget
*/
WIDGET_METHOD( lua_setVisible )
	if( lua_isboolean(L,2) )
		self->setVisible( tobool(2) );
METHOD_END( 0 )

WIDGET_METHOD( lua_getVisible )
	lua_pushboolean(L,self->getVisible());
METHOD_END( 1 )

WIDGET_METHOD( lua_setPosition )
	self->setPosition(
		luaL_checkint(L,2),
		luaL_checkint(L,3));
METHOD_END(0)

WIDGET_METHOD( lua_getPosition )
	MyGUI::IntPoint pt = self->getPosition();
	lua_pushinteger(L,pt.left);
	lua_pushinteger(L,pt.top);
METHOD_END(2)

WIDGET_METHOD( lua_setSize )
	self->setSize(
				  luaL_checkint(L,2),
				  luaL_checkint(L,3)
				  );
METHOD_END(0)

WIDGET_METHOD( lua_getSize )
	MyGUI::IntSize s = self->getSize();
	lua_pushinteger(L,s.width);
	lua_pushinteger(L,s.height);
METHOD_END(2)

WIDGET_METHOD( lua_getName )
	lua_pushstring(L,self->getName().c_str());
METHOD_END(1)

WIDGET_METHOD( lua_setAlpha )
	self->setAlpha((float)luaL_checknumber(L,2));
METHOD_END(0)

WIDGET_METHOD( lua_getAlpha )
	lua_pushnumber(L,self->getAlpha());
METHOD_END(1)

WIDGET_METHOD( lua_setAlign )
	self->setAlign( MyGUI::Align::parse2(tostring(2)));
	MyGUI::Widget* pp = self->getParent();
	if( pp )
	{
		MyGUI::IntSize size = pp->getSize();
		self->_setAlign(size,size);
	}
	else
	{
		MyGUI::IntSize size = MyGUI::RenderManager::getInstance().getViewSize();
		self->_setAlign(size,size);
	}
METHOD_END(0)

WIDGET_METHOD( lua_getAlign )
	lua_pushstring(L,self->getAlign().print2().c_str());
METHOD_END(1)

WIDGET_METHOD( lua_setColor )
	if( lua_isstring(L,2) )
	{
		self->setColour( MyGUI::Colour(lua_tostring(L,2) ));
	}
	else
	{
		self->setColour( MyGUI::Colour(
			(float)luaL_checknumber(L,2),
			(float)luaL_checknumber(L,3),
			(float)luaL_checknumber(L,4)
			) );
	}
METHOD_END(0)

WIDGET_METHOD( lua_getParent )
	MyGUI::Widget* w = self->getParent();
	if( w )
		lua_bindWidget(L,w);
	else
	{
		lua_pushnil(L);
	}
METHOD_END(1)

WIDGET_METHOD( lua_getTypeName )
	lua_pushstring(L,self->getTypeName().c_str());
METHOD_END(1)

WIDGET_METHOD( lua_child )
	MyGUI::Widget* w = self->findWidget(tostring(2));
	if( w )
	{
		lua_bindWidget(L,w);
	}
	else
	{
		lua_pushnil(L);
	}
METHOD_END(1)

WIDGET_METHOD( lua_childs )
	lua_newtable(L);
	for( size_t i = 0;i<self->getChildCount();i++ )
	{
		lua_bindWidget(L,self->getChildAt(i));
		lua_rawseti(L,-2,i+1);
	}
METHOD_END(1)

WIDGET_METHOD( lua_destroyWidget )
MyGUI::Gui::getInstance().destroyWidget(self);
METHOD_END(0)

WIDGET_METHOD( attachToWidget )
MyGUI::Widget* w = (MyGUI::Widget*)lua_cast(L,2,"Widget");
if( w )
	self->attachToWidget(w);
METHOD_END(0)

WIDGET_METHOD( detachFromWidget )
if( lua_isstring(L,2) )
	self->detachFromWidget(tostring(2));
else
	self->detachFromWidget();
METHOD_END(0)

WIDGET_METHOD( changeWidgetSkin )
self->changeWidgetSkin(tostring(2));
METHOD_END(0)

WIDGET_METHOD( setWidgetStyle )
	MyGUI::WidgetStyle ws = MyGUI::WidgetStyle::parse(tostring(2));
	if( lua_isstring(L,3) )
		self->setWidgetStyle(ws,lua_tostring(L,3));
	else
		self->setWidgetStyle(ws);
METHOD_END(0)

WIDGET_METHOD( getWidgetStyle )
	lua_pushstring( L,self->getWidgetStyle().print().c_str() );
METHOD_END(1)

WIDGET_METHOD( setProperty )
self->setProperty(tostring(2),tostring(3));
METHOD_END(0)

WIDGET_METHOD( lua_setCoord )
self->setCoord( 
			   toint(2),
			   toint(3),
			   toint(4),
			   toint(5)
			   );
METHOD_END(0)

WIDGET_METHOD( lua_setRealPosition )
self->setRealPosition(
			  (float)luaL_checknumber(L,2),
			  (float)luaL_checknumber(L,3)
			  );
METHOD_END(0)

WIDGET_METHOD( lua_setRealSize )
self->setRealSize(
			  (float)luaL_checknumber(L,2),
			  (float)luaL_checknumber(L,3)
			  );
METHOD_END(0)

WIDGET_METHOD( lua_setRealCoord )
self->setRealCoord(
			  (float)luaL_checknumber(L,2),
			  (float)luaL_checknumber(L,3),
			  (float)luaL_checknumber(L,4),
			  (float)luaL_checknumber(L,5)
			  );
METHOD_END(0)

WIDGET_METHOD( setEnabled )
self->setEnabled(tobool(2));
METHOD_END(0)

WIDGET_METHOD( setEnabledSilent )
self->setEnabledSilent(tobool(2));
METHOD_END(0)

WIDGET_METHOD( getEnabled )
lua_pushboolean(L,self->getEnabled());
METHOD_END(1)

WIDGET_METHOD( getInheritedEnabled )
lua_pushboolean(L,self->getInheritedEnabled());
METHOD_END(1)

WIDGET_METHOD( getClientCoord )
MyGUI::IntCoord ic = self->getClientCoord();
lua_pushinteger(L,ic.left);
lua_pushinteger(L,ic.top);
lua_pushinteger(L,ic.width);
lua_pushinteger(L,ic.height);
METHOD_END(4)

WIDGET_METHOD( getClientWidget )
MyGUI::Widget* w = self->getClientWidget();
if( w )
	lua_bindWidget(L,w);
else
	lua_pushnil(L);
METHOD_END(1)

WIDGET_METHOD( isRootWidget )
lua_pushboolean(L,self->isRootWidget());
METHOD_END(1)

WIDGET_METHOD( getParentSize )
MyGUI::IntSize is = self->getParentSize();
lua_pushinteger(L,is.width);
lua_pushinteger(L,is.height);
METHOD_END(2)

//创建一个子窗口
WIDGET_METHOD( createChild )
string _type(tostring(2));
string _skin(tostring(3));
string _align(tostring(4));
string _name(tostring(5));
lua_bindWidget(L,self->createWidgetRealT(_type,_skin,
						MyGUI::FloatCoord(),
						MyGUI::Align::parse2(_align),
						_name));
METHOD_END(1)

/*
	Widget Event
*/
static void widgetClick( MyGUI::Widget* _sender )
{
	LuaManager::getSingleton().callBindEventVoid(_sender,"eventMouseButtonClick");
}

static void widgetPressed( MyGUI::Widget* _sender,int left,int top,MyGUI::MouseButton _id )
{
	LuaManager::getSingleton().callBindEvent3Int(
		_sender,"eventMouseButtonPressed",left,top,_id.getValue());
}

static void widgetReleased(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id)
{
	LuaManager::getSingleton().callBindEvent3Int(
		_sender,"eventMouseButtonReleased",_left,_top,_id.getValue());
}

static void widgetMouseMove(MyGUI::Widget* _sender,int left,int top)
{
	LuaManager::getSingleton().callBindEvent2Int(
		_sender,"eventMouseMove",left,top);
}

static void widgetLostFocus(MyGUI::Widget* _sender,MyGUI::Widget* _new)
{
	LuaManager::getSingleton().bindWidget(_new);
	LuaManager::getSingleton().callBindEvent1Obj(_sender,"eventMouseLostFocus",_new);
}

static void widgetSetFocus(MyGUI::Widget* _sender, MyGUI::Widget* _old)
{
	LuaManager::getSingleton().bindWidget(_old);
	LuaManager::getSingleton().callBindEvent1Obj(_sender,"eventMouseSetFocus",_old);
}

static void widgetMouseDrag(MyGUI::Widget* _sender,
							int _left, int _top, MyGUI::MouseButton _id)
{
	LuaManager::getSingleton().callBindEvent3Int(
		_sender,"eventMouseDrag",_left,_top,_id.getValue());
}

static void widgetMouseWheel(MyGUI::Widget* _sender, int _rel)
{
	LuaManager::getSingleton().callBindEvent1Int(
		_sender,"eventMouseWheel",_rel);
}

static void widgetDoubleClick(MyGUI::Widget* _sender )
{
	LuaManager::getSingleton().callBindEventVoid(_sender,"eventMouseButtonDoubleClick");
}

static void widgetToolTip(MyGUI::Widget* _sender, const MyGUI::ToolTipInfo& _info)
{
	LuaManager::getSingleton().callBindEventVoid(_sender,"eventToolTip");
}

static void widgetRootKeyChangeFocus(MyGUI::Widget* _sender, bool _focus)
{
	LuaManager::getSingleton().callBindEvent1Bool(_sender,"eventRootKeyChangeFocus",_focus);
}

static void widgetRootMouseChangeFocus(MyGUI::Widget* _sender, bool _focus)
{
	LuaManager::getSingleton().callBindEvent1Bool(_sender,"eventRootMouseChangeFocus",_focus);
}

static void widgetKeyPressed(MyGUI::Widget* _sender, MyGUI::KeyCode _key, MyGUI::Char _char)
{
	LuaManager::getSingleton().callBindEvent2Int(_sender,"eventKeyButtonPressed",_key.getValue(),_char);
}

static void widgetKeyReleased(MyGUI::Widget* _sender, MyGUI::KeyCode _key)
{
	LuaManager::getSingleton().callBindEvent1Int(_sender,"eventKeyButtonReleased",_key.getValue());
}

static void widgetKeySetFocus(MyGUI::Widget* _sender, MyGUI::Widget* _old)
{
	LuaManager::getSingleton().bindWidget(_old);
	LuaManager::getSingleton().callBindEvent1Obj(_sender,"eventKeySetFocus",_old);
}

static void widgetKeyLostFocus(MyGUI::Widget* _sender, MyGUI::Widget* _new)
{
	LuaManager::getSingleton().bindWidget(_new);
	LuaManager::getSingleton().callBindEvent1Obj(_sender,"eventKeyLostFocus",_new);
}

static void eventChangeProperty(MyGUI::Widget* _sender,const std::string& _key, const std::string& _value)
{
	LuaManager::getSingleton().callBindEvent2String(_sender,"eventChangeProperty",_key.c_str(),_value.c_str());
}

/*
	Window
*/
static void windowButtonPressed(MyGUI::Window* _sender, const std::string& _name)
{
	LuaManager::getSingleton().callBindEvent1String(_sender,"eventWindowButtonPressed",_name.c_str());
}

static void windowChangeCoord(MyGUI::Window* _sender)
{
	LuaManager::getSingleton().callBindEventVoid(_sender,"eventWindowChangeCoord");
}

/*
	EditBox
*/
static void eventEditSelectAccept(MyGUI::EditBox* _sender)
{
	LuaManager::getSingleton().callBindEventVoid(_sender,"eventEditSelectAccept");
}

static void eventEditTextChange(MyGUI::EditBox* _sender)
{
	LuaManager::getSingleton().callBindEventVoid(_sender,"eventEditTextChange");
}

/*
	ListBox
*/
static void eventListSelectAccept(MyGUI::ListBox* _sender, size_t _index)
{
	LuaManager::getSingleton().callBindEvent1Int(_sender,"eventListSelectAccept",_index);
}

static void eventListChangePosition(MyGUI::ListBox* _sender, size_t _index)
{
	LuaManager::getSingleton().callBindEvent1Int(_sender,"eventListChangePosition",_index);
}

static void eventListMouseItemActivate(MyGUI::ListBox* _sender, size_t _index)
{
	LuaManager::getSingleton().callBindEvent1Int(_sender,"eventListMouseItemActivate",_index);
}

static void eventListMouseItemFocus(MyGUI::ListBox* _sender, size_t _index)
{
	LuaManager::getSingleton().callBindEvent1Int(_sender,"eventListMouseItemFocus",_index);
}

static void eventListChangeScroll(MyGUI::ListBox* _sender, size_t _position)
{
	LuaManager::getSingleton().callBindEvent1Int(_sender,"eventListChangeScroll",_position);
}

/*
	ComboBox
*/
static void eventComboAccept(MyGUI::ComboBox* _sender, size_t _index)
{
	LuaManager::getSingleton().callBindEvent1Int(_sender,"eventComboAccept",_index);
}

static void eventComboChangePosition(MyGUI::ComboBox* _sender, size_t _index)
{
	LuaManager::getSingleton().callBindEvent1Int(_sender,"eventComboChangePosition",_index);
}

/*
	ScrollBar
*/
static void eventScrollChangePosition(MyGUI::ScrollBar* _sender, size_t _position)
{
	LuaManager::getSingleton().callBindEvent1Int(_sender,"eventScrollChangePosition",_position);
}

/*
	TabControl
*/
static void eventTabChangeSelect(MyGUI::TabControl* _sender, size_t _index)
{
	LuaManager::getSingleton().callBindEvent1Int(_sender,"eventTabChangeSelect",_index);
}

/*
	DDContainer
*/
static void eventStartDrag(MyGUI::DDContainer* _sender, const MyGUI::DDItemInfo& _info, bool& _result)
{
	_result = LuaManager::getSingleton().callBindEventDDContainer
		(_sender,"eventStartDrag",_info.sender,_info.sender_index,_info.receiver,_info.receiver_index );
}

static void eventRequestDrop(MyGUI::DDContainer* _sender, const MyGUI::DDItemInfo& _info, bool& _result)
{
	_result = LuaManager::getSingleton().callBindEventDDContainer
		(_sender,"eventRequestDrop",_info.sender,_info.sender_index,_info.receiver,_info.receiver_index );
}

static void eventDropResult(MyGUI::DDContainer* _sender, const MyGUI::DDItemInfo& _info, bool _result)
{
	_result = LuaManager::getSingleton().callBindEventDDContainer
		(_sender,"eventDropResult",_info.sender,_info.sender_index,_info.receiver,_info.receiver_index );
}

static void eventChangeDDState(MyGUI::DDContainer* _sender, MyGUI::DDItemState _state)
{
	char* s;
	if( _state == MyGUI::DDItemState::Accept )
		s = "Accept";
	else if( _state == MyGUI::DDItemState::End )
		s = "End";
	else if( _state == MyGUI::DDItemState::Miss )
		s = "Miss";
	else if( _state == MyGUI::DDItemState::None )
		s = "None";
	else if( _state == MyGUI::DDItemState::Refuse )
		s = "Refuse";
	else if( _state == MyGUI::DDItemState::Start )
		s = "Start";
	LuaManager::getSingleton().callBindEvent1String(_sender,"eventChangeDDState",s);
}

static void requestDragWidgetInfo(MyGUI::DDContainer* _sender, MyGUI::Widget*& _item, MyGUI::IntCoord& _dimension)
{
	/*
		BUG 没有实现
	*/
	WARNING_LOG("DragWidgetInfo not implemen yet");
}

/*
	ItemBox
*/
static void requestCreateWidgetItem(MyGUI::ItemBox* _sender, MyGUI::Widget* _item)
{
	LuaManager::getSingleton().bindWidget(_item);
	LuaManager::getSingleton().callBindEvent1Obj(_sender,"requestCreateWidgetItem",_item);
}

static void requestCoordItem(MyGUI::ItemBox* _sender, MyGUI::IntCoord& _coord, bool _drag)
{
	LuaManager::getSingleton().callBindEvent4Int1Bool(_sender,"requestCoordItem",
		_coord.left,_coord.top,_coord.width,_coord.height,_drag);
}

static void requestDrawItem(MyGUI::ItemBox* _sender, MyGUI::Widget* _item, const MyGUI::IBDrawItemInfo& _info)
{
	LuaManager::getSingleton().callBindEvent1Obj1IBDrawItemInfo(_sender,"requestDrawItem",_item,_info);
}

static void eventSelectItemAccept(MyGUI::ItemBox* _sender, size_t _index)
{
	LuaManager::getSingleton().callBindEvent1Int(_sender,"eventSelectItemAccept",_index);
}

static void eventChangeItemPosition(MyGUI::ItemBox* _sender, size_t _index)
{
	LuaManager::getSingleton().callBindEvent1Int(_sender,"eventChangeItemPosition",_index);
}

static void eventMouseItemActivate(MyGUI::ItemBox* _sender, size_t _index)
{
	LuaManager::getSingleton().callBindEvent1Int(_sender,"eventMouseItemActivate",_index);
}

static void eventNotifyItem(MyGUI::ItemBox* _sender, const MyGUI::IBNotifyItemData& _info)
{
	LuaManager::getSingleton().callBindEvent1IBDrawItemInfo(_sender,"eventNotifyItem",_info);
}

/*
	MenuControl
*/
static void eventMenuCtrlAccept(MyGUI::MenuControl* _sender, MyGUI::MenuItem* _item)
{
	LuaManager::getSingleton().bindWidget(_item);
	LuaManager::getSingleton().callBindEvent1Obj(_sender,"eventMenuCtrlAccept",_item);
}

static void eventMenuCtrlClose(MyGUI::MenuControl* _sender)
{
	LuaManager::getSingleton().callBindEventVoid(_sender,"eventMenuCtrlClose");
}

static void lua_errorevent( lua_State* L,const string& e )
{
	string msg("Don't Know event name ");
	msg += e;
	lua_errorstring(L,msg);
}

#define EVENT_SET3( cls,ev,func ) lua_setfield(L,-2,#ev);\
	lua_pop(L,1);\
	((MyGUI::cls*)self)->ev=MyGUI::newDelegate(func);

#define EVENT_SET2( cls,ev,func ) lua_setfield(L,-2,#ev);\
	lua_pop(L,1);\
	((MyGUI::cls*)self)->ev-=MyGUI::newDelegate(func);\
	((MyGUI::cls*)self)->ev+=MyGUI::newDelegate(func);

#define EVENT_SET( ev,func ) EVENT_SET2(Widget,ev,func)

/*
	Widget事件注册
*/
WIDGET_METHOD( lua_setScript )
	LuaBind* plb = (LuaBind*)lua_touserdata(L,1);
	const char * e = tostring(2);

	if( plb->refUTable==LUA_REFNIL )
	{
		//为该绑定对象创建一个附加表
		lua_newtable(L);
		plb->refUTable = luaL_ref(L,LUA_REGISTRYINDEX);
	}

	lua_rawgeti(L,LUA_REGISTRYINDEX,plb->refUTable);
	lua_pushvalue(L,3);

	if( strcmp(e,"eventMouseButtonClick") == 0 )
	{
		EVENT_SET( eventMouseButtonClick,widgetClick );
	}
	else if( strcmp(e,"eventMouseButtonPressed") == 0 )
	{
		EVENT_SET( eventMouseButtonPressed,widgetPressed );
	}
	else if( strcmp(e,"eventMouseMove")==0 )
	{
		EVENT_SET( eventMouseMove,widgetMouseMove );
	}
	else if( strcmp(e,"eventMouseLostFocus")==0 )
	{
		EVENT_SET( eventMouseLostFocus,widgetLostFocus );
	}
	else if( strcmp(e,"eventMouseSetFocus")==0 )
	{
		EVENT_SET( eventMouseSetFocus,widgetSetFocus );
	}
	else if( strcmp(e,"eventMouseDrag")==0 )
	{
		EVENT_SET( eventMouseDrag,widgetMouseDrag );
	}
	else if( strcmp(e,"eventMouseWheel")==0 )
	{
		EVENT_SET( eventMouseWheel,widgetMouseWheel );
	}
	else if( strcmp(e,"eventMouseButtonReleased")==0 )
	{
		EVENT_SET( eventMouseButtonReleased,widgetReleased );
	}
	else if( strcmp(e,"eventMouseButtonDoubleClick")==0 )
	{
		EVENT_SET( eventMouseButtonDoubleClick,widgetDoubleClick );
	}
	else if( strcmp(e,"eventKeySetFocus")==0 )
	{
		EVENT_SET( eventKeySetFocus,widgetKeySetFocus );
	}
	else if( strcmp(e,"eventKeyLostFocus")==0 )
	{
		EVENT_SET( eventKeyLostFocus,widgetKeyLostFocus );
	}
	else if( strcmp(e,"eventKeyButtonPressed")==0 )
	{
		EVENT_SET( eventKeyButtonPressed,widgetKeyPressed );
	}
	else if( strcmp(e,"eventKeyButtonReleased")==0 )
	{
		EVENT_SET( eventKeyButtonReleased,widgetKeyReleased );
	}
	else if( strcmp(e,"eventRootMouseChangeFocus")==0 )
	{
		EVENT_SET( eventRootMouseChangeFocus,widgetRootMouseChangeFocus );
	}
	else if( strcmp(e,"eventRootKeyChangeFocus")==0 )
	{
		EVENT_SET( eventRootKeyChangeFocus,widgetRootKeyChangeFocus );
	}
	else if( strcmp(e,"eventToolTip")==0 )
	{
		EVENT_SET( eventToolTip,widgetToolTip );
	}
	else if( strcmp(e,"eventChangeProperty")==0 )
	{
		EVENT_SET( eventChangeProperty,eventChangeProperty );
	}
	/*
		Window特有事件
	*/
	else if( self->getTypeName()=="Window" && 
		(strcmp(e,"eventWindowButtonPressed")==0||strcmp(e,"eventWindowChangeCoord")==0) )
	{
		if( strcmp(e,"eventWindowButtonPressed") == 0 )
		{
			EVENT_SET2( Window,eventWindowButtonPressed,windowButtonPressed );
		}
		else if(  strcmp(e,"eventWindowChangeCoord") == 0 )
		{
			EVENT_SET2( Window,eventWindowChangeCoord,windowChangeCoord );
		}
	}
	/*
		EditBox事件
	*/
	else if( self->getTypeName()=="EditBox" &&
		(strcmp(e,"eventEditSelectAccept")==0||strcmp(e,"eventEditTextChange")==0))
	{
		if( strcmp(e,"eventEditSelectAccept") == 0 )
		{
			EVENT_SET2( EditBox,eventEditSelectAccept,eventEditSelectAccept );
		}
		else if(  strcmp(e,"eventEditTextChange") == 0 )
		{
			EVENT_SET2( EditBox,eventEditTextChange,eventEditTextChange );
		}
	}
	/*
		ListBox事件
	*/
	else if( self->getTypeName()=="ListBox" &&
		(strcmp(e,"eventListSelectAccept")==0 || strcmp(e,"eventListChangePosition")==0 ||
		 strcmp(e,"eventListMouseItemActivate")==0 || strcmp(e,"eventListMouseItemFocus")==0 ||
		 strcmp(e,"eventListChangeScroll")==0)
		)
	{
		if( strcmp(e,"eventListSelectAccept") == 0 )
		{
			EVENT_SET2( ListBox,eventListSelectAccept,eventListSelectAccept );
		}
		else if( strcmp(e,"eventListChangePosition") == 0 )
		{
			EVENT_SET2( ListBox,eventListChangePosition,eventListChangePosition );
		}
		else if( strcmp(e,"eventListMouseItemActivate") == 0 )
		{
			EVENT_SET2( ListBox,eventListMouseItemActivate,eventListMouseItemActivate );
		}
		else if( strcmp(e,"eventListMouseItemFocus") == 0 )
		{
			EVENT_SET2( ListBox,eventListMouseItemFocus,eventListMouseItemFocus );
		}
		else if( strcmp(e,"eventListChangeScroll") == 0 )
		{
			EVENT_SET2( ListBox,eventListChangeScroll,eventListChangeScroll );
		}
	}
	/*
		ComboBox
	*/
	else if( self->getTypeName()=="ComboBox" &&
		(strcmp(e,"eventComboAccept")==0||strcmp(e,"eventComboChangePosition")==0))
	{
		if( strcmp(e,"eventComboAccept") == 0 )
		{
			EVENT_SET2( ComboBox,eventComboAccept,eventComboAccept );
		}
		else if( strcmp(e,"eventComboChangePosition") == 0 )
		{
			EVENT_SET2( ComboBox,eventComboChangePosition,eventComboChangePosition );
		}
	}
	/*
		ScrollBar
	*/
	else if( self->getTypeName()=="ScrollBar" && strcmp(e,"eventScrollChangePosition")==0 )
	{
		if( strcmp(e,"eventScrollChangePosition") == 0 )
		{
			EVENT_SET2( ScrollBar,eventScrollChangePosition,eventScrollChangePosition );
		}
	}
	/*
		TabControl
	*/
	else if( self->getTypeName()=="TabControl" || strcmp(e,"eventTabChangeSelect")==0  )
	{
		if( strcmp(e,"eventTabChangeSelect") == 0 )
		{
			EVENT_SET2( TabControl,eventTabChangeSelect,eventTabChangeSelect );
		}
	}
	/*
		DDContainer
	*/
	else if( self->getTypeName()=="DDContainer"  )
	{
		if( strcmp(e,"eventStartDrag") == 0 )
		{
			EVENT_SET2( DDContainer,eventStartDrag,eventStartDrag );
		}
		else if( strcmp(e,"eventRequestDrop") == 0 )
		{
			EVENT_SET2( DDContainer, eventRequestDrop,eventRequestDrop );
		}
		else if( strcmp(e,"eventDropResult") == 0 )
		{
			EVENT_SET2( DDContainer, eventDropResult,eventDropResult );
		}
		else if( strcmp(e,"eventChangeDDState") == 0 )
		{
			EVENT_SET2( DDContainer, eventChangeDDState,eventChangeDDState );
		}
		else if( strcmp(e,"requestDragWidgetInfo") == 0 )
		{
			EVENT_SET3( DDContainer, requestDragWidgetInfo,requestDragWidgetInfo );
		}
		else
		{
			lua_pop(L,2);
			lua_errorevent(L,e);
		}
	}
	/*
		ItemBox
	*/
	else if( self->getTypeName()=="ItemBox"  )
	{
		if( strcmp(e,"requestCreateWidgetItem") == 0 )
		{
			EVENT_SET3( ItemBox, requestCreateWidgetItem,requestCreateWidgetItem );
		}
		else if( strcmp(e,"requestCoordItem") == 0 )
		{
			EVENT_SET3( ItemBox, requestCoordItem,requestCoordItem );
		}
		else if( strcmp(e,"requestDrawItem") == 0 )
		{
			EVENT_SET3( ItemBox, requestDrawItem,requestDrawItem );
		}
		else if( strcmp(e,"eventSelectItemAccept") == 0 )
		{
			EVENT_SET2( ItemBox, eventSelectItemAccept,eventSelectItemAccept );
		}
		else if( strcmp(e,"eventChangeItemPosition") == 0 )
		{
			EVENT_SET2( ItemBox, eventChangeItemPosition,eventChangeItemPosition );
		}
		else if( strcmp(e,"eventMouseItemActivate") == 0 )
		{
			EVENT_SET2( ItemBox, eventMouseItemActivate,eventMouseItemActivate );
		}
		else if( strcmp(e,"eventNotifyItem") == 0 )
		{
			EVENT_SET2( ItemBox, eventNotifyItem,eventNotifyItem );
		}
		else
		{
			lua_pop(L,2);
			lua_errorevent(L,e);
		}
	}
	/*
		MenuControl
	*/
	else if( self->getTypeName()=="MenuControl" || self->getTypeName()=="MenuBar" )
	{
		if( strcmp(e,"eventMenuCtrlAccept") == 0 )
		{
			EVENT_SET2( MenuControl, eventMenuCtrlAccept,eventMenuCtrlAccept );
		}
		else if( strcmp(e,"eventMenuCtrlClose") == 0 )
		{
			EVENT_SET2( MenuControl, eventMenuCtrlClose,eventMenuCtrlClose );
		}
		else
		{
			lua_pop(L,2);
			lua_errorevent(L,e);
		}
	}
	else
	{
		lua_pop(L,2);
		lua_errorevent(L,e);
	}
METHOD_END(0)

static const struct luaL_Reg widgetMethod[]=
{
	{"setVisible",lua_setVisible},
	{"getVisible",lua_getVisible},
	{"setPosition",lua_setPosition},
	{"getPosition",lua_getPosition},
	{"setSize",lua_setSize},
	{"getSize",lua_getSize},
	{"setCoord",lua_setCoord},
	{"setRealPosition",lua_setRealPosition},
	{"setRealSize",lua_setRealSize},
	{"setRealCoord",lua_setRealCoord},
	{"getName",lua_getName},
	{"setAlpha",lua_setAlpha},
	{"getAlpha",lua_getAlpha},
	{"setAlign",lua_setAlign},
	{"getAlign",lua_getAlign},
	{"setColor",lua_setColor},
	{"getParent",lua_getParent},
	{"setEnabled",setEnabled},
	{"setEnabledSilent",setEnabledSilent},
	{"getEnabled",getEnabled},
	{"getInheritedEnabled",getInheritedEnabled},
	{"getClientCoord",getClientCoord},
	{"getClientWidget",getClientWidget},
	{"isRootWidget",isRootWidget},
	{"getParentSize",getParentSize},
	{"child",lua_child},
	{"childs",lua_childs},
	{"setScript",lua_setScript},
	{"getTypeName",lua_getTypeName},
	{"destroy",lua_destroyWidget},
	{"attachToWidget",attachToWidget},
	{"detachFromWidget",detachFromWidget},
	{"changeWidgetSkin",changeWidgetSkin},
	{"setWidgetStyle",setWidgetStyle},
	{"getWidgetStyle",getWidgetStyle},
	{"setProperty",setProperty},
	{"createChild",createChild},
	{nullptr,nullptr}
};

/*
	TextBox
*/
#define TEXTBOX_METHOD( fun ) METHOD( fun,TextBox )

TEXTBOX_METHOD( lua_setCaption )
	size_t len;
	const char* s = luaL_checklstring(L,2,&len);
	self->setCaption(toUString(string(s,len)));
METHOD_END(0)

TEXTBOX_METHOD( lua_getCaption )
	const string& s = self->getCaption().asUTF8();
	lua_pushlstring( L,s.data(),s.size() );
METHOD_END(1)

TEXTBOX_METHOD( lua_getTextRegion )
MyGUI::IntCoord ic = self->getTextRegion();
lua_pushinteger(L,ic.left);
lua_pushinteger(L,ic.top);
lua_pushinteger(L,ic.width);
lua_pushinteger(L,ic.height);
METHOD_END(4)

TEXTBOX_METHOD( lua_getTextSize )
MyGUI::IntSize is = self->getTextSize();
lua_pushinteger(L,is.width);
lua_pushinteger(L,is.height);
METHOD_END(2)

TEXTBOX_METHOD( lua_setFontName )
self->setFontName(tostring(2));
METHOD_END(0)

TEXTBOX_METHOD( lua_getFontName )
lua_pushstring(L,self->getFontName().c_str());
METHOD_END(1)

TEXTBOX_METHOD( lua_setFontHeight )
self->setFontHeight(toint(2));
METHOD_END(0)

TEXTBOX_METHOD( lua_getFontHeight )
lua_pushinteger(L,self->getFontHeight());
METHOD_END(1)

TEXTBOX_METHOD( lua_setTextAlign )
MyGUI::Align _align = MyGUI::Align::parse2(tostring(2));
self->setTextAlign( _align );
METHOD_END(0)

TEXTBOX_METHOD( lua_getTextAlign )
lua_pushstring(L,self->getTextAlign().print().c_str());
METHOD_END(1)

TEXTBOX_METHOD( lua_setTextColor )
double r = luaL_checknumber(L,2);
double g = luaL_checknumber(L,3);
double b = luaL_checknumber(L,4);
double a = luaL_checknumber(L,5);
self->setTextColour(MyGUI::Colour((float)r,(float)g,(float)b,(float)a));
METHOD_END(0)

TEXTBOX_METHOD( lua_getTextColor )
const MyGUI::Colour& c = self->getTextColour();
lua_pushnumber(L,c.red);
lua_pushnumber(L,c.green);
lua_pushnumber(L,c.blue);
lua_pushnumber(L,c.alpha);
METHOD_END(4)

TEXTBOX_METHOD( lua_setTextShadow )
self->setTextShadow(tobool(2));
METHOD_END(0)

TEXTBOX_METHOD( lua_getTextShadow )
lua_pushboolean(L,self->getTextShadow());
METHOD_END(1)

static const struct luaL_Reg textboxMethod[]=
{
	{"setCaption",lua_setCaption},
	{"getCaption",lua_getCaption},
	{"getTextRegion",lua_getTextRegion},
	{"getTextSize",lua_getTextSize},
	{"setFontName",lua_setFontName},
	{"getFontName",lua_getFontName},
	{"setFontHeight",lua_setFontHeight},
	{"getFontHeight",lua_getFontHeight},
	{"setTextAlign",lua_setTextAlign},
	{"getTextAlign",lua_getTextAlign},
	{"setTextColor",lua_setTextColor},
	{"getTextColor",lua_getTextColor},
	{"setTextShadow",lua_setTextShadow},
	{"getTextShadow",lua_getTextShadow},
	{nullptr,nullptr}
};

/*
	Button
*/
#define BUTTON_METHOD(fun) METHOD( fun,Button)

BUTTON_METHOD(setStateSelected)
self->setStateSelected(tobool(2));
METHOD_END(0)

BUTTON_METHOD(getStateSelected)
lua_pushboolean(L,self->getStateSelected());
METHOD_END(1)

BUTTON_METHOD(setModeImage)
self->setModeImage(tobool(2));
METHOD_END(0)

BUTTON_METHOD(getModeImage)
lua_pushboolean(L,self->getModeImage());
METHOD_END(1)

BUTTON_METHOD(setImageResource)
self->setImageResource(tostring(2));
METHOD_END(0)

BUTTON_METHOD(setImageGroup)
self->setImageGroup(tostring(2));
METHOD_END(0)

BUTTON_METHOD(setImageName)
self->setImageName(tostring(2));
METHOD_END(0)

static const struct luaL_Reg buttonMethod[]=
{
	{"setStateSelected",setStateSelected},
	{"getStateSelected",getStateSelected},
	{"setModeImage",setModeImage},
	{"getModeImage",getModeImage},
	{"setImageResource",setImageResource},
	{"setImageGroup",setImageGroup},
	{"setImageName",setImageName},
	{nullptr,nullptr}
};

/*
	EditBox
*/
#define EDITBOX_METHOD(fun) METHOD( fun,EditBox)

EDITBOX_METHOD(getTextSelectionStart)
lua_pushinteger(L,self->getTextSelectionStart());
METHOD_END(1)

EDITBOX_METHOD(getTextSelectionEnd)
lua_pushinteger(L,self->getTextSelectionEnd());
METHOD_END(1)

EDITBOX_METHOD(getTextSelectionLength)
lua_pushinteger(L,self->getTextSelectionLength());
METHOD_END(0)

EDITBOX_METHOD(getTextInterval)
	const string& s = 
	self->getTextInterval(toint(2),toint(3)).asUTF8();
lua_pushlstring(L,s.data(),s.size());
METHOD_END(1)

EDITBOX_METHOD(setTextIntervalColour)
self->setTextIntervalColour(
							toint(2),
							toint(3),
							MyGUI::Colour(
								(float)luaL_checknumber(L,4),
								(float)luaL_checknumber(L,5),
								(float)luaL_checknumber(L,6),
								(float)luaL_checknumber(L,7)
							)
							);
METHOD_END(0)

EDITBOX_METHOD(setTextSelection)
self->setTextSelection(
					   toint(2),
					   toint(3)
					   );
METHOD_END(0)

EDITBOX_METHOD(deleteTextSelection)
self->deleteTextSelection();
METHOD_END(0)

EDITBOX_METHOD(getTextSelection)
string s = self->getTextSelection().asUTF8();
lua_pushlstring(L,s.data(),s.size());
METHOD_END(1)

EDITBOX_METHOD(isTextSelection)
lua_pushboolean(L,self->isTextSelection());
METHOD_END(0)

EDITBOX_METHOD(setTextSelectionColour)
self->setTextSelectionColour(
							MyGUI::Colour(
								(float)luaL_checknumber(L,2),
								(float)luaL_checknumber(L,3),
								(float)luaL_checknumber(L,4),
								(float)luaL_checknumber(L,5)
							)
						);
METHOD_END(0)

EDITBOX_METHOD(setTextCursor)
self->setTextCursor(toint(2));
METHOD_END(0)

EDITBOX_METHOD(getTextCursor)
lua_pushinteger(L,self->getTextCursor());
METHOD_END(1)

EDITBOX_METHOD(setOnlyText)
size_t len;
const char* s = luaL_checklstring(L,2,&len);
self->setOnlyText( toUString(string(s,len)) );
METHOD_END(0)

EDITBOX_METHOD(getOnlyText)
string s = self->getOnlyText().asUTF8();
lua_pushlstring(L,s.data(),s.size());
METHOD_END(1)

EDITBOX_METHOD(getTextLength)
lua_pushinteger(L,self->getTextLength());
METHOD_END(1)

EDITBOX_METHOD(setOverflowToTheLeft)
self->setOverflowToTheLeft(tobool(2));
METHOD_END(0)

EDITBOX_METHOD(getOverflowToTheLeft)
lua_pushboolean(L,self->getOverflowToTheLeft());
METHOD_END(1)

EDITBOX_METHOD(setMaxTextLength)
self->setMaxTextLength(toint(2));
METHOD_END(0)

EDITBOX_METHOD(getMaxTextLength)
lua_pushinteger(L,self->getMaxTextLength());
METHOD_END(1)

EDITBOX_METHOD(insertText)
size_t len;
const char* s = luaL_checklstring(L,2,&len);
if( lua_isnumber(L,3) )
	self->insertText( toUString(string(s,len)),toint(3));
else
	self->insertText( toUString(string(s,len)) );
METHOD_END(0)

EDITBOX_METHOD(addText)
size_t len;
const char* s = luaL_checklstring(L,2,&len);
self->addText(toUString(string(s,len)));
METHOD_END(0)

EDITBOX_METHOD(eraseText)
self->eraseText(toint(2),toint(3));
METHOD_END(0)

EDITBOX_METHOD(setEditReadOnly)
self->setEditReadOnly(tobool(2));
METHOD_END(0)

EDITBOX_METHOD(getEditReadOnly)
lua_pushboolean(L,self->getEditReadOnly());
METHOD_END(1)

EDITBOX_METHOD(setEditPassword)
self->setEditPassword(tobool(2));
METHOD_END(0)

EDITBOX_METHOD(getEditPassword)
lua_pushboolean(L,self->getEditPassword());
METHOD_END(1)

EDITBOX_METHOD(setEditMultiLine)
self->setEditMultiLine(tobool(2));
METHOD_END(0)

EDITBOX_METHOD(getEditMultiLine)
lua_pushboolean(L,self->getEditMultiLine());
METHOD_END(1)

EDITBOX_METHOD(setEditStatic)
self->setEditStatic(tobool(2));
METHOD_END(0)

EDITBOX_METHOD(getEditStatic)
lua_pushboolean(L,self->getEditStatic());
METHOD_END(1)

EDITBOX_METHOD(setPasswordChar)
self->setPasswordChar((MyGUI::Char)toint(2));
METHOD_END(0)

EDITBOX_METHOD(getPasswordChar)
lua_pushinteger(L,(int)self->getPasswordChar());
METHOD_END(1)

EDITBOX_METHOD(setEditWordWrap)
self->setEditWordWrap(tobool(2));
METHOD_END(0)

EDITBOX_METHOD(getEditWordWrap)
lua_pushboolean(L,self->getEditWordWrap());
METHOD_END(1)

EDITBOX_METHOD(setTabPrinting)
self->setTabPrinting(tobool(2));
METHOD_END(0)

EDITBOX_METHOD(getTabPrinting)
lua_pushboolean(L,self->getTabPrinting());
METHOD_END(0)

EDITBOX_METHOD(getInvertSelected)
lua_pushboolean(L,self->getInvertSelected());
METHOD_END(1)

EDITBOX_METHOD(setInvertSelected)
self->setInvertSelected(tobool(2));
METHOD_END(0)

EDITBOX_METHOD(setVisibleVScroll)
self->setVisibleVScroll(tobool(2));
METHOD_END(0)

EDITBOX_METHOD(isVisibleVScroll)
lua_pushboolean(L,self->isVisibleVScroll());
METHOD_END(1)

EDITBOX_METHOD(getVScrollRange)
lua_pushinteger(L,self->getVScrollRange());
METHOD_END(1)

EDITBOX_METHOD(getVScrollPosition)
lua_pushinteger(L,self->getVScrollPosition());
METHOD_END(1)

EDITBOX_METHOD(setVScrollPosition)
self->setVScrollPosition(toint(2));
METHOD_END(0)

EDITBOX_METHOD(setVisibleHScroll)
self->setVisibleHScroll(tobool(2));
METHOD_END(0)

EDITBOX_METHOD(isVisibleHScroll)
lua_pushboolean(L,self->isVisibleHScroll());
METHOD_END(1)

EDITBOX_METHOD(getHScrollRange)
lua_pushinteger(L,self->getHScrollRange());
METHOD_END(1)

EDITBOX_METHOD(getHScrollPosition)
lua_pushinteger(L,self->getHScrollPosition());
METHOD_END(1)

EDITBOX_METHOD(setHScrollPosition)
self->setHScrollPosition(toint(2));
METHOD_END(1)


static const struct luaL_Reg EditBoxMethod[]=
{
	{"getTextSelectionStart",getTextSelectionStart},
	{"getTextSelectionEnd",getTextSelectionEnd},
	{"getTextSelectionLength",getTextSelectionLength},
	{"getTextInterval",getTextInterval},
	{"setTextIntervalColour",setTextIntervalColour},
	{"setTextSelection",setTextSelection},
	{"deleteTextSelection",deleteTextSelection},
	{"getTextSelection",getTextSelection},
	{"isTextSelection",isTextSelection},
	{"setTextSelectionColour",setTextSelectionColour},
	{"setTextCursor",setTextCursor},
	{"getTextCursor",getTextCursor},
	{"setOnlyText",setOnlyText},
	{"getOnlyText",getOnlyText},
	{"getTextLength",getTextLength},
	{"setOverflowToTheLeft",setOverflowToTheLeft},
	{"getOverflowToTheLeft",getOverflowToTheLeft},
	{"setMaxTextLength",setMaxTextLength},
	{"getMaxTextLength",getMaxTextLength},
	{"insertText",insertText},
	{"addText",addText},
	{"eraseText",eraseText},
	{"setEditReadOnly",setEditReadOnly},
	{"getEditReadOnly",getEditReadOnly},
	{"setEditPassword",setEditPassword},
	{"getEditPassword",getEditPassword},
	{"setEditMultiLine",setEditMultiLine},
	{"getEditMultiLine",getEditMultiLine},
	{"setEditStatic",setEditStatic},
	{"getEditStatic",getEditStatic},
	{"setPasswordChar",setPasswordChar},
	{"getPasswordChar",getPasswordChar},
	{"setEditWordWrap",setEditWordWrap},
	{"getEditWordWrap",getEditWordWrap},
	{"setTabPrinting",setTabPrinting},
	{"getTabPrinting",getTabPrinting},
	{"getInvertSelected",getInvertSelected},
	{"setInvertSelected",setInvertSelected},
	{"setVisibleVScroll",setVisibleVScroll},
	{"isVisibleVScroll",isVisibleVScroll},
	{"getVScrollRange",getVScrollRange},
	{"getVScrollPosition",getVScrollPosition},
	{"setVScrollPosition",setVScrollPosition},
	{"setVisibleHScroll",setVisibleHScroll},
	{"isVisibleHScroll",isVisibleHScroll},
	{"getHScrollRange",getHScrollRange},
	{"getHScrollPosition",getHScrollPosition},
	{"setHScrollPosition",setHScrollPosition},
	{nullptr,nullptr}
};

/*
	Window
*/
#define WINDOW_METHOD(fun) METHOD( fun,Window)

WINDOW_METHOD(setVisibleSmooth)
self->setVisibleSmooth(tobool(2));
METHOD_END(0)

WINDOW_METHOD(destroySmooth)
self->destroySmooth();
METHOD_END(0)

WINDOW_METHOD(setAutoAlpha)
self->setAutoAlpha(tobool(2));
METHOD_END(0)

WINDOW_METHOD(getAutoAlpha)
lua_pushboolean(L,self->getAutoAlpha());
METHOD_END(1)

WINDOW_METHOD(getCaptionWidget)
lua_bindWidget( L,self->getCaptionWidget() );
METHOD_END(1)

WINDOW_METHOD(setMinSize)
self->setMinSize(toint(2),toint(3));
METHOD_END(0)

WINDOW_METHOD(getMinSize)
MyGUI::IntSize s = self->getMinSize();
lua_pushinteger(L,s.width);
lua_pushinteger(L,s.height);
METHOD_END(2)

WINDOW_METHOD(setMaxSize)
self->setMaxSize(
				 MyGUI::IntSize(
				 toint(2),
				 toint(3)
				 ));
METHOD_END(0)

WINDOW_METHOD(getMaxSize)
MyGUI::IntSize s = self->getMaxSize();
lua_pushinteger(L,s.width);
lua_pushinteger(L,s.height);
METHOD_END(2)

WINDOW_METHOD(setSnap)
self->setSnap(tobool(2));
METHOD_END(0)

WINDOW_METHOD(getSnap)
lua_pushboolean(L,self->getSnap());
METHOD_END(1)

WINDOW_METHOD(setMovable)
self->setMovable(tobool(2));
METHOD_END(0)

WINDOW_METHOD(getMovable)
lua_pushboolean(L,self->getMovable());
METHOD_END(1)

WINDOW_METHOD(getActionScale)
const MyGUI::IntCoord& ic = self->getActionScale();
lua_pushinteger(L,ic.left);
lua_pushinteger(L,ic.top);
lua_pushinteger(L,ic.width);
lua_pushinteger(L,ic.height);
METHOD_END(4)

static const struct luaL_Reg WindowMethod[]=
{
	{"setVisibleSmooth",setVisibleSmooth},
	{"destroySmooth",destroySmooth},
	{"setAutoAlpha",setAutoAlpha},
	{"getAutoAlpha",getAutoAlpha},
	{"getCaptionWidget",getCaptionWidget},
	{"setMinSize",setMinSize},
	{"getMinSize",getMinSize},
	{"setMaxSize",setMaxSize},
	{"getMaxSize",getMaxSize},
	{"setSnap",setSnap},
	{"getSnap",getSnap},
	{"setMovable",setMovable},
	{"getMovable",getMovable},
	{"getActionScale",getActionScale},
	{nullptr,nullptr}
};

/*
	ListBox
*/
#define LISTBOX_METHOD(fun) METHOD( fun,ListBox)

LISTBOX_METHOD(getItemCount)
lua_pushinteger(L,self->getItemCount());
METHOD_END(1)

LISTBOX_METHOD(insertItemAt)
assert_rang(2)
INSERT_ITEM_AT()
METHOD_END(0)

LISTBOX_METHOD(addItem)
ADD_ITEM()
METHOD_END(0)

LISTBOX_METHOD(removeItemAt)
assert_rang(2)
self->removeItemAt(toint(2));
METHOD_END(0)

LISTBOX_METHOD(removeAllItems)
self->removeAllItems();
METHOD_END(0)

LISTBOX_METHOD(swapItemsAt)
assert_rang(2)
assert_rang(3)
self->swapItemsAt(toint(2),toint(3));
METHOD_END(0)

LISTBOX_METHOD(findItemIndexWith)
size_t len;
const char* s = luaL_checklstring( L,2,&len);
lua_pushinteger(L,self->findItemIndexWith(toUString(string(s,len))));
METHOD_END(1)

LISTBOX_METHOD(getIndexSelected)
lua_pushinteger(L,self->getIndexSelected());
METHOD_END(1)

LISTBOX_METHOD(setIndexSelected)
assert_rang(2)
self->setIndexSelected(toint(2));
METHOD_END(0)

LISTBOX_METHOD(clearIndexSelected)
self->clearIndexSelected();
METHOD_END(0)

LISTBOX_METHOD(setItemDataAt)
SET_ITEM_DATA_AT()
METHOD_END(0)

LISTBOX_METHOD(clearItemDataAt)
assert_rang(2)
self->clearItemDataAt(toint(2));
METHOD_END(0)

//可以绑定两种数据int和char*
LISTBOX_METHOD(getItemDataAt)
GET_ITEM_DATA_AT()
METHOD_END(1)

LISTBOX_METHOD(setItemNameAt)
assert_rang(2)
size_t len;
const char* s = luaL_checklstring(L,3,&len);
self->setItemNameAt(toint(2),toUString(string(s,len)));
METHOD_END(0)

LISTBOX_METHOD(getItemNameAt)
assert_rang(2)
string s = self->getItemNameAt(toint(2)).asUTF8();
lua_pushlstring(L,s.data(),s.size());
METHOD_END(1)

LISTBOX_METHOD(beginToItemAt)
assert_rang(2)
self->beginToItemAt(toint(2));
METHOD_END(0)

LISTBOX_METHOD(beginToItemFirst)
self->beginToItemFirst();
METHOD_END(0)

LISTBOX_METHOD(beginToItemLast)
self->beginToItemLast();
METHOD_END(0)

LISTBOX_METHOD(beginToItemSelected)
self->beginToItemSelected();
METHOD_END(0)

LISTBOX_METHOD(isItemVisibleAt)
lua_pushboolean(L,self->isItemVisibleAt(toint(2),tobool(3)));
METHOD_END(1)

LISTBOX_METHOD(isItemSelectedVisible)
lua_pushboolean(L,self->isItemSelectedVisible(tobool(2)));
METHOD_END(1)

LISTBOX_METHOD(setScrollVisible)
self->setScrollVisible(tobool(2));
METHOD_END(0)

LISTBOX_METHOD(setScrollPosition)
self->setScrollPosition(toint(2));
METHOD_END(0)

LISTBOX_METHOD(getOptimalHeight)
lua_pushinteger(L,self->getOptimalHeight());
METHOD_END(1)

static const struct luaL_Reg ListBoxMethod[]=
{
	{"getItemCount",getItemCount},
	{"insertItemAt",insertItemAt},
	{"addItem",addItem},
	{"removeItemAt",removeItemAt},
	{"removeAllItems",removeAllItems},
	{"swapItemsAt",swapItemsAt},
	{"findItemIndexWith",findItemIndexWith},
	{"getIndexSelected",getIndexSelected},
	{"setIndexSelected",setIndexSelected},
	{"clearIndexSelected",clearIndexSelected},
	{"setItemDataAt",setItemDataAt},
	{"clearItemDataAt",clearItemDataAt},
	{"getItemDataAt",getItemDataAt},
	{"setItemNameAt",setItemNameAt},
	{"getItemNameAt",getItemNameAt},
	{"beginToItemAt",beginToItemAt},
	{"beginToItemFirst",beginToItemFirst},
	{"beginToItemLast",beginToItemLast},
	{"beginToItemSelected",beginToItemSelected},
	{"isItemVisibleAt",isItemVisibleAt},
	{"isItemSelectedVisible",isItemSelectedVisible},
	{"setScrollVisible",setScrollVisible},
	{"setScrollPosition",setScrollPosition},
	{"getOptimalHeight",getOptimalHeight},

	{nullptr,nullptr}
};

/*
	ComboBox
*/
#define COMBOBOX_METHOD(fun) METHOD( fun,ComboBox)
COMBOBOX_METHOD(combobox_getItemCount)
lua_pushinteger(L,self->getItemCount());
METHOD_END(1)

COMBOBOX_METHOD(combobox_insertItemAt)
INSERT_ITEM_AT()
METHOD_END(0)

COMBOBOX_METHOD(combobox_addItem)
ADD_ITEM()
METHOD_END(0)

COMBOBOX_METHOD(combobox_removeItemAt)
assert_rang(2)
self->removeItemAt(toint(2));
METHOD_END(0)

COMBOBOX_METHOD(combobox_removeAllItems)
self->removeAllItems();
METHOD_END(0)

COMBOBOX_METHOD(combobox_findItemIndexWith)
size_t len;
const char* s = luaL_checklstring( L,2,&len);
lua_pushinteger(L,self->findItemIndexWith(toUString(string(s,len))));
METHOD_END(0)

COMBOBOX_METHOD(combobox_getIndexSelected)
lua_pushinteger(L,self->getIndexSelected());
METHOD_END(1)

COMBOBOX_METHOD(combobox_setIndexSelected)
assert_rang(2)
self->setIndexSelected(toint(2));
METHOD_END(0)

COMBOBOX_METHOD(combobox_clearIndexSelected)
self->clearIndexSelected();
METHOD_END(0)

COMBOBOX_METHOD(combobox_setItemDataAt)
SET_ITEM_DATA_AT()
METHOD_END(0)

COMBOBOX_METHOD(combobox_clearItemDataAt)
assert_rang(2)
self->clearItemDataAt(toint(2));
METHOD_END(0)

COMBOBOX_METHOD(combobox_getItemDataAt)
GET_ITEM_DATA_AT()
METHOD_END(1)

COMBOBOX_METHOD(combobox_setItemNameAt)
assert_rang(2)
size_t len;
const char* s = luaL_checklstring(L,3,&len);
self->setItemNameAt(toint(2),toUString(string(s,len)));
METHOD_END(0)

COMBOBOX_METHOD(combobox_getItemNameAt)
assert_rang(2)
string s = self->getItemNameAt(toint(2)).asUTF8();
lua_pushlstring(L,s.data(),s.size());
METHOD_END(1)

COMBOBOX_METHOD(combobox_beginToItemAt)
assert_rang(2)
self->beginToItemAt(toint(2));
METHOD_END(0)

COMBOBOX_METHOD(combobox_beginToItemFirst)
self->beginToItemFirst();
METHOD_END(0)

COMBOBOX_METHOD(combobox_beginToItemLast)
self->beginToItemLast();
METHOD_END(0)

COMBOBOX_METHOD(combobox_beginToItemSelected)
self->beginToItemSelected();
METHOD_END(0)

COMBOBOX_METHOD(combobox_setComboModeDrop)
self->setComboModeDrop(tobool(2));
METHOD_END(0)

COMBOBOX_METHOD(combobox_getComboModeDrop)
lua_pushboolean(L,self->getComboModeDrop());
METHOD_END(1)

COMBOBOX_METHOD(combobox_setSmoothShow)
self->setSmoothShow(tobool(2));
METHOD_END(0)

COMBOBOX_METHOD(combobox_getSmoothShow)
lua_pushboolean(L,self->getSmoothShow());
METHOD_END(1)

COMBOBOX_METHOD(combobox_setMaxListLength)
self->setMaxListLength(toint(2));
METHOD_END(0)

COMBOBOX_METHOD(combobox_getMaxListLength)
lua_pushinteger(L,self->getMaxListLength());
METHOD_END(1)

COMBOBOX_METHOD(combobox_setFlowDirection)
self->setFlowDirection(MyGUI::FlowDirection::parse(tostring(2)));
METHOD_END(0)

COMBOBOX_METHOD(combobox_getFlowDirection)
lua_pushstring(L,self->getFlowDirection().print().c_str());
METHOD_END(1)

static const struct luaL_Reg ComboBoxMethod[]=
{
	{"getItemCount",combobox_getItemCount},
	{"insertItemAt",combobox_insertItemAt},
	{"addItem",combobox_addItem},
	{"removeItemAt",combobox_removeItemAt},
	{"removeAllItems",combobox_removeAllItems},
	{"findItemIndexWith",combobox_findItemIndexWith},
	{"getIndexSelected",combobox_getIndexSelected},
	{"setIndexSelected",combobox_setIndexSelected},
	{"clearIndexSelected",combobox_clearIndexSelected},
	{"setItemDataAt",combobox_setItemDataAt},
	{"clearItemDataAt",combobox_clearItemDataAt},
	{"getItemDataAt",combobox_getItemDataAt},
	{"setItemNameAt",combobox_setItemNameAt},
	{"getItemNameAt",combobox_getItemNameAt},
	{"beginToItemAt",combobox_beginToItemAt},
	{"beginToItemFirst",combobox_beginToItemFirst},
	{"beginToItemLast",combobox_beginToItemLast},
	{"beginToItemSelected",combobox_beginToItemSelected},
	{"setComboModeDrop",combobox_setComboModeDrop},
	{"getComboModeDrop",combobox_getComboModeDrop},
	{"setSmoothShow",combobox_setSmoothShow},
	{"getSmoothShow",combobox_getSmoothShow},
	{"setMaxListLength",combobox_setMaxListLength},
	{"getMaxListLength",combobox_getMaxListLength},
	{"setFlowDirection",combobox_setFlowDirection},
	{"getFlowDirection",combobox_getFlowDirection},
	{nullptr,nullptr}
};

/*
	ScrollBar
*/
#define SCROLLBAR_METHOD(fun) METHOD( fun,ScrollBar)

SCROLLBAR_METHOD(setVerticalAlignment)
self->setVerticalAlignment(tobool(2));
METHOD_END(0)

SCROLLBAR_METHOD(getVerticalAlignment)
lua_pushboolean(L,self->getVerticalAlignment());
METHOD_END(1)

SCROLLBAR_METHOD(setScrollRange)
self->setScrollRange(toint(2));
METHOD_END(0)

SCROLLBAR_METHOD(getScrollRange)
lua_pushinteger(L,self->getScrollRange());
METHOD_END(1)

SCROLLBAR_METHOD(scrollbar_setScrollPosition)
self->setScrollPosition(toint(2));
METHOD_END(0)

SCROLLBAR_METHOD(getScrollPosition)
lua_pushinteger(L,self->getScrollPosition());
METHOD_END(1)

SCROLLBAR_METHOD(setScrollPage)
self->setScrollPage(toint(2));
METHOD_END(0)

SCROLLBAR_METHOD(getScrollPage)
lua_pushinteger(L,self->getScrollPage());
METHOD_END(1)

SCROLLBAR_METHOD(setScrollViewPage)
self->setScrollViewPage(toint(2));
METHOD_END(0)

SCROLLBAR_METHOD(getScrollViewPage)
lua_pushinteger(L,self->getScrollViewPage());
METHOD_END(1)

SCROLLBAR_METHOD(getLineSize)
lua_pushinteger(L,self->getLineSize());
METHOD_END(1)

SCROLLBAR_METHOD(setTrackSize)
self->setTrackSize(toint(2));
METHOD_END(0)

SCROLLBAR_METHOD(getTrackSize)
lua_pushinteger(L,self->getTrackSize());
METHOD_END(1)

SCROLLBAR_METHOD(setMinTrackSize)
self->setMinTrackSize(toint(2));
METHOD_END(0)

SCROLLBAR_METHOD(getMinTrackSize)
lua_pushinteger(L,self->getMinTrackSize());
METHOD_END(1)

SCROLLBAR_METHOD(setMoveToClick)
self->setMoveToClick(tobool(2));
METHOD_END(1)

SCROLLBAR_METHOD(getMoveToClick)
lua_pushboolean(L,self->getMoveToClick());
METHOD_END(1)

static const struct luaL_Reg ScrollBarMethod[]=
{
	{"setVerticalAlignment",setVerticalAlignment},
	{"getVerticalAlignment",getVerticalAlignment},
	{"setScrollRange",setScrollRange},
	{"getScrollRange",getScrollRange},
	{"setScrollPosition",scrollbar_setScrollPosition},
	{"getScrollPosition",getScrollPosition},
	{"setScrollPage",setScrollPage},
	{"getScrollPage",getScrollPage},
	{"setScrollViewPage",setScrollViewPage},
	{"getScrollViewPage",getScrollViewPage},
	{"getLineSize",getLineSize},
	{"setTrackSize",setTrackSize},
	{"getTrackSize",getTrackSize},
	{"setMinTrackSize",setMinTrackSize},
	{"getMinTrackSize",getMinTrackSize},
	{"setMoveToClick",setMoveToClick},
	{"getMoveToClick",getMoveToClick},
	{nullptr,nullptr}
};

/*
	TabControl
*/
#define TABCONTROL_METHOD(fun) METHOD( fun,TabControl)

TABCONTROL_METHOD(TabControl_getItemCount)
lua_pushinteger(L,self->getItemCount());
METHOD_END(1)

TABCONTROL_METHOD(TabControl_insertItemAt)
INSERT_ITEM_AT()
METHOD_END(0)

TABCONTROL_METHOD(TabControl_addItem)
ADD_ITEM()
METHOD_END(0)

TABCONTROL_METHOD(TabControl_removeItemAt)
assert_rang(2)
self->removeItemAt(toint(2));
METHOD_END(0)

TABCONTROL_METHOD(TabControl_removeItem)
MyGUI::TabItem* p = (MyGUI::TabItem*)lua_cast(L,2,"TabItem");
if( p )
	self->removeItem( p );
METHOD_END(0)

TABCONTROL_METHOD(TabControl_removeAllItems)
self->removeAllItems();
METHOD_END(0)

TABCONTROL_METHOD(TabControl_getItemAt)
assert_rang(2);
lua_bindWidget(L,self->getItemAt(toint(2)));
METHOD_END(1)

TABCONTROL_METHOD(TabControl_getItemIndex)
MyGUI::TabItem* p = (MyGUI::TabItem*)lua_cast(L,2,"TabItem");
if( p )
	lua_pushinteger(L,self->getItemIndex( p ));
else
	lua_pushinteger(L,MyGUI::ITEM_NONE);
METHOD_END(1)

TABCONTROL_METHOD(TabControl_findItemIndex)
MyGUI::TabItem* p = (MyGUI::TabItem*)lua_cast(L,2,"TabItem");
if( p )
	lua_pushinteger(L,self->findItemIndex( p ));
else
	lua_pushinteger(L,MyGUI::ITEM_NONE);
METHOD_END(1)

TABCONTROL_METHOD(TabControl_findItemIndexWith)
size_t len;
const char* s = luaL_checklstring( L,2,&len );
lua_pushinteger(L,self->findItemIndexWith(toUString(string(s,len))));
METHOD_END(1)

TABCONTROL_METHOD(TabControl_findItemWith)
size_t len;
const char* s = luaL_checklstring( L,2,&len );
lua_bindWidget(L,self->findItemWith(toUString(string(s,len))));
METHOD_END(1)

TABCONTROL_METHOD(TabControl_swapItems)
assert_rang(2)
assert_rang(3)
self->swapItems(toint(2),toint(3));
METHOD_END(0)

TABCONTROL_METHOD(TabControl_getIndexSelected)
lua_pushinteger(L,self->getIndexSelected());
METHOD_END(1)

TABCONTROL_METHOD(TabControl_setIndexSelected)
assert_rang(2)
self->setIndexSelected(toint(2));
METHOD_END(0)

TABCONTROL_METHOD(TabControl_setItemSelected)
MyGUI::TabItem* p = (MyGUI::TabItem*)lua_cast(L,2,"TabItem");
if( p )
	self->setItemSelected(p);
METHOD_END(0)

TABCONTROL_METHOD(TabControl_setItemDataAt)
SET_ITEM_DATA_AT()
METHOD_END(0)

TABCONTROL_METHOD(TabControl_clearItemDataAt)
assert_rang(2)
self->clearItemDataAt(toint(2));
METHOD_END(0)

TABCONTROL_METHOD(TabControl_getItemDataAt)
GET_ITEM_DATA_AT()
METHOD_END(1)

TABCONTROL_METHOD(TabControl_setItemNameAt)
assert_rang(2)
size_t len;
const char* s = luaL_checklstring(L,3,&len);
self->setItemNameAt(toint(2),toUString(string(s,len)));
METHOD_END(0)

TABCONTROL_METHOD(TabControl_getItemNameAt)
assert_rang(2)
string s = self->getItemNameAt(toint(2)).asUTF8();
lua_pushlstring(L,s.data(),s.size());
METHOD_END(1)

TABCONTROL_METHOD(TabControl_beginToItemAt)
assert_rang(2)
self->beginToItemAt(toint(2));
METHOD_END(0)

TABCONTROL_METHOD(TabControl_beginToItemFirst)
MyGUI::TabItem* p = (MyGUI::TabItem*)lua_cast(L,2,"TabItem");
if( p )
self->beginToItem(p);
METHOD_END(0)

TABCONTROL_METHOD(TabControl_beginToItemLast)
self->beginToItemLast();
METHOD_END(0)

TABCONTROL_METHOD(TabControl_beginToItemSelected)
self->beginToItemSelected();
METHOD_END(0)

TABCONTROL_METHOD(TabControl_setButtonWidthAt)
assert_rang(2)
self->setButtonWidthAt(toint(2),toint(3));
METHOD_END(0)

TABCONTROL_METHOD(TabControl_setButtonWidth)
MyGUI::TabItem* p = (MyGUI::TabItem*)lua_cast(L,2,"TabItem");
if( p )
self->setButtonWidth(p,toint(3));
METHOD_END(0)

TABCONTROL_METHOD(TabControl_getButtonWidthAt)
assert_rang(2)
lua_pushinteger(L,self->getButtonWidthAt(toint(2)));
METHOD_END(1)

TABCONTROL_METHOD(TabControl_getButtonWidth)
MyGUI::TabItem* p = (MyGUI::TabItem*)lua_cast(L,2,"TabItem");
if( p )
	lua_pushinteger(L,self->getButtonWidth(p));
METHOD_END(1)

TABCONTROL_METHOD(TabControl_setButtonDefaultWidth)
self->setButtonDefaultWidth(toint(2));
METHOD_END(0)

TABCONTROL_METHOD(TabControl_getButtonDefaultWidth)
lua_pushinteger(L,self->getButtonDefaultWidth());
METHOD_END(1)

TABCONTROL_METHOD(TabControl_setButtonAutoWidth)
self->setButtonAutoWidth(tobool(2));
METHOD_END(0)

TABCONTROL_METHOD(TabControl_getButtonAutoWidth)
lua_pushboolean(L,self->getButtonAutoWidth());
METHOD_END(1)

TABCONTROL_METHOD(TabControl_setSmoothShow)
self->setSmoothShow(tobool(2));
METHOD_END(0)

TABCONTROL_METHOD(TabControl_getSmoothShow)
lua_pushboolean(L,self->getSmoothShow());
METHOD_END(1)

static const struct luaL_Reg TabControlMethod[]=
{
	{"getItemCount",TabControl_getItemCount},
	{"insertItemAt",TabControl_insertItemAt},
	{"addItem",TabControl_addItem},
	{"removeItemAt",TabControl_removeItemAt},
	{"removeItem",TabControl_removeItem},
	{"removeAllItems",TabControl_removeAllItems},
	{"getItemAt",TabControl_getItemAt},
	{"getItemIndex",TabControl_getItemIndex},
	{"findItemIndex",TabControl_findItemIndex},
	{"findItemIndexWith",TabControl_findItemIndexWith},
	{"findItemWith",TabControl_findItemWith},
	{"swapItems",TabControl_swapItems},
	{"getIndexSelected",TabControl_getIndexSelected},
	{"setIndexSelected",TabControl_setIndexSelected},
	{"clearIndexSelected",TabControl_setItemSelected},
	{"setItemDataAt",TabControl_setItemDataAt},
	{"clearItemDataAt",TabControl_clearItemDataAt},
	{"getItemDataAt",TabControl_getItemDataAt},
	{"setItemNameAt",TabControl_setItemNameAt},
	{"getItemNameAt",TabControl_getItemNameAt},
	{"beginToItemAt",TabControl_beginToItemAt},
	{"beginToItemFirst",TabControl_beginToItemFirst},
	{"beginToItemLast",TabControl_beginToItemLast},
	{"beginToItemSelected",TabControl_beginToItemSelected},
	{"setButtonWidthAt",TabControl_setButtonWidthAt},
	{"setButtonWidth",TabControl_setButtonWidth},
	{"getButtonWidthAt",TabControl_getButtonWidthAt},
	{"getButtonWidth",TabControl_getButtonWidth},
	{"setButtonDefaultWidth",TabControl_setButtonDefaultWidth},
	{"getButtonDefaultWidth",TabControl_getButtonDefaultWidth},
	{"setButtonAutoWidth",TabControl_setButtonAutoWidth},
	{"getButtonAutoWidth",TabControl_getButtonAutoWidth},
	{"setSmoothShow",TabControl_setSmoothShow},
	{"getSmoothShow",TabControl_getSmoothShow},

	{nullptr,nullptr}
};

/*
	DDContainer
*/
#define DDContainer_METHOD(fun) METHOD( fun,DDContainer)

DDContainer_METHOD(setNeedDragDrop)
self->setNeedDragDrop(tobool(2));
METHOD_END(0)

DDContainer_METHOD(getNeedDragDrop)
lua_pushboolean(L,self->getNeedDragDrop());
METHOD_END(1)

DDContainer_METHOD(resetDrag)
self->resetDrag();
METHOD_END(0)

static const struct luaL_Reg DDContainerMethod[]=
{
	{"setNeedDragDrop",setNeedDragDrop},
	{"getNeedDragDrop",getNeedDragDrop},
	{"resetDrag",resetDrag},
	{nullptr,nullptr}
};

/*
	ItemBox
*/
#define ITEMBOX_METHOD(fun) METHOD( fun,ItemBox)

ITEMBOX_METHOD(ItemBox_getItemCount)
lua_pushinteger(L,self->getItemCount());
METHOD_END(1)

ITEMBOX_METHOD(ItemBox_insertItemAt)
assert_rang(2)
if( lua_isstring(L,3) )
self->insertItemAt(toint(2),string(tostring(3)));
else
self->insertItemAt(toint(2));
METHOD_END(0)

ITEMBOX_METHOD(ItemBox_addItem)
self->addItem(string(tostring(2)));
METHOD_END(0)

ITEMBOX_METHOD(ItemBox_removeItemAt)
assert_rang(2)
self->removeItemAt(toint(2));
METHOD_END(0)

ITEMBOX_METHOD(ItemBox_removeAllItems)
self->removeAllItems();
METHOD_END(0)

ITEMBOX_METHOD(ItemBox_redrawAllItems)
self->redrawAllItems();
METHOD_END(0)

ITEMBOX_METHOD(ItemBox_getIndexSelected)
lua_pushinteger(L,self->getIndexSelected());
METHOD_END(1)

ITEMBOX_METHOD(ItemBox_setIndexSelected)
assert_rang(2)
self->setIndexSelected(toint(2));
METHOD_END(0)

ITEMBOX_METHOD(ItemBox_clearIndexSelected)
self->clearIndexSelected();
METHOD_END(0)

ITEMBOX_METHOD(ItemBox_clearItemDataAt)
assert_rang(2)
self->clearItemDataAt(toint(2));
METHOD_END(0)

ITEMBOX_METHOD(ItemBox_setItemDataAt)
SET_ITEM_DATA_AT()
METHOD_END(0)

ITEMBOX_METHOD(ItemBox_getItemDataAt)
GET_ITEM_DATA_AT()
METHOD_END(1)

ITEMBOX_METHOD(ItemBox_setVerticalAlignment)
self->setVerticalAlignment(tobool(2));
METHOD_END(0)

ITEMBOX_METHOD(ItemBox_getVerticalAlignment)
lua_pushinteger(L,self->getVerticalAlignment());
METHOD_END(1)

ITEMBOX_METHOD(ItemBox_getIndexByWidget)
MyGUI::Widget* p = (MyGUI::Widget*)lua_cast(L,2,"Widget");
if( p )
	lua_pushinteger(L,self->getIndexByWidget(p));
else
	lua_pushinteger(L,MyGUI::ITEM_NONE);
METHOD_END(1)

ITEMBOX_METHOD(ItemBox_getWidgetDrag)
lua_bindWidget(L,self->getWidgetDrag());
METHOD_END(1)

ITEMBOX_METHOD(ItemBox_getWidgetByIndex)
lua_bindWidget(L,self->getWidgetByIndex(toint(2)));
METHOD_END(1)

ITEMBOX_METHOD(ItemBox_resetDrag)
self->resetDrag();
METHOD_END(0)

static const struct luaL_Reg ItemBoxMethod[]=
{
	{"getItemCount",ItemBox_getItemCount},
	{"insertItemAt",ItemBox_insertItemAt},
	{"addItem",ItemBox_addItem},
	{"removeItemAt",ItemBox_removeItemAt},
	{"removeAllItems",ItemBox_removeAllItems},
	{"redrawAllItems",ItemBox_redrawAllItems},
	{"getIndexSelected",ItemBox_getIndexSelected},
	{"setIndexSelected",ItemBox_setIndexSelected},
	{"clearIndexSelected",ItemBox_clearIndexSelected},
	{"clearItemDataAt",ItemBox_clearItemDataAt},
	{"setItemDataAt",ItemBox_setItemDataAt},
	{"getItemDataAt",ItemBox_getItemDataAt},
	{"setVerticalAlignment",ItemBox_setVerticalAlignment},
	{"getVerticalAlignment",ItemBox_getVerticalAlignment},
	{"getIndexByWidget",ItemBox_getIndexByWidget},
	{"getWidgetDrag",ItemBox_getWidgetDrag},
	{"getWidgetByIndex",ItemBox_getWidgetByIndex},
	{"resetDrag",ItemBox_resetDrag},

	{nullptr,nullptr}
};

/*
	MenuBar 没有要实现的内容见MyGUI::MenuBar
*/
static const struct luaL_Reg MenuBarMethod[]=
{
	{nullptr,nullptr}
};

/*
	MenuControl
*/
#define MENUCONTROL_METHOD(fun) METHOD( fun,MenuControl)

MENUCONTROL_METHOD(MenuControl_getItemCount)
pint(self->getItemCount());
METHOD_END(1)

MENUCONTROL_METHOD(MenuControl_insertItemAt)
assert_rang(2)
s_len(3);
MyGUI::MenuItem* p = self->insertItemAt(toint(2),
				   toUString(string(s,len)),
				   MyGUI::MenuItemType::parse(tostring(4)),
				   tostring(5));
lua_bindWidget(L,p);
METHOD_END(1)

MENUCONTROL_METHOD(MenuControl_addItem)
s_len(2);
MyGUI::MenuItem* p = self->addItem(toUString(string(s,len)),
			  MyGUI::MenuItemType::parse(tostring(3)),
			  tostring(4));
lua_bindWidget(L,p);
METHOD_END(1)

MENUCONTROL_METHOD(MenuControl_removeItemAt)
assert_rang(2)
self->removeItemAt(toint(2));
METHOD_END(0)

MENUCONTROL_METHOD(MenuControl_removeItem)
MyGUI::MenuItem* p = (MyGUI::MenuItem*)lua_cast(L,2,"MenuItem");
if( p )
self->removeItem(p);
METHOD_END(0)

MENUCONTROL_METHOD(MenuControl_removeAllItems)
self->removeAllItems();
METHOD_END(0)

MENUCONTROL_METHOD(MenuControl_getItemAt)
assert_rang(2)
lua_bindWidget(L,self->getItemAt(toint(2)));
METHOD_END(1)

MENUCONTROL_METHOD(MenuControl_getItemIndex)
MyGUI::MenuItem* p = (MyGUI::MenuItem*)lua_cast(L,2,"MenuItem");
if( p )
toint(self->getItemIndex(p));
else
toint(MyGUI::ITEM_NONE);
METHOD_END(1)

MENUCONTROL_METHOD(MenuControl_findItemIndex)
MyGUI::MenuItem* p = (MyGUI::MenuItem*)lua_cast(L,2,"MenuItem");
if( p )
pint(self->findItemIndex(p));
else
toint(MyGUI::ITEM_NONE);
METHOD_END(1)

MENUCONTROL_METHOD(MenuControl_findItemWith)
s_len(2);
lua_bindWidget(L,self->findItemWith(toUString(string(s,len))));
METHOD_END(1)

MENUCONTROL_METHOD(MenuControl_setItemDataAt)
SET_ITEM_DATA_AT()
METHOD_END(0)

MENUCONTROL_METHOD(MenuControl_setItemData)
MyGUI::MenuItem* p = (MyGUI::MenuItem*)lua_cast(L,2,"MenuItem");
if(p)
self->setItemData(p,string(tostring(3)));
METHOD_END(0)

MENUCONTROL_METHOD(MenuControl_clearItemDataAt)
assert_rang(2)
self->clearItemDataAt(toint(2));
METHOD_END(0)

MENUCONTROL_METHOD(MenuControl_clearItemData)
MyGUI::MenuItem* p = (MyGUI::MenuItem*)lua_cast(L,2,"MenuItem");
if(p)
self->clearItemData(p);
METHOD_END(0)

MENUCONTROL_METHOD(MenuControl_getItemDataAt)
GET_ITEM_DATA_AT()
METHOD_END(0)

MENUCONTROL_METHOD(MenuControl_setItemIdAt)
assert_rang(2)
self->setItemIdAt(toint(2),tostring(3));
METHOD_END(0)

MENUCONTROL_METHOD(MenuControl_setItemId)
MyGUI::MenuItem* p = (MyGUI::MenuItem*)lua_cast(L,2,"MenuItem");
if(p)
self->setItemId(p,tostring(3));
METHOD_END(0)

MENUCONTROL_METHOD(MenuControl_getItemById)
lua_bindWidget(L,self->getItemById(tostring(2)));
METHOD_END(1)

MENUCONTROL_METHOD(MenuControl_findItemById)
lua_bindWidget(L,self->findItemById(tostring(2),tobool(3)));
METHOD_END(1)

MENUCONTROL_METHOD(MenuControl_getItemIndexById)
pint(self->getItemIndexById(tostring(2)));
METHOD_END(1)

MENUCONTROL_METHOD(MenuControl_setItemNameAt)
assert_rang(2)
s_len(3);
self->setItemNameAt(toint(2),toUString(string(s,len)));
METHOD_END(0)

MENUCONTROL_METHOD(MenuControl_setItemName)
s_len(3);
MyGUI::MenuItem* p = (MyGUI::MenuItem*)lua_cast(L,2,"MenuItem");
if(p)
self->setItemName(p,toUString(string(s,len)));
METHOD_END(0)

MENUCONTROL_METHOD(MenuControl_getItemNameAt)
assert_rang(2)
string s = self->getItemNameAt(toint(2)).asUTF8();
lua_pushlstring(L,s.data(),s.size());
METHOD_END(1)

MENUCONTROL_METHOD(MenuControl_getItemName)
MyGUI::MenuItem* p = (MyGUI::MenuItem*)lua_cast(L,2,"MenuItem");
if(p){
	string s = self->getItemName(p).asUTF8();
	lua_pushlstring(L,s.data(),s.size());
}else
{
	lua_pushnil(L);
}
METHOD_END(1)

MENUCONTROL_METHOD(MenuControl_findItemIndexWith)
s_len(2);
pint(self->findItemIndexWith(toUString(string(s,len))));
METHOD_END(1)

MENUCONTROL_METHOD(MenuControl_setItemChildVisibleAt)
assert_rang(2)
self->setItemChildVisibleAt(toint(2),tobool(3));
METHOD_END(0)

MENUCONTROL_METHOD(MenuControl_setItemChildVisible)
MyGUI::MenuItem* p = (MyGUI::MenuItem*)lua_cast(L,2,"MenuItem");
if(p)
self->setItemChildVisible(p,tobool(3));
METHOD_END(0)

MENUCONTROL_METHOD(MenuControl_createItemChildTAt)
METHOD_END(0)

MENUCONTROL_METHOD(MenuControl_createItemChildT)
METHOD_END(0)

MENUCONTROL_METHOD(MenuControl_getItemChildAt)
assert_rang(2)
lua_bindWidget(L,self->getItemChildAt(toint(2)));
METHOD_END(1)

MENUCONTROL_METHOD(MenuControl_getItemChild)
MyGUI::MenuItem* p = (MyGUI::MenuItem*)lua_cast(L,2,"MenuItem");
if(p)
lua_bindWidget(L,self->getItemChild(p));
else
lua_pushnil(L);
METHOD_END(1)

MENUCONTROL_METHOD(MenuControl_createItemChildAt)
lua_bindWidget(L,self->createItemChildAt(toint(2)));
METHOD_END(1)

MENUCONTROL_METHOD(MenuControl_createItemChild)
MyGUI::MenuItem* p = (MyGUI::MenuItem*)lua_cast(L,2,"MenuItem");
if(p)
lua_bindWidget(L,self->createItemChild(p));
else
lua_pushnil(L);
METHOD_END(1)

MENUCONTROL_METHOD(MenuControl_removeItemChildAt)
assert_rang(2)
self->removeItemChildAt(toint(2));
METHOD_END(0)

MENUCONTROL_METHOD(MenuControl_removeItemChild)
MyGUI::MenuItem* p = (MyGUI::MenuItem*)lua_cast(L,2,"MenuItem");
if(p)
self->removeItemChild(p);
METHOD_END(0)

MENUCONTROL_METHOD(MenuControl_getItemTypeAt)
assert_rang(2)
pstring(self->getItemTypeAt(toint(2)).print().c_str());
METHOD_END(1)

MENUCONTROL_METHOD(MenuControl_getItemType)
MyGUI::MenuItem* p = (MyGUI::MenuItem*)lua_cast(L,2,"MenuItem");
if(p)
pstring(self->getItemType(p).print().c_str());
else
lua_pushnil(L);
METHOD_END(1)

MENUCONTROL_METHOD(MenuControl_setItemTypeAt)
assert_rang(2)
self->setItemTypeAt(toint(2),MyGUI::MenuItemType::parse(tostring(3)));
METHOD_END(0)

MENUCONTROL_METHOD(MenuControl_setItemType)
MyGUI::MenuItem* p = (MyGUI::MenuItem*)lua_cast(L,2,"MenuItem");
if(p)
self->setItemType(p,MyGUI::MenuItemType::parse(tostring(3)));
METHOD_END(0)

MENUCONTROL_METHOD(MenuControl_setPopupAccept)
self->setPopupAccept(tobool(2));
METHOD_END(0)

MENUCONTROL_METHOD(MenuControl_getPopupAccept)
pbool(self->getPopupAccept());
METHOD_END(1)

MENUCONTROL_METHOD(MenuControl_getMenuItemParent)
lua_bindWidget(L,self->getMenuItemParent());
METHOD_END(1)

MENUCONTROL_METHOD(MenuControl_setVerticalAlignment)
self->setVerticalAlignment(tobool(2));
METHOD_END(0)

MENUCONTROL_METHOD(MenuControl_getVerticalAlignment)
pbool(self->getVerticalAlignment());
METHOD_END(1)

static const struct luaL_Reg MenuControlMethod[]=
{
	{"getItemCount",MenuControl_getItemCount},
	{"insertItemAt",MenuControl_insertItemAt},
	{"addItem",MenuControl_addItem},
	{"removeItemAt",MenuControl_removeItemAt},
	{"removeItem",MenuControl_removeItem},
	{"removeAllItems",MenuControl_removeAllItems},
	{"getItemAt",MenuControl_getItemAt},
	{"getItemIndex",MenuControl_getItemIndex},
	{"findItemIndex",MenuControl_findItemIndex},
	{"findItemWith",MenuControl_findItemWith},
	{"setItemDataAt",MenuControl_setItemDataAt},
	{"setItemData",MenuControl_setItemData},
	{"clearItemDataAt",MenuControl_clearItemDataAt},
	{"clearItemData",MenuControl_clearItemData},
	{"getItemDataAt",MenuControl_getItemDataAt},
	{"setItemIdAt",MenuControl_setItemIdAt},
	{"setItemId",MenuControl_setItemId},
	{"getItemById",MenuControl_getItemById},
	{"findItemById",MenuControl_findItemById},
	{"getItemIndexById",MenuControl_getItemIndexById},
	{"setItemNameAt",MenuControl_setItemNameAt},
	{"setItemName",MenuControl_setItemName},
	{"getItemNameAt",MenuControl_getItemNameAt},
	{"getItemName",MenuControl_getItemName},
	{"findItemIndexWith",MenuControl_findItemIndexWith},
	{"setItemChildVisibleAt",MenuControl_setItemChildVisibleAt},
	{"setItemChildVisible",MenuControl_setItemChildVisible},
	{"createItemChildTAt",MenuControl_createItemChildTAt},
	{"createItemChildT",MenuControl_createItemChildT},
	{"getItemChildAt",MenuControl_getItemChildAt},
	{"getItemChild",MenuControl_getItemChild},
	{"createItemChildAt",MenuControl_createItemChildAt},
	{"createItemChild",MenuControl_createItemChild},
	{"removeItemChildAt",MenuControl_removeItemChildAt},
	{"removeItemChild",MenuControl_removeItemChild},
	{"getItemTypeAt",MenuControl_getItemTypeAt},
	{"getItemType",MenuControl_getItemType},
	{"setItemTypeAt",MenuControl_setItemTypeAt},
	{"setItemType",MenuControl_setItemType},
	{"setPopupAccept",MenuControl_setPopupAccept},
	{"getPopupAccept",MenuControl_getPopupAccept},
	{"getMenuItemParent",MenuControl_getMenuItemParent},
	{"setVerticalAlignment",MenuControl_setVerticalAlignment},
	{"getVerticalAlignment",MenuControl_getVerticalAlignment},

	{nullptr,nullptr}
};

/*
	MenuItem
*/
#define MENUITEM_METHOD(fun) METHOD( fun,MenuItem)

MENUITEM_METHOD(MenuItem_setItemName)
s_len(2);
self->setItemName(toUString(string(s,len)));
METHOD_END(0)

MENUITEM_METHOD(MenuItem_getItemName)
string s = self->getItemName().asUTF8();
lua_pushlstring(L,s.data(),s.size());
METHOD_END(1)

MENUITEM_METHOD(MenuItem_setItemData)
self->setItemData(string(tostring(2)));
METHOD_END(0)

MENUITEM_METHOD(MenuItem_getItemData)
string* ps = self->getItemData<string>(false);
if(ps)
    pstring(ps->c_str());
else
    lua_pushnil(L);
METHOD_END(1)

MENUITEM_METHOD(MenuItem_removeItem)
self->removeItem();
METHOD_END(0)

MENUITEM_METHOD(MenuItem_setItemId)
self->setItemId(tostring(2));
METHOD_END(0)

MENUITEM_METHOD(MenuItem_getItemId)
pstring(self->getItemId().c_str());
METHOD_END(1)

MENUITEM_METHOD(MenuItem_createItemChild)
lua_bindWidget(L,self->createItemChild());
METHOD_END(1)

MENUITEM_METHOD(MenuItem_setItemType)
self->setItemType(MyGUI::MenuItemType::parse(tostring(2)));
METHOD_END(0)

MENUITEM_METHOD(MenuItem_getItemType)
pstring(self->getItemType().print().c_str());
METHOD_END(1)

MENUITEM_METHOD(MenuItem_setItemChildVisible)
self->setItemChildVisible(tobool(2));
METHOD_END(0)

MENUITEM_METHOD(MenuItem_getMenuCtrlParent)
lua_bindWidget(L,self->getMenuCtrlParent());
METHOD_END(1)

MENUITEM_METHOD(MenuItem_getItemChild)
lua_bindWidget(L,self->getItemChild());
METHOD_END(1)

MENUITEM_METHOD(MenuItem_getItemChecked)
pbool(self->getItemChecked());
METHOD_END(1)

MENUITEM_METHOD(MenuItem_setItemChecked)
self->setItemChecked(tobool(2));
METHOD_END(0)

static const struct luaL_Reg MenuItemMethod[]=
{
	{"setItemName",MenuItem_setItemName},
	{"getItemName",MenuItem_getItemName},
	{"setItemData",MenuItem_setItemData},
	{"getItemData",MenuItem_getItemData},
	{"removeItem",MenuItem_removeItem},
	{"setItemId",MenuItem_setItemId},
	{"getItemId",MenuItem_getItemId},
	{"createItemChild",MenuItem_createItemChild},
	{"setItemType",MenuItem_setItemType},
	{"getItemType",MenuItem_getItemType},
	{"setItemChildVisible",MenuItem_setItemChildVisible},
	{"getMenuCtrlParent",MenuItem_getMenuCtrlParent},
	{"getItemChild",MenuItem_getItemChild},
	{"getItemChecked",MenuItem_getItemChecked},
	{"setItemChecked",MenuItem_setItemChecked},

	{nullptr,nullptr}
};

/*
	MultiListBox
*/
static const struct luaL_Reg MultiListBoxMethod[]=
{
	{nullptr,nullptr}
};

/*
	MultiListItem
*/
static const struct luaL_Reg MultiListItemMethod[]=
{
	{nullptr,nullptr}
};

/*
	PopupMenu 该类没有内容完全继承至MenuControl
*/
static const struct luaL_Reg PopupMenuMethod[]=
{
	{nullptr,nullptr}
};

/*
	ProgressBar
*/
#define PROGRESSBAR_METHOD(fun) METHOD( fun,ProgressBar)

PROGRESSBAR_METHOD( ProgressBar_setProgressRange)
self->setProgressRange(toint(2));
METHOD_END(0)

PROGRESSBAR_METHOD( ProgressBar_getProgressRange)
pint(self->getProgressRange());
METHOD_END(1)

PROGRESSBAR_METHOD( ProgressBar_setProgressPosition)
self->setProgressPosition(toint(2));
METHOD_END(0)

PROGRESSBAR_METHOD( ProgressBar_getProgressPosition)
pint(self->getProgressPosition());
METHOD_END(1)

PROGRESSBAR_METHOD( ProgressBar_setProgressAutoTrack)
self->setProgressAutoTrack(tobool(2));
METHOD_END(0)

PROGRESSBAR_METHOD( ProgressBar_getProgressAutoTrack)
tobool(self->getProgressAutoTrack());
METHOD_END(1)

PROGRESSBAR_METHOD( ProgressBar_setFlowDirection)
self->setFlowDirection(MyGUI::FlowDirection().parse(tostring(2)));
METHOD_END(0)

PROGRESSBAR_METHOD( ProgressBar_getFlowDirection)
pstring(self->getFlowDirection().print().c_str());
METHOD_END(1)


static const struct luaL_Reg ProgressBarMethod[]=
{
	{"setProgressRange",ProgressBar_setProgressRange},
	{"getProgressRange",ProgressBar_getProgressRange},
	{"setProgressPosition",ProgressBar_setProgressPosition},
	{"getProgressPosition",ProgressBar_getProgressPosition},
	{"setProgressAutoTrack",ProgressBar_setProgressAutoTrack},
	{"getProgressAutoTrack",ProgressBar_getProgressAutoTrack},
	{"setFlowDirection",ProgressBar_setFlowDirection},
	{"getFlowDirection",ProgressBar_getFlowDirection},
	{nullptr,nullptr}
};


/*
	ScrollView
*/
#define SCROLLVIEW_METHOD(fun) METHOD( fun,ScrollView)

SCROLLVIEW_METHOD(ScrollView_setVisibleVScroll)
self->setVisibleVScroll(tobool(2));
METHOD_END(0)

SCROLLVIEW_METHOD(ScrollView_isVisibleVScroll)
pbool(self->isVisibleVScroll());
METHOD_END(1)

SCROLLVIEW_METHOD(ScrollView_setVisibleHScroll)
self->setVisibleHScroll(tobool(2));
METHOD_END(0)

SCROLLVIEW_METHOD(ScrollView_isVisibleHScroll)
pbool(self->isVisibleHScroll());
METHOD_END(1)

SCROLLVIEW_METHOD(ScrollView_setCanvasAlign)
self->setCanvasAlign(MyGUI::Align().parse(tostring(2)));
METHOD_END(0)

SCROLLVIEW_METHOD(ScrollView_getCanvasAlign)
pstring(self->getCanvasAlign().print().c_str());
METHOD_END(1)

SCROLLVIEW_METHOD(ScrollView_setCanvasSize)
self->setCanvasSize(toint(2),toint(3));
METHOD_END(0)

SCROLLVIEW_METHOD(ScrollView_getCanvasSize)
MyGUI::IntSize size = self->getCanvasSize();
pint(size.width);
pint(size.height);
METHOD_END(2)

SCROLLVIEW_METHOD(ScrollView_getViewCoord)
MyGUI::IntCoord ic = self->getViewCoord();
pint(ic.left);
pint(ic.top);
pint(ic.width);
pint(ic.height);
METHOD_END(4)

SCROLLVIEW_METHOD(ScrollView_setViewOffset)
self->setViewOffset(MyGUI::IntPoint(toint(2),toint(3)));
METHOD_END(0)

SCROLLVIEW_METHOD(ScrollView_getViewOffset)
MyGUI::IntPoint pt = self->getViewOffset();
pint(pt.left);
pint(pt.top);
METHOD_END(2)


static const struct luaL_Reg ScrollViewMethod[]=
{
	{"setVisibleVScroll",ScrollView_setVisibleVScroll},
	{"isVisibleVScroll",ScrollView_isVisibleVScroll},
	{"setVisibleHScroll",ScrollView_setVisibleHScroll},
	{"isVisibleHScroll",ScrollView_isVisibleHScroll},
	{"setCanvasAlign",ScrollView_setCanvasAlign},
	{"getCanvasAlign",ScrollView_getCanvasAlign},
	{"setCanvasSize",ScrollView_setCanvasSize},
	{"getCanvasSize",ScrollView_getCanvasSize},
	{"getViewCoord",ScrollView_getViewCoord},
	{"setViewOffset",ScrollView_setViewOffset},
	{"getViewOffset",ScrollView_getViewOffset},
	{nullptr,nullptr}
};

/*
	TabItem
*/
METHOD(TabItem_setButtonWidth,TabItem)
self->setButtonWidth(toint(2));
METHOD_END(0)

static const struct luaL_Reg TabItemMethod[]=
{
	{"setButtonWidth",TabItem_setButtonWidth},
	{nullptr,nullptr}
};

/*
	ImageBox
*/

static const struct luaL_Reg ImageBoxMethod[]=
{
	{nullptr,nullptr}
};

/*
	Canvas
*/

static const struct luaL_Reg canvasMethod[]=
{
	{nullptr,nullptr}
};

/*
	MyGUI::UString
*/
#define USTRING_METHOD( fun ) METHOD(fun,UString)
#define assert_ustring_rang( n ) \
if( (size_t)toint(n)>=self->size() || (size_t)toint(n)<0 ){\
	lua_errorstring(L,"UString out of rang.");\
	return 0;\
}

#define assert_ustring_rang_open( n ) \
if( (size_t)toint(n)> self->size() || (size_t)toint(n)<0 ){\
	lua_errorstring(L,"UString out of rang.");\
	return 0;\
}

USTRING_METHOD(lua_size)
pint(self->size());
METHOD_END(1)

USTRING_METHOD(lua_length)
pint(self->length());
METHOD_END(1)

USTRING_METHOD(lua_max_size)
pint(self->max_size());
METHOD_END(1)

USTRING_METHOD(lua_reserve)
self->reserve(toint(2));
METHOD_END(0)

USTRING_METHOD(lua_resize)
self->resize(toint(2));
METHOD_END(0)

USTRING_METHOD(lua_swap)
MyGUI::UString * from = (MyGUI::UString*)lua_cast(L,2,"UString");
if( from )
	self->swap( *from );
METHOD_END(0)

USTRING_METHOD(lua_empty)
pbool(self->empty());
METHOD_END(1)

USTRING_METHOD(lua_capacity)
pint(self->capacity());
METHOD_END(1)

USTRING_METHOD(lua_clear)
self->clear();
METHOD_END(0)

USTRING_METHOD(lua_substr)
assert_ustring_rang(2);
lua_bindComplete(L,"mygui.UString",new MyGUI::UString(self->substr(toint(2),toint(3))));
METHOD_END(1)

USTRING_METHOD(lua_toUTF8)
string s = self->asUTF8();
lua_pushlstring(L,s.data(),s.size());
METHOD_END(1)

USTRING_METHOD(lua_at)
assert_ustring_rang(2);
pint(self->at(toint(2)));
METHOD_END(1)

/*
	us.append(luastr)
	us.append(ustr)
*/
USTRING_METHOD(lua_append)
if( lua_isstring(L,2) )
{
	s_len(2);
	self->append(toUString(string(s,len)));
}
else
if( lua_isuserdata(L,2) )
{
	MyGUI::UString * us = (MyGUI::UString*)lua_cast(L,2,"UString");
	if( us )
		self->append(*us);
}
METHOD_END(0)

USTRING_METHOD(lua_insert)
assert_ustring_rang_open(2);
if( lua_isstring(L,3) )
{
	s_len(3);
	self->insert( toint(2),toUString(string(s,len)) );
}
else
if( lua_isuserdata(L,3) )
{
	MyGUI::UString * us = (MyGUI::UString*)lua_cast(L,3,"UString");
	if( us )
		self->insert( toint(2),*us );
}
METHOD_END(0)

USTRING_METHOD(lua_erase)
assert_ustring_rang(2);
self->erase(toint(2),toint(3));
METHOD_END(0)

USTRING_METHOD(lua_replace)
assert_ustring_rang_open(2);
if( lua_isstring(L,4) )
{
	s_len(4);
	self->replace( toint(2),toint(3),toUString(string(s,len)) );
}
else
if( lua_isuserdata(L,4) )
{
	MyGUI::UString * us = (MyGUI::UString*)lua_cast(L,4,"UString");
	if( us )
		self->replace( toint(2),toint(3),*us );
}
METHOD_END(0)

USTRING_METHOD(lua_compare)
int ret = 0;
if( lua_isstring(L,2) )
{
	s_len(2);
	ret = self->compare(toUString(string(s,len)));
}
else
if( lua_isuserdata(L,2) )
{
	MyGUI::UString * us = (MyGUI::UString*)lua_cast(L,2,"UString");
	if( us )
		ret = self->compare( *us );
	else
	{
		lua_errorstring(L,"argument @2 is not UString" );
	}
}
else
{
	lua_errorstring(L,"invaild argument @2" );
}
pint(ret);
METHOD_END(1)

USTRING_METHOD(lua_find)
MyGUI::UString::size_type ret = MyGUI::UString::npos;
assert_ustring_rang_open(3);
if( lua_isstring(L,2) )
{
	s_len(2);
	ret = self->find( toUString(string(s,len)),toint(3) );
}
else if( lua_isuserdata(L,2) )
{
	MyGUI::UString * us = (MyGUI::UString*)lua_cast(L,2,"UString");
	if( us )
		ret = self->find( *us,toint(3) );
}
pint(ret);
METHOD_END(1)

USTRING_METHOD(lua_rfind)
MyGUI::UString::size_type ret = MyGUI::UString::npos;
assert_ustring_rang_open(3);
if( lua_isstring(L,2) )
{
	s_len(2);
	ret = self->rfind( toUString(string(s,len)),toint(3) );
}
else if( lua_isuserdata(L,2) )
{
	MyGUI::UString * us = (MyGUI::UString*)lua_cast(L,2,"UString");
	if( us )
		ret = self->rfind( *us,toint(3) );
}
pint(ret);
METHOD_END(1)

USTRING_METHOD(lua_UStringtostring)
string str = self->asUTF8();
lua_pushlstring(L,str.data(),str.size());
METHOD_END(1)

/*
	Lua可以用下面的3种方法创建一个UString对象
	us = gui.newUString( string )
	us = gui.newUString( ustring )
	us = gui.newUString()
*/
static int lua_newUString( lua_State* L )
{
	const char* meta = "mygui.UString";
	if( lua_isstring(L,1) )
	{
		s_len(1);
		lua_bindComplete(L,meta,new MyGUI::UString(string(s,len)));
	}
	else if( lua_isuserdata(L,1) )
	{
		MyGUI::UString* ustr = (MyGUI::UString*)lua_cast(L,1,"UString");
		if( ustr )
		{
			lua_bindComplete(L,meta,new MyGUI::UString(*ustr));
		}
		else
		{
			ostringstream os;
			os << "argument @"<<1<<" can't be cast to '"<<meta<<"'";
			lua_errorstring(L,os.str());
		}
	}
	else
	{
		lua_bindComplete(L,meta,new MyGUI::UString());
	}
	return 1;
}

//UString 回收函数
static int lua_gcUString( lua_State* L )
{
	MyGUI::UString* self = (MyGUI::UString*)lua_cast(L,1,"UString");
	delete self;
	return 0;
}

static const struct luaL_Reg ustringMethod[]=
{
	{"size",lua_size},
	{"length",lua_length},
	{"max_size",lua_max_size},
	{"reserve",lua_reserve},
	{"resize",lua_resize},
	{"swap",lua_swap},
	{"empty",lua_empty},
	{"capacity",lua_capacity},
	{"clear",lua_clear},
	{"substr",lua_substr},
	{"toUTF8",lua_toUTF8},
	{"at",lua_at},
	{"append",lua_append},
	{"insert",lua_insert},
	{"erase",lua_erase},
	{"replace",lua_replace},
	{"compare",lua_compare},
	{"find",lua_find},
	{"rfind",lua_rfind},
	{"__gc",lua_gcUString},
	{"__tostring",lua_UStringtostring},
	{nullptr,nullptr}
};

void luaopen_gui( lua_State* L )
{
	LuaManager& lm=LuaManager::getSingleton();

	lm.registerClass("mygui.Widget",widgetMethod);
	lm.registerClass("mygui.TextBox",widgetMethod,textboxMethod);
	lm.registerClass("mygui.Button",widgetMethod,textboxMethod,buttonMethod);
	lm.registerClass("mygui.Canvas",widgetMethod,canvasMethod);
	lm.registerClass("mygui.EditBox",widgetMethod,textboxMethod,EditBoxMethod);
	lm.registerClass("mygui.ImageBox",widgetMethod,ImageBoxMethod);
	lm.registerClass("mygui.Window",widgetMethod,textboxMethod,WindowMethod);
	lm.registerClass("mygui.ListBox",widgetMethod,ListBoxMethod);
	lm.registerClass("mygui.ComboBox",widgetMethod,textboxMethod,
		EditBoxMethod,ComboBoxMethod);
	lm.registerClass("mygui.ScrollBar",widgetMethod,ScrollBarMethod);
	lm.registerClass("mygui.TabControl",widgetMethod,TabControlMethod);
	lm.registerClass("mygui.DDContainer",widgetMethod,DDContainerMethod);
	lm.registerClass("mygui.ItemBox",widgetMethod,DDContainerMethod,ItemBoxMethod);
	lm.registerClass("mygui.MenuControl",widgetMethod,MenuControlMethod);
	lm.registerClass("mygui.MenuBar",widgetMethod,MenuControlMethod,MenuBarMethod);
	lm.registerClass("mygui.MenuItem",widgetMethod,textboxMethod,buttonMethod,MenuItemMethod);
	lm.registerClass("mygui.MultiListBox",widgetMethod,MultiListBoxMethod);
	lm.registerClass("mygui.MultiListItem",widgetMethod,textboxMethod,MultiListItemMethod);
	lm.registerClass("mygui.PopupMenu",widgetMethod,MenuControlMethod,PopupMenuMethod);
	lm.registerClass("mygui.ProgressBar",widgetMethod,ProgressBarMethod);
	lm.registerClass("mygui.ScrollView",widgetMethod,ScrollViewMethod);
	lm.registerClass("mygui.TabItem",widgetMethod,textboxMethod,TabItemMethod);
	
	lm.registerGlobal("gui",guiLibs);
	//MyGUI::UString
	lm.registerClass("mygui.UString",ustringMethod);
}
