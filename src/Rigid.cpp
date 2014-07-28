#include "stdheader.h"
#include "Geometry.h"
#include "Game.h"
#include "Rigid.h"
#include "GeometryObject.h"
#include "StringUtility.h"

/*
    两种初始化方法,1给出一个几何对象,2通过load加载进行初始化
 */
Rigid::Rigid()
{
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
    if(mGeom)
        dGeomSetBody(mGeom->getGeomID(), mBodyID);
}

Rigid::~Rigid(){
	/*删除物理引擎中的刚体标识
	*/
	dBodyDestroy(mBodyID);
    breakAllJoint();
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
    
    _visual2physic();
}

void Rigid::translate( const Ogre::Vector3& move ){
	mNode->translate( move,Ogre::Node::TS_WORLD );
	const Ogre::Vector3& v3 = mNode->getPosition();
	dBodySetPosition(mBodyID,v3.x,v3.y,v3.z);
    
    _visual2physic();
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

Joint* Rigid::getJointAt( int i )
{
	return mJoints.at(i);
}

void Rigid::load( MyGUI::xml::ElementPtr node )
{
    string ms;
	VisualObject::load(node);
    if(node->findAttribute("mass", ms))
    {
        mMassDensity = toValue(ms,(dReal)1);
    }
    _init();
	_visual2physic();
}

void Rigid::save( MyGUI::xml::ElementPtr node )
{
	VisualObject::save(node);
    node->addAttribute("mass", mMassDensity);
}

void Rigid::breakAllJoint()
{
    for(JointVec::iterator i=mJoints.begin();
        i!=mJoints.end();++i)
    {
        for( int j=0;j<2;++j )
        {
            if( this==(*i)->mRigid[j].get() )
            {
                (*i)->mRigid[j].reset();
                continue;
            }
        }
    }
    mJoints.clear();
}

void Rigid::breakJoint(Joint* jp)
{
    for( JointVec::iterator it=mJoints.begin();it!=mJoints.end();++it)
    {
        if( *it == jp )
        {
            mJoints.erase(it);
            for( int i=0;i<2;++i )
            {
                if(jp->mRigid[i].get()==this)
                {
                    jp->mRigid[i].reset();
                }
            }
            break;
        }
    }
}

void registerCoreObject()
{
	ObjectFactory& factory = ObjectFactory::getSingleton();
	factory.registerFactory<Rigid>("Rigid");
	factory.registerFactory<VisualObject>("VisualObject");
}