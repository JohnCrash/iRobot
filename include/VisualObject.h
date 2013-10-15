#ifndef _VISUAL_OBJECT_H_
#define _VISUAL_OBJECT_H_
#include "ClassInfo.h"

class GeometryObject;
class RigidManager;

typedef boost::shared_ptr<GeometryObject> GeometryObjectPtr;

/*一个接口，主要用于控制可以控制的对象
*/
class VisualObject:public Object
{
public:
	VisualObject();

	VisualObject(GeometryObjectPtr geo);

	virtual ~VisualObject();
	
	void setMaterialName( const string& name );
	string getMaterialName() const;

	/* 一般修改了刚体的几何参数后被调用调用
	*/
	virtual void updatePhysica();

	//取得节点原点
	virtual Ogre::Vector3 getPosition() const;
	/*取得节点坐标轴在世界坐标中的向量
	*/
	virtual Ogre::Matrix3 getLocalAxis() const;

	virtual void setPosition(const Ogre::Vector3& v);
	//对刚体进行平移
	virtual void translate( const Ogre::Vector3& move );
	//对刚体进行旋转
	virtual void rotate( const Ogre::Vector3& axis,const Ogre::Real angle );
	
	virtual void Enable();
	virtual void Disable();

	//没有实现，详细信息见Rigid
	virtual void _update(){}

	string getName() const;
	void setName( const string name );
	/*用于移动工具，在物体周围安放移动控制器
	*/
	//一条射线和刚体的交点
	virtual vector<Ogre::Real> Intersect( const Ogre::Ray& ray,bool bSort );

	//设置刚体施放投射阴影，默认投射阴影
	void setCastShadows( bool b );

	//取得几何对象
	GeometryObjectPtr getGeometryObject() const;

	//显示或者关闭包围盒
	void showBoundingBox( bool b );

	string getEntityName() const;

	virtual void load( MyGUI::xml::ElementPtr node );
	virtual void save( MyGUI::xml::ElementPtr node );

	RTTI_DERIVED(VisualObject);
protected:
	void _init();
	/*一个求交算法的内部结构
	*/
	class VisitRend:
		public Ogre::Renderable::Visitor
	{
	public:
		VisitRend(const Ogre::Ray& ray );
		vector<Ogre::Real> mResult;
	protected:
		Ogre::Ray mRay;
		virtual void visit(Ogre::Renderable *rend, Ogre::ushort lodIndex, bool isDebug, Ogre::Any *pAny);
	};

	void _clearBoundingBox();
	void _createBoundingBox();
	void _updateBoundingBox();

	Ogre::SceneNode* mNode;
	GeometryObjectPtr mGeom;

	bool bShowBoundingBox;
	Ogre::ManualObject* mBoundingBox;

	string mName; //名称
};

typedef boost::shared_ptr<VisualObject> VisualObjectPtr;

#endif