#ifndef _GAME_H_
#define _GAME_H_

#include "Stage.h"
#include "BaseManager.h"
#include "SimpleUI.h"
#include "Geometry.h"
#include "ObjectFactory.h"
#include "RigidManager.h"
#include "CameraControlet.h"
#include "RigidToolManager.h"
#include "Registry.h"
#include "LuaManager.h"
#include "HotkeyManager.h"
#include "LuaControlet.h"
/* Game 提供一个对Ogre和MyGUI的基本抽象
	包括对Ogre选项的配置
*/

class Game :
	public Ogre::Singleton<Game>,
	public base::BaseManager
{
public:
	Game();
	void showOptionsDialog();
	void setCameraControlet( const string& name );
	void execute( const string& resource,Game::ResetType rt );
protected:
	virtual void createScene();
	virtual void destroyScene();
	virtual void setupResources();

	void setShadowTechnique(Ogre::ShadowTechnique st);
	Ogre::ShadowTechnique getShadowTechnique( size_t sel );
	Ogre::ShadowTechnique getShadowMode( size_t sel );

	virtual bool frameStarted(const Ogre::FrameEvent& _evt);

	virtual void injectKeyPress(MyGUI::KeyCode _key, MyGUI::Char _text);

	void notifyButtonClick(MyGUI::Widget* _sender);
	void notifyComboChange(MyGUI::ComboBox* _sender, size_t _index);
	void notifyWindowButtonPressed(MyGUI::Window* _sender, const string& _button);
	void simpleDataChange(SimpleData* sd);

	void loadBase( const string& filename );

	GettextManager* mGettextManager;
	ObjectFactory* mObjectFactory;
	Geometry* mGeometry;
	RigidManager* mRigidManager;
	RigidToolManager* mRigidToolManager;
	LuaManager* mLuaManager;
	HotkeyManager* mHotkeyManager;
	LuaControlet* mLuaC;
	Registry* mRegistry;

	bool mNeedReset;

	SimpleUI mUI;

	ControletPtr mCameraControlet;
};

typedef Game tGame;

#endif
