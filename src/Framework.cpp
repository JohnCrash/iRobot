#include "stdheader.h"
#include "Framework.h"
#include "ObjectFactory.h"

Framework::Framework()
{
}

Framework::~Framework()
{
}

void Framework::load( MyGUI::xml::ElementPtr node )
{
    mName = node->findAttribute("name");
    MyGUI::xml::ElementEnumerator e=node->getElementEnumerator();
    while(e.next())
    {
        string name = e->getName();
        if( name == "body" )
        {
            ObjectFactory& factory = ObjectFactory::getSingleton();
            ObjectPtr p = factory.createObject("Rigid");
            if( p )
            {
                p->load(e.current());
            }
        }
    }
}

/*
    注意:这里要保证没有循环连接
 */
void Framework::save( MyGUI::xml::ElementPtr node )
{
	if( !mName.empty() )
		node->addAttribute("name",mName);
    //这里做检测保证框架没有循环连接的铰链
    if(!checkCycle())
    {
        WARNING_LOG("Framework have cycle.");
        return;
    }
    
    RigidPtr body = getBodyRigid();
    if(body)
    {
        MyGUI::xml::ElementPtr child = node->createChild("body");
        saveRigid(child, body);
    }
}

/* 一组递归函数,用来保存
 */
void Framework::saveJoint(MyGUI::xml::ElementPtr node,JointPtr joint,RigidPtr other)
{
    joint->save(node);
    MyGUI::xml::ElementPtr child = node->createChild("rigid");
    if( joint->mRigid1 == other && joint->mRigid2 )
    {
        saveRigid(child,joint->mRigid2);
    }
    else if( joint->mRigid2 == other && joint->mRigid1 )
    {
        saveRigid(child,joint->mRigid1);
    }
}

void Framework::saveRigid(MyGUI::xml::ElementPtr node,RigidPtr rgd)
{
    rgd->save(node);
    for(JointMap::iterator i=rgd->mJoints.begin();i!=rgd->mJoints.end();++i)
    {
        MyGUI::xml::ElementPtr js = node->createChild("joint");
        saveJoint(js, *i,rgd);
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

bool Framework::checkCycle()
{
    RigidMap m;
    //....
    return false;
}

RigidPtr Framework::getBodyRigid()
{
    RigidMap m;
    vector<int> c;
    for(JointMap::iterator i=mJoints.begin();i!=mJoints.end();++i)
    {
        if( (*i)->mRigid1 )
        {
            BOOST_AUTO(it, find(m.begin(),m.end(),(*i)->mRigid1));
            if(it!=m.end())
            {
                c[it-m.begin()]++;
            }
            else
            {
                m.push_back(*it);
                c.push_back(1);
            }
        }
    }
    int maxn = 0;
    int maxi = -1;
    for(vector<int>::iterator i=c.begin();i!=c.end();++i)
    {
        if(*i>maxn)
        {
            maxn = *i;
            maxi = i-c.begin();
        }
    }
    if(maxi>0)
    {
        return m[maxi];
    }
    return RigidPtr();
}
