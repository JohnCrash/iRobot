#ifndef _CONTROLET_H_
#define _CONTROLET_H_
#include "Input.h"

class Controlet:
	public base::Input,
	public Ogre::FrameListener
{
public:
	Controlet();
	 virtual ~Controlet();
	
	 void setEnable( bool b );
	 bool isEnable();

	 bool isGlobal() const;
	void setGlobal( bool b );

	//Input
	virtual void injectMouseMove(int _absx, int _absy, int _absz){}
	virtual void injectMousePress(int _absx, int _absy, MyGUI::MouseButton _id){}
	virtual void injectMouseRelease(int _absx, int _absy, MyGUI::MouseButton _id){}
	virtual void injectKeyPress(MyGUI::KeyCode _key, MyGUI::Char _text){}
	virtual void injectKeyRelease(MyGUI::KeyCode _key){}
	//Ogre::FrameListener
	virtual bool frameStarted(const Ogre::FrameEvent& evt)
	{ return true; }
	virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt)
	{ return true; }
	virtual bool frameEnded(const Ogre::FrameEvent& evt)
	{ return true; }
protected:
	bool mEnable;
	bool mGlobal;
};

typedef boost::shared_ptr<Controlet> ControletPtr;

#endif