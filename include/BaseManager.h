/*!
	该文件修改至MyGUI的同名文件
*/

#ifndef __BASE_MANAGER_H__
#define __BASE_MANAGER_H__
#ifdef WIN32
#include "win32/InputManager.h"
#else
#include "mac/InputManager.h"
#endif
#include "PointerManager.h"
#include "StatisticInfo.h"
#include "InputFocusInfo.h"
#include "Console.h"
#include "Controlet.h"
#include "InputFilter.h"
#include "Log.h"

namespace base
{

	class BaseManager :
		public InputManager,
		public PointerManager,
		public Ogre::FrameListener,
		public Ogre::WindowEventListener,
		public Ogre::Renderable::Visitor
	{
	public:
		BaseManager();
		virtual ~BaseManager();

		virtual void prepare();
		bool init();
		void shutdown();
		void quit();

		enum ResetType
		{
			NORMAL,
			FAST
		};
		void reset( ResetType rt );

		void setWindowCaption(const std::wstring& _text);
		std::wstring getWindowCaption() const;

		void createDefaultScene();

		void setResourceFilename(const std::string& _flename);

		//返回从游戏开始进行了多少帧
		unsigned long getFrameCount();

		diagnostic::StatisticInfo* getStatisticInfo();
		diagnostic::InputFocusInfo* getFocusInput();

	/*internal:*/
		Ogre::SceneManager* getSceneManager();
		Ogre::Camera* getCamera();

		//x ,y 是屏幕点，该函数返回一个可移动对象的名称
		string pickMovableObject( int x,int y );
		//取得平面的尺寸
		int getScreenWidth();
		int getScreenHeight();
		//求可绘制对象的交点，这些Real值是到ray原点的距离
		vector<Ogre::Real> Intersect(const Ogre::Ray& ray,Ogre::Renderable *rend);

		string getResourcePath() const;

#ifdef WIN32
		void run();
#else
		bool isReset(){ return mReset; }
		bool renderOneFrame();
#endif
	protected:
		virtual void createScene(){}
		virtual void destroyScene(){}

		virtual void setupResources();

		virtual void injectMouseMove(int _absx, int _absy, int _absz);
		virtual void injectMousePress(int _absx, int _absy, MyGUI::MouseButton _id);
		virtual void injectMouseRelease(int _absx, int _absy, MyGUI::MouseButton _id);
		virtual void injectKeyPress(MyGUI::KeyCode _key, MyGUI::Char _text);
		virtual void injectKeyRelease(MyGUI::KeyCode _key);

		void createGui();
		void destroyGui();

		virtual void createSceneManager();
		virtual void destroySceneManager();

		virtual bool frameStarted(const Ogre::FrameEvent& _evt);
		virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
		virtual bool frameEnded(const Ogre::FrameEvent& _evt);
		virtual void windowResized(Ogre::RenderWindow* _rw);
		virtual void windowClosed(Ogre::RenderWindow* _rw);

		virtual void visit (Ogre::Renderable *rend, Ogre::ushort lodIndex, bool isDebug, Ogre::Any *pAny);

		void addGroupLocation( const string& group,const string& type,const string& location,bool recursive );

		typedef vector<string> LuaVector;

		bool loadBase( const string& filename,
			LuaVector& doScript,
		LuaVector& locationScript,
		LuaVector& loadScript );

		MyGUI::Gui* mGUI;
		MyGUI::OgrePlatform* mPlatform;
		diagnostic::StatisticInfo* mInfo;
		diagnostic::InputFocusInfo* mFocusInfo;

		Ogre::Root* mRoot;
		Ogre::Camera* mCamera;
		Ogre::SceneManager* mSceneManager;
		Ogre::RenderWindow* mWindow;

		bool mExit;
		bool mReset;
		bool mFastReset;

		Ogre::String mResourcePath;
		std::string mPluginCfgName;
		std::string mResourceXMLName;
		std::string mResourceFileName;

		Ogre::SceneNode* mNode;

		CoutSource* mCoutSource;
		
		unsigned long mFrameCount;

		Console mConsole;

		Ogre::RaySceneQuery* mSceneQuery;
		
		InputFilter* mInputFilter;

		unsigned long mPickFrameCount;
		int mPickX;
		int mPickY;
		string mPickName;
		Ogre::Ray mRay;
		bool mIntersect;
		Ogre::Real mIntersectDistance;

		LuaVector mPreLuaScript;
		LuaVector mPreLuaLocation;
		LuaVector mPreLoad;

		private:
			void _fastReset();
	};

} // namespace base

#endif // __BASE_MANAGER_H__
