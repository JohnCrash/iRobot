#ifndef _RIGID_MANAGER_H_
#define _RIGID_MANAGER_H_
#include "Rigid.h"
#include "Joint.h"
#include "Framework.h"

/*除了管理刚体还管理铰链等和物理相关的对象
*/
class RigidManager:public Ogre::Singleton<RigidManager>
{
public:
	RigidManager();
	virtual ~RigidManager();

	//返回平面x,y位置的对象
	VisualObjectPtr pickObject( int x,int y );

	//取得ODE dWorldID
	dWorldID getWorldID() const;
	dSpaceID getSpaceID() const;

	/*进行帧模拟
	*/
	void SimulationFrame( Ogre::Real t );
	//b = true开始模拟 false 停止模拟
	void goSimulation( bool b ); //双线程模式
	bool isSimulation() const;
	void goSimulationSingle( bool b ); //单线程
	/*模拟中间用来检测两个集合体的碰撞，
		仅仅管理器内部调用
	*/
	void _collide( dGeomID o1,dGeomID o2);

	/* 向场景中放入对象 */
	void addNode( VisualObjectPtr rp );
	void removeNode( VisualObjectPtr rptr );

    void addFramework( const FrameworkPtr& pt);
    void removeFramework( const FrameworkPtr& pt);
	//清除全部模拟对象
	void clearAllNode();

	void load(MyGUI::xml::ElementPtr node);
	void save(MyGUI::xml::ElementPtr node);
	bool loadSceneFromFile(const string& name);
	bool saveSceneFromFile(const string& name);
protected:
	void _simstep(); //模拟步
	void _simLoop(); //多线程模拟循环
	void stopSimthread(); //停止模拟线程

	typedef list<VisualObjectPtr> ObjectMap;

	boost::barrier mBarrier; //相当与一个会和点，用于多线程模拟的同步
	boost::thread* mThread; //模拟线程
	bool mSimFlag; //判断模拟是否还继续
	bool mIsSingle; //单线程还是多线程

	ObjectMap mObjects;
    FrameworkMap mFramework;
    
	/* RididManager的物理引擎部分
	*/
	dWorldID mWorld;
	dSpaceID mSpace;
	//碰撞铰链组
	dJointGroupID mContactGroup;

	bool mSimulation;
	//模拟步长
	Ogre::Real mSimStep;
};

#endif