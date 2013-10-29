#ifndef _JOINT_H_
#define _JOINT_H_
#include "ClassInfo.h"

/*
	ODE关节
*/
class Rigid;

typedef boost::weak_ptr<Rigid> RigidWeakPtr;
typedef boost::shared_ptr<Rigid> RigidPtr;

class Joint:public Object
{
    friend class Framework;
    friend class Rigid;
public:
	Joint();
	virtual ~Joint();

	void Enable();
	void Disable();
	bool isEnabled() const;

	dJointType getJointType() const;

	void setJointData(void* data);
	void* getJointData();

	dJointID getJointID() const;
	//取得和该铰链连接的刚体
	RigidPtr getJointRigid( int i ) const;

	//断开连接
	void breakAllRigid();
    void breakRigid( RigidPtr p );
	/*
		建立相关的数据关联
	*/
	void linkRigid(RigidPtr b1,RigidPtr b2);
    
	virtual void load( MyGUI::xml::ElementPtr node );
	virtual void save( MyGUI::xml::ElementPtr node );

	RTTI_DERIVED(Joint);
protected:
	string mName;
    
	dJointID mJointID;
	/*和这个Joint相连接的刚体，当然ODE中已经有相关的信息
	这里这么做是为了数据结构的便利
	*/
	RigidPtr mRigid1;
	RigidPtr mRigid2;
};

typedef boost::shared_ptr<Joint> JointPtr;
typedef boost::weak_ptr<Joint> JointWeakPtr;
typedef vector<JointPtr> JointMap;

/*
	球形关节
*/
class JointBall:public Joint
{
public:
	JointBall();
	virtual void load( MyGUI::xml::ElementPtr node );
	virtual void save( MyGUI::xml::ElementPtr node );

	RTTI_DERIVED(JointBall);
	//在世界坐标系下设置锚点
	void setBallAnchor( const Ogre::Vector3& v3 );
	//返回锚点,n=0返回物体1的锚点.n=1返回物体2的锚点
	Ogre::Vector3 getBallAnchor( int n );
};

/*
	合页关节
*/
class JointHinge:public Joint
{
public:
	JointHinge();
	//设置合页旋转轴上的一点
	void setHingeAnchor( const Ogre::Vector3& v3 );
	Ogre::Vector3 getHingeAnchor();
	void setHingeAxis( const Ogre::Vector3& v3 );
	Ogre::Vector3 getHingeAxis();

	virtual void load( MyGUI::xml::ElementPtr node );
	virtual void save( MyGUI::xml::ElementPtr node );

	RTTI_DERIVED(JointHinge);
};

/*
	滑动关节
*/
class JointSlider:public Joint
{
public:
	JointSlider();
	virtual void load( MyGUI::xml::ElementPtr node );
	virtual void save( MyGUI::xml::ElementPtr node );

	RTTI_DERIVED(JointSlider);
};

void registerJointFactory();

#endif