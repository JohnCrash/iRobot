#include "stdheader.h"
#include "Geometry.h"
#include "RigidManager.h"
#include "RigidTool.h"
#include "Math3d.h"
#include "GeometryObject.h"

MoveTool::MoveTool():
	mRelative(Ogre::Node::TS_WORLD)
{
}

MoveTool::~MoveTool(){
}

void MoveTool::NotifyObjectMove( const string& name,const Ogre::Vector3& mv ){
	VisualObjectPtr rigid = mRigid.lock();
	if( rigid ){
		rigid->translate( mv );
		controlRigid( rigid,mRelative );
	}
}

/*从刚体的中间取射线，方向是dir，然后和刚体有交点若干
	取最外面的交点，设置控制器的位置。
*/
void MoveTool::setRigidControlet( LinearControletPtr& lcpForward,
								 LinearControletPtr& lcpBackward,
								 const VisualObjectPtr& prigid,
								 const Ogre::Vector3& dir )
{
	Ogre::Ray ray;
	vector<Ogre::Real> result;

	ray.setOrigin( prigid->getPosition() );
	ray.setDirection( dir );
	result = prigid->Intersect( ray,true );
	if( !result.empty() ){
		lcpForward->setLinear(Math3d::RayPoint( ray,result[0] ),dir);
		lcpBackward->setLinear(Math3d::RayPoint( ray,result[result.size()-1] ),-dir);
	}
}

void MoveTool::controlRigid( const VisualObjectPtr& rigid,
							const Ogre::Node::TransformSpace relativeTo
							)
{
	mRigid = rigid;

	if( !rigid ){
		//删除
		for( int i = 0;i<6;++i )
			mLinear[i].reset();
		return;
	}
	
	if( !mLinear[0] ){
		//重新创建
		for( int i = 0;i < 6;++i ){
			string name("_movetool");
			name += boost::lexical_cast<string>( i );
			mLinear[i].reset(new LinearControlet( name,
				Ogre::Vector3(0,0,50),
				Ogre::Vector3::NEGATIVE_UNIT_Z,
				boost::bind( &MoveTool::NotifyObjectMove,this,_1,_2 )
				));
		}
	}

	mRelative = relativeTo;

	//放置6个方向上的控制器
	if( relativeTo==Ogre::Node::TS_WORLD ){
		//移动方向平行与世界系坐标轴
		setRigidControlet( mLinear[0],mLinear[1],rigid,Ogre::Vector3::UNIT_X );
		setRigidControlet( mLinear[2],mLinear[3],rigid,Ogre::Vector3::UNIT_Y );
		setRigidControlet( mLinear[4],mLinear[5],rigid,Ogre::Vector3::UNIT_Z );
	}else if( relativeTo==Ogre::Node::TS_LOCAL ){
		//移动方向平行与世界系坐标轴
		Ogre::Matrix3 mat3 = rigid->getLocalAxis();
		setRigidControlet( mLinear[0],mLinear[1],rigid,mat3.GetColumn(0) );
		setRigidControlet( mLinear[2],mLinear[3],rigid,mat3.GetColumn(1) );
		setRigidControlet( mLinear[4],mLinear[5],rigid,mat3.GetColumn(2) );
	}else if( relativeTo==Ogre::Node::TS_PARENT ){
		//???这个相对有父节点没有实现
	}
}

RotateTool::RotateTool(){
}

RotateTool::~RotateTool(){
}

void RotateTool::controlRigid( const VisualObjectPtr& rigid ){
	mRigid = rigid;

	if( !rigid ){
		mRotate.reset();
		mOrient[0].reset();
		mOrient[1].reset();
		return;
	}
	if( !mRotate ){
		//重新创建
		mRotate.reset( new RotateControlet( "_rotate",
			Ogre::Vector3(0,0,0),Ogre::Vector3::NEGATIVE_UNIT_Z,
			10,boost::bind( &RotateTool::NotifObjectRoate,this,_1,_2,_3 ) 
			));
		mOrient[0].reset( new OrientationControlet( "_orientF",
			Ogre::Vector3(0,0,0),Ogre::Vector3::NEGATIVE_UNIT_Z,
			boost::bind( &RotateTool::NotifObjectRoate,this,_1,_2,_3 ) 
			));
		mOrient[1].reset( new OrientationControlet( "_orientB",
			Ogre::Vector3(0,0,0),Ogre::Vector3::NEGATIVE_UNIT_Z,
			boost::bind( &RotateTool::NotifObjectRoate,this,_1,_2,_3 ) 
			));
	}

	GeometryObjectPtr go = rigid->getGeometryObject();
	pair<Ogre::Ray,Ogre::Real> result = go->getSpinParam( rigid );
	mRotate->setRotate( result.first.getOrigin(),
		result.first.getDirection(),result.second );

	Ogre::Ray ray;
	ray.setOrigin( result.first.getOrigin() );
	ray.setDirection( result.first.getDirection() );
	vector<Ogre::Real> res = rigid->Intersect( ray,true );
	if( !res.empty() ){
		Ogre::Vector3 axis = Math3d::RayPoint2( ray,res[0] ) - ray.getOrigin();
		mOrient[0]->setOrient( result.first.getOrigin(),axis );
		axis = Math3d::RayPoint2( ray,res[res.size()-1] )  - ray.getOrigin();
		mOrient[1]->setOrient( result.first.getOrigin(),axis );
	}
}

void RotateTool::NotifObjectRoate(const string& name,
								   const Ogre::Vector3& axis,
								  const Ogre::Real angle )
{
	VisualObjectPtr rigid = mRigid.lock();
	if( rigid ){
		rigid->rotate( axis,angle );
		//直接重新计算位置，效率上差点
		controlRigid( rigid );
	}
}

ModifyTool::ModifyTool()
{
}

ModifyTool::~ModifyTool()
{
}

void ModifyTool::controlRigid( const VisualObjectPtr& rigid )
{
	if( rigid ){
		rigid->getGeometryObject()->modifyControlet( rigid );
	}else{
		VisualObjectPtr obj = mRigid.lock();
		if( obj )
			obj->getGeometryObject()->modifyControlet( rigid ); //rigid = null
	}
	mRigid = rigid;
}

MoveTool2d::MoveTool2d()
{
}

MoveTool2d::~MoveTool2d()
{
}

void MoveTool2d::controlRigid( const VisualObjectPtr& ptr,const Ogre::Plane& p )
{
	mRigid = ptr;

	if( ptr ){
		if( !mPC )
			mPC = PlaneControletPtr( new PlaneControlet("_movetool2d",
				p,
				boost::bind(&MoveTool2d::NotifyObjectMove,this,_1,_2 )
				));
		else
			mPC->setPlane( p );

		if( !mY ){
			mY = LinearControletPtr( new LinearControlet("_movetool2d",
				Ogre::Vector3(),Ogre::Vector3::UNIT_Y,
				boost::bind(&MoveTool2d::NotifyObjectMove,this,_1,_2 )
				));
		}
		Ogre::Ray ray;
		vector<Ogre::Real> result;

		ray.setOrigin( ptr->getPosition() );
		ray.setDirection( Ogre::Vector3::NEGATIVE_UNIT_Y );
		result = ptr->Intersect( ray,true );
		if( !result.empty() ){
			mY->setLinear( Math3d::RayPoint(ray,result[0]),Ogre::Vector3::NEGATIVE_UNIT_Y );
		}
	}else{
		mPC.reset();
		mY.reset();
	}
}

void MoveTool2d::NotifyObjectMove( const string& name,const Ogre::Vector3& mv )
{
	VisualObjectPtr obj = mRigid.lock();
	if( obj ){
		obj->translate( mv );
		controlRigid( obj,Ogre::Plane(Ogre::Vector3::UNIT_Y,obj->getPosition()) );
	}
}