#include "stdheader.h"
#include "Geometry.h"
#include "Joint.h"
#include "RigidManager.h"
#include "ObjectFactory.h"
#include "StringUtility.h"

Joint::Joint()
{
}

Joint::~Joint()
{
	dJointDestroy(mJointID);
}

void Joint::Enable()
{
	dJointEnable(mJointID);
}

void Joint::Disable()
{
	dJointDisable(mJointID);
}

bool Joint::isEnabled() const
{
	return dJointIsEnabled(mJointID)==0?false:true;
}

dJointType Joint::getJointType() const
{
	return dJointGetType(mJointID);
}

void Joint::setJointData(void* data)
{
	dJointSetData(mJointID,data);
}

void* Joint::getJointData()
{
	return dJointGetData(mJointID);
}

dJointID Joint::getJointID() const
{
	return mJointID;
}

RigidPtr Joint::getJointRigid( int i ) const
{
	switch(i)
	{
	case 0:return mRigid1;
	case 1:return mRigid2;
	default:return RigidPtr();
	}
}

void Joint::linkRigid(RigidPtr b1,RigidPtr b2)
{
    if( b1 && b2 )
    {
        dJointAttach(mJointID,b1->getBodyID(),b2->getBodyID());
        mRigid1 = b1;
        mRigid2 = b2;
    }
}

void Joint::save(MyGUI::xml::ElementPtr node)
{
    node->addAttribute("name", mName);
}

void Joint::load(MyGUI::xml::ElementPtr node)
{
    mName = node->findAttribute("name");
}

void Joint::breakAllRigid()
{
    if( mRigid1 )mRigid1->breakJoint(JointPtr(this));
    if( mRigid2 )mRigid2->breakJoint(JointPtr(this));
    mRigid1.reset();
    mRigid2.reset();
}

void Joint::breakRigid( RigidPtr p )
{
    if( p )p->breakJoint(JointPtr(this));
}

JointBall::JointBall()
{
	dWorldID worldID = RigidManager::getSingleton().getWorldID();
	mJointID = dJointCreateBall(worldID,0);
}

void JointBall::load( MyGUI::xml::ElementPtr node )
{
    Joint::load(node);
    setBallAnchor(toVec3(node->findAttribute("anchor"),Ogre::Vector3()));
}

void JointBall::save( MyGUI::xml::ElementPtr node )
{
    Joint::save(node);
    node->addAttribute("anchor", vec3toString(getBallAnchor(0)));
}

void JointBall::setBallAnchor( const Ogre::Vector3& v3 )
{
	dJointSetBallAnchor(mJointID,v3.x,v3.y,v3.z);
}

Ogre::Vector3 JointBall::getBallAnchor( int n )
{
	dVector3 v;
	if( n == 0 )
	{
		dJointGetBallAnchor(mJointID,v);
	}
	else if( n == 1 )
	{
		dJointGetBallAnchor2(mJointID,v);
	}
	else
	{
		ERROR_LOG("getBallAnchor invaild argument "<<n );
	}
	return Ogre::Vector3((Ogre::Real)v[0],(Ogre::Real)v[1],(Ogre::Real)v[2]);
}

JointHinge::JointHinge()
{
	dWorldID worldID = RigidManager::getSingleton().getWorldID();
	mJointID = dJointCreateHinge(worldID,0);
}

void JointHinge::setHingeAnchor( const Ogre::Vector3& v3 )
{
	dJointSetHingeAnchor(mJointID,v3.x,v3.y,v3.z);
}

Ogre::Vector3 JointHinge::getHingeAnchor()
{
	dVector3 v;
	dJointGetHingeAnchor(mJointID,v);
	return Ogre::Vector3((Ogre::Real)v[0],(Ogre::Real)v[1],(Ogre::Real)v[2]);
}

void JointHinge::setHingeAxis( const Ogre::Vector3& v3 )
{
	dJointSetHingeAxis(mJointID,v3.x,v3.y,v3.z);
}

Ogre::Vector3 JointHinge::getHingeAxis()
{
	dVector3 v;
	dJointGetHingeAxis(mJointID,v);
	return Ogre::Vector3((Ogre::Real)v[0],(Ogre::Real)v[1],(Ogre::Real)v[2]);
}

void JointHinge::load( MyGUI::xml::ElementPtr node )
{
    Joint::load(node);
    setHingeAnchor(toVec3(node->findAttribute("anchor"),Ogre::Vector3()));
    setHingeAxis(toVec3(node->findAttribute("axis"),Ogre::Vector3()));
}

void JointHinge::save( MyGUI::xml::ElementPtr node )
{
    Joint::save(node);
    node->addAttribute("anchor", vec3toString(getHingeAnchor()));
    node->addAttribute("axis", vec3toString(getHingeAxis()));
}

JointSlider::JointSlider()
{
	dWorldID worldID = RigidManager::getSingleton().getWorldID();
	mJointID = dJointCreateSlider(worldID,0);
}

void JointSlider::load( MyGUI::xml::ElementPtr node )
{
    Joint::load(node);
}

void JointSlider::save( MyGUI::xml::ElementPtr node )
{
    Joint::save(node);
}

void registerJointFactory()
{
	ObjectFactory& factory = ObjectFactory::getSingleton();
	factory.registerFactory<JointBall>("JointBall");
	factory.registerFactory<JointHinge>("JointHinge");
	factory.registerFactory<JointSlider>("JointSlider");
}
