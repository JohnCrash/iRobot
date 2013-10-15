#include "stdheader.h"
#include "Framework.h"

Framework::Framework()
{
}

Framework::~Framework()
{
}

void Framework::load( MyGUI::xml::Document doc )
{
}

/*
	将具有最多连接的刚体当成根节点
*/
void Framework::save( MyGUI::xml::Document doc )
{
	RigidPtr rootRigid = _getRoot();
	doc.createDeclaration();
	MyGUI::xml::ElementPtr root = doc.createRoot("Framework");

	if( !mName.empty() )
		root->addAttribute("name",mName);
	if( rootRigid )
	{
		rootRigid->save( root );
	}
}

RigidPtr Framework::_getRoot()
{
	RigidPtr root;
	int jmax = -1;
	for( RigidMap::iterator i = mRigids.begin();i!=mRigids.end();++i )
	{
		int n = (*i)->getJointsCount();
		if( n > jmax )
		{
			jmax = n;
			root = (*i);
		}
	}
	return root;
}

void Framework::addRigidNode( RigidPtr b )
{
}

void Framework::addJointNode( JointPtr j )
{
}