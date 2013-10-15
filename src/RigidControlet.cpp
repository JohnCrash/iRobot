#include "stdheader.h"
#include "Geometry.h"
#include "RigidControlet.h"
#include "Game.h"
#include "Math3d.h"
#include "InputFilter.h"

RigidControlet::RigidControlet(const string& name,Ogre::Real s):
	mName(name),
	mPick(false),
	mMouseX(0),
	mMouseY(0),
	mScale(s),
	mNode(nullptr),
	mEntity(nullptr),
	mMesh(),
	bSelect(false)
{
}

void RigidControlet::setVisible( bool b ){
	mNode->setVisible( b );
	/*效率考虑，因为RigidControlet使用一个Controlet接口
		在不需要的时候禁止打可以略微减少调用
	*/
	setEnable( b );
}

void RigidControlet::mouseFocus( int x,int y ){
	string name = Game::getSingleton().pickMovableObject( x,y );
	if( name == mName || mPick ){
		if( !bSelect ){
			mEntity->setMaterialName( "ControletSelectColor" );
			bSelect = true;
		}
	}else{
		if( bSelect ){
			mEntity->setMaterialName( "ControletColor" );
			bSelect = false;
		}
	}
}

void RigidControlet::createEntity(){
	mMesh = getMesh();

	mEntity = Game::getSingleton().getSceneManager()->createEntity( mName,mMesh->getName() );
	mNode = Game::getSingleton().getSceneManager()->
		getRootSceneNode()->
		createChildSceneNode( mName );
	mNode->attachObject(mEntity);
	
	if( bSelect )
		mEntity->setMaterialName( "ControletSelectColor" );
	else
		mEntity->setMaterialName( "ControletColor" );
}

void RigidControlet::rebuildEntity(){
	destroyEntity();
	createEntity();
}

void RigidControlet::destroyEntity(){
	if( mNode ){
		mNode->detachObject( mEntity );
		Game::getSingleton().getSceneManager()->destroyEntity( mEntity);

		Game::getSingleton().getSceneManager()->destroySceneNode(mNode);

		Geometry::getSingleton().destroyGemetry(mMesh);
	}
}

RigidControlet::~RigidControlet(){
}

void RigidControlet::injectMousePress(int _absx, int _absy, MyGUI::MouseButton _id){
	if( _id == MyGUI::MouseButton::Left ){
		string name = Game::getSingleton().pickMovableObject( _absx,_absy );
		if( name == mName ){
			mPick = true;
			mMouseX = _absx;
			mMouseY = _absy;
			InputFilter::getSingleton().setCapture();
		}
	}
}

void RigidControlet::injectMouseRelease(int _absx, int _absy, MyGUI::MouseButton _id){
	mPick = false;
	InputFilter::getSingleton().releaseCapture();
}

LinearControlet::LinearControlet( const string& name,
	const Ogre::Vector3& pos, //位置
	const Ogre::Vector3& dir, //方向
	LinearNotifyEvent func,
	Ogre::Real scale
	):RigidControlet(name,scale){

	mNotify = func;
	//调整节点位置和方向
	setLinear( pos,dir,scale );
}

LinearControlet::~LinearControlet(){
	destroyEntity();
}

void LinearControlet::setLimit( Ogre::Real mi,Ogre::Real ma ){
	mLimited[0] = mi;
	mLimited[1] = ma;
}

void LinearControlet::setLinear( const Ogre::Vector3& p,
								const Ogre::Vector3& dir,
								const Ogre::Real s )
{
	mPosition = p;
	mDirection = dir;
	//最小和最大值
	mLimited[0] = Ogre::Math::NEG_INFINITY;
	mLimited[1] = Ogre::Math::POS_INFINITY;
	if( mScale != s || !mNode ){
		mScale = s;
		rebuildEntity();
	}
	mNode->setPosition( p );
	mNode->setDirection( dir,Ogre::Node::TS_WORLD );
}

Ogre::MeshPtr LinearControlet::getMesh(){
	Ogre::MeshPtr mesh;
	
	mesh = Ogre::MeshManager::getSingleton().getByName( "LinearControletArrow" );

	if( !mesh.isNull() ){
		//先删除在重新建立
		return mesh;
	}

	Geometry::tPath path;
	//构造一个箭头
	path.push_back( Ogre::Vector3(0,0,1) );
	path.push_back( Ogre::Vector3(0.4f,0,0.4f) );
	path.push_back( Ogre::Vector3(0.2f,0,0.4f) );
	path.push_back( Ogre::Vector3(0.2f,0,0) );
	path.push_back( Ogre::Vector3(0,0,0) );
	for( int i = 0;i < (int)path.size();++i ){
		path[i] *= mScale;
		path[i].z += mScale/2;
	}
	mesh = Geometry::getSingleton().createSpin("LinearControletArrow",path );

	return mesh;
}

void LinearControlet::injectMouseMove(int _absx, int _absy, int _absz){
	if( mPick && 
		(_absx!=mMouseX ||
		_absy!=mMouseY) ){
		/*鼠标按下点(mMouseX,mMouseY)连接眼睛的位置，在空间形成一条直线(直线A)。
		同样(_absx,_absy)与眼睛相连也形成一条直线(直线B)。这两条直线可以构成一个平面(叫V平面吧)。
		而LinearControlet的位置和方向在空间也形成一条直线(C)。该直线在V上的投影直线与直线A与B相交
		形成一个矢量，把该矢量投影回C,形成一个移动向量。
		简化点：这个问题等价与两个直线间的最短距离。A与C B与C
		*/
		Game& game = Game::getSingleton();
		int sx = game.getScreenWidth();
		int sy = game.getScreenHeight();
		Ogre::Vector3 eye = game.getCamera()->getPosition();
		Ogre::Ray A = game.getCamera()->getCameraToViewportRay(
				(Ogre::Real)mMouseX/(Ogre::Real)sx,
				(Ogre::Real)mMouseY/(Ogre::Real)sy
			);
		Ogre::Ray B = game.getCamera()->getCameraToViewportRay(
				(Ogre::Real)_absx/(Ogre::Real)sx,
				(Ogre::Real)_absy/(Ogre::Real)sy
			);
		Ogre::Matrix3 m3 = mNode->getLocalAxes();
		Ogre::Vector3 v3 = mNode->getPosition();
		//m3[0][2],m3[1][2],m3[2][2] 是Z坐标轴在世界坐标中的矢量
		Ogre::Ray C(v3,Ogre::Vector3(m3[0][2],m3[1][2],m3[2][2]));
		Math3d::CalcResult result1 = Math3d::CalcNearPoint( C,A );
		Math3d::CalcResult result2 = Math3d::CalcNearPoint( C,B );
		if( result1.first && result2.first ){
			Ogre::Vector3 a = result1.second;
			Ogre::Vector3 b = result2.second;
			/*将移动限制在mLimited之间，mLimited是两个直线参数
			*/
			Ogre::Ray ray(mPosition,-mDirection);
			Ogre::Real t = Math3d::RayParameter( ray,v3 + b-a );
			if( t < mLimited[0] )
				t = mLimited[0];
			if( t > mLimited[1] )
				t = mLimited[1];
			Ogre::Vector3 p = Math3d::RayPoint( ray,t );

			mNode->setPosition( p );
			mNotify( mName,p-v3 );
		}
		mMouseX = _absx;
		mMouseY = _absy;
	}
	mouseFocus( _absx,_absy );
}

RotateControlet::RotateControlet( const string& name,
		const Ogre::Vector3& pos,//旋转中心点
		const Ogre::Vector3& axis, //旋转轴
		const Ogre::Real raduis, //半径
		RotateNotifyEvent func,
		Ogre::Real scale
		):
	RigidControlet(name,scale),
	mRaduis(raduis){
	
	setRotate( pos,axis,raduis,scale );

	mNotify = func;
}

RotateControlet::~RotateControlet(){
	destroyEntity();
}

Ogre::MeshPtr RotateControlet::getMesh(){
	Ogre::MeshPtr mesh;

	mesh = Ogre::MeshManager::getSingleton().getByName( "RotateControletRing" );

	if( !mesh.isNull() ){
		//先删除在重新建立
		return mesh;
	}

	Geometry::tPath path;
	//构造构造一个圆环
	for( int i = 0;i < Geometry::DEFAULT_DENSITY;++i ){
		Ogre::Real d = (Ogre::Real)((i*2.0f*Ogre::Math::PI)/(Geometry::DEFAULT_DENSITY-1));
		path.push_back( 
			Ogre::Vector3(
				mScale*sinf(d)+mRaduis,
				0,
				mScale*cosf(d))
			);
	}

	mesh = Geometry::getSingleton().createSpin("RotateControletRing",path );
	return mesh;
}

void RotateControlet::setRotate( const Ogre::Vector3& o,
								const Ogre::Vector3& ax,
								const Ogre::Real r,
								const Ogre::Real s )
{
	if( !Ogre::Math::RealEqual( mRaduis,r+2*s) || mScale != s || !mNode ){
		//重新调整外观
		mRaduis = r+2*s;
		mScale = s;
		rebuildEntity();
	}
	mNode->setPosition( o );
	mNode->setDirection( ax,Ogre::Node::TS_WORLD );
}

void RotateControlet::injectMouseMove(int _absx, int _absy, int _absz){
	if( mPick && 
		(_absx!=mMouseX ||
		_absy!=mMouseY) ){
		/*上一个鼠标位置和眼睛组成的射线A，新的鼠标位置和眼睛组成的射线B
		它们和旋转盘相交，交点和旋转盘中心组成的矢量的角度，既是旋转角
		*/
		Game& game = Game::getSingleton();
		int sx = game.getScreenWidth();
		int sy = game.getScreenHeight();
		Ogre::Vector3 eye = game.getCamera()->getPosition();
		Ogre::Ray A = game.getCamera()->getCameraToViewportRay(
				(Ogre::Real)mMouseX/(Ogre::Real)sx,
				(Ogre::Real)mMouseY/(Ogre::Real)sy
			);
		Ogre::Ray B = game.getCamera()->getCameraToViewportRay(
				(Ogre::Real)_absx/(Ogre::Real)sx,
				(Ogre::Real)_absy/(Ogre::Real)sy
			);
		//计算和旋转盘的交点，旋转盘是一个平面
		//平面的法向量是Local Z轴，平面通过Local原点
		Ogre::Matrix3 m3 = mNode->getLocalAxes();
		Ogre::Vector3 v3 = mNode->getPosition();
		Ogre::Vector3 axis(m3[0][2],m3[1][2],m3[2][2]);
		Ogre::Plane P(axis,v3 ); //旋转盘平面
		Math3d::CalcResult result1 = Math3d::CalcRayPlanePoint( A,P );
		Math3d::CalcResult result2 = Math3d::CalcRayPlanePoint( B,P );
		if( result1.first && result2.first ){
			//方程都有解
			Ogre::Vector3 a,b;
			a = result1.second;
			b = result2.second;
			a -= v3;b -= v3;
			a.normalise();
			b.normalise();
			Ogre::Vector3 c = b.crossProduct(a);
			Ogre::Real sinv = c.length();
			Ogre::Real ang = asinf(sinv);

			if( axis.dotProduct( c ) > 0 )
				ang = -ang;

			mNode->rotate( axis,Ogre::Radian(ang),Ogre::Node::TS_WORLD );

			mNotify( mName,axis,ang );
		}
		mMouseX = _absx;
		mMouseY = _absy;
	}

	mouseFocus( _absx,_absy );
}

OrientationControlet::OrientationControlet( const string& name,
		const Ogre::Vector3& pos,//旋转中心点
		const Ogre::Vector3& axis, //旋转轴
		OrientationNotifyEvent func,
		Ogre::Real scale
		):RigidControlet(name,scale){
	mNotify = func;
	//调整节点位置和方向
	setOrient( pos,axis,scale );
}

void OrientationControlet::createEntity(){
	mMesh = getMesh();
	mEntity = Game::getSingleton().getSceneManager()->createEntity( mName,mMesh->getName() );

	mNode = Game::getSingleton().getSceneManager()->
		getRootSceneNode()->
		createChildSceneNode( mName );
	mNodeLocal = mNode->createChildSceneNode( mName+"Local" );
	mNodeLocal->attachObject(mEntity);

	mEntity->setMaterialName( "ControletColor" );
}

void OrientationControlet::destroyEntity(){
	if( mNode ){
		mNodeLocal->detachObject( mEntity );
		Game::getSingleton().getSceneManager()->destroyEntity( mEntity);
		Game::getSingleton().getSceneManager()->destroySceneNode( mNodeLocal );
		Game::getSingleton().getSceneManager()->destroySceneNode(mNode);

		Geometry::getSingleton().destroyGemetry(mMesh);
	}
}

OrientationControlet::~OrientationControlet(){
	destroyEntity();
}

void OrientationControlet::injectMousePress(int _absx, int _absy, MyGUI::MouseButton _id)
{
	if( _id == MyGUI::MouseButton::Left ){
		string name = Game::getSingleton().pickMovableObject( _absx,_absy );
		if( name == mName ){
			mPick = true;
			mMouseX = _absx;
			mMouseY = _absy;
			InputFilter::getSingleton().setCapture();

			/*判断开始时候，控制球是在球的正面还是背面
			*/
			Game& game = Game::getSingleton();
			int sx = game.getScreenWidth();
			int sy = game.getScreenHeight();
			Ogre::Ray B = game.getCamera()->getCameraToViewportRay(
					(Ogre::Real)_absx/(Ogre::Real)sx,
					(Ogre::Real)_absy/(Ogre::Real)sy
				);
			Ogre::Matrix3 m3 = mNode->getLocalAxes();
			Ogre::Vector3 axis = m3.GetColumn(2);
			if( axis.dotProduct(B.getDirection())>0 )
				mNearFar = false;
			else
				mNearFar = true;
		}
	}
}

void OrientationControlet::injectMouseMove(int _absx, int _absy, int _absz){
	if( mPick && 
		(_absx!=mMouseX ||
		_absy!=mMouseY) ){
		/*计算和摇杆球面的交点
		*/
		Game& game = Game::getSingleton();
		int sx = game.getScreenWidth();
		int sy = game.getScreenHeight();
		Ogre::Vector3 eye = game.getCamera()->getPosition();
		Ogre::Ray A = game.getCamera()->getCameraToViewportRay(
				(Ogre::Real)mMouseX/(Ogre::Real)sx,
				(Ogre::Real)mMouseY/(Ogre::Real)sy
			);
		Ogre::Ray B = game.getCamera()->getCameraToViewportRay(
				(Ogre::Real)_absx/(Ogre::Real)sx,
				(Ogre::Real)_absy/(Ogre::Real)sy
			);
		//计算和旋转盘的交点，旋转盘是一个平面
		//平面的法向量是Local Z轴，平面通过Local原点
		Ogre::Matrix3 m3 = mNode->getLocalAxes();
		Ogre::Vector3 v3 = mNode->getPosition();
		Ogre::Vector3 axis = m3.GetColumn(2);

		Ogre::Sphere S(v3,mRaduis);

		Math3d::CalcResult result1 = Math3d::CalcRaySphereNearFarPoint(A,S,mNearFar);
		Math3d::CalcResult result2 = Math3d::CalcRaySphereNearFarPoint(B,S,mNearFar);
		if( result1.first && result2.first ){//都相交
			//然后使用直线的参数方程计算出具体的焦点
			Ogre::Vector3 a = result1.second;
			Ogre::Vector3 b = result2.second;
			a -= v3;b -= v3;
			a.normalise();
			b.normalise();
			Ogre::Vector3 c = b.crossProduct(a);
			Ogre::Real sinv = c.length();
			Ogre::Real ang = -asinf(sinv);
			c.normalise();

			mNode->rotate( c,Ogre::Radian(ang),Ogre::Node::TS_WORLD );
			mNotify( mName,c,ang );
		}
		mMouseX = _absx;
		mMouseY = _absy;
	}

	mouseFocus( _absx,_absy );
}

Ogre::MeshPtr OrientationControlet::getMesh(){
	Ogre::MeshPtr mesh;
	
	mesh = Ogre::MeshManager::getSingleton().getByName( "OrientationControletSphere" );

	if( !mesh.isNull() ){
		//先删除在重新建立
		return mesh;
	}

	mesh = Geometry::getSingleton().createSphere("OrientationControletSphere",mScale );

	return mesh;
}

void OrientationControlet::setOrient( const Ogre::Vector3& o,
		const Ogre::Vector3& ax,
		const Ogre::Real s )
{
	if( mScale != s || !mNode ){
		//重新调整外观
		mScale = s;
		rebuildEntity();
	}
	mRaduis = ax.length() + 2*s;
	
	mNode->setPosition( o );
	mNode->setDirection( ax,Ogre::Node::TS_WORLD );
	mNodeLocal->setPosition( Ogre::Vector3(0,0,mRaduis) );
}

RaduisControlet::RaduisControlet( const string& name,
		const Ogre::Vector3& pos,//旋转中心点
		const Ogre::Vector3& axis, //旋转轴
		const Ogre::Real raduis, //半径
		RaduisNotifyEvent func,
		Ogre::Real scale
		):
	RigidControlet(name,scale),
	mRaduis(raduis){
	
	setRaduis( pos,axis,raduis,scale );

	mNotify = func;
}

RaduisControlet::~RaduisControlet(){
	destroyEntity();
}

void RaduisControlet::setRaduis( const Ogre::Vector3& o,
	const Ogre::Vector3& ax,
	const Ogre::Real r,
	const Ogre::Real s )
{
	if( !Ogre::Math::RealEqual( mRaduis,r+2*s) || mScale != s || !mNode ){
		//重新调整外观
		mRaduis = r+2*s;
		mScale = s;
		rebuildEntity();
	}
	mNode->setPosition( o );
	mNode->setDirection( ax,Ogre::Node::TS_WORLD );
}

Ogre::MeshPtr RaduisControlet::getMesh()
{
	Ogre::MeshPtr mesh;
	
	mesh = Ogre::MeshManager::getSingleton().getByName( "RaduisControletRing" );

	if( !mesh.isNull() ){
		//先删除在重新建立
		return mesh;
	}

	Geometry::tPath path;
	//构造构造一个圆环
	for( int i = 0;i < Geometry::DEFAULT_DENSITY;++i ){
		Ogre::Real d = (Ogre::Real)((i*2.0f*Ogre::Math::PI)/(Geometry::DEFAULT_DENSITY-1));
		path.push_back( 
			Ogre::Vector3(
				mScale*sinf(d)+mRaduis,
				0,
				mScale*cosf(d))
			);
	}

	mesh = Geometry::getSingleton().createSpin("RaduisControletRing",path );

	return mesh;
}

void RaduisControlet::injectMouseMove(int _absx, int _absy, int _absz)
{
	if( mPick && 
		(_absx!=mMouseX ||
		_absy!=mMouseY) ){
		Game& game = Game::getSingleton();
		int sx = game.getScreenWidth();
		int sy = game.getScreenHeight();
		Ogre::Vector3 eye = game.getCamera()->getPosition();

		Ogre::Ray B = game.getCamera()->getCameraToViewportRay(
				(Ogre::Real)_absx/(Ogre::Real)sx,
				(Ogre::Real)_absy/(Ogre::Real)sy
			);
		Ogre::Matrix3 m3 = mNode->getLocalAxes();
		Ogre::Vector3 v3 = mNode->getPosition();
		Math3d::CalcResult result = Math3d::CalcRayPlanePoint( B,
			Ogre::Plane(m3.GetColumn(2),v3) );

		if( result.first ){
			Ogre::Real r = v3.distance( result.second );
			Ogre::Real R = r-2*mScale;
			if( R <= 1 )
				R = 1;
			
			setRaduis( v3,m3.GetColumn(2),R,mScale );
			mNotify( mName,R );
		}
		mMouseX = _absx;
		mMouseY = _absy;
	}
	mouseFocus( _absx,_absy );
}

PlaneControlet::PlaneControlet( const string& name,
		const Ogre::Plane& plane,
		LinearNotifyEvent func,
		const Ogre::Real s ):
	RigidControlet( name,s )
{
	mNotify = func;
	setPlane( plane );
}

PlaneControlet::~PlaneControlet()
{
}

void PlaneControlet::setPlane( const Ogre::Plane& plane )
{
	mPlane = plane;
}

void PlaneControlet::injectMouseMove(int _absx, int _absy, int _absz){
	if( mPick && 
		(_absx!=mMouseX ||
		_absy!=mMouseY) ){
		Game& game = Game::getSingleton();
		int sx = game.getScreenWidth();
		int sy = game.getScreenHeight();
		Ogre::Vector3 eye = game.getCamera()->getPosition();
		Ogre::Ray A = game.getCamera()->getCameraToViewportRay(
				(Ogre::Real)mMouseX/(Ogre::Real)sx,
				(Ogre::Real)mMouseY/(Ogre::Real)sy
			);
		Ogre::Ray B = game.getCamera()->getCameraToViewportRay(
				(Ogre::Real)_absx/(Ogre::Real)sx,
				(Ogre::Real)_absy/(Ogre::Real)sy
			);
		/*求射线A和B在平面mPlane上的两个交点
		*/
		Math3d::CalcResult result1 = Math3d::CalcRayPlanePoint( A,mPlane );
		Math3d::CalcResult result2 = Math3d::CalcRayPlanePoint( B,mPlane );
		if( result1.first && result2.first )
		{
			mNotify( mName,result2.second-result1.second );
		}
		mMouseX = _absx;
		mMouseY = _absy;
	}
}
