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
            parent->mRigid[1] = rgp;
            parent->linkRigid(parent->mRigid[0], parent->mRigid[1]);
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
            jpt->mRigid[0] = parent;
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
    if(checkCycle())
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
    if( joint->mRigid[0] == other && joint->mRigid[1] )
    {
        saveRigid(child,joint->mRigid[1]);
    }
    else if( joint->mRigid[1] == other && joint->mRigid[0] )
    {
        saveRigid(child,joint->mRigid[0]);
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

//沿着连接搜索
bool Framework::walkCycle(RigidPtr rp,JointPtr parent,JointMap& map)
{
    if(rp)
    {
        for(JointMap::iterator i=rp->mJoints.begin();i!=rp->mJoints.end();++i)
        {
            if(*i!=parent)
            {
                BOOST_AUTO(it,find(map.begin(),map.end(),*i));
                if( it!=map.end() )
                { //发现重复
                    return false;
                }
                RigidPtr g = (*i)->other(rp);
                if( g )
                {
                    map.push_back(*i);
                    if( !walkCycle(g,*i,map) )
                        return false; //发现
                    map.pop_back();
                }
            }
        }
    }
    return true;
}

/*
返回false没有圈,返回true有圈
这个算法不能确定所有的Joint都是相连接的
 */
bool Framework::checkCycle()
{
    JointMap m;
    JointPtr jp;
    if( mJoints.empty() )
        return false;
    jp = mJoints[0];
    for(int i=0;i<2;++i )
    {
        m.push_back(jp);
        if( !walkCycle(jp->mRigid[0],jp,m) )
            return true;
        m.pop_back();
    }
    return false;
}

RigidPtr Framework::getBodyRigid()
{
    RigidMap m;
    vector<int> c;
    for(JointMap::iterator i=mJoints.begin();i!=mJoints.end();++i)
    {
        for( int j=0;j<2;++j)
        {
            if( (*i)->mRigid[j] )
            {
                BOOST_AUTO(it, find(m.begin(),m.end(),(*i)->mRigid[j]));
                if(it!=m.end())
                {
                    c[it-m.begin()]++;
                }
                else
                {
                    m.push_back((*i)->mRigid[j]);
                    c.push_back(1);
                }
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
