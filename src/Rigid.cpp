#include "stdheader.h"
#include "Geometry.h"
#include "Game.h"
#include "Rigid.h"
#include "GeometryObject.h"
#include "StringUtility.h"

Rigid::Rigid()
{
	_init();
}

Rigid::Rigid(GeometryObjectPtr geo):
VisualObject(geo)
{
	_init();
}

void Rigid::_init()
{
	mBodyID = dBodyCreate(RigidManager::getSingleton().getWorldID());
	//默认质量密度
	mMassDensity = 1;
}

Rigid::~Rigid(){
	/*删除物理引擎中的刚体标识
	*/
	dBodyDestroy(mBodyID);
}

void Rigid::_update(){
	const dReal* dPos = dBodyGetPosition(mBodyID);

	mNode->setPosition((Ogre::Real)dPos[0],
		(Ogre::Real)dPos[1],
		(Ogre::Real)dPos[2]);

	const dReal* dQ = dBodyGetQuaternion(mBodyID);

	mNode->setOrientation(Ogre::Quaternion((Ogre::Real)dQ[0],
		(Ogre::Real)dQ[1],
		(Ogre::Real)dQ[2],
		(Ogre::Real)dQ[3]));
}

void Rigid::_visual2physic()
{
	const Ogre::Vector3& v3 = mNode->getPosition();
	dBodySetPosition(mBodyID,v3.x,v3.y,v3.z);
	const Ogre::Quaternion& q = mNode->getOrientation();
	dQuaternion dq;
	dq[0] = q.w;
	dq[1] = q.x;
	dq[2] = q.y;
	dq[3] = q.z;
	dBodySetQuaternion(mBodyID,dq);
}

void Rigid::setPosition( const Ogre::Vector3& v ){
	mNode->setPosition( v );
	//同步物理引擎中的刚体位置
	dBodySetPosition(mBodyID,v.x,v.y,v.z);
}

//旋转刚体
void Rigid::rotate( const Ogre::Vector3& axis,const Ogre::Real angle ){
	mNode->rotate( axis,Ogre::Radian(angle),Ogre::Node::TS_WORLD );
	const Ogre::Quaternion& q = mNode->getOrientation();
	dQuaternion dQ;
	dQ[0] = q.w;
	dQ[1] = q.x;
	dQ[2] = q.y;
	dQ[3] = q.z;
	dBodySetQuaternion(mBodyID,dQ);
}

void Rigid::translate( const Ogre::Vector3& move ){
	mNode->translate( move,Ogre::Node::TS_WORLD );
	const Ogre::Vector3& v3 = mNode->getPosition();
	dBodySetPosition(mBodyID,v3.x,v3.y,v3.z);
}

void Rigid::updatePhysica()
{
	_updateBoundingBox();
	mGeom->updatePhysica( mBodyID,mMass,mMassDensity );
}

void Rigid::updateRigid()
{
	mGeom->updateGeometry();
	mGeom->updatePhysica( mBodyID,mMass,mMassDensity );
}

dBodyID Rigid::getBodyID() const
{
	return mBodyID;
}

void Rigid::Enable()
{
	VisualObject::Enable();
	dBodyEnable(mBodyID);
	updatePhysica();
}

void Rigid::Disable()
{
	VisualObject::Disable();
	dBodyDisable(mBodyID);
}

int Rigid::getJointsCount()
{
	return mJoints.size();
}

JointPtr Rigid::getJointAt( int i )
{
	return mJoints.at(i);
}

void Rigid::load( MyGUI::xml::ElementPtr node )
{
	VisualObject::load(node);
	_visual2physic();
}

void Rigid::save( MyGUI::xml::ElementPtr node )
{
	VisualObject::save(node);
}

void Rigid::breakAllJoint()
{
    for(JointMap::iterator i=mJoints.begin();
        i!=mJoints.end();++i)
    {
        if( this==(*i)->mRigid1.get() )
        {
            (*i)->mRigid1.reset();
            continue;
        }
        if( this==(*i)->mRigid2.get() )
        {
            (*i)->mRigid2.reset();
        }
    }
    mJoints.clear();
}

void Rigid::breakJoint(JointPtr jp)
{
    BOOST_AUTO(it,find(mJoints.begin(),mJoints.end(),jp));
    if(it!=mJoints.end())
    {
        mJoints.erase(it);
        if(jp->mRigid1.get()==this)
        {
            jp->mRigid1.reset();
        }
        if(jp->mRigid2.get()==this)
        {
            jp->mRigid2.reset();
        }
    }
}

void registerCoreObject()
{
	ObjectFactory& factory = ObjectFactory::getSingleton();
	factory.registerFactory<Rigid>("Rigid");
	factory.registerFactory<VisualObject>("VisualObject");
}