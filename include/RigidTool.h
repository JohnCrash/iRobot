#ifndef _RIGIDTOOL_H_
#define _RIGIDTOOL_H_

#include "RigidControlet.h"
#include "VisualObject.h"

/*以前使用的Rigid，后来根据需要增加Object层。而名称没有改变
*/
class RigidTool
{
protected:
	boost::weak_ptr<VisualObject> mRigid;
};

/*在一个物体周围放置LinearControlet用来移动这个物体
*/
class MoveTool:public RigidTool
{
public:
	MoveTool();
	virtual ~MoveTool();
	virtual void controlRigid( const VisualObjectPtr& ptr,
		const Ogre::Node::TransformSpace relativeTo=Ogre::Node::TS_WORLD );
protected:
	void NotifyObjectMove( const string& name,const Ogre::Vector3& mv );

	void setRigidControlet( LinearControletPtr& lcpForward,
		LinearControletPtr& lcpBackward,
		const VisualObjectPtr& prigid,
		const Ogre::Vector3& dir );

	Ogre::Node::TransformSpace mRelative;
	//代表6个方向
	//0,1代表x轴正负,2,3 y,4,5 z
	LinearControletPtr mLinear[6];
};

typedef boost::shared_ptr<MoveTool> MoveToolPtr;

//让物体在一个平面上移动
class MoveTool2d:public RigidTool
{
public:
	MoveTool2d();
	virtual ~MoveTool2d();
	virtual void controlRigid( const VisualObjectPtr& ptr,const Ogre::Plane& p );
protected:
	void NotifyObjectMove( const string& name,const Ogre::Vector3& mv );

	LinearControletPtr mY; //一个向上的Y轴
	PlaneControletPtr mPC; //一个平面移动
};

typedef boost::shared_ptr<MoveTool2d> MoveTool2dPtr;

class RotateTool:public RigidTool
{
public:
	RotateTool();
	virtual ~RotateTool();
	virtual void controlRigid( const VisualObjectPtr& ptr );
protected:
	void NotifObjectRoate(const string& name,
		const Ogre::Vector3& axis,
		const Ogre::Real angle );
	//自旋
	RotateControletPtr mRotate;
	//调整自旋方向,一边一个
	OrientationControletPtr mOrient[2];
};

typedef boost::shared_ptr<RotateTool> RotateToolPtr;

/*修改刚体的外形工具，如调整球的大小，方盒的长宽高等
*/
class ModifyTool:public RigidTool
{
public:
	ModifyTool();
	virtual ~ModifyTool();
	virtual void controlRigid( const VisualObjectPtr& ptr );
};

typedef boost::shared_ptr<ModifyTool> ModifyToolPtr;

#endif