#include "stdheader.h"
#include "LuaManager.h"
#include "LuaExport.h"

#define XML_METHOD( fun,T ) \
static int fun( lua_State* L )\
{\
	MyGUI::xml::T* self = (MyGUI::xml::T*)lua_cast( L,1,#T );\
	if( self == nullptr )return 0;\

#define METHOD_END( ret ) \
	return ret; }\

#define Element_METHOD( fun ) XML_METHOD( fun,Element )

//在指针obj被析构时的通知函数
static void notifyDelete( const MyGUI::xml::ElementPtr& obj )
{
	LuaManager::getSingleton().unbind(obj);
}

static void bindElement(lua_State*L,const MyGUI::xml::ElementPtr& obj)
{
	if( obj )
	{
		obj->setNotifyDelete( notifyDelete );
		lua_bind(L,"xml.Element",obj);
	}
}

Element_METHOD(ele_createChild)
	MyGUI::xml::ElementPtr obj;
	string content;
	MyGUI::xml::ElementType _type = MyGUI::xml::ElementType::Normal;
	if( lua_isstring(L,3) )
		content = tostring(3);
	if( lua_isstring(L,4) )
	{
		string type = tostring(4);
		if( type == "Comment" )
			_type = MyGUI::xml::ElementType::Comment;
		else if( type == "Declaration")
			_type = MyGUI::xml::ElementType::Declaration;
		else if( type == "Normal" )
			_type = MyGUI::xml::ElementType::Normal;
		else
			_type = MyGUI::xml::ElementType::MAX;
	}
	obj = self->createChild(tostring(2),content,_type);
	bindElement(L,obj);
METHOD_END(1)

Element_METHOD(ele_removeChild)
	MyGUI::xml::ElementPtr obj = (MyGUI::xml::ElementPtr)lua_cast(L,2,"Element");
	if( obj )
		self->removeChild( obj );
METHOD_END(0)

Element_METHOD(ele_addAttribute)
	self->addAttribute(tostring(2),tostring(3));
METHOD_END(0)

Element_METHOD(ele_removeAttribute)
	self->removeAttribute(tostring(2));
METHOD_END(0)

Element_METHOD(ele_setAttribute)
	self->setAttribute(tostring(2),tostring(3));
METHOD_END(0)

Element_METHOD(ele_addContent)
	self->addContent(tostring(2));
METHOD_END(0)

Element_METHOD(ele_setContent)
	self->setContent(tostring(2));
METHOD_END(0)

Element_METHOD(ele_clear)
	self->clear();
METHOD_END(0)

Element_METHOD(ele_findAttribute)
	string value;
	bool b = self->findAttribute(tostring(2),value);
	pbool(b);
	pstring(value.c_str());
METHOD_END(2)

Element_METHOD(ele_getName)
	pstring(self->getName().c_str());
METHOD_END(1)

Element_METHOD(ele_getContent)
	pstring(self->getContent().c_str());
METHOD_END(1)

Element_METHOD(ele_getParent)
	MyGUI::xml::ElementPtr obj = self->getParent();
	if( obj )
		bindElement(L,obj);
	else
		lua_pushnil(L);
METHOD_END(1)

Element_METHOD(ele_getType)
	MyGUI::xml::ElementType _type = self->getType();
	string stype;
	if( _type == MyGUI::xml::ElementType::Normal )
		stype = "Normal";
	else if( _type == MyGUI::xml::ElementType::Declaration )
		stype = "Declaration";
	else if( _type == MyGUI::xml::ElementType::Comment )
		stype = "Comment";
	else if( _type == MyGUI::xml::ElementType::MAX )
		stype = "MAX";
	pstring(stype.c_str());
METHOD_END(1)

//返回一个包括全部子节点的表
Element_METHOD(ele_getChildren)
	MyGUI::xml::ElementEnumerator e = self->getElementEnumerator();
	lua_newtable(L);
	int i = 1;
	while( e.next() )
	{
		MyGUI::xml::ElementPtr node = e.current();
		lua_pushinteger(L,i++);
		bindElement(L,node);
		lua_settable(L,-3);
	}
METHOD_END(1)

Element_METHOD(ele_getAttributes)
	MyGUI::xml::VectorAttributes& a = const_cast<MyGUI::xml::VectorAttributes&>(self->getAttributes());
	lua_newtable(L);

	for( MyGUI::xml::VectorAttributes::iterator i = a.begin();i!=a.end();++i )
	{
		lua_pushlstring(L,i->second.c_str(),i->second.size());
		lua_setfield(L,-2,i->first.c_str());
	}
METHOD_END(1)
/*
	实现对MyGUI::xml的Lua输出
*/
static const struct luaL_Reg xmlElementMethod[]=
{
	{"createChild",ele_createChild},
	{"removeChild",ele_removeChild},
	{"addAttribute",ele_addAttribute},
	{"removeAttribute",ele_removeAttribute},
	{"setAttribute",ele_setAttribute},
	{"addContent",ele_addContent},
	{"setContent",ele_setContent},
	{"clear",ele_clear},
	{"findAttribute",ele_findAttribute},
	{"getName",ele_getName},
	{"getContent",ele_getContent},
	{"getParent",ele_getParent},
	{"getType",ele_getType},
	{"getChildren",ele_getChildren},
	{"getAttributes",ele_getAttributes},
	{nullptr,nullptr}
};

#define DOCUMENT_METHOD( fun ) XML_METHOD(fun,Document)

DOCUMENT_METHOD(xml_open)
	s_len(2);
	pbool(self->open(string(s,len)));
METHOD_END(1)

DOCUMENT_METHOD(xml_save)
	s_len(2);
	pbool(self->save(string(s,len)));
METHOD_END(1)

DOCUMENT_METHOD(xml_clear)
	self->clear();
METHOD_END(0)

DOCUMENT_METHOD(xml_createDeclaration)
	MyGUI::xml::ElementPtr ele;
	if( lua_isstring(L,2) && lua_isstring(L,3) )
	{
		ele = self->createDeclaration(tostring(2),tostring(3));
	}
	else if( lua_isstring(L,2) )
	{
		ele = self->createDeclaration(tostring(2));
	}
	else
	{
		ele = self->createDeclaration();
	}
	bindElement(L,ele);
METHOD_END(1)

DOCUMENT_METHOD(xml_createRoot)
	bindElement(L,self->createRoot(tostring(2)));
METHOD_END(1)

DOCUMENT_METHOD(xml_getRoot)
	bindElement(L,self->getRoot());
METHOD_END(1)

DOCUMENT_METHOD(xml_clearLastError)
	self->clearLastError();
METHOD_END(0)

DOCUMENT_METHOD(xml_getLastError)
	pstring(self->getLastError().c_str());
METHOD_END(1)

static int lua_gcDocument(lua_State* L)
{
	MyGUI::xml::Document* self = (MyGUI::xml::Document*)lua_cast(L,1,"Document");
	delete self;
	return 0;
}

static const struct luaL_Reg xmlDocumentMethod[]=
{
	{"open",xml_open},
	{"save",xml_save},
	{"clear",xml_clear},
	{"createDeclaration",xml_createDeclaration},
	{"createRoot",xml_createRoot},
	{"getRoot",xml_getRoot},
	{"clearLastError",xml_clearLastError},
	{"getLastError",xml_getLastError},
	{"__gc",lua_gcDocument},
	{nullptr,nullptr}
};

static int newDocument( lua_State* L )
{ 
	lua_bindComplete(L,"xml.Document",new MyGUI::xml::Document());
	return 1;
}

static const struct luaL_Reg xmlLibs[]=
{
	{"newDocument",newDocument},
	{nullptr,nullptr}
};

void luaopen_xml( lua_State* L )
{
	LuaManager& lm=LuaManager::getSingleton();

	lm.registerClass("xml.Document",xmlDocumentMethod);
	lm.registerClass("xml.Element",xmlElementMethod);

	lm.registerGlobal("xml",xmlLibs);
}
