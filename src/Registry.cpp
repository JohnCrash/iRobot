#include "stdheader.h"
#include "Registry.h"
#include "Game.h"

SINGLETON(Registry)

Registry::Registry():
mFileName("registry.xml")
{
	string name = Game::getSingleton().getResourcePath()+"/"+mFileName;
	load(name);
}

Registry::~Registry()
{
	string name = Game::getSingleton().getResourcePath()+"/"+mFileName;
	save(name);
	mRegs.clear();
}

void Registry::save(const string& file)
{
	MyGUI::xml::Document doc;
	doc.createDeclaration();
	MyGUI::xml::ElementPtr root = doc.createRoot("Registry");
	for( RegMap::iterator i = mRegs.begin();i!=mRegs.end();++i )
	{
		MyGUI::xml::ElementPtr node = root->createChild("str");
		node->addAttribute(i->first,i->second);
	}
	doc.save(file);
}

void Registry::load(const string& file)
{
	MyGUI::xml::Document doc;
	if( doc.open(file) )
	{
		MyGUI::xml::ElementPtr root = doc.getRoot();
		if( root && root->getName()=="Registry" )
		{
			MyGUI::xml::ElementEnumerator ele = root->getElementEnumerator();
			while(ele.next() )
			{
				MyGUI::xml::ElementPtr node = ele.current();
				if( node->getName()=="str" )
				{
					MyGUI::xml::VectorAttributes atts = node->getAttributes();
					for( MyGUI::xml::VectorAttributes::iterator i = atts.begin();i!=atts.end();++i )
					{
						mRegs[i->first] = i->second;
					}
				}
			}
		}
	}
}

string Registry::get(const string& key)
{
	RegMap::iterator it = mRegs.find(key);
	if( it == mRegs.end() )
		return "";
	else
		return it->second;
}

void Registry::set(const string& key,const string& value)
{
	mRegs[key] = value;
}