#ifndef _GEOMETRYOBJECT_H_
#define _GEOMETRYOBJECT_H_
#include "RigidTool.h"
#include "ClassInfo.h"

class GeometryObject:public Object
{
public:
	GeometryObject();
	GeometryObject( int des,
		bool hasGeomID=false );

	virtual ~GeometryObject();

	//网格密度
	int getGeometryMeshDensity() const;
	void setGeometryMeshDensity( int des );
	
	void setMaterialName( const string& name );
	string getMaterialName() const;

	void setCastShadows( bool b );

	Ogre::Entity* getEntity() const;
	Ogre::MeshPtr getMesh() const;
	bool hasGeomID() const;
	dGeomID getGeomID() const;
	/*改变了几何参数，或者密度参数等。需要重新计算物体的质心
		质量，惯量等物理参数
	*/
	virtual void updateGeometry();
	/*改变了几何参数，或者密度参数等。需要重新计算物体的质心
		质量，惯量等物理参数
	*/
	virtual void updatePhysica( dBodyID body,dMass mass,dReal density );

	/*返回:轴矢量和自旋半径
	 该函数用于RotateTool用来旋转调整刚体 
	*/  
	virtual pair<Ogre::Ray,Ogre::Real> getSpinParam( const VisualObjectPtr op );
	/*用于修改几何对象的接口,op没有对象表示删除Controlet
	*/
	virtual void modifyControlet( const VisualObjectPtr op );

	virtual void load( MyGUI::xml::ElementPtr node );
	virtual void save( MyGUI::xml::ElementPtr node );

	RTTI_DERIVED(GeometryObject);
protected:
	string getUUID();
	void rebuildGeometry();
	void notifyObject();
	int mDensity; //网格密度

	bool mHasGeomID;
	dGeomID mGeomID;

	//材质名称
	string mMaterialName;

	Ogre::MeshPtr mMesh;
	Ogre::Entity* mEntity;

	boost::weak_ptr<VisualObject> mObject; //临时对象用在修改对象上

	//产生一个GeometryObject的名称
	static Ogre::NameGenerator gGeometryObjectNameGenerator;
};

//球
class Sphere :public GeometryObject
{
public:
	Sphere();
	Sphere(
		Ogre::Real r,
		int des,
		bool hasGeomID=false);
	virtual ~Sphere();

	virtual void updateGeometry();
	virtual void updatePhysica( dBodyID body,dMass mass,dReal density );

	virtual void modifyControlet( const VisualObjectPtr op );

	virtual pair<Ogre::Ray,Ogre::Real> getSpinParam( const VisualObjectPtr op );

	virtual void load( MyGUI::xml::ElementPtr node );
	virtual void save( MyGUI::xml::ElementPtr node );

	//设置半径
	Ogre::Real getRadius() const;
	void setRadius( Ogre::Real r );

	RTTI_DERIVED(Sphere);
protected:
	void _init();
	Ogre::Real mRadius; //半径

	RaduisControletPtr mRaduisControlet;

	void NotifObjectRaduis( const string& n,const Ogre::Real r);
};

//长方体
class Box :public GeometryObject
{
public:
	Box();
	Box(
		Ogre::Real v[3],
		int des,
		bool hasGeomID=false);
	virtual ~Box();

	virtual void updateGeometry();
	virtual void updatePhysica( dBodyID body,dMass mass,dReal density );

	//设置长宽高
	void setVolume( const Ogre::Real vol[3] );
	const Ogre::Real* getVolume() const;

	virtual void modifyControlet( const VisualObjectPtr op );

	virtual void load( MyGUI::xml::ElementPtr node );
	virtual void save( MyGUI::xml::ElementPtr node );

	RTTI_DERIVED(Box);
protected:
	void _init();
	Ogre::Real mVolume[3];

	LinearControletPtr mLinear[6];

	void NotifObjectLinear(const string& name,
		const Ogre::Vector3& mo);

	void setLinearControlet( LinearControletPtr& lcpForward,
									 LinearControletPtr& lcpBackward,
									 const VisualObjectPtr& prigid,
									 const Ogre::Vector3& dir );
};

//圆柱体
class Cylinder:public GeometryObject
{
public:
	Cylinder();
	Cylinder(
		Ogre::Real r,
		Ogre::Real l,
		int des,
		bool hasGeomID=false);
	virtual ~Cylinder();

	virtual void updateGeometry();
	virtual void updatePhysica( dBodyID body,dMass mass,dReal density );

	//设置半径和长度
	void setCylinder( Ogre::Real radius,Ogre::Real len );  
	Ogre::Real getRadius() const;
	Ogre::Real getLength() const;

	virtual pair<Ogre::Ray,Ogre::Real> getSpinParam( const VisualObjectPtr op );

	virtual void modifyControlet( const VisualObjectPtr op );

	virtual void load( MyGUI::xml::ElementPtr node );
	virtual void save( MyGUI::xml::ElementPtr node );

	RTTI_DERIVED(Cylinder);
protected:
	void _init();
	Ogre::Real mRadius;
	Ogre::Real mLength;

	RaduisControletPtr mRaduisControlet;
	LinearControletPtr mLinear[2];

	void NotifObjectLinear(const string& name,
		const Ogre::Vector3& mo);
	void NotifObjectRaduis( const string& n,const Ogre::Real r);
};

//平面
class Quad:public GeometryObject
{
public:
	Quad();
	Quad( const Ogre::Plane& plane,
		Ogre::Real width,
		Ogre::Real height,
		int xseg,
		int yseg,
		bool hasGeomID=false );
	virtual ~Quad();

	virtual void updateGeometry();

	virtual void load( MyGUI::xml::ElementPtr node );
	virtual void save( MyGUI::xml::ElementPtr node );

	RTTI_DERIVED(Quad);

	void getQuad( Ogre::Plane& plane,Ogre::Real& width,
		Ogre::Real& height,int& xseg,int& yseg);
	void setQuad( const Ogre::Plane& plane,
		Ogre::Real width,Ogre::Real height,
		int xseg,int yseg );
protected:
	void _init();
	Ogre::Plane mPlane;
	Ogre::Real mWidth;
	Ogre::Real mHeight;
	int mXSegments;
	int mYSegments;
	/* Convex描述
	void _updateConvex();
	//描述一个物理的矩形面
	dReal mPlanes[4]; //面的法矢量，和距离
	dReal mPoints[12]; //4个点x,y,z
	unsigned int mPolygons[5]; //结构如下(顶点数|1号顶点|2号顶点|...)
	*/
};

//注册几何对象
void registerGeometryFactory();

#endif