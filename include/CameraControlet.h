#ifndef _CAMERACONTROLET_H_
#define _CAMERACONTROLET_H_
#include "Controlet.h"

class SimpleCameraControlet:public Controlet
{
public:
	SimpleCameraControlet( Ogre::Camera* pcam );
	virtual ~SimpleCameraControlet();
protected:
	//Input
	virtual void injectMouseMove(int _absx, int _absy, int _absz);
	virtual void injectMousePress(int _absx, int _absy, MyGUI::MouseButton _id);
	virtual void injectMouseRelease(int _absx, int _absy, MyGUI::MouseButton _id);
	virtual void injectKeyPress(MyGUI::KeyCode _key, MyGUI::Char _text);
	virtual void injectKeyRelease(MyGUI::KeyCode _key);
	virtual bool frameStarted(const Ogre::FrameEvent& evt);

	void RotatY( Ogre::Real s );
	void RotatXZ( Ogre::Real s );
	void Zoom( Ogre::Real s );

	Ogre::Camera* mCam;

	int mWheelZ;

	bool mKeyPassedY;
	bool mKeyPassedXZ;

	Ogre::Real mRY;
	Ogre::Real mRXZ;

	Ogre::Real mSpeed;
	Ogre::Real mAngleSpeed;
	Ogre::Real mMin;
	Ogre::Real mMax;

	Ogre::Vector3 mLookPos;//观察点

	int mPrevZoom;
};

typedef boost::shared_ptr<SimpleCameraControlet> SimpleCameraControletPtr;

#endif