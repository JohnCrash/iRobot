#include "stdheader.h"
#include "Framework.h"

Framework::Framework()
{
}

Framework::~Framework()
{
}

void Framework::load( MyGUI::xml::ElementPtr node )
{
}

void Framework::idAll()
{
}

void Framework::reConstruct()
{
}

void Framework::save( MyGUI::xml::ElementPtr node )
{
	if( !mName.empty() )
		node->addAttribute("name",mName);
    for(JointMap::iterator i=mJoints.begin();i!=mJoints.end();++i)
    {
        MyGUI::xml::ElementPtr child = node->createChild("Joint");
        child->addAttribute("id", (*i)->mid);
        (*i)->save(child);
    }
}

void Framework::addJoint( JointPtr j )
{
    mJoints.push_back(j);
}

void Framework::removeJoint( JointPtr j )
{
    BOOST_AUTO(it,find(mJoints.begin(),mJoints.end(),j) );
	if(it!=mJoints.end())
    {
        j->breakAllRigid(); //断开
        mJoints.erase(it);
    }
}
