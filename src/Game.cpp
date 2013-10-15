#include "stdheader.h"
#include "ObjectFactory.h"
#include "GeometryObject.h"
#include "Game.h"
#include "MainMenu.h"

SINGLETON(Game)

void Game::loadBase( const string& filename )
{
	LuaVector luado,lualoc,load;
	BaseManager::loadBase( filename,luado,lualoc,load );

	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	for( LuaVector::iterator i = lualoc.begin();i!=lualoc.end();i++ )
	{
		mLuaManager->addLuaLocation( *i );
	}

	for( LuaVector::iterator j = luado.begin();j!=luado.end();j++ )
	{
		mLuaManager->doFile( *j );
	}

//	for( LuaVector::iterator k = load.begin();k!=load.end();k++ )
//	{
//		loadBase( *k );
//	}
}

void Game::execute( const string& resource,Game::ResetType rt )
{
	mPreLoad.clear();
	mPreLoad.push_back(resource);
	reset(rt);
}

void Game::createScene()
{
	mRegistry = new Registry();
	//创建Geometry单一对象Singleton
	mGettextManager = new GettextManager();
	mGeometry = new Geometry();
	mObjectFactory = new ObjectFactory();
	//创建RigidManager单一对象
	mRigidManager = new RigidManager();
	mRigidToolManager = new RigidToolManager();
	mHotkeyManager = new HotkeyManager();
	mLuaManager = new LuaManager();
	mLuaC  = new LuaControlet();

	mLuaManager->setLuaLocation("");
	for( LuaVector::iterator i = mPreLuaLocation.begin();i!=mPreLuaLocation.end();i++ )
	{
		mLuaManager->addLuaLocation( *i );
	}

	for( LuaVector::iterator j = mPreLuaScript.begin();j!=mPreLuaScript.end();j++ )
	{
		mLuaManager->doFile( *j );
	}

	for( LuaVector::iterator k = mPreLoad.begin();k!=mPreLoad.end();k++ )
	{
		loadBase( *k );
	}
	//打开阴影
	setShadowTechnique(
		static_cast<Ogre::ShadowTechnique>(
		static_cast<int>(Ogre::SHADOWDETAILTYPE_STENCIL) |
		static_cast<int>(Ogre::SHADOWDETAILTYPE_MODULATIVE) )
		);

	if( mInfo )
		mInfo->setVisible( false );
}

void Game::destroyScene()
{
	//通知shutdown
	mLuaManager->callEventVoid("eventShutdown");
	//清楚以前的视角控制器
	setCameraControlet("");

	delete mLuaC;

	delete mLuaManager;

	delete mHotkeyManager;

	delete mRigidToolManager;

	delete mRigidManager;
	mRigidManager = nullptr;

	delete mObjectFactory;
	mObjectFactory = nullptr;

	delete mGeometry;
	mGeometry = nullptr;

	delete mGettextManager;
	mGettextManager = nullptr;

	delete mRegistry;
	mRegistry = nullptr;
}

void Game::setupResources(){
	BaseManager::setupResources();
}

Game::Game():
	BaseManager()
{
	mLuaC = nullptr;
}

void Game::showOptionsDialog(){
	if( !mUI ){
		mUI.load("Options.layout","Game");
		try{
			if( mUI ){
				SimpleDataUI sdu(mUI["OgreView"],MyGUI::newDelegate(this,&Game::simpleDataChange));
				
				assert(mRoot && mRoot->getRenderSystem());

				mNeedReset = false;

				Ogre::ConfigOptionMap& mp = mRoot->getRenderSystem()->getConfigOptions();
				
				for( Ogre::ConfigOptionMap::iterator i=mp.begin();i!=mp.end();++i ){
					vector<MyGUI::UString> v(i->second.possibleValues.size());
					copy(i->second.possibleValues.begin(),i->second.possibleValues.end(),v.begin());
					sdu.add( i->first,SimpleData(i->first,i->second.currentValue,v) );
				}
				sdu.reLayout(2,5);

				mUI["Cancel"]->eventMouseButtonClick += newDelegate(this, &Game::notifyButtonClick);
				mUI["Apply"]->eventMouseButtonClick += newDelegate(this, &Game::notifyButtonClick);

				MyGUI::ComboBox* pcombo = mUI["shadow"]->castType<MyGUI::ComboBox>(false);
				pcombo->addItem( "None" );
				pcombo->addItem( "Stencil" );
				pcombo->addItem( "Texture" );
				Ogre::ShadowTechnique st = getSceneManager()->getShadowTechnique();
				if( st & Ogre::SHADOWDETAILTYPE_STENCIL )
					pcombo->setIndexSelected(1);
				else if( st & Ogre::SHADOWDETAILTYPE_TEXTURE )
					pcombo->setIndexSelected(2);
				else
					pcombo->setIndexSelected(0);
				pcombo->eventComboChangePosition += newDelegate(this,&Game::notifyComboChange);
				pcombo = mUI["lighting"]->castType<MyGUI::ComboBox>(false);
				pcombo->addItem( "Modulative" );
				pcombo->addItem( "Additive" );
				if( st & Ogre::SHADOWDETAILTYPE_ADDITIVE )
					pcombo->setIndexSelected( 1 );
				else if( st & Ogre::SHADOWDETAILTYPE_MODULATIVE )
					pcombo->setIndexSelected( 0 );
				pcombo->eventComboChangePosition += newDelegate(this,&Game::notifyComboChange);

				mUI->eventWindowButtonPressed += newDelegate(this, &Game::notifyWindowButtonPressed);
			}
		}catch( out_of_range& e ){
			WARNING_LOG(e.what());
		}
	}
}

Ogre::ShadowTechnique Game::getShadowTechnique( size_t sel ){
	switch( sel ){
		case 0:
			return Ogre::SHADOWTYPE_NONE;
		case 1:
			return Ogre::SHADOWDETAILTYPE_STENCIL;
		case 2: 
			return Ogre::SHADOWDETAILTYPE_TEXTURE;
	}
	return Ogre::SHADOWTYPE_NONE;
}

Ogre::ShadowTechnique Game::getShadowMode( size_t sel ){
	switch( sel ){
		case 0:
			return Ogre::SHADOWDETAILTYPE_MODULATIVE;
		case 1:
			return Ogre::SHADOWDETAILTYPE_ADDITIVE;
	}
	return Ogre::SHADOWDETAILTYPE_ADDITIVE;
}

void Game::notifyComboChange(MyGUI::ComboBox* _sender, size_t _index){
	int st = static_cast<int>(Ogre::SHADOWTYPE_NONE);

	if( mUI.IsWidget(_sender,"shadow") ){
		st = static_cast<int>(getShadowTechnique( _index ));
		if( st != static_cast<int>(Ogre::SHADOWTYPE_NONE) ){
			try{
				MyGUI::ComboBox* pcombo = mUI["lighting"]->castType<MyGUI::ComboBox>(false);
				size_t index = pcombo->getIndexSelected();
				st |= static_cast<int>(getShadowMode( index ));
			}catch( out_of_range& e ){
				WARNING_LOG(e.what());
			}
		}
	}else if( mUI.IsWidget(_sender,"lighting") ){
		try{
			MyGUI::ComboBox* pcombo = mUI["shadow"]->castType<   MyGUI::ComboBox>(false);
			int sel = pcombo->getIndexSelected();
			st = static_cast<int>(getShadowTechnique( sel ));
		}catch( out_of_range& e ){
			WARNING_LOG(e.what());
			return;
		}
		if( st != static_cast<int>(Ogre::SHADOWTYPE_NONE) )
			st |= static_cast<int>(getShadowMode( _index ));
	}
setShadowTechnique( static_cast<Ogre::ShadowTechnique>(st) );
}

void Game::setShadowTechnique(Ogre::ShadowTechnique st){
	if( mSceneManager ){
		if( st & Ogre::SHADOWDETAILTYPE_TEXTURE )
		{
			//材质阴影方式
			Ogre::ShadowCameraSetupPtr mCurrentShadowCameraSetup = 
			Ogre::ShadowCameraSetupPtr(new Ogre::PSSMShadowCameraSetup ());
			
			//材质设置，来自于Ogre sample
			mSceneManager->setShadowCameraSetup(mCurrentShadowCameraSetup);
			mSceneManager->setShadowTexturePixelFormat(Ogre::PF_X8R8G8B8);
			mSceneManager->setShadowTextureCasterMaterial(Ogre::StringUtil::BLANK);
			mSceneManager->setShadowTextureReceiverMaterial(Ogre::StringUtil::BLANK);
			mSceneManager->setShadowTextureSelfShadow(false);
			//调整材质大小
			//getSceneManager()->setShadowTextureSize(2048);
		}
		mSceneManager->setShadowTechnique( st );
	}
}

//数据发生改变
void Game::simpleDataChange(SimpleData* psd){
	assert( psd );
	try{
		mNeedReset = true;
		MyGUI::Button* pb = mUI["Apply"]->castType<MyGUI::Button>(false);
		if( pb )pb->setStateSelected(true);
	}catch(  out_of_range& e ){
		WARNING_LOG(e.what());
	}
}

void Game::notifyWindowButtonPressed(MyGUI::Window* _sender, const string& _button){
	if( _button=="close" )
		mUI.close();
}

void Game::notifyButtonClick(MyGUI::Widget* _sender){
	if( mUI.IsWidget(_sender,"Cancel") ){
		mUI.close();
	}else if( mUI.IsWidget(_sender,"Apply") ){
		if( !mNeedReset ){
			mUI.close();
			return;
		}
		try{
			SimpleDataUI sdu(mUI["OgreView"]);
			Ogre::ConfigOptionMap& mp = mRoot->getRenderSystem()->getConfigOptions();
			for( Ogre::ConfigOptionMap::iterator i=mp.begin();i!=mp.end();++i ){
				SimpleData sd = sdu.get( i->first );
				if( sd.type==SimpleData::STRING&&sd.str != i->second.currentValue ){
					mRoot->getRenderSystem()->setConfigOption(i->first,sd.str);
				}
			}
			mUI.close();
			mRoot->queueEndRendering();
			reset(NORMAL);
		}catch( out_of_range& e ){
			WARNING_LOG(e.what());
		}
	}
}

void Game::injectKeyPress(MyGUI::KeyCode _key, MyGUI::Char _text)
{
	if( _text == '`' )
	{
		if( !mConsole.isOpen() )
			mConsole.openConsole();
		else
			mConsole.closeConsole();
		return;
	}

	BaseManager::injectKeyPress( _key,_text );
	/*
	if (_key == MyGUI::KeyCode::Escape)
	{
		quit();
		return;
	}else */
	if (_key == MyGUI::KeyCode::SysRq)
	{
		std::ifstream stream;
		std::string file;
		do
		{
			stream.close();
			static size_t num = 0;
			const size_t max_shot = 100;
			if (num == max_shot)
			{
				WARNING_LOG( "The limit of screenshots is exceeded : " << max_shot);
				return;
			}
			file = MyGUI::utility::toString("screenshot_", ++num, ".png");
			stream.open(file.c_str());
		}
		while (stream.is_open());
		mWindow->writeContentsToFile(file);
		return;
	}
	else if (_key == MyGUI::KeyCode::F12)
	{
		bool visible = mFocusInfo->getFocusVisible();
		mFocusInfo->setFocusVisible(!visible);
	}
	else if(_key == MyGUI::KeyCode::F5)
	{
		//重新启动lua
		reset(NORMAL);
	}
	// change polygon mode
	// TODO: polygon mode require changes in platform
	else if (_key == MyGUI::KeyCode::F8)
	{
		getCamera()->setPolygonMode(Ogre::PM_SOLID);
	}
	else if (_key == MyGUI::KeyCode::F9)
	{
		getCamera()->setPolygonMode(Ogre::PM_WIREFRAME);
	}
	else if (_key == MyGUI::KeyCode::F10)
	{
		getCamera()->setPolygonMode(Ogre::PM_POINTS);
	}
	else if( _key == MyGUI::KeyCode::F11 )
	{
		mInfo->setVisible( !mInfo->getVisible() );
	}

#if OGRE_VERSION >= MYGUI_DEFINE_VERSION(1, 7, 0) && OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
	else if (_key == MyGUI::KeyCode::F1)
	{
		mWindow->getViewport(0)->setOrientationMode(Ogre::OR_DEGREE_0, false);
		mPlatform->getRenderManagerPtr()->setRenderWindow(mWindow);
	}
	else if (_key == MyGUI::KeyCode::F2)
	{
		mWindow->getViewport(0)->setOrientationMode(Ogre::OR_DEGREE_90, false);
		mPlatform->getRenderManagerPtr()->setRenderWindow(mWindow);
	}
	else if (_key == MyGUI::KeyCode::F3)
	{
		mWindow->getViewport(0)->setOrientationMode(Ogre::OR_DEGREE_180, false);
		mPlatform->getRenderManagerPtr()->setRenderWindow(mWindow);
	}
	else if (_key == MyGUI::KeyCode::F4)
	{
		mWindow->getViewport(0)->setOrientationMode(Ogre::OR_DEGREE_270, false);
		mPlatform->getRenderManagerPtr()->setRenderWindow(mWindow);
	}
#endif
}

bool Game::frameStarted(const Ogre::FrameEvent& _evt){
	bool ret = BaseManager::frameStarted( _evt );
	/*这里是物理模拟部分。
		这里是不是可以考虑使用多线程来处理呢？
	*/
	if( mRigidManager ){
		mRigidManager->SimulationFrame( _evt.timeSinceLastFrame );
	}
	return ret;
}

/*
	"simple"
	设置一个视角控制方式。在切换场景的适合重置。
*/
void Game::setCameraControlet( const string& name )
{
	if( name == "simple" )
	{
		mCameraControlet = ControletPtr(new SimpleCameraControlet( mCamera ));
	}
	else
	{
		mCameraControlet.reset();
	}
}

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE

/* Mac在编译的时候有名字冲突,Mac头文件中的id于ode中的冲突
 */
static tGame app;

void _rander_init()
{
    try {
        app.prepare();
		app.init();
	} catch( MyGUI::Exception& e ) {
		std::cerr << "An exception has occured" << " : " << e.getFullDescription().c_str();
	}
}

bool _rander_one_frame()
{
	if( Ogre::Root::getSingletonPtr() && Ogre::Root::getSingleton().isInitialised()){
		if( !app.renderOneFrame() ){
			app.shutdown();
			//是否要重启
			if( app.isReset() ){
				app.prepare();
				app.init();
			}
            else
                return false;
		}
		return true;
	}
	return false;
}
#endif