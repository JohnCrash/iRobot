#ifndef _CLASSINFO_H_
#define _CLASSINFO_H_

/*	运行时类信息，使用MyGUI的相关部分
*/
#define RTTI_DERIVED(cls) \
	MYGUI_RTTI_DERIVED(cls)

class Object:public MyGUI::IObject
{
public:
	virtual void load( MyGUI::xml::ElementPtr node ) = 0;
	virtual void save( MyGUI::xml::ElementPtr node ) = 0;
	RTTI_DERIVED(Object);
};

typedef boost::shared_ptr<Object> ObjectPtr;

typedef boost::function<Object* ()> CreateObjectFunc;

template <typename Type>
class GenericFactory
{
public:
	static CreateObjectFunc getFactory()
	{
		return createFromFactory;
	}

private:
	static Object* createFromFactory()
	{
		return new Type();
	}
};

#endif