#ifndef _TESTSTAGE_H_
#define _TESTSTAGE_H_

#include "CameraControlet.h"

class TestStage:
	public Stage,
	public Controlet,
	public Ogre::ResourceGroupListener
{
public:
	virtual bool openStage();
	virtual void closeStage();

	void addObject( const string& name );
	void setCurrentObject( const string& name );
	virtual void injectKeyPress(MyGUI::KeyCode _key, MyGUI::Char _text);
protected:
	SimpleUI mUI;
	SimpleUI mMatDialog;

	string mCurrentObjectName;

	SimpleCameraControletPtr mCamControlet;

	string mSelectName;

	string mMaterial;
	int mSerial;
	
	Ogre::SceneNode* mNode;
	Ogre::Entity* mEntity;

	enum RigidToolEnum{
		MOVE,
		ROTATE,
		MOVE_LOCAL,
		MODIFY,
		MOVE2D
	};
	size_t mProgress;
	RigidToolEnum mRigidTool;
	size_t mFirstTexture;
	string mMatName;
	size_t mTechIndex;
private:
	void openMaterialDialog();
	void selectMaterial(size_t _index = MyGUI::ITEM_NONE);
	void selectTechnique(size_t _index);
	void selectPass(size_t _index );
	void initPassTabPage( size_t index );
	void initMaterialTabPage( string material );
	void initTechniqueTabPage( size_t index );
	string getStringLine( const string& s,uint32 b,uint32 e );

	string getMaterialScript( const string& material,string& filename );

	void applyMaterial( const string& name );

	string real2str( Ogre::Real r,int p );
	string color2str( const Ogre::ColourValue& c );

	void RigidToolCheckMenu(RigidToolEnum e);
	void simulation( bool b,bool isSingle );
	virtual void injectMousePress(int _absx, int _absy, MyGUI::MouseButton _id);
	virtual void injectMouseRelease(int _absx, int _absy, MyGUI::MouseButton _id);

	//初始化材质列表
	void initResurceList();
	//将一个texture转换成material
	void tex2mat( const string name );

	void notifyMatListChange(MyGUI::ListBox* _sender, size_t _index);
	void notifyListChangePosition(MyGUI::ListBox* _sender, size_t _index);
	void notifyButtonClick(MyGUI::Widget* _sender);
	void notifyButtonClick2(MyGUI::Widget* _sender);
	void simpleDataChange(SimpleData* psd);
	void NotifyObjectMove( const string& name,const Ogre::Vector3& v );
	void NotifyObjectRotate( const string& name,const Ogre::Real angle );
	void NotifyObjectOrient( const string& name,const Ogre::Vector3& v,const Ogre::Real angle );
	void NotifyWindowButtonPressed(MyGUI::Window* _sender, const string& _button);

	virtual void resourceGroupScriptingStarted(const Ogre::String& groupName, size_t scriptCount);
	virtual void scriptParseStarted(const Ogre::String& scriptName, bool& skipThisScript);
	virtual void scriptParseEnded(const Ogre::String& scriptName, bool skipped);
	virtual void resourceGroupScriptingEnded(const Ogre::String& groupName);
	virtual void resourceGroupLoadStarted(const Ogre::String& groupName, size_t resourceCount);
	virtual void resourceLoadStarted(const Ogre::ResourcePtr& resource);
    virtual void resourceLoadEnded(void);
    virtual void worldGeometryStageStarted(const Ogre::String& description);
    virtual void worldGeometryStageEnded(void);
    virtual void resourceGroupLoadEnded(const Ogre::String& groupName);
};

#endif