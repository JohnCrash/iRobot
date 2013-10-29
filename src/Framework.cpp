#include "stdheader.h"
#include "Framework.h"
#include "ObjectFactory.h"

Framework::Framework()
{
}

Framework::~Framework()
{
    removeAllJoint();
}

void Framework::removeAllJoint()
{
    for(JointMap::iterator i=mJoints.begin();i!=mJoints.end();++i)
    {
        (*i)->breakAllRigid();
    }
    mJoints.clear();
}

void Framework::load( MyGUI::xml::ElementPtr node )
{
    removeAllJoint();
    mName = node->findAttribute("name");
    MyGUI::xml::ElementEnumerator e=node->getElementEnumerator();
    while(e.next())
    {
        MyGUI::xml::ElementPtr node = e.current();
        if( node->getName() == "body" )
        {
            loadRigid(node, JointPtr());
        }
        break;
    }
}

void Framework::loadRigid( MyGUI::xml::ElementPtr node,JointPtr parent )
{
    ObjectFactory& factory = ObjectFactory::getSingleton();
    RigidPtr rgp = boost::dynamic_pointer_cast<Rigid>(factory.createObject("Rigid"));
    if(rgp)
    {
        if( parent )
        {
            parent->mRigid2 = rgp;
            parent->linkRigid(parent->mRigid1, parent->mRigid2);
        }
        rgp->load(node);
        MyGUI::xml::ElementEnumerator ce = node->getElementEnumerator();
        while(ce.next())
        {
            MyGUI::xml::ElementPtr child = ce.current();
            if( child->getName() == "joint" )
            {
                loadJoint( child,rgp );
            }
        }
    }
    else
    {
        WARNING_LOG("Factory can't make Rigid object!");
    }
}

void Framework::loadJoint( MyGUI::xml::ElementPtr node,RigidPtr parent )
{
    ObjectFactory& factory = ObjectFactory::getSingleton();
    string typeName = node->findAttribute("type");
    JointPtr jpt = boost::dynamic_pointer_cast<Joint>(factory.createObject(typeName));
    if(jpt)
    {
        if( parent )
        {
            jpt->mRigid1 = parent;
        }
        addJoint(jpt);
        jpt->load(node);
        MyGUI::xml::ElementEnumerator ce = node->getElementEnumerator();
        while(ce.next())
        {
            MyGUI::xml::ElementPtr child = ce.current();
            if( child->getName() == "rigid" )
            {
                loadRigid(child, jpt);
            }
            break;
        }
    }
    else
    {
        WARNING_LOG("Factory can't make "<<typeName);
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
    node->addAttribute("type", joint->getTypeName());
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
        MyGUI::xml::ElementPtr js = node->createChild((*i)->getTypeName());
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

void registerFrameworkFactory()
{
	ObjectFactory& factory = ObjectFactory::getSingleton();
	factory.registerFactory<Framework>("Framework");
}
