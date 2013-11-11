#ifndef _RIGID_H_
#define _RIGID_H_

#include "VisualObject.h"
#include "Joint.h"

class RigidManager;

typedef vector<RigidPtr> RigidMap;

//刚体首先是一个碰撞体
class Rigid
	:public VisualObject
{
	friend class Joint;
    friend class Framework;
public:
	Rigid();
	Rigid(GeometryObjectPtr geo);

	virtual ~Rigid();
	/*更新物理属性，如质量，惯量等等
	*/
	virtual void updatePhysica();

	virtual void setPosition(const Ogre::Vector3& v);
	//对刚体进行平移
	virtual void translate( const Ogre::Vector3& move );
	//对刚体进行旋转
	virtual void rotate( const Ogre::Vector3& axis,const Ogre::Real angle );

	virtual void Enable();
	virtual void Disable();

	//将updateGeometry和updatePhysica结合起来
	void updateRigid();

	/*由RigidManager调用，ODE模拟结束。将ODE数据
		更新到Node上以得到视觉化模拟结果
	*/
	virtual void _update();

	//和所有的铰链Joint断开,使得对方也不引用我
	void breakAllJoint();
    void breakJoint(Joint* jp);
    
	dBodyID getBodyID() const;

	//遍历Joint
	int getJointsCount(); //返回和该刚体相连接的铰链数目
	Joint* getJointAt( int i );

	virtual void load( MyGUI::xml::ElementPtr node );
	virtual void save( MyGUI::xml::ElementPtr node );

	RTTI_DERIVED(Rigid);
protected:
	void _init();
	void _visual2physic();

	/*下面是物体的物理属性
	*/
	dBodyID mBodyID;
	dMass mMass;
	dReal mMassDensity; //质量密度
	/*和该Rigid相连的Joint，当然ODE中已经有相关的信息
		这里这么做是为了数据结构的便利
	*/
	JointVec mJoints;
};

void registerCoreObject();

#endif
