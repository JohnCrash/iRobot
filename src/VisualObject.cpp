#include "stdheader.h"
#include "Geometry.h"
#include "Game.h"
#include "GeometryObject.h"
#include "VisualObject.h"
#include "StringUtility.h"

VisualObject::VisualObject()
:mNode(nullptr)
{
	_init();
}

VisualObject::VisualObject(GeometryObjectPtr geo)
:mNode(nullptr)
{
	mGeom = geo;
	_init();
}

void VisualObject::_init()
{
	string name;

	if( !mNode )
	{
		bShowBoundingBox = false;

		mBoundingBox = nullptr;
		//创建一个不具名节点
		mNode = Game::getSingleton().getSceneManager()->
			getRootSceneNode()->
			createChildSceneNode();
		setCastShadows(true);
		mNode->setVisible(false);
	}
	if( mGeom )
		mNode->attachObject( mGeom->getEntity() );
}

VisualObject::~VisualObject(){
	_clearBoundingBox();
	mNode->detachObject( mGeom->getEntity() );
	Game::getSingleton().getSceneManager()->destroySceneNode(mNode);
}

void VisualObject::updatePhysica()
{
	_updateBoundingBox();
}

GeometryObjectPtr VisualObject::getGeometryObject() const
{
	return mGeom;
}

string VisualObject::getName() const
{
	return mName;
}

void VisualObject::setName( const string name )
{
	mName = name;
}

void VisualObject::setPosition( const Ogre::Vector3& v ){
	mNode->setPosition( v );
	if( mGeom->hasGeomID() ){
		dGeomID gid = mGeom->getGeomID();
		dGeomSetPosition( gid,v.x,v.y,v.z );
	}
}

//旋转刚体
void VisualObject::rotate( const Ogre::Vector3& axis,const Ogre::Real angle ){
	mNode->rotate( axis,Ogre::Radian(angle),Ogre::Node::TS_WORLD );
	if( mGeom->hasGeomID() ){
		dGeomID gid = mGeom->getGeomID();
		dQuaternion Q,dQ,rQ;
		dGeomGetQuaternion( gid,dQ );
		dQFromAxisAndAngle( rQ,axis.x,axis.y,axis.z,angle );
		dQMultiply0( Q,rQ,dQ );
		dGeomSetQuaternion( gid,Q );
	}
}

void VisualObject::translate( const Ogre::Vector3& move ){
	mNode->translate( move,Ogre::Node::TS_WORLD );
	if( mGeom->hasGeomID() ){
		dGeomID gid = mGeom->getGeomID();
		const dReal* p = dGeomGetPosition( gid );
		dGeomSetPosition(gid,p[0]+move.x,p[1]+move.y,p[2]+move.z);
	}
}

void VisualObject::setMaterialName( const string& name ){
	mGeom->setMaterialName( name );
}

string VisualObject::getMaterialName() const
{
	return mGeom->getMaterialName();
}

void VisualObject::setCastShadows( bool b )
{
	if( mGeom )
		mGeom->setCastShadows( b );
}

Ogre::Matrix3 VisualObject::getLocalAxis() const
{
	return mNode->getLocalAxes();
}

Ogre::Vector3 VisualObject::getPosition() const
{
	return mNode->getPosition();
}

VisualObject::VisitRend::VisitRend( const Ogre::Ray& ray ):
	mRay(ray)
{
}

void VisualObject::VisitRend::visit(Ogre::Renderable *rend, Ogre::ushort lodIndex, 
				   bool isDebug, Ogre::Any *pAny)
{
	vector<Ogre::Real> result = Game::getSingleton().Intersect( mRay,rend );
	for( vector<Ogre::Real>::iterator i=result.begin();i!=result.end();++i )
		mResult.push_back( *i );
}

vector<Ogre::Real> VisualObject::Intersect( const Ogre::Ray& ray,bool bSort ){
	VisitRend rend(ray);
	mGeom->getEntity()->visitRenderables( &rend );
	if( bSort && !rend.mResult.empty() ){
		//对结果进行排序，由近到远
		sort( rend.mResult.begin(),rend.mResult.end() );
	}
	return rend.mResult;
}

string VisualObject::getEntityName() const
{
	return mGeom->getEntity()->getName();
}

void VisualObject::showBoundingBox( bool b )
{
	//MeshPtr m = mGeom->getMesh();
	if( b )
	{
		_createBoundingBox();
	}
	else
	{
		_clearBoundingBox();
	}
	bShowBoundingBox = b;
}

void VisualObject::_clearBoundingBox()
{
	if( mBoundingBox )
	{
		mNode->detachObject(mBoundingBox);
		Game::getSingleton().getSceneManager()->destroyManualObject(mBoundingBox);
		mBoundingBox = nullptr;
	}
}

void VisualObject::_createBoundingBox()
{
	if( !mBoundingBox )
	{
		mBoundingBox = Game::getSingleton().getSceneManager()->createManualObject();
		Ogre::AxisAlignedBox box = mGeom->getMesh()->getBounds();
		Ogre::Vector3 m = box.getMinimum();
		Ogre::Vector3 n = box.getMaximum();
		mBoundingBox->begin("ControletColor",Ogre::RenderOperation::OT_LINE_LIST);
		mBoundingBox->position(m.x,m.y,m.z);
		mBoundingBox->position(n.x,m.y,m.z);
		mBoundingBox->position(n.x,m.y,m.z);
		mBoundingBox->position(n.x,n.y,m.z);
		mBoundingBox->position(n.x,n.y,m.z);
		mBoundingBox->position(m.x,n.y,m.z);
		mBoundingBox->position(m.x,n.y,m.z);
		mBoundingBox->position(m.x,m.y,m.z);

		mBoundingBox->position(m.x,m.y,n.z);
		mBoundingBox->position(n.x,m.y,n.z);
		mBoundingBox->position(n.x,m.y,n.z);
		mBoundingBox->position(n.x,n.y,n.z);
		mBoundingBox->position(n.x,n.y,n.z);
		mBoundingBox->position(m.x,n.y,n.z);
		mBoundingBox->position(m.x,n.y,n.z);
		mBoundingBox->position(m.x,m.y,n.z);

		mBoundingBox->position(m.x,m.y,m.z);
		mBoundingBox->position(m.x,m.y,n.z);
		mBoundingBox->position(n.x,m.y,m.z);
		mBoundingBox->position(n.x,m.y,n.z);
		mBoundingBox->position(n.x,n.y,m.z);
		mBoundingBox->position(n.x,n.y,n.z);
		mBoundingBox->position(m.x,n.y,m.z);
		mBoundingBox->position(m.x,n.y,n.z);
		mBoundingBox->end();
		mNode->attachObject(mBoundingBox);
	}
}

void VisualObject::_updateBoundingBox()
{
	if( mBoundingBox )
	{
		_clearBoundingBox();
		_createBoundingBox();
	}
}

void VisualObject::Enable()
{
	if( mNode )
		mNode->setVisible(true);
	if( mGeom )
		dSpaceAdd(RigidManager::getSingleton().getSpaceID(),
		mGeom->getGeomID());
}

void VisualObject::Disable()
{
	if( mNode )
		mNode->setVisible(false);
	if( mGeom )
		dSpaceRemove(RigidManager::getSingleton().getSpaceID(),
		mGeom->getGeomID());
}

void VisualObject::load( MyGUI::xml::ElementPtr node )
{
	mName = node->findAttribute("name");
	mNode->setPosition( toVec3( node->findAttribute("position") ) );
	mNode->setOrientation( toQua( node->findAttribute("orientation") ) );
	MyGUI::xml::ElementEnumerator ee = node->getElementEnumerator();
	ObjectFactory& factory = ObjectFactory::getSingleton();
	while( ee.next() )
	{
		MyGUI::xml::ElementPtr geo = ee.current();

		if( factory.isExist(geo->getName()) )
		{
			ObjectPtr o = factory.createObject(geo->getName());
			if( o )
			{
				o->load( geo );
				mGeom = boost::dynamic_pointer_cast<GeometryObject>(o);
				if( mGeom )
					break;
			}
		}
	}
	_init();
}

void VisualObject::save( MyGUI::xml::ElementPtr node )
{
	if( !mName.empty() )
		node->addAttribute("name",mName);
	node->addAttribute("position",vec3toString( mNode->getPosition() ));
	node->addAttribute("orientation",quatoString(mNode->getOrientation()));
	MyGUI::xml::ElementPtr child = node->createChild(mGeom->getTypeName());
	mGeom->save( child );
}