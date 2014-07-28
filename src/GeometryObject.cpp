#include "stdheader.h"
#include "Math3d.h"
#include "Geometry.h"
#include "Game.h"
#include "VisualObject.h"
#include "GeometryObject.h"
#include "StringUtility.h"
#include "ObjectFactory.h"

Ogre::NameGenerator GeometryObject::gGeometryObjectNameGenerator("G");

string GeometryObject::getUUID()
{
	return gGeometryObjectNameGenerator.generate();
}

GeometryObject::GeometryObject():
		mDensity(Geometry::DEFAULT_DENSITY),
		mHasGeomID(false),
		mEntity(nullptr)
{
}

GeometryObject::GeometryObject( int des,
							   bool hasGeomID ):
		mDensity(des),
		mHasGeomID(hasGeomID)
{
}

void GeometryObject::updatePhysica( dBodyID body,dMass mass,dReal density )
{
}

GeometryObject::~GeometryObject()
{
	if( mHasGeomID )
		dGeomDestroy(mGeomID);

	Game::getSingleton().getSceneManager()->destroyEntity( mEntity);
	Geometry::getSingleton().destroyGemetry( mMesh );
}

int GeometryObject::getGeometryMeshDensity() const
{
	return mDensity;
}

void GeometryObject::updateGeometry()
{
}

void GeometryObject::notifyObject()
{
	VisualObjectPtr obj = mObject.lock();
	if( obj )obj->updatePhysica();
}

void GeometryObject::setGeometryMeshDensity( int des )
{
	mDensity = des;
}

Ogre::Entity* GeometryObject::getEntity() const
{
	return mEntity;
}

Ogre::MeshPtr GeometryObject::getMesh() const
{
	return mMesh;
}

void GeometryObject::modifyControlet( const VisualObjectPtr op )
{
}

pair<Ogre::Ray,Ogre::Real> GeometryObject::getSpinParam( const VisualObjectPtr op ){
	/*一般情况下使用较长的那个方向为轴
	*/
	using namespace Ogre;
	const AxisAlignedBox& box = mMesh->getBounds();
	const Vector3& mi = box.getMinimum();
	const Vector3& ma = box.getMaximum();

	Vector3 o = op->getPosition() + box.getCenter();
	Matrix3 mat3 = op->getLocalAxis();
	if( std::abs(ma.x-mi.x) > std::abs(ma.y-mi.y) &&
		std::abs(ma.x-mi.x) > std::abs(ma.z-mi.z) ){
		//local x 轴是自旋轴
		Real r = Math::Sqrt( (ma.y-mi.y)*(ma.y-mi.y)+(ma.z-mi.z)*(ma.z-mi.z) )/2;
		
		return pair<Ray,Real>(Ray( o, mat3.GetColumn(0) * std::abs(ma.x-mi.x)/2 ),r );
	}else if( std::abs(ma.y-mi.y) > std::abs(ma.z-mi.z) ){
		//local y 轴是自旋轴
		Real r = Math::Sqrt( (ma.x-mi.x)*(ma.x-mi.x)+(ma.z-mi.z)*(ma.z-mi.z) )/2;
		return pair<Ray,Real>( Ray( o, mat3.GetColumn(1) * std::abs(ma.y-mi.y)/2 ),r );
	}else{
		Real r = Math::Sqrt( (ma.x-mi.x)*(ma.x-mi.x)+(ma.y-mi.y)*(ma.y-mi.y) )/2;
		return pair<Ray,Real>( Ray( o, mat3.GetColumn(2) * std::abs(ma.z-mi.z)/2 ),r );
	}
}

void GeometryObject::setCastShadows( bool b )
{
	mEntity->setCastShadows( b );
}

void GeometryObject::setMaterialName( const string& name ){
	mMaterialName = name;
	mEntity->setMaterialName( name );
}

string GeometryObject::getMaterialName() const
{
	return mMaterialName;
}

bool GeometryObject::hasGeomID() const
{
	return mHasGeomID;
}

dGeomID GeometryObject::getGeomID() const
{
	return mGeomID;
}

//重建几何
void GeometryObject::rebuildGeometry(){
	mMesh->reload();
	/*如果Mesh重新装载的话，需要重新初始化Entity
		不然你重新设置材质setMaterialName，稍后在绘制Entity时
		还会调用_initialise导致材质丢失
	*/
 	mEntity->_initialise(true);
	mEntity->setMaterialName( mMaterialName );
}

void GeometryObject::load( MyGUI::xml::ElementPtr node )
{
	mDensity = toValue( node->findAttribute("density"),Geometry::DEFAULT_DENSITY);
	mMaterialName = node->findAttribute("material");
	mHasGeomID = toValue(node->findAttribute("hasGeomID"),false);
}

void GeometryObject::save( MyGUI::xml::ElementPtr node )
{
	node->addAttribute("density",mDensity);
	if( !mMaterialName.empty() )
		node->addAttribute("material",mMaterialName);
	node->addAttribute("hasGeomID",toString(mHasGeomID));
}

Sphere::Sphere()
{
}

Sphere::Sphere(Ogre::Real r,int des,bool hasGeomID):
	GeometryObject(des,hasGeomID),
	mRadius(r)
{
	_init();
}

void Sphere::_init()
{
	string name = getUUID();
	mMesh = Geometry::getSingleton().createSphere( name,mRadius,mDensity );
	mEntity = Game::getSingleton().getSceneManager()->createEntity( name,name );

	if( mHasGeomID )
		mGeomID = dCreateSphere(0,mRadius);

	mEntity->setMaterialName(mMaterialName);
}

Sphere::~Sphere()
{
}

void Sphere::updateGeometry(){
	Geometry::Param* p=Geometry::getSingleton().getGeometryParam( mMesh );
	if( p ){
		p->density = mDensity;
		p->data = Geometry::tSphere(mRadius);
		
		if( mHasGeomID )
			dGeomSphereSetRadius(mGeomID,mRadius );

		rebuildGeometry();
	}
}

pair<Ogre::Ray,Ogre::Real> Sphere::getSpinParam( const VisualObjectPtr op ){
	Ogre::Matrix3 mat3 = op->getLocalAxis();
	return pair<Ogre::Ray,Ogre::Real>( 
		Ogre::Ray(op->getPosition(),mat3.GetColumn(2)*mRadius),
		mRadius
		);
}

void Sphere::modifyControlet( const VisualObjectPtr op )
{
	mObject = op;
	if( op ){
		Ogre::Matrix3 mat3 = op->getLocalAxis();
		mRaduisControlet.reset( new RaduisControlet("_sphereRaduis",
			op->getPosition(),
			mat3.GetColumn(2),
			mRadius,
			boost::bind( &Sphere::NotifObjectRaduis,this,_1,_2 ) 
			) );
	}else{
		mRaduisControlet.reset();
	}
}

void Sphere::updatePhysica( dBodyID body,dMass mass,dReal density )
{
	dMassSetSphere(&mass,density,mRadius);
	dMassAdjust(&mass,1);
	dBodySetMass(body,&mass);
}

void Sphere::NotifObjectRaduis( const string& n,const Ogre::Real r )
{
	setRadius( r );
	updateGeometry();
	notifyObject();
}

void Sphere::load( MyGUI::xml::ElementPtr node )
{
	GeometryObject::load( node );
	mRadius = toValue( node->findAttribute("radius"),(Ogre::Real)1 );
	_init();
}

void Sphere::save( MyGUI::xml::ElementPtr node )
{
	GeometryObject::save( node );
	node->addAttribute("radius",toString(mRadius) );
}

Ogre::Real Sphere::getRadius() const
{
	return mRadius;
}

void Sphere::setRadius( Ogre::Real r )
{
	mRadius = r;
}

Box::Box():
GeometryObject()
{
}

Box::Box(Ogre::Real v[3],
		int des,
		bool hasGeomID):
		GeometryObject(des,hasGeomID)
{
	setVolume( v );
	_init();
}

void Box::_init()
{
	string name = getUUID();
	mMesh = Geometry::getSingleton().createBox( name,mVolume,mDensity );
	mEntity = Game::getSingleton().getSceneManager()->createEntity( name,name );

	if( mHasGeomID )
		mGeomID = dCreateBox(0,mVolume[0],mVolume[1],mVolume[2]);

	mEntity->setMaterialName(mMaterialName);
}

Box::~Box()
{
}

void Box::updatePhysica( dBodyID body,dMass mass,dReal density )
{
	dMassSetBox(&mass,density,mVolume[0],mVolume[1],mVolume[2]);
	dMassAdjust(&mass,1);
	dBodySetMass(body,&mass);
}

void Box::modifyControlet( const VisualObjectPtr op )
{
	mObject = op;
	if( op ){
		for( int i = 0;i < 6;++i )
		{
			string name("_boxModifytool");
			name += boost::lexical_cast<string>( i );
			mLinear[i].reset(new LinearControlet( name,
				Ogre::Vector3(0,0,50),
				Ogre::Vector3::NEGATIVE_UNIT_Z,
				boost::bind( &Box::NotifObjectLinear,this,_1,_2 )
				));
		}
		Ogre::Matrix3 mat3 = op->getLocalAxis();
		setLinearControlet( mLinear[0],mLinear[1],op,mat3.GetColumn(0) );
		setLinearControlet( mLinear[2],mLinear[3],op,mat3.GetColumn(1) );
		setLinearControlet( mLinear[4],mLinear[5],op,mat3.GetColumn(2) );
	}else{
		for( int i = 0;i<6;++i )
			mLinear[i].reset();
	}
}

void Box::NotifObjectLinear(const string& name,
	const Ogre::Vector3& move)
{
	int i,n;

	VisualObjectPtr obj = mObject.lock();
	if( obj )
	{
		Ogre::Matrix3 mat3 = obj->getLocalAxis();
		Ogre::Vector3 vz3;
		Ogre::Vector3 v3 = obj->getPosition();
		Ogre::Real delta = move.length()*2;
		//这种情况下name=_boxModifytool?,?是一个0-5的数
		if( name.size() < 1 ){
			ERROR_LOG( "name.size() < 1" )
			return;
		}
		try{
			n = boost::lexical_cast<int>(name.substr( name.size()-1,1 ));
		}catch( boost::bad_lexical_cast& e ){
			ERROR_LOG( e.what() )
			return;
		}
		if( n == 0 || n == 1 ) //x轴
			i = 0;
		else if( n == 2 || n == 3 ) //y轴
			i = 1;
		else if( n == 4 || n == 5 )
			i = 2;
		else{
			ERROR_LOG( n )
			return;
		}
		if( vz3.dotProduct(move) < 0 ){//同方向
			if( n%2 )
				mVolume[i] += delta;
			else
				mVolume[i] -= delta;
		}else{//反方向
			if( n%2 )
				mVolume[i] -= delta;
			else
				mVolume[i] += delta;
		}
		//做一个限制
		if( mVolume[i]<1 )mVolume[i] = 1;
		
		updateGeometry();
		
		notifyObject();
		//重新调整控制点位置
		setLinearControlet( mLinear[0],mLinear[1],obj,mat3.GetColumn(0) );
		setLinearControlet( mLinear[2],mLinear[3],obj,mat3.GetColumn(1) );
		setLinearControlet( mLinear[4],mLinear[5],obj,mat3.GetColumn(2) );
	}
}

void Box::setLinearControlet( LinearControletPtr& lcpForward,
								 LinearControletPtr& lcpBackward,
								 const VisualObjectPtr& prigid,
								 const Ogre::Vector3& dir )
{
	Ogre::Ray ray;
	vector<Ogre::Real> result;
	Ogre::Real width;
	ray.setOrigin( prigid->getPosition() );
	ray.setDirection( dir );
	result = prigid->Intersect( ray,true );
	if( !result.empty() ){
		width = result[result.size()-1]-result[0];
		lcpForward->setLinear(Math3d::RayPoint( ray,result[0] ),dir);
		lcpForward->setLimit(-width/2,Ogre::Math::POS_INFINITY );
		lcpBackward->setLinear(Math3d::RayPoint( ray,result[result.size()-1] ),-dir);
		lcpBackward->setLimit(-width/2,Ogre::Math::POS_INFINITY );
	}
}

void Box::updateGeometry()
{
	Geometry::Param* p=Geometry::getSingleton().getGeometryParam( mMesh );
	if( p ){
		p->density = mDensity;
		p->data = Geometry::tBox(mVolume);

		if( mHasGeomID )
			dGeomBoxSetLengths( mGeomID,mVolume[0],mVolume[1],mVolume[2]);
	
		rebuildGeometry();
	}
}

void Box::setVolume( const Ogre::Real vol[3] )
{
	mVolume[0] = vol[0];
	mVolume[1] = vol[1];
	mVolume[2] = vol[2];
}

void Box::load( MyGUI::xml::ElementPtr node )
{
	GeometryObject::load( node );
	mVolume[0] = toValue(node->findAttribute("width"),(Ogre::Real)1.0);
	mVolume[1] = toValue(node->findAttribute("height"),(Ogre::Real)1.0);
	mVolume[2] = toValue(node->findAttribute("lenght"),(Ogre::Real)1.0);
	_init();
}

void Box::save( MyGUI::xml::ElementPtr node )
{
	GeometryObject::save( node );
	node->addAttribute("width",toString(mVolume[0]));
	node->addAttribute("height",toString(mVolume[1]));
	node->addAttribute("lenght",toString(mVolume[2]));
}

const Ogre::Real* Box::getVolume() const
{
	return mVolume;
}

Cylinder::Cylinder():
GeometryObject()
{
}

Cylinder::Cylinder(Ogre::Real r,
				   Ogre::Real l,
				   int des,
				   bool hasGeomID):
	GeometryObject(des,hasGeomID)
{
	setCylinder( r,l );
	_init();
}

void Cylinder::_init()
{
	string name = getUUID();
	mMesh = Geometry::getSingleton().createCylinder( name,mRadius,mLength,mDensity );
	mEntity = Game::getSingleton().getSceneManager()->createEntity( name,name );

	if( mHasGeomID )
		mGeomID = dCreateCylinder(0,mRadius,mLength);

	mEntity->setMaterialName(mMaterialName);
}

Cylinder::~Cylinder()
{
}

void Cylinder::updatePhysica( dBodyID body,dMass mass,dReal density )
{
	dMassSetCylinder(&mass,density,2,mRadius,mLength);
	dMassAdjust(&mass,1);
	dBodySetMass(body,&mass);
}

void Cylinder::updateGeometry()
{
	Geometry::Param* p=Geometry::getSingleton().getGeometryParam( mMesh );
	if( p ){
		p->density = mDensity;
		p->data = Geometry::tCylinder(mRadius,mLength);
	
		if( mHasGeomID )
			dGeomCylinderSetParams( mGeomID,mRadius,mLength );

		rebuildGeometry();
	}
}

pair<Ogre::Ray,Ogre::Real> Cylinder::getSpinParam( const VisualObjectPtr op )
{
	using namespace Ogre;
	const AxisAlignedBox& box = mMesh->getBounds();
	Vector3 o = op->getPosition() + box.getCenter();
	Matrix3 mat3 = op->getLocalAxis();
	return pair<Ray,Real>( Ray(o,mat3.GetColumn(2)*mLength/2),mRadius );
}

void Cylinder::modifyControlet( const VisualObjectPtr op )
{
	mObject = op;
	if( op ){
		Ogre::Matrix3 mat3 = op->getLocalAxis();

		Ogre::Vector3 v3 = op->getPosition();

		mRaduisControlet.reset( new RaduisControlet("_cylinderRaduis",
			v3,
			mat3.GetColumn(2),
			mRadius,
			boost::bind( &Cylinder::NotifObjectRaduis,this,_1,_2 ) 
			) );
		//调整长度
		mLinear[0].reset( new LinearControlet("_cylinderLength0",
			v3+mat3.GetColumn(2)*mLength/2,
			-mat3.GetColumn(2),
			boost::bind( &Cylinder::NotifObjectLinear,this,_1,_2)
			));
		mLinear[0]->setLimit( -mLength/2,Ogre::Math::POS_INFINITY );
		mLinear[1].reset( new LinearControlet("_cylinderLength1",
			v3-mat3.GetColumn(2)*mLength/2,
			mat3.GetColumn(2),
			boost::bind( &Cylinder::NotifObjectLinear,this,_1,_2)
			));
		mLinear[1]->setLimit( -mLength/2,Ogre::Math::POS_INFINITY );
	}else{
		mRaduisControlet.reset();
		mLinear[0].reset();
		mLinear[1].reset();
	}
}

void Cylinder::NotifObjectLinear(const string& name,
	const Ogre::Vector3& move)
{
	VisualObjectPtr obj = mObject.lock();
	if( obj )
	{
		Ogre::Real delta;
		Ogre::Matrix3 mat3 = obj->getLocalAxis();
		Ogre::Vector3 vz3;
		Ogre::Vector3 v3 = obj->getPosition();
		delta = move.length()*2;
		if( name=="_cylinderLength0" ){
			vz3 = -mat3.GetColumn(2);
			//调整_length1的位置
			mLinear[1]->setLinear( 
				v3-mat3.GetColumn(2)*mLength/2,
				mat3.GetColumn(2) );
			mLinear[1]->setLimit( -mLength/2,Ogre::Math::POS_INFINITY );
		}else if( name=="_cylinderLength1" ){
			vz3 = mat3.GetColumn(2);
			//调整_length0的位置
			mLinear[0]->setLinear( 
				v3+mat3.GetColumn(2)*mLength/2,
				-mat3.GetColumn(2) );
			mLinear[0]->setLimit( -mLength/2,Ogre::Math::POS_INFINITY );
		}
		if( vz3.dotProduct(move) < 0 ){//同方向
			mLength += delta;
		}else{//反方向
			mLength -= delta;
		}
		//做一个限制
		if( mLength<1 )mLength = 1;

		updateGeometry();
		
		notifyObject();
	}
}

void Cylinder::NotifObjectRaduis( const string& n,const Ogre::Real r)
{
	mRadius = r;
	updateGeometry();
		
	notifyObject();
}

void Cylinder::setCylinder( Ogre::Real radius,Ogre::Real len )
{
	mRadius = radius;
	mLength = len;
}

void Cylinder::load( MyGUI::xml::ElementPtr node )
{
	GeometryObject::load( node );
	mRadius = toValue(node->findAttribute("radius"),(Ogre::Real)1);
	mLength = toValue(node->findAttribute("length"),(Ogre::Real)1);
	_init();
}

void Cylinder::save( MyGUI::xml::ElementPtr node )
{
	GeometryObject::save( node );
	node->addAttribute("radius",toString(mRadius));
	node->addAttribute("length",toString(mLength));
}

Ogre::Real Cylinder::getRadius() const
{
	return mRadius;
}

Ogre::Real Cylinder::getLength() const
{
	return mLength;
}

Quad::Quad()
:GeometryObject()
{
}

Quad::Quad( const Ogre::Plane& plane,
	Ogre::Real width,
	Ogre::Real height,
	int xseg,
	int yseg,
	bool hasGeomID ):
	GeometryObject(Geometry::DEFAULT_DENSITY,hasGeomID)
{
	setQuad( plane,width,height,xseg,yseg );
	_init();
}

void Quad::_init()
{
	string name = getUUID();
	mMesh = Ogre::MeshManager::getSingleton().createPlane( name,
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			mPlane,mWidth,mHeight,mXSegments,mYSegments,
			true,1,8,8,Ogre::Vector3::UNIT_Z );

	mEntity = Game::getSingleton().getSceneManager()->createEntity( name,name );

	if( mHasGeomID )
	{
		//FixMe 暂时使用一个薄薄的盒子代替
		mGeomID = dCreateBox( 0,mWidth,1,mHeight );
//		_updateConvex();
//		mGeomID = dCreateConvex( 0,mPlanes,1,mPoints,4,mPolygons );
	}
	mEntity->setMaterialName(mMaterialName);
}

Quad::~Quad()
{
}
/*
void Quad::_updateConvex()
{
	mPlanes[0] = mPlane.normal.x;
	mPlanes[1] = mPlane.normal.y;
	mPlanes[2] = mPlane.normal.z;
	mPlanes[3] = mPlane.d;

	//这里我只要保持和createPlane出来的平面保持一直就好
	//参加OgreMeshManager.cpp中的loadManualPlane函数
    Ogre::Matrix4 xlate, xform, rot;
    Ogre::Matrix3 rot3;
	Ogre::Vector3 vec;

	xlate = rot = Ogre::Matrix4::IDENTITY;
    // Determine axes
	Ogre::Vector3 zAxis, yAxis, xAxis;
    zAxis = mPlane.normal;
    zAxis.normalise();
    yAxis = Ogre::Vector3::UNIT_Z;
    yAxis.normalise();
    xAxis = yAxis.crossProduct(zAxis);
    if (xAxis.length() == 0)
    {
		WARNING_LOG("The upVector you supplied is parallel to the plane normal, so is not valid.");
		return;
    }

    rot3.FromAxes(xAxis, yAxis, zAxis);
    rot = rot3;

    // Set up standard xform from origin
    xlate.setTrans(mPlane.normal * -mPlane.d);

    // concatenate
    xform = xlate * rot;
	for( int i = 0;i<4;++i )
	{
		switch(i)
		{
		case 0:
			vec.x = mWidth;
			vec.y = mHeight;
			break;
		case 1:
			vec.x = -mWidth;
			vec.y = mHeight;
			break;
		case 2:
			vec.x = -mWidth;
			vec.y = -mHeight;
			break;
		case 3:
			vec.x = mWidth;
			vec.y = -mHeight;
			break;
		}
		vec.z = 0.0f;
        // Transform by orientation and distance
        vec = xform.transformAffine(vec);
		mPoints[3*i] = vec.x;
		mPoints[3*i+1] = vec.y;
		mPoints[3*i+2] = vec.z;
	}

	mPolygons[0] = 4; //4个顶点
	mPolygons[1] = 0;
	mPolygons[2] = 1;
	mPolygons[3] = 2;
	mPolygons[4] = 3;

}
*/
void Quad::updateGeometry()
{
	string name = mMesh->getName();
	Ogre::MeshManager::getSingleton().remove( name );

	mMesh = Ogre::MeshManager::getSingleton().createPlane( name,
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		mPlane,mWidth,mHeight,mXSegments,mYSegments,
		true,1,8,8,Ogre::Vector3::UNIT_Z );
	
	mEntity->setMaterialName( mMaterialName );

	if( mHasGeomID )
	{
	//	_updateConvex();
	//	dGeomSetConvex( mGeomID,mPlanes,1,mPoints,4,mPolygons );
		dGeomBoxSetLengths( mGeomID,mWidth,1,mHeight);
	}
}

void Quad::setQuad( const Ogre::Plane& plane,
	Ogre::Real width,Ogre::Real height,
	int xseg,int yseg )
{
	mPlane = plane;
	mWidth = width;
	mHeight = height;
	mXSegments = xseg;
	mYSegments = yseg;
}

void Quad::getQuad( Ogre::Plane& plane,Ogre::Real& width,
		Ogre::Real& height,int& xseg,int& yseg)
{
	plane = mPlane;
	width = mWidth;
	height = mHeight;
	xseg = mXSegments;
	yseg = mYSegments;
}

void Quad::load( MyGUI::xml::ElementPtr node )
{
	GeometryObject::load( node );
	mPlane.normal.x = toValue(node->findAttribute("a"),(Ogre::Real)0);
	mPlane.normal.y = toValue(node->findAttribute("b"),(Ogre::Real)0);
	mPlane.normal.z = toValue(node->findAttribute("c"),(Ogre::Real)1);
	mPlane.d = toValue(node->findAttribute("d"),(Ogre::Real)0);
	mWidth = toValue(node->findAttribute("width"),(Ogre::Real)1);
	mHeight = toValue(node->findAttribute("height"),(Ogre::Real)1);
	mXSegments = toValue(node->findAttribute("xsegment"),8);
	mYSegments = toValue(node->findAttribute("ysegment"),8);
	_init();
}

void Quad::save( MyGUI::xml::ElementPtr node )
{
	GeometryObject::save( node );
	node->addAttribute("a",toString(mPlane.normal.x));
	node->addAttribute("b",toString(mPlane.normal.y));
	node->addAttribute("c",toString(mPlane.normal.z));
	node->addAttribute("d",toString(mPlane.d));
	node->addAttribute("width",toString(mWidth));
	node->addAttribute("height",toString(mHeight));
	node->addAttribute("xsegment",toString(mXSegments));
	node->addAttribute("ysegment",toString(mYSegments));
}

void registerGeometryFactory()
{
	ObjectFactory& factory = ObjectFactory::getSingleton();
	factory.registerFactory<Box>("Box");
	factory.registerFactory<Sphere>("Sphere");
	factory.registerFactory<Cylinder>("Cylinder");
	factory.registerFactory<Quad>("Quad");
}