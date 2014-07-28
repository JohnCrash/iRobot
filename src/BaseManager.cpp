/*该文件修改至MyGUI的同名文件
*/
#include "stdheader.h"
#include "BaseManager.h"
#include "Math3d.h"

#if WIN32
#include <windows.h>
#endif

namespace base
{

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#include <CoreFoundation/CoreFoundation.h>
	// This function will locate the path to our application on OS X,
	// unlike windows you can not rely on the curent working directory
	// for locating your configuration files and resources.
	std::string macBundlePath()
	{
		char path[1024];
		CFBundleRef mainBundle = CFBundleGetMainBundle();
		assert(mainBundle);
		CFURLRef mainBundleURL = CFBundleCopyBundleURL(mainBundle);
		assert(mainBundleURL);
		CFStringRef cfStringRef = CFURLCopyFileSystemPath( mainBundleURL, kCFURLPOSIXPathStyle);
		assert(cfStringRef);
		CFStringGetCString(cfStringRef, path, 1024, kCFStringEncodingASCII);
		CFRelease(mainBundleURL);
		CFRelease(cfStringRef);
		return std::string(path);
	}
#endif

	BaseManager::BaseManager() :
		mGUI(nullptr),
		mInfo(nullptr),
		mFocusInfo(nullptr),
		mRoot(nullptr),
		mCamera(nullptr),
		mSceneManager(nullptr),
		mWindow(nullptr),
		mExit(false),
		mReset(true),
		mFastReset(false),
		mPluginCfgName("plugins.cfg"),
		mResourceXMLName("resources.xml"),
		mResourceFileName("mygui/MyGUI_Core.xml"),
		mNode(nullptr),
		mCoutSource(nullptr),
		mFrameCount(0)
	{
		#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
			mResourcePath = macBundlePath() + "/Contents/Resources/";
			mResourceXMLName = macBundlePath()+"/Contents/Resources/"+mResourceXMLName;
		#else
			mResourcePath = "../";
			mResourceXMLName = mResourcePath + mResourceXMLName;
		#endif
	}

	BaseManager::~BaseManager()
	{
	}

	void BaseManager::prepare()
	{
	}

	bool BaseManager::init()
	{
		Ogre::String pluginsPath;

		mFrameCount = 0;
		mPickFrameCount = 0;
		mPickX = 0;
		mPickY = 0;

		#ifndef OGRE_STATIC_LIB
			pluginsPath = mResourcePath + mPluginCfgName;
		#endif

		mFastReset = false;
		mReset = false;
		mExit = false;
		
		mInputFilter = new InputFilter();

		mRoot = new Ogre::Root(pluginsPath, mResourcePath + "ogre.cfg", mResourcePath + "Ogre.log");

		Ogre::Log* plog = Ogre::LogManager::getSingleton().getDefaultLog();
		if( plog ){
			mCoutSource = new CoutSource();
			plog->addListener( mCoutSource );
		}
        
		//恢复Ogre配置，如果第一次配置启动一个Ogre配置对话框
		if (!mRoot->restoreConfig())
		{
			if (!mRoot->showConfigDialog()) return false;
		}

		mWindow = mRoot->initialise(true);

		createSceneManager();

		// Set default mipmap level (NB some APIs ignore this)
		Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

		mRoot->addFrameListener(this);
		Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

		size_t handle = 0;
		mWindow->getCustomAttribute("WINDOW", &handle);

		mSceneQuery = mSceneManager->createRayQuery( Ogre::Ray() );

		createGui();
        
		createInput(handle);

		createPointerManager(handle);
        
		createScene();

		windowResized(mWindow);

		mRoot->getRenderSystem()->_initRenderTargets();

		setPointerVisible(false);

		return true;
	}
	
	void BaseManager::createSceneManager(){
		mSceneManager = mRoot->createSceneManager(Ogre::ST_GENERIC, "BaseSceneManager");

		mCamera = mSceneManager->createCamera("BaseCamera");
		mCamera->setNearClipDistance(5);
		mCamera->setPosition(100, 100, 100);
		mCamera->lookAt(0, 0, 0);

		// Create one viewport, entire window
		Ogre::Viewport* vp = mWindow->addViewport(mCamera);
		// Alter the camera aspect ratio to match the viewport
		mCamera->setAspectRatio((float)vp->getActualWidth() / (float)vp->getActualHeight());

		mSceneManager->setAmbientLight(Ogre::ColourValue::White);
		Ogre::Light* light = mSceneManager->createLight("MainLight");
		light->setType(Ogre::Light::LT_DIRECTIONAL);
		Ogre::Vector3 vec(-0.3f, -0.3f, -0.3f);
		vec.normalise();
		light->setDirection(vec);
	}

	void BaseManager::destroySceneManager(){
		if (mSceneManager)
		{
			mSceneManager->clearScene();
			/*如果删除全部Camera将可能引起崩溃，因为阴影算法中的某些部分
			也创建Camera，如果这里全部删除。在阴影算法释放Camera时将会崩溃
			Texture阴影算法就会引发这个问题
			这里建议仅仅释放自己分配的Camera
			*/
			//mSceneManager->destroyAllCameras();
			if( mCamera )
				mSceneManager->destroyCamera( mCamera );
			mSceneManager = nullptr;
			mCamera = nullptr;
		}
	}

#ifdef WIN32
	void BaseManager::run()
	{
		while( mReset ){
			prepare();

			if( init() ){
				mRoot->startRendering();
				shutdown();
			}
		}
	}
#else//APPLE
	bool BaseManager::renderOneFrame()
	{
		return mRoot->renderOneFrame();
	}
#endif

	void BaseManager::shutdown()
	{
		mPreLuaScript.clear();
		mPreLuaLocation.clear();
		mPreLoad.clear();

		destroyScene();

		destroyPointerManager();

		destroyInput();

		destroyGui();

		mSceneManager->destroyQuery( mSceneQuery );
		mSceneQuery = nullptr;

		destroySceneManager();

		//清除内部帧监听器
		//必须在destroyScene等后面被施放
		delete mInputFilter;
		mInputFilter = nullptr;

		if (mRoot)
		{
			Ogre::RenderWindow* window = mRoot->getAutoCreatedWindow();
			if (window)
				window->removeAllViewports();
			OGRE_DELETE mRoot;
			mRoot = nullptr;
		}

		if( mCoutSource ){
			//删除mCoutSource，不要过早删除它。
			delete mCoutSource;
			mCoutSource = nullptr;
		}
	}

	void BaseManager::createGui()
	{
		mPlatform = new MyGUI::OgrePlatform();
		mPlatform->initialise(mWindow, mSceneManager,Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
        
        //setupResources()中使用了DataManager,而DataManager在OgrePlatform::initialise中初始化
        setupResources();
        
		if( mCoutSource ){
			//这里加入一个监听器，MyGUI并不负责删除监听器(BUG)
			MyGUI::LogManager::getInstance().getDefaultSource()->addLogListener(mCoutSource);
			//关闭默认的控制台输出
			MyGUI::LogManager::getInstance().setSTDOutputEnabled(false);
		}

		mGUI = new MyGUI::Gui();
		mGUI->initialise(mResourceFileName);

		mInfo = new diagnostic::StatisticInfo();
		mFocusInfo = new diagnostic::InputFocusInfo();
	}

	void BaseManager::destroyGui()
	{
		if (mGUI)
		{
			if (mInfo)
			{
				delete mInfo;
				mInfo = nullptr;
			}

			if (mFocusInfo)
			{
				delete mFocusInfo;
				mFocusInfo = nullptr;
			}

			mGUI->shutdown();
			delete mGUI;
			mGUI = nullptr;
		}

		if (mPlatform)
		{
			mPlatform->shutdown();
			delete mPlatform;
			mPlatform = nullptr;
		}
	}

	void BaseManager::setupResources()
	{
		loadBase(mResourceXMLName,mPreLuaScript,mPreLuaLocation,mPreLoad);
	}
	
	string BaseManager::getResourcePath() const
	{
		return mResourcePath;
	}

	bool BaseManager::loadBase( 
		const string& filename,
		LuaVector& doScript,
		LuaVector& locationScript,
		LuaVector& loadScript
		)
	{
		MyGUI::xml::Document doc;
		
		if( !doc.open(mResourcePath+filename) )
		{
			if( !doc.open(filename) )
			{
				MyGUI::IDataStream* pdata = MyGUI::DataManager::getInstance().getData(filename);
				if( pdata )
				{
					if( !doc.open(pdata) )
					{
						MyGUI::DataManager::getInstance().freeData(pdata);
						ERROR_LOG(doc.getLastError());
						return false;
					}
					MyGUI::DataManager::getInstance().freeData(pdata);
				}
				else
				{
					ERROR_LOG(doc.getLastError());
					return false;
				}
			}
		}
		
		MyGUI::xml::ElementPtr root = doc.getRoot();
		if (root == nullptr || root->getName() != "base")
			return false;
//		LuaVector groups;

		MyGUI::xml::ElementEnumerator node = root->getElementEnumerator();
		while( node.next() )
		{
			string name = node->getName();
			if( name == "script" )
			{
				string lua;
				if( node->findAttribute("do",lua) )
					doScript.push_back( lua ); //等到LuaManager被创建后在一起执行
				if( node->findAttribute("location",lua) )
				{
					if( find(locationScript.begin(),locationScript.end(),lua)==locationScript.end() )
						locationScript.push_back( lua );
				}
				if( node->findAttribute("load",lua) )
				{
					loadScript.push_back(lua);
				}
			}
			else if( name == "group" )
			{
				string name;
				if( node->findAttribute("name",name) )
				{
//					if( find(groups.begin(),groups.end(),name)==groups.end() )
//						groups.push_back(name);
					MyGUI::xml::ElementEnumerator child = node->getElementEnumerator();
					while( child.next() )
					{
						string type,location,recursive;
						if( child->getName() == "repository" )
						{
							if( child->findAttribute("type",type) && 
								child->findAttribute("recursive",recursive) &&
								child->findAttribute("location",location) )
							{
								//加入组
								addGroupLocation(name,type,location,recursive=="true"?true:false);
							}
						}
					}
				}
			}
		}
		return true;
	}

	void BaseManager::addGroupLocation( const string& group,const string& type,const string& location,bool recursive )
	{
		Ogre::ResourceGroupManager::getSingleton().addResourceLocation(location,type, group,recursive);
	}

	//删除全部窗口，施放默认组中的数据
	void BaseManager::_fastReset()
	{
		if (mInfo)
		{
			delete mInfo;
			mInfo = nullptr;
		}

		if (mFocusInfo)
		{
			delete mFocusInfo;
			mFocusInfo = nullptr;
		}
		mConsole.closeConsole();
		mGUI->destroyAllChildWidget();

		Ogre::ResourceGroupManager::getSingleton().destroyResourceGroup
			(Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		Ogre::ResourceGroupManager::getSingleton().createResourceGroup
			(Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		/*
		Ogre::ResourceGroupManager::getSingleton().unloadResourceGroup
			(Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		Ogre::ResourceGroupManager::getSingleton().clearResourceGroup
			(Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		*/
		mInfo = new diagnostic::StatisticInfo();
		mFocusInfo = new diagnostic::InputFocusInfo();
	}

	bool BaseManager::frameStarted(const Ogre::FrameEvent& evt)
	{
		if (mExit)
			return false;

		if (!mGUI)
			return true;

		if( mFastReset )
		{
			mFastReset = false;
			destroyScene();
			_fastReset();
			createScene();
		}

		captureInput();

		//更新控制台内容
		mConsole.updateConsole();
		
		if( mInputFilter )
			mInputFilter->frameStarted( evt );

		if (mInfo)
		{
			static float time = 0;
			time += evt.timeSinceLastFrame;
			if (time > 1&&mInfo->getVisible())
			{
				time -= 1;
				try
				{
					const Ogre::RenderTarget::FrameStats& stats = mWindow->getStatistics();
					mInfo->change("FPS", (int)stats.lastFPS);
					mInfo->change("triangle", stats.triangleCount);
					mInfo->change("batch", stats.batchCount);
					mInfo->change("batch gui", MyGUI::OgreRenderManager::getInstance().getBatchCount());
					mInfo->update();
				}
				catch (...)
				{
					MYGUI_LOG(Warning, "Error get statistics");
				}
			}
		}

		return true;
	}
	
	void BaseManager::reset( ResetType rt )
	{
		if( rt == NORMAL )
		{
			mReset = true;
			quit();
		}
		else
		{
			//等等下一个渲染周期开始的时候重新启动
			mFastReset = true;
		}
	}

	unsigned long BaseManager::getFrameCount(){
		return mFrameCount;
	}

	bool BaseManager::frameRenderingQueued(const Ogre::FrameEvent& evt){
		if( mInputFilter )
			mInputFilter->frameRenderingQueued( evt );
		return true;
	}

	bool BaseManager::frameEnded(const Ogre::FrameEvent& evt)
	{
		if( mInputFilter )
			mInputFilter->frameEnded( evt );
		++mFrameCount;
		return true;
	}

	void BaseManager::windowResized(Ogre::RenderWindow* _rw)
	{
		int width = (int)_rw->getWidth();
		int height = (int)_rw->getHeight();

		if (mCamera)
		{
			mCamera->setAspectRatio((float)width / (float)height);

			setInputViewSize(width, height);
		}
	}

	void BaseManager::windowClosed(Ogre::RenderWindow* _rw)
	{
		quit();
	}

	void BaseManager::setWindowCaption(const std::wstring& _text)
	{
	#if WIN32
		size_t handle = 0;
		mWindow->getCustomAttribute("WINDOW", &handle);
		::SetWindowTextW((HWND)handle, _text.c_str());
	#endif
	}

	std::wstring BaseManager::getWindowCaption() const
	{
	#if WIN32
		std::wstring _text;
		size_t handle = 0;
		mWindow->getCustomAttribute("WINDOW", &handle);
		_text.resize(256);
		int n = ::GetWindowTextW((HWND)handle, (LPWSTR)_text.c_str(),_text.size()-1);
		_text.resize(n);
		return _text;
	#endif
	}

	void BaseManager::createDefaultScene()
	{
		mNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
		try
		{
			Ogre::Entity* entity = mSceneManager->createEntity("Mikki.mesh", "Mikki.mesh");			
			mNode->attachObject(entity);
		}
		catch (Ogre::FileNotFoundException&)
		{
			WARNING_LOG( "Can't load Mikki.mesh" )
		}

		try
		{
			Ogre::MeshManager::getSingleton().createPlane(
				"FloorPlane", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
				Ogre::Plane(Ogre::Vector3::UNIT_Y, 0), 1000, 1000, 1, 1, true, 1, 1, 1, Ogre::Vector3::UNIT_Z);

			Ogre::Entity* entity = getSceneManager()->createEntity("FloorPlane", "FloorPlane");
			entity->setMaterialName("Ground");
			mNode->attachObject(entity);
		}
		catch (Ogre::FileNotFoundException&)
		{
			return;
		}
	}

	void BaseManager::injectMouseMove(int _absx, int _absy, int _absz)
	{
		if (!mGUI)
			return;

		if( mInputFilter )
			mInputFilter->injectMouseMove( _absx,_absy,_absz );
	}

	void BaseManager::injectMousePress(int _absx, int _absy, MyGUI::MouseButton _id)
	{
		if (!mGUI)
			return;

		if( mInputFilter )
			mInputFilter->injectMousePress( _absx,_absy,_id );
	}

	void BaseManager::injectMouseRelease(int _absx, int _absy, MyGUI::MouseButton _id)
	{
		if (!mGUI)
			return;

		if( mInputFilter )
			mInputFilter->injectMouseRelease( _absx,_absy,_id );

	}

	void BaseManager::injectKeyPress(MyGUI::KeyCode _key, MyGUI::Char _text)
	{
		if (!mGUI)
			return;

		if( mInputFilter )
			mInputFilter->injectKeyPress( _key,_text );
	}

	void BaseManager::injectKeyRelease(MyGUI::KeyCode _key)
	{
		if (!mGUI)
			return;

		if( mInputFilter )
			mInputFilter->injectKeyRelease( _key );
	}

	void BaseManager::quit()
	{
        if( !mExit )
        {
            mExit = true;
            mConsole.closeConsole();
            mRoot->saveConfig();
            mRoot->queueEndRendering();
        }
	}

	void BaseManager::setResourceFilename(const std::string& _flename)
	{
		mResourceFileName = _flename;
	}

	diagnostic::StatisticInfo* BaseManager::getStatisticInfo()
	{
		return mInfo;
	}

	diagnostic::InputFocusInfo* BaseManager::getFocusInput()
	{
		return mFocusInfo;
	}

	Ogre::SceneManager* BaseManager::getSceneManager()
	{
		return mSceneManager;
	}

	Ogre::Camera* BaseManager::getCamera()
	{
		return mCamera;
	}


	int BaseManager::getScreenWidth(){
		return mWindow->getWidth();
	}

	int BaseManager::getScreenHeight(){
		return mWindow->getHeight();
	}

	/*求对象rend和射线ray的全部交点到射线原点的距离
	*/
	vector<Ogre::Real> BaseManager::Intersect(const Ogre::Ray& ray,Ogre::Renderable *rend){
		Ogre::RenderOperation op;
		Ogre::VertexElementType vtype;
		size_t offset,pkgsize,source,indexNums,vertexNums;
		vector<Ogre::Real> result;

		rend->getRenderOperation( op );

		if( !op.indexData || 
			op.operationType==Ogre::RenderOperation::OT_LINE_LIST || 
			op.operationType==Ogre::RenderOperation::OT_LINE_STRIP ||
			op.operationType==Ogre::RenderOperation::OT_POINT_LIST )
			return result;

		Ogre::VertexDeclaration* pvd = op.vertexData->vertexDeclaration;

		source = -1;
		for( size_t i = 0;i < pvd->getElementCount();++i ){
			if( pvd->getElement(i)->getSemantic()==Ogre::VES_POSITION ){
				source = pvd->getElement(i)->getSource();
				offset = pvd->getElement(i)->getOffset();
				vtype = pvd->getElement(i)->getType();
				break;
			}
		}
		if( source == - 1 || vtype != Ogre::VET_FLOAT3 ) //别的格式目前没有考虑
			return result;

		/*source对应与一个缓存区
		  getVertexSize(source)求缓存区中一个紧密数据包的大小
		  例如：一个数据包里面包括POSITION,COLOR,NORMAL,TEXCROOD然后在这个缓冲
			区中循环。而getVertexSize求出这个包的字节大小
		  例如POSITION(FLOAT3) TEXCROOD(FLOAT2) 这样前面的是12字节后面的是8字节
			getVertexSize返回20
		*/
		pkgsize = pvd->getVertexSize(source);

		Ogre::HardwareVertexBufferSharedPtr hvb = op.vertexData->vertexBufferBinding->getBuffer(source);
		Ogre::HardwareIndexBufferSharedPtr ivb = op.indexData->indexBuffer;
		Ogre::HardwareIndexBuffer::IndexType indexType = op.indexData->indexBuffer->getType();
		/*先将顶点数据复制一份，然后变换到世界坐标系
		*/
		vertexNums = hvb->getNumVertices();
		indexNums = ivb->getNumIndexes();
		 
		boost::scoped_array<float> vp( new float[3*vertexNums] );
		boost::scoped_array<unsigned int> ip( new unsigned int[indexNums] );
		
		{
			Ogre::Vector3 p3;
			Ogre::Matrix4 mat;
			rend->getWorldTransforms( &mat );
			float* preal = (float*)hvb->lock( Ogre::HardwareBuffer::HBL_READ_ONLY );
			float* ptarget = vp.get();
			//这里考虑到对齐，我假设offset和pkgsize都可以被sizeof(float)整除
			preal += offset/sizeof(float);
			size_t strip = pkgsize/sizeof(float);
			for( size_t i = 0; i < vertexNums;++i ){
				p3.x = *preal;
				p3.y = *(preal+1);
				p3.z = *(preal+2);
				p3 = mat * p3;
				*ptarget++ = p3.x;
				*ptarget++ = p3.y;
				*ptarget++ = p3.z;
				preal += strip;
			}
			hvb->unlock();
		}
		//拷贝出顶点数据
		
		{
			unsigned int* pindex32 = ip.get();
			
			if( indexType==Ogre::HardwareIndexBuffer::IT_16BIT ){
				unsigned short* pi16 = (unsigned short*)ivb->lock( Ogre::HardwareBuffer::HBL_READ_ONLY );
				copy( pi16,pi16+indexNums,pindex32 );
			}else
				memcpy( pindex32,ivb->lock( Ogre::HardwareBuffer::HBL_READ_ONLY ),ivb->getSizeInBytes() );
			ivb->unlock();
		}
		/*数据都准备好了，vp保存了变换好的顶点，ip保存了顶点索引
			下面根据情况求交点
		*/
		switch( op.operationType ){
			case Ogre::RenderOperation::OT_TRIANGLE_LIST:
				{ /* 0,1,2 组成一个三角 3,4,5 下一个...
				  */
					Ogre::Vector3 a[3],n;
					int index,k = 0;
					float* preal = vp.get();
					unsigned int* pindex = ip.get();
					for( size_t i = 0;i<indexNums;++i ){
						if( pindex[i] < vertexNums ){
							index = pindex[i]*3; //对应与格式VET_FLOAT3
							a[k].x = preal[index];
							a[k].y = preal[index+1];
							a[k].z = preal[index+2];
							if( k == 2 ){//三个点都填满了
								//这里使用的是Math3d的求交函数，而不是Ogre::Math的
								//原因就在于Ogre::Math的求交点函数不能得到射线背面那个负的交点
								std::pair<bool,Ogre::Real> res = Math3d::intersects(ray,a[0],a[1],a[2],true,true);
								if( res.first )
									result.push_back( res.second );
								k = 0;
							}else
								k++;
						}else{
							WARNING_LOG("Game::Intersect"<<" Invalid index rang out" << " pindex["<<i<<"]="<<pindex[i] << "("<<vertexNums<<")");
							return result;
						}
					}
				}
				break;
			case Ogre::RenderOperation::OT_TRIANGLE_FAN:
				{/* 0,1,2组成一个三角 0,2,3 组成下一个 0,3,4...
				 */
					assert( false||"Game::Intersect can't support OT_TRIANGLE_FAN " );
				}
				break;
			case Ogre::RenderOperation::OT_TRIANGLE_STRIP:
				{//0,1,2组成第一个三角 1,2,3 组成下一个 2,3,4...
					assert( false||"Game::Intersect can't support OT_TRIANGLE_STRIP " );
				}
				break;
			default:;
		}
		return result;
	}

	/*遍历一个Renderable的全部可绘制三角求射线mRay和它的交点
	*/
	void BaseManager::visit(Ogre::Renderable *rend, Ogre::ushort lodIndex, bool isDebug, Ogre::Any *pAny){
		//如果有交点
		vector<Ogre::Real> result = Intersect( mRay,rend );
		mIntersect = !(result.empty());
		if( mIntersect ){
			//做一个排序
			sort( result.begin(),result.end() );
			//求离眼睛最进的距离
			mIntersectDistance = result[0];
		}
	}

	//该函数不支持多线程
	string BaseManager::pickMovableObject( int x,int y ){
		//如果相同的帧同时相同的x,y就直接返回结果
		if( mPickFrameCount == getFrameCount() && 
			mPickX == x && mPickY == y ){
			return mPickName;
		}

		mRay = mCamera->getCameraToViewportRay(
				(Ogre::Real)x/(Ogre::Real)mWindow->getWidth(),
				(Ogre::Real)y/(Ogre::Real)mWindow->getHeight()
				);

		mSceneQuery->setRay( mRay );

		//距离排序
		mSceneQuery->setSortByDistance(true);
		Ogre::RaySceneQueryResult result = mSceneQuery->execute();

		//execute不能执行像素级的查询，它仅仅是和包围盒进行比较
		//可以考虑使用渲染到纹理，使用1x1的纹理来对查询结果进行像素级别的查询
		//这里使用逐个求交点的办法
		Ogre::MovableObject* pmo = nullptr;
		Ogre::Real dis;
		for( size_t i = 0;i < result.size();++i ){
			if( result[i].movable && 
				result[i].movable->isVisible() &&  //不可见的
				result[i].movable->isInScene() ){ //不在场景中的
				mIntersect = false;
				result[i].movable->visitRenderables(this);
				if( mIntersect ){
					//考虑到同一帧中间有多个地方的代码需要调用该函数
					//这样使用内存换速度
					if( !pmo || dis>mIntersectDistance ){
						dis = mIntersectDistance;
						pmo = result[i].movable;
					}				
				}
			}
		}

		if( pmo ){
			mPickX = x;
			mPickY = y;
			mPickFrameCount = getFrameCount();
			mPickName = pmo->getName();
			return mPickName;
		}
		//返回一个空字串表示没有
		return "";
	}
} // namespace base
