#ifndef _INPUTFILTER_H_
#define _INPUTFILTER_H_

#include "Controlet.h"

/* MyGUI和其他的屏幕操作有冲突
	InputFilter立足于解决这个问题
*/
class InputFilter:
	public Ogre::Singleton<InputFilter>,
	public base::Input,
	public Ogre::FrameListener
{
public:
	InputFilter();

	void addControlet( Controlet* pf );
	void removeControlet( Controlet* pf );

	 void setCapture();
	 void releaseCapture();

	//Input
	virtual void injectMouseMove(int _absx, int _absy, int _absz);
	virtual void injectMousePress(int _absx, int _absy, MyGUI::MouseButton _id);
	virtual void injectMouseRelease(int _absx, int _absy, MyGUI::MouseButton _id);
	virtual void injectKeyPress(MyGUI::KeyCode _key, MyGUI::Char _text);
	virtual void injectKeyRelease(MyGUI::KeyCode _key);
	//Ogre::FrameListener
	virtual bool frameStarted(const Ogre::FrameEvent& evt);
	virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
	virtual bool frameEnded(const Ogre::FrameEvent& evt);
protected:
	bool IsMouseFocus();
	bool IsKeyFocus();

	void synchStaticVector();

	typedef vector<Controlet*> ControletVector;

	//临时的静态表，在做循环时候用内容和mControlet相同
	ControletVector mStaticVector;
	ControletVector mControlet;
	bool mHasChange;

	bool mCapture;
};

const char* ScanCodeToText( int code );

#endif