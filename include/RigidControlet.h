#ifndef _RIGIDCONTROLET_H_
#define _RIGIDCONTROLET_H_

#include "Controlet.h"

class RigidControlet:
	public Controlet
{
public:
	RigidControlet(const string& name,Ogre::Real s);
	virtual ~RigidControlet();

	virtual void setVisible( bool b );
protected:
	virtual Ogre::MeshPtr getMesh(){ return Ogre::MeshPtr(); }
	virtual void rebuildEntity();
	virtual void createEntity(); //用来初始化mNode,mEntity
	virtual void destroyEntity();//用来施放mNode,mEntity

	virtual void mouseFocus( int x,int y );
	virtual void injectMousePress(int _absx, int _absy, MyGUI::MouseButton _id);
	virtual void injectMouseRelease(int _absx, int _absy, MyGUI::MouseButton _id);

	string mName; //名称
	bool mPick;
	int mMouseX;
	int mMouseY;
	Ogre::Real mScale;

	Ogre::SceneNode* mNode;
	Ogre::Entity* mEntity;
	Ogre::MeshPtr mMesh;
	bool bSelect;
};

/*这里直接使用回调函数是最快的
	或者使用一个抽象类界面回调
*/
typedef boost::function<void (const string&,const Ogre::Vector3&)> LinearNotifyEvent;

/*用来控制在一个直线上的平移
*/
class LinearControlet:
	public RigidControlet
{
public:
	LinearControlet( const string& name,
		const Ogre::Vector3& pos, //位置
		const Ogre::Vector3& dir, //方向
		LinearNotifyEvent func,
		Ogre::Real scale = 10.0f//箭头的缩放比例
		);
	virtual ~LinearControlet();
	//调整LinearControlet,p,dir构成一条射线
	void setLinear( const Ogre::Vector3& p,
		const Ogre::Vector3& dir,
		const Ogre::Real s = 10.f );
	//设置在直线上的区间限制,mi,ma是最小和最大参数
	void setLimit( Ogre::Real mi,Ogre::Real ma );
protected:
	//继承者可以通过重写下面函数来修改箭头外观
	virtual Ogre::MeshPtr getMesh();

	virtual void injectMouseMove(int _absx, int _absy, int _absz);

	LinearNotifyEvent mNotify;

	Ogre::Vector3 mPosition;
	Ogre::Vector3 mDirection;
	Ogre::Real mLimited[2]; //0最小，1最大
};

typedef boost::shared_ptr<LinearControlet> LinearControletPtr;

typedef boost::function<void (const string&,const Ogre::Vector3&,const Ogre::Real)> RotateNotifyEvent;

/*用来控制一个旋转
*/
class RotateControlet:
	public RigidControlet
{
public:
	RotateControlet( const string& name,
		const Ogre::Vector3& pos,//旋转中心点
		const Ogre::Vector3& axis, //旋转轴
		const Ogre::Real raduis, //半径
		RotateNotifyEvent func,
		Ogre::Real scale = 2.0f
		);
	virtual ~RotateControlet();
	//调整RotateControlet,o是原点,ax是旋转轴,r是半径
	void setRotate( const Ogre::Vector3& o,
		const Ogre::Vector3& ax,
		const Ogre::Real r,
		const Ogre::Real s = 2.0f );
protected:
	virtual Ogre::MeshPtr getMesh();
	virtual void injectMouseMove(int _absx, int _absy, int _absz);

	Ogre::Real mRaduis;
	RotateNotifyEvent mNotify;
};

typedef boost::shared_ptr<RotateControlet> RotateControletPtr;

typedef boost::function<void (const string&,const Ogre::Vector3&,const Ogre::Real)>
			OrientationNotifyEvent;
/* 控制方位，类似一个摇杆
*/
class OrientationControlet:
	public RigidControlet
{
public:
	OrientationControlet( const string& name,
		const Ogre::Vector3& pos,//旋转中心点
		const Ogre::Vector3& axis, //旋转轴，同时其长度表示控制柄的长度
		OrientationNotifyEvent func,
		Ogre::Real scale = 3.0f 
		);
	virtual ~OrientationControlet();
	//调整OrientationControlet
	void setOrient( const Ogre::Vector3& o,
		const Ogre::Vector3& ax,//旋转轴，同时其长度表示控制柄的长度
		const Ogre::Real s = 3.0f );
protected:
	virtual Ogre::MeshPtr getMesh();
	virtual void injectMousePress(int _absx, int _absy, MyGUI::MouseButton _id);
	virtual void injectMouseMove(int _absx, int _absy, int _absz);
	virtual void createEntity(); 
	virtual void destroyEntity();

	Ogre::SceneNode* mNodeLocal;
	Ogre::Real mRaduis;
	OrientationNotifyEvent mNotify; 
	bool mNearFar;
};

typedef boost::shared_ptr<OrientationControlet> OrientationControletPtr;

/* 一个半径调节器，如调节球，和柱体的半径。
	它像一个可调节的圆箍
*/
typedef boost::function<void (const string&,const Ogre::Real)>
			RaduisNotifyEvent;

class RaduisControlet:
	public RigidControlet
{
public:
	RaduisControlet( const string& name,
		const Ogre::Vector3& pos,//旋转中心点
		const Ogre::Vector3& axis, //旋转轴
		const Ogre::Real raduis, //半径
		RaduisNotifyEvent func,
		Ogre::Real scale = 2.0f
		);
	virtual ~RaduisControlet();
	//调整RaduisControlet,o是原点,ax是旋转轴,r是半径
	void setRaduis( const Ogre::Vector3& o,
		const Ogre::Vector3& ax,
		const Ogre::Real r,
		const Ogre::Real s = 2.0f );
protected:
	virtual Ogre::MeshPtr getMesh();
	virtual void injectMouseMove(int _absx, int _absy, int _absz);

	Ogre::Real mRaduis;
	RaduisNotifyEvent mNotify;
};

typedef boost::shared_ptr<RaduisControlet> RaduisControletPtr;

//平面移动
class PlaneControlet:
	public RigidControlet
{
public:
	PlaneControlet( const string& name,
		const Ogre::Plane& plane,
		LinearNotifyEvent func,
		const Ogre::Real s = 2.0f );
	virtual ~PlaneControlet();

	void setPlane( const Ogre::Plane& plane );
protected:
	virtual void injectMouseMove(int _absx, int _absy, int _absz);

	Ogre::Plane mPlane;
	LinearNotifyEvent mNotify;
};

typedef boost::shared_ptr<PlaneControlet> PlaneControletPtr;

//空间移动
#endif
