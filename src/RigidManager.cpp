#include "stdheader.h"
#include "Geometry.h"
#include "Game.h"
#include "RigidManager.h"
#include "VisualObject.h"
#include "GeometryObject.h"

SINGLETON(RigidManager)


RigidManager::RigidManager():mBarrier(2){
	mSimulation = false;

	registerCoreObject();
	registerGeometryFactory();
	registerJointFactory();

	/*初始话ODE物理引擎
	*/
	dInitODE2(0);
	mWorld = dWorldCreate();
	mSpace = dHashSpaceCreate(0);
	mContactGroup = dJointGroupCreate(0);
	//设置重力加速度
	dWorldSetGravity(mWorld,0,-9.81,0);

	mSimStep = 0.01f;
	mThread = nullptr;
}

RigidManager::~RigidManager(){
	//停止模拟线程如果它在运行的话
	stopSimthread();
	
	clearAllNode();
	/*释放ODE物理引擎
	*/
	dJointGroupDestroy(mContactGroup);
	dSpaceDestroy(mSpace);
	dWorldDestroy(mWorld);
	dCloseODE();
}

void RigidManager::load(MyGUI::xml::ElementPtr root)
{
	MyGUI::xml::ElementEnumerator ee = root->getElementEnumerator();
	ObjectFactory& factory = ObjectFactory::getSingleton();
	while( ee.next() )
	{
		MyGUI::xml::ElementPtr node = ee.current();
		VisualObjectPtr o = boost::dynamic_pointer_cast<VisualObject>(factory.createObject( node->getName() ));
		if( o )
		{
			o->load( node );
			addNode( o );
		}
	}
}

void RigidManager::save(MyGUI::xml::ElementPtr root)
{
	for( ObjectMap::iterator it = mObjects.begin();it!=mObjects.end();++it )
	{
		MyGUI::xml::ElementPtr child = root->createChild((*it)->getTypeName());
		(*it)->save( child );
	}
}

bool RigidManager::loadSceneFromFile(const string& name)
{
	MyGUI::xml::Document doc;
	if( doc.open(name) )
	{
		MyGUI::xml::ElementPtr root = doc.getRoot();
		if( root && root->getName()=="Scene" )
		{
			load(root);
			return true;
		}
	}
	else
	{
		WARNING_LOG("RigidManager::loadSceneFromFile can't open file "<<name);
	}
	return false;
}

bool RigidManager::saveSceneFromFile(const string& name)
{
	MyGUI::xml::Document doc;
	doc.createDeclaration();
	MyGUI::xml::ElementPtr root = doc.createRoot("Scene");
	save(root);
	return doc.save(name);
}

void RigidManager::clearAllNode(){
	for( ObjectMap::iterator it = mObjects.begin();it!=mObjects.end();++it )
		(*it)->Disable();
	mObjects.clear();
}

dWorldID RigidManager::getWorldID() const
{
	return mWorld;
}

dSpaceID RigidManager::getSpaceID() const
{
	return mSpace;
}

void RigidManager::addNode( VisualObjectPtr rp )
{
	rp->Enable();
	
	mObjects.push_back(rp);
}

void RigidManager::removeNode( VisualObjectPtr rp )
{
	if( rp )
	{
		rp->Disable();

		//从模拟表中删除对象
		BOOST_AUTO(it,find(mObjects.begin(),mObjects.end(),rp) );
		if( it != mObjects.end() )
			mObjects.erase(it);
	}
}

//碰撞中转回调函数
static void CollideCallback(void *data, dGeomID o1, dGeomID o2)
{
	RigidManager* prm = static_cast<RigidManager*>(data);
	prm->_collide( o1,o2 );
}

void RigidManager::_collide( dGeomID o1,dGeomID o2)
{
  int i,n;

  const int N = 10;
  dContact contact[N];
  n = dCollide (o1,o2,N,&contact[0].geom,sizeof(dContact));
  if (n > 0) {
    for (i=0; i<n; i++) {
      contact[i].surface.mode = dContactSlip1 | dContactSlip2 |
	dContactSoftERP | dContactSoftCFM | dContactApprox1;
      contact[i].surface.mu = dInfinity;
      contact[i].surface.slip1 = 0.1;
      contact[i].surface.slip2 = 0.1;
      contact[i].surface.soft_erp = 0.5;
      contact[i].surface.soft_cfm = 0.3;
      dJointID c = dJointCreateContact (mWorld,mContactGroup,&contact[i]);
      dJointAttach (c,
		    dGeomGetBody(contact[i].geom.g1),
		    dGeomGetBody(contact[i].geom.g2));
    }
  }
}

/*这里单独使用一个线程来处理模拟
*/
void RigidManager::SimulationFrame( Ogre::Real t ){
	if( !mSimulation )return;

	if( t > 0.0001 && t < 0.1 )
		mSimStep = t;

	if( mIsSingle )
	{
		_simstep(); //单线程模式直接执行模拟
	}
	else
	{
		mBarrier.wait(); //模拟线程和绘制线程回合在这样点，然后同时进行
	}

	//将ODE几何体的坐标更新到刚体节点上
	for( BOOST_AUTO(i,mObjects.begin());i!=mObjects.end();++i )
	{
		(*i)->_update();
	}
}

bool RigidManager::isSimulation() const
{
	return mSimulation;
}

void RigidManager::goSimulationSingle( bool b )
{
	stopSimthread();

	mSimulation = b;
	mIsSingle = true;
}

void RigidManager::stopSimthread()
{
	if( mThread )
	{
		mSimFlag = false;
		mBarrier.wait(); //在这一点回合，然后_simstep将结束
		mThread->join(); //等待_simstep结束
		delete mThread;
		mThread = nullptr;
	}
}

void RigidManager::goSimulation( bool b ){
	mSimulation = b;
	mIsSingle = false;
	if( b )
	{
		stopSimthread();
		mSimFlag = true;
		mThread = new boost::thread(boost::bind(&RigidManager::_simLoop,this));
	}
	else
		stopSimthread();
}

void RigidManager::_simLoop()
{
	while(mSimFlag)
	{
		_simstep();

		mBarrier.wait();
	}
}

void RigidManager::_simstep()
{
	//检查碰撞，如果有将创建接触铰链
	dSpaceCollide (mSpace,this,&CollideCallback);

	dWorldQuickStep (mWorld,mSimStep);

	//删除全部的接触铰链
	dJointGroupEmpty (mContactGroup);
}

VisualObjectPtr RigidManager::pickObject( int x,int y )
{
	string name = Game::getSingleton().pickMovableObject(x,y);

	for(BOOST_AUTO(it,mObjects.begin());it!=mObjects.end();++it)
	{
		if( (*it)->getEntityName() == name )
			return (*it);
	}
	return VisualObjectPtr();
}