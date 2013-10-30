#include "stdheader.h"
#include "Game.h"
#include "CameraControlet.h"

SimpleCameraControlet::SimpleCameraControlet( Ogre::Camera* pcam ):
	mCam(pcam),
	mWheelZ(-1),
	mKeyPassedY(false),
	mKeyPassedXZ(false),
	mRY(0),
	mRXZ(0),
	mSpeed(100),//每秒移动100个单位
	mAngleSpeed(120),//速度每秒120度
	mMin(10),//顶部保留10度
	mMax(90),//不能到地面以下
	mLookPos(0,0,0)
{	
	mPrevZoom = 10000;
}

SimpleCameraControlet::~SimpleCameraControlet(){
	mCam = nullptr;
}

void SimpleCameraControlet::injectMouseMove(int _absx, int _absy, int _absz){
	if( mWheelZ != _absz ){
		/*防止大的跳动
		*/
		if( mWheelZ!=-1 ){
			mPrevZoom = abs(mWheelZ-_absz);
			Zoom( (mWheelZ-_absz) );
		}
		mWheelZ = _absz;
	}
}

//s是距离,+远离原点
void SimpleCameraControlet::Zoom( Ogre::Real s ){
	Ogre::Vector3 cv = mCam->getPosition();
	cv = cv - mLookPos;
	Ogre::Vector3 v(cv);
	Ogre::Real d = v.normalise();
	d += s;
	if( d < 10 )d = 10; //做一个限定
	v *= d;
	mCam->setPosition( v );
}

void SimpleCameraControlet::injectMousePress(int _absx, int _absy, MyGUI::MouseButton _id){
}

void SimpleCameraControlet::injectMouseRelease(int _absx, int _absy, MyGUI::MouseButton _id){
}

void SimpleCameraControlet::injectKeyPress(MyGUI::KeyCode _key, MyGUI::Char _text){
	if( _key==MyGUI::KeyCode::ArrowUp ){
		mKeyPassedXZ = true;
		mRXZ = 1;
	}
	
	if( _key==MyGUI::KeyCode::ArrowDown ){
		mKeyPassedXZ = true;
		mRXZ = -1;
	}
	
	if( _key==MyGUI::KeyCode::ArrowLeft ){
		mKeyPassedY = true;
		mRY = 1;
	}
	
	if(_key== MyGUI::KeyCode::ArrowRight ){
		mKeyPassedY = true;
		mRY = -1;
	}
}

void SimpleCameraControlet::injectKeyRelease(MyGUI::KeyCode _key){
	if( _key==MyGUI::KeyCode::ArrowUp||
		_key==MyGUI::KeyCode::ArrowDown )
		mKeyPassedXZ = false;

	if( _key==MyGUI::KeyCode::ArrowLeft||
		_key==MyGUI::KeyCode::ArrowRight ){
		mKeyPassedY = false;
	}
}

//绕Y轴（向上的轴）旋转摄像机
void SimpleCameraControlet::RotatY( Ogre::Real s ){
	Ogre::Vector3 v3 = mCam->getPosition();

	v3 = v3 - mLookPos;
	Ogre::Quaternion q(Ogre::Radian(Ogre::Degree(s)),Ogre::Vector3::UNIT_Y);
	v3 = q * v3 + mLookPos;
	mCam->setPosition( v3 );
	mCam->lookAt(mLookPos);
}

//绕一个轴旋转，这个轴在XZ平面上，且垂直与原点到摄像机的矢量。
void SimpleCameraControlet::RotatXZ( Ogre::Real s ){
	Ogre::Vector3 v3 = mCam->getPosition();
	v3 = v3 - mLookPos;
	//v3不能和Y轴平行
	Ogre::Vector3 v = v3;
	v.normalise();
	Ogre::Real cosv = v.dotProduct( Ogre::Vector3::UNIT_Y );
	//做一个限制，顶部有10度
	Ogre::Degree de(mMin);
	//s>0表示已经小于10度了，还想减少
	if( cosv > cosf(de.valueRadians() ) && s>0 )
		return;
	//不能大有90度（从下面看）
	Ogre::Degree de2(mMax);
	if( cosv < cosf(de2.valueRadians() ) && s<0 )
		return;
	//使用叉积计算旋转轴
	Ogre::Vector3 axis(v3.crossProduct(Ogre::Vector3::UNIT_Y));
	axis.normalise();
	Ogre::Quaternion q(Ogre::Radian(Ogre::Degree(s)),axis);
	v3 = q * v3 + mLookPos;
	mCam->setPosition( v3 );
	mCam->lookAt(mLookPos);
}

bool SimpleCameraControlet::frameStarted(const Ogre::FrameEvent& evt){
	if( mKeyPassedY ){
		RotatY( evt.timeSinceLastFrame*mAngleSpeed*mRY );
	}
	if( mKeyPassedXZ ){
		RotatXZ( evt.timeSinceLastFrame*mAngleSpeed*mRXZ );
	}
	return true;
}