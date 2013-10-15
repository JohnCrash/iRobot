#include "stdheader.h"
#include "Stage.h"
#include "SimpleUI.h"
#include "Geometry.h"
#include "TestStage.h"
#include "Game.h"
#include "GeometryObject.h"
#include "RigidManager.h"

#ifdef _MSC_VER
#pragma warning( disable : 4275 )
#endif
#include "OgreScriptLexer.h"
#include "OgreScriptParser.h"

void TestStage::closeStage(){
	setEnable( false );
	mCamControlet.reset();

	mUI.close();
	mMatDialog.close();

	Game::getSingleton().getSceneManager()->setSkyDome(false,"");
}

bool TestStage::openStage(){
	if( mUI )return false;

	setEnable( true );

	mUI.load("TestGeometry.layout","Test");
	
//	MyGUI::ResourceManager::getInstance().load("MyGUI_BlackOrangeTheme.xml");

	try{
		if( mUI ){
			MyGUI::IntSize size = mUI["MainMenu"]->getSize();
			size.width = Game::getSingleton().getScreenWidth();
			mUI["MainMenu"]->setSize( size );

			char* cmds[] = {"Exit","Options","Update","MatBut","box",
				"sphere","cylinder","_box","_sphere","_cylinder","_plane",
				"_properties","move","rotate","delete","movelocal",
				"modify","ogreResource","run","pause","_delete","_planemove",
				"_material","_run2thread"
			};
			
			for( int i = 0;i<sizeof(cmds)/sizeof(char*);++i )
				mUI[ cmds[i] ]->eventMouseButtonClick += newDelegate(this, &TestStage::notifyButtonClick);

			mUI["PropWnd"]->castType<MyGUI::Window>(false)->eventWindowButtonPressed += 
				newDelegate(this, &TestStage::NotifyWindowButtonPressed);

			initResurceList();

			MyGUI::ListBox* pl = mUI["Objects"]->castType<MyGUI::ListBox>(false);
			if( pl )
				pl->eventListChangePosition += newDelegate( this,&TestStage::notifyListChangePosition );
		}
	}catch(std::out_of_range& e){
		WARNING_LOG( e.what() );
	}

	//创建一个地面
	ObjectPtr floor = RigidManager::getSingleton().createPlane("floor",
		Ogre::Plane(Ogre::Vector3::UNIT_Y, 0), 5000, 5000, 10, 10);

	floor->setMaterialName("Ogre/Tusks");
	floor->setCastShadows( false );
	addObject(  "floor" );

	//设置一个镜头控制
	mMaterial = "Ogre/Earth";
	mSerial = 0;
	mRigidTool = MOVE;
	RigidToolCheckMenu( MOVE );

	//设置天空盒
	Game::getSingleton().getSceneManager()->setSkyDome(true, "Examples/CloudySky");

	Ogre::Camera* pCam = Game::getSingleton().getCamera();
	mCamControlet.reset(new SimpleCameraControlet(pCam));
	//重新设置Camera位置
	pCam->setPosition(100,100,100);
	pCam->lookAt(0,0,0);
	return true;
}

/*
	两个窗口的关闭操作
	1属性对话(隐藏)
	2材质对话(关闭)
*/
void TestStage::NotifyWindowButtonPressed(MyGUI::Window* _sender, const string& _button){
	if( _button == "close" ){
		if( mUI.IsWidget(_sender,"PropWnd") )
			_sender->setVisible( false );
		else if( mMatDialog.IsWidget(_sender,"MaterialDialog") )
			mMatDialog.close();
	}
}

void TestStage::initResurceList()
{
	try{
		MyGUI::ListBox *plist = mUI["MatList"]->castType<MyGUI::ListBox>(false);

		plist->removeAllItems();

		Ogre::ResourceManager::ResourceMapIterator it = Ogre::MaterialManager::getSingleton().getResourceIterator();
		while( it.hasMoreElements() ){
			plist->addItem(it.getNext()->getName());
		}
		it = Ogre::TextureManager::getSingleton().getResourceIterator();
		//小于mFirstTexture的都是材质
		mFirstTexture = plist->getItemCount();
		plist->addItem("-------Texture--------");
		while( it.hasMoreElements() ){
			plist->addItem(it.getNext()->getName());
		}
	}catch(std::out_of_range& e){
		WARNING_LOG( e.what() );
	}
}

void TestStage::tex2mat( const string name )
{
	Ogre::MaterialManager& mm = Ogre::MaterialManager::getSingleton();
	Ogre::ResourcePtr rp = mm.getByName( name );
	if( rp.isNull() ){
		Ogre::MaterialPtr mp = mm.create( name,
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
		Ogre::Pass* ps = mp->getTechnique(0)->getPass(0);
		Ogre::TextureUnitState* ts;
		if( ps )
			ps->createTextureUnitState();
		ps->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
		ps->setSceneBlendingOperation(Ogre::SBO_ADD);
		ts = ps->getTextureUnitState(0);
		ts->setColourOperation(Ogre::LBO_ALPHA_BLEND);
		ts->setTextureName( name );
	}
}

void TestStage::injectMousePress(int _absx, int _absy, MyGUI::MouseButton _id){
	Game& g=Game::getSingleton();
	//Ogre::SceneManager* psm = g.getSceneManager();

	string name = g.pickMovableObject(_absx,_absy);
	if( RigidManager::getSingleton().hasRigid( name ) )
		setCurrentObject( name );
	else if( RigidManager::getSingleton().hasObject( name ) )
		setCurrentObject( name );
}

void TestStage::injectMouseRelease(int _absx, int _absy, MyGUI::MouseButton _id){
}

void TestStage::NotifyObjectMove( const string& name,const Ogre::Vector3& v ){
	if(  name == "moveUP" ){
	}else if( name == "moveDOWN" ){
	}
}

void TestStage::NotifyObjectRotate( const string& name,const Ogre::Real angle ){
	if( name == "rotate" ){
	}
}

void TestStage::NotifyObjectOrient( const string& name,const Ogre::Vector3& v,const Ogre::Real angle ){
}

void TestStage::setCurrentObject( const string& name ){
	if( !mSelectName.empty() ){
		SimpleDataUI sdu(mUI["GeometryParam"]);
		sdu.clear();
	}

	mSelectName = name;

	try{
		SimpleDataUI sdu(mUI["GeometryParam"],MyGUI::newDelegate(this,&TestStage::simpleDataChange));
		ObjectPtr ptr = RigidManager::getSingleton().getRigid(mSelectName);
		if( !ptr ){
			ptr = RigidManager::getSingleton().getObject(mSelectName);
		}
		switch( mRigidTool )
		{
		case MOVE:
			RigidToolManager::getSingleton().moveControl( ptr,Ogre::Node::TS_WORLD );
			break;
		case ROTATE:
			RigidToolManager::getSingleton().rotateControl( ptr );
			break;
		case MOVE_LOCAL:
			RigidToolManager::getSingleton().moveControl( ptr,Ogre::Node::TS_LOCAL );
			break;
		case MODIFY:
			RigidToolManager::getSingleton().modifyControl( ptr);
			break;
		case MOVE2D:
			RigidToolManager::getSingleton().move2dControl( ptr);
			break;
		}
		MyGUI::ListBox* pl = mUI["Objects"]->castType<MyGUI::ListBox>(false);
		size_t index = pl->findItemIndexWith( name );
		if( index != MyGUI::ITEM_NONE )
			pl->setIndexSelected( index );
		if( ptr ){
			//设置几何属性
			GeometryObjectPtr obj = ptr->getGeometryObject();
			switch( obj->getGeometryType() ){
				case Geometry::SPHERE:
					{
						Sphere* psphere = dynamic_cast<Sphere*>(obj.get());
						sdu.add( MyGUI::UString("density:"),SimpleData("des",(double)psphere->getGeometryMeshDensity()) );
						sdu.add( "radius:",SimpleData("r",(double)psphere->getRadius()) );
					}
					break;
				case Geometry::BOX:
					{
						Box* pbox = dynamic_cast<Box*>(obj.get());
						const Ogre::Real* v = pbox->getVolume();
						sdu.add( "width:",SimpleData("w",(double)v[0]) );
						sdu.add( "height:",SimpleData("h",(double)v[1]) );
						sdu.add( "length:",SimpleData("d",(double)v[2]) );
					}
					break;
				case Geometry::CYLINDER:
					{
						Cylinder* pcy = dynamic_cast<Cylinder*>(obj.get());
						sdu.add( MyGUI::UString("density:"),SimpleData("des",(double)pcy->getGeometryMeshDensity()) );
						sdu.add( "radius:",SimpleData("r",(double)pcy->getLength() ));
						sdu.add( "length:",SimpleData("l",(double)pcy->getLength() ));
					}
					break;
				case Geometry::PLANE:
					{
						Quad* pp = dynamic_cast<Quad*>(obj.get());
						Ogre::Plane plane;
						Ogre::Real width,height;
						int xseg,yseg;
						pp->getQuad( plane,width,height,xseg,yseg );
						sdu.add( "width:",SimpleData("w",(double)width) );
						sdu.add( "height:",SimpleData("h",(double)height) );
						sdu.add( "xsegments:",SimpleData("x",(double)xseg) );
						sdu.add( "ysegments:",SimpleData("y",(double)yseg) );
					}
					break;
				default:;
			}
			sdu.reLayout( 2,10 );
		}else{
			MSG( "setCurrentObject invalid name:"<<name );
		}

		selectMaterial();
	}catch( out_of_range& e ){
		WARNING_LOG(e.what());
	}
}

/* 调整物体的几何属性
*/
void TestStage::simpleDataChange(SimpleData* psd){
	ObjectPtr ptr = RigidManager::getSingleton().getRigid(mSelectName);
	if( !ptr )
		ptr = RigidManager::getSingleton().getObject(mSelectName);
	if( ptr ){
		GeometryObjectPtr obj = ptr->getGeometryObject();
		switch( obj->getGeometryType() ){
			case Geometry::SPHERE:
				{
					Sphere* psphere = dynamic_cast<Sphere*>(obj.get());
					if( psd->name == "des" )
						psphere->setGeometryMeshDensity( (int)psd->real );
					else if( psd->name == "r" )
						psphere->setRadius( (Ogre::Real)psd->real );
				}
				break;
			case Geometry::BOX:
				{
					Box* pbox = dynamic_cast<Box*>(obj.get());
					const Ogre::Real* v = pbox->getVolume();
					Ogre::Real vol[3];
					vol[0] = v[0];vol[1] = v[1];vol[2] = v[2];
					if( psd->name == "w" )
						vol[0] = (Ogre::Real)psd->real;
					else if( psd->name == "h" )
						vol[1] = (Ogre::Real)psd->real;
					else if( psd->name == "d" )
						vol[2] = (Ogre::Real)psd->real;
					pbox->setVolume( vol );
				}
				break;
			case Geometry::CYLINDER:
				{
					Cylinder* pcy = dynamic_cast<Cylinder*>(obj.get());
					Ogre::Real radius,length;
					radius = pcy->getRadius();
					length = pcy->getLength();
					if( psd->name == "des" )
						pcy->setGeometryMeshDensity( (int)psd->real );
					else if( psd->name == "r" )
						radius = (Ogre::Real)psd->real;
					else if( psd->name == "l" )
						length = (Ogre::Real)psd->real;
					pcy->setCylinder( radius,length );
				}
				break;
			case Geometry::PLANE:
				{
					Quad* pp = dynamic_cast<Quad*>(obj.get());
					Ogre::Plane plane;
					Ogre::Real width,height;
					int xseg,yseg;
					pp->getQuad( plane,width,height,xseg,yseg );
					if( psd->name == "w" )
						width = (Ogre::Real)psd->real;
					else if( psd->name == "h" )
						height = (Ogre::Real)psd->real;
					else if( psd->name == "x" )
						xseg = (int)psd->real;
					else if( psd->name == "y" )
						yseg = (int)psd->real;
					pp->setQuad( plane,width,height,xseg,yseg );
				}
				break;
			default:;
		}
	}
}

void TestStage::notifyListChangePosition(MyGUI::ListBox* _sender, size_t _index){
	if( mUI.IsWidget(_sender,"Objects") && _index!=MyGUI::ITEM_NONE ){
		setCurrentObject( _sender->getItemNameAt(_index) );
	}
}

void TestStage::addObject( const string& name ){
	if( mUI ){
		try{
			MyGUI::ListBox* pc = mUI["Objects"]->castType<MyGUI::ListBox>(false);
			if( pc ){
				pc->addItem( name );
				pc->setIndexSelected( pc->getItemCount()-1 );
			}
		}catch( out_of_range& e ){
			WARNING_LOG(e.what());
		}
	}
}

void TestStage::injectKeyPress(MyGUI::KeyCode _key, MyGUI::Char _text){
	if( _key == MyGUI::KeyCode::F10 )
		Game::getSingleton().openStage("MainMenu");
}

//设置工具菜单的状态
void TestStage::RigidToolCheckMenu( RigidToolEnum e )
{
	string name;

	for( int i = 0;i < 2;i++ ){
		switch( mRigidTool )
		{
		case MOVE:
			name = "move";	
			break;
		case ROTATE:
			name = "rotate";
			break;
		case MOVE_LOCAL:
			name = "movelocal";
			break;
		case MODIFY:
			name = "modify";
			break;
		case MOVE2D:
			name = "_planemove";
			break;
		}
		try{
			MyGUI::MenuItem * pitem = 
				mUI[name]->castType<MyGUI::MenuItem>(false);
			if( pitem )
				pitem->setItemChecked( i==0?false:true );
		}catch( std::out_of_range& exp ){
			WARNING_LOG(exp.what());
			return;
		}
		mRigidTool = e;
	}
}

void TestStage::simulation( bool b,bool isSingle )
{
	try{
		string name[3] = {"run","pause","_run2thread"};

		for( int i = 0;i<3;++i ){
			MyGUI::MenuItem * pitem = 
				mUI[name[i]]->castType<MyGUI::MenuItem>(false);
			if( pitem )
			{
				if( b && isSingle && name[i] == "run" )
					pitem->setItemChecked(true);
				else if( b && !isSingle && name[i] == "_run2thread" )
					pitem->setItemChecked(true);
				else if( !b && name[i] == "pause" )
					pitem->setItemChecked(true);
				else
					pitem->setItemChecked(false);
			}
		}
	}catch( std::out_of_range& e ){
		WARNING_LOG(e.what());
	}

	if( isSingle )
		RigidManager::getSingleton().goSimulationSingle(b);
	else
		RigidManager::getSingleton().goSimulation(b);
}

void TestStage::applyMaterial( const string& name )
{
	ObjectPtr ptr = RigidManager::getSingleton().getRigid(mSelectName);
	if( !ptr ){
		ptr = RigidManager::getSingleton().getObject(mSelectName);
	}
	if( ptr){
		ptr->setMaterialName( name );
	}
}

void TestStage::notifyButtonClick(MyGUI::Widget* _sender){
	if( mUI.IsWidget(_sender,"Exit") ){
		Game::getSingleton().openStage("MainMenu");
	}else if( mUI.IsWidget(_sender,"Options") ){
		Game::getSingleton().showOptionsDialog();
	}else if( mUI.IsWidget(_sender,"Update") ){//更新几何体
		ObjectPtr ptr = RigidManager::getSingleton().getRigid(mSelectName);
		if( !ptr )ptr = RigidManager::getSingleton().getObject(mSelectName);
		if( ptr ){
			ptr->getGeometryObject()->updateGeometry();
			Rigid* prigid = dynamic_cast<Rigid*>(ptr.get());
			if( prigid )
				prigid->updatePhysica();
		}
	}else if( mUI.IsWidget(_sender,"MatBut") ){ //使用材质
		MyGUI::ListBox *plist = mUI["MatList"]->castType<MyGUI::ListBox>(false);
		if( plist ){
			size_t sel = plist->getIndexSelected();
			if( sel!=-1 ){
				mMaterial = plist->getItemNameAt(sel);
				if( sel > mFirstTexture ) //如果不是material将进行一次转换操作
					tex2mat( mMaterial );
				applyMaterial( mMaterial );
			}
		}
	}else if( mUI.IsWidget(_sender,"box") ){
		string name = "box";
		name += boost::lexical_cast<string>(++mSerial);

		Ogre::Real v[3] = {10,10,10};
		RigidPtr rp( RigidManager::getSingleton().createRigidBox( name,v ) );
		rp->setPosition(Ogre::Vector3(0,500,0));
		rp->setMaterialName( mMaterial );
		setCurrentObject( name );
		addObject( name );
	}else if( mUI.IsWidget(_sender,"sphere") ){
		string name = "sphere";
		name += boost::lexical_cast<string>(++mSerial);

		RigidPtr rp( RigidManager::getSingleton().createRigidSphere( name,10,12 ) );
		rp->setPosition(Ogre::Vector3(0,500,0));
		rp->setMaterialName( mMaterial );
		setCurrentObject( name );
		addObject( name );
	}else if( mUI.IsWidget(_sender,"cylinder") ){
		string name = "cylinder";
		name += boost::lexical_cast<string>(++mSerial);

		RigidPtr rp( RigidManager::getSingleton().createRigidCylinder( name,100,100,12 ) );
		rp->setPosition(Ogre::Vector3(0,500,0));
		rp->setMaterialName( mMaterial );
		setCurrentObject( name );
		addObject( name );
	}else if( mUI.IsWidget(_sender,"_box") ){
		string name = "_box";
		name += boost::lexical_cast<string>(++mSerial);

		Ogre::Real v[3] = {100,100,100};
		ObjectPtr rp( RigidManager::getSingleton().createBox( name,v,true ) );
		rp->setPosition(Ogre::Vector3(0,500,0));
		rp->setMaterialName( mMaterial );
		setCurrentObject( name );
		addObject( name );
	}else if( mUI.IsWidget(_sender,"_cylinder") ){
		string name = "_cylinder";
		name += boost::lexical_cast<string>(++mSerial);

		ObjectPtr rp( RigidManager::getSingleton().createCylinder( name,100,100,true ) );
		rp->setPosition(Ogre::Vector3(0,500,0));
		rp->setMaterialName( mMaterial );
		setCurrentObject( name );
		addObject( name );
	}else if( mUI.IsWidget(_sender,"_sphere") ){
		string name = "_sphere";
		name += boost::lexical_cast<string>(++mSerial);

		ObjectPtr rp( RigidManager::getSingleton().createSphere( name,100,true ) );
		rp->setPosition(Ogre::Vector3(0,500,0));
		rp->setMaterialName( mMaterial );
		setCurrentObject( name );
		addObject( name );
	}else if( mUI.IsWidget(_sender,"_plane") ){
		string name = "_plane";
		name += boost::lexical_cast<string>(++mSerial);

		ObjectPtr rp( RigidManager::getSingleton().createPlane( name,
			Ogre::Plane(Ogre::Vector3::UNIT_Y, 0), 500, 500, 10, 10));

		rp->setPosition(Ogre::Vector3(0,0,0));
		rp->setMaterialName( mMaterial );
		setCurrentObject( name );
		addObject( name );
	}else if( mUI.IsWidget(_sender,"_properties") ){
		try{
			initResurceList();
			mUI["PropWnd"]->setVisible(true);
		}catch( std::out_of_range& e){
			WARNING_LOG( e.what() );
		}
	}else if( mUI.IsWidget(_sender,"move") ){
		RigidToolCheckMenu( MOVE );
		setCurrentObject( mSelectName );
	}else if( mUI.IsWidget(_sender,"_planemove") ){
		RigidToolCheckMenu(MOVE2D);
		setCurrentObject( mSelectName );
	}else if( mUI.IsWidget(_sender,"rotate") ){
		RigidToolCheckMenu(ROTATE);
		setCurrentObject( mSelectName );
	}else if( mUI.IsWidget(_sender,"movelocal") ){
		RigidToolCheckMenu(MOVE_LOCAL);
		setCurrentObject( mSelectName );
	}else if( mUI.IsWidget(_sender,"modify") ){
		RigidToolCheckMenu(MODIFY);
		setCurrentObject( mSelectName );
	}else if( mUI.IsWidget(_sender,"delete") || mUI.IsWidget(_sender,"_delete") ){
		if( RigidManager::getSingleton().hasRigid( mSelectName ) )
			RigidManager::getSingleton().destroyRigid( mSelectName );
		else if( RigidManager::getSingleton().hasObject( mSelectName ) )
			RigidManager::getSingleton().destroyObject( mSelectName );
		try{
			MyGUI::ListBox* pl = mUI["Objects"]->castType<MyGUI::ListBox>(false);
			size_t index = pl->findItemIndexWith( mSelectName );
			if( index != MyGUI::ITEM_NONE ){
				pl->removeItemAt( index );
				if( index==0 && pl->getItemCount()>0 ){
					pl->setIndexSelected( index );
				}else if( index >= pl->getItemCount() && pl->getItemCount()>0 ){
					pl->setIndexSelected( --index );
				}
				if( pl->getItemCount()>0 )
					setCurrentObject( pl->getItemNameAt( index ) );
				else
					setCurrentObject( "" );
			}
		}catch(std::out_of_range& e ){
			WARNING_LOG(e.what());
		}
	}else if( mUI.IsWidget(_sender,"ogreResource") ){
		Ogre::ConfigFile cf;

		cf.load( "H:\\Source\\ogre_src_v1-7-3\\VC2008\\bin\\debug\\resources_d.cfg" );
		Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
		string sec, type, arch;
		while( seci.hasMoreElements() ){
			sec = seci.peekNextKey();
			Ogre::ConfigFile::SettingsMultiMap* settings = seci.getNext();
			Ogre::ConfigFile::SettingsMultiMap::iterator i;
			for (i = settings->begin(); i != settings->end(); i++)
			{
				type = i->first;
				arch = i->second;
				Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch, type, sec);
			}
		}
		MyGUI::Widget* pp =nullptr;
		try{
			pp = mUI["progressWindow"];
		}catch( std::out_of_range& e){
			WARNING_LOG( e.what() );
		}
		if( pp )pp->setVisible(true);
		Ogre::ResourceGroupManager::getSingleton().addResourceGroupListener( this );
		Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
		Ogre::ResourceGroupManager::getSingleton().removeResourceGroupListener(this);
		if( pp )pp->setVisible(false);

		initResurceList();
	}else if(mUI.IsWidget(_sender,"run")){
		simulation( true,true );
	}else if(mUI.IsWidget(_sender,"pause")){
		simulation( false,true );
	}else if(mUI.IsWidget(_sender,"_run2thread")){
		simulation( true,false );
	}else if(mUI.IsWidget(_sender,"_material")){
		//打开材质对话栏
		openMaterialDialog();
	}
}

void TestStage::openMaterialDialog()
{
	if( mMatDialog )
		return;
	mMatDialog.load("Material.layout","Mat");
	//close
	mMatDialog->eventWindowButtonPressed += 
		newDelegate(this, &TestStage::NotifyWindowButtonPressed);
	try{
		char* cmds[] = {"_apply"
		};
		
		for( int i = 0;i<sizeof(cmds)/sizeof(char*);++i )
			mMatDialog[ cmds[i] ]->eventMouseButtonClick += newDelegate(this, &TestStage::notifyButtonClick2 );

		//初始化材质列表
		MyGUI::ListBox* plist = mMatDialog["_matlist"]->castType<MyGUI::ListBox>(false);
		Ogre::ResourceManager::ResourceMapIterator it = 
			Ogre::MaterialManager::getSingleton().getResourceIterator();
		while( it.hasMoreElements() ){
			plist->addItem(it.getNext()->getName());
		}
		//所有列表使用一个函数notifyMatListChange
		plist->eventListChangePosition += newDelegate( this,&TestStage::notifyMatListChange );
		plist = mMatDialog["_techlist"]->castType<MyGUI::ListBox>(false);
		plist->eventListChangePosition += newDelegate( this,&TestStage::notifyMatListChange );
		plist = mMatDialog["_passlist"]->castType<MyGUI::ListBox>(false);
		plist->eventListChangePosition += newDelegate( this,&TestStage::notifyMatListChange );
		selectMaterial();
	}catch( std::out_of_range& e ){
		WARNING_LOG(e.what());
	}
}

void TestStage::notifyButtonClick2(MyGUI::Widget* _sender)
{
	if( mMatDialog.IsWidget(_sender,"_apply") )
	{
		applyMaterial(mMatName);
	}
}

void TestStage::selectMaterial( size_t _index )
{
	if( mMatDialog ){
		try{
			MyGUI::ListBox* plist = mMatDialog["_matlist"]->castType<MyGUI::ListBox>(false);

			//对当前选择的物体的材质进行编辑
			size_t index;
			if( _index == MyGUI::ITEM_NONE ){
				ObjectPtr obj = RigidManager::getSingleton().getRigid(mSelectName);
				if( !obj )
					obj = RigidManager::getSingleton().getObject(mSelectName);
				string material;
				if( obj )
					material = obj->getMaterialName();
				else
					material = mMaterial;
				index = plist->findItemIndexWith( material );
			}else{
				index = _index;
			}
			if( index != MyGUI::ITEM_NONE ){

				plist->setIndexSelected( index );

				initMaterialTabPage( plist->getItemNameAt(index) );
			}
		}catch( std::out_of_range& e ){
			WARNING_LOG(e.what());
		}
	}
}

//取得字串从第b行到e行中间的字串，包括e
string TestStage::getStringLine( const string& s,uint32 b,uint32 e )
{
	char buf[256];
	std::istringstream is(s);
	string str;
	uint32 count = 1;
	while( is.getline( buf,255 ) )
	{
		if( count >= b && count <= e )
		{
			str += buf;
			str += "\n";
		}
		if( count > e )
			break;
		count++;
	}
	return str;
}

//取得名称为material的材质脚本字串
string TestStage::getMaterialScript( const string& material,string& filename )
{
	Ogre::MaterialPtr mp = Ogre::MaterialManager::getSingleton().getByName( material );
	if( !mp.isNull() )
	{
		string group;
		Ogre::ResourceGroupManager& rgm = 
			Ogre::ResourceGroupManager::getSingleton();
		Ogre::StringVector sv = rgm.getResourceGroups();
		for( Ogre::StringVector::iterator i = sv.begin();i!=sv.end();++i )
		{
		if( rgm.resourceExists((*i),
				mp->getOrigin()))
			{
				Ogre::DataStreamPtr dsp = rgm.openResource( mp->getOrigin(),(*i) );
				if( !dsp.isNull() )
				{
					Ogre::ScriptLexer lexer;
					Ogre::ScriptParser parser;
					string str = dsp->getAsString();
					filename = mp->getOrigin();
					Ogre::ScriptTokenListPtr stl = lexer.tokenize( str,mp->getOrigin() );
					Ogre::ConcreteNodeListPtr nodes = parser.parse( stl );
					
					for( Ogre::ConcreteNodeList::iterator i = nodes->begin();i!=nodes->end();++i )
					{
						if( (*i)->token == "material" && (*i)->children.size()==3 &&
							(*i)->children.front()->token == material )
						{
							uint32 begin,end;
							begin = (*i)->children.front()->line;
							end = (*i)->children.back()->line;
							return getStringLine( str,begin,end );
						}
					}
				}
			}
		}
	}
	return "";
}

void TestStage::initMaterialTabPage( string material )
{
	try{
		Ogre::MaterialManager& mm = Ogre::MaterialManager::getSingleton();
		Ogre::MaterialPtr mp = mm.getByName( material );
		mMatName = material;
		if( !mp.isNull() ){
			MyGUI::ListBox* ptech = mMatDialog["_techlist"]->castType<MyGUI::ListBox>(false);
			
			//清空
			ptech->removeAllItems();
			size_t count = mp->getNumTechniques();
			for( size_t i = 0;i<count;++i ){
				ptech->addItem(boost::lexical_cast<string>(i)+mp->getTechnique(i)->getName());
			}
			//初始化页面
			mMatDialog.checkItem( "mat_transparent",mp->isTransparent() );
			mMatDialog.checkItem( "mat_receive",mp->getReceiveShadows() );
			mMatDialog.checkItem( "mat_transparency",mp->getTransparencyCastsShadows() );
			//取出材质脚本
			MyGUI::EditBox* pedit = mMatDialog["_matrialEdit"]->castType<MyGUI::EditBox>(false);
			if( pedit )
			{
				string str,filename;

				//_scrptText
				str = getMaterialScript( material,filename );
				mMatDialog.setCaption<MyGUI::TextBox>("_scrptText",(boost::format("Script:%s")%filename).str() );
				pedit->setCaption( str );
			}
			//选择Technique
			ptech->setIndexSelected( 0 );
			selectTechnique(0);
		}
	}catch( std::out_of_range& e ){
		WARNING_LOG(e.what());
	}
}

void TestStage::selectTechnique(size_t _index)
{
	try{
		Ogre::MaterialManager& mm = Ogre::MaterialManager::getSingleton();
		Ogre::MaterialPtr mp = mm.getByName( mMatName );
		mTechIndex = _index;
		if( !mp.isNull() ){
			MyGUI::ListBox* pass = mMatDialog["_passlist"]->castType<MyGUI::ListBox>(false);
			
			//清空
			pass->removeAllItems();

			Ogre::Technique* tp = mp->getTechnique( _index );
			if( !tp )return;

			for( size_t i = 0;i<tp->getNumPasses();++i ){
				pass->addItem(boost::lexical_cast<string>(i)+tp->getPass(i)->getName());
			}
			//初始化页面
			mMatDialog.checkItem( "tech_transparent",tp->isTransparent() );
			mMatDialog.checkItem( "tech_depthwrite",tp->isDepthWriteEnabled() );
			mMatDialog.checkItem( "tech_depth",tp->isDepthCheckEnabled() );
			mMatDialog.checkItem( "tech_color",tp->hasColourWriteDisabled() );
			//选择Technique
			pass->setIndexSelected(0);
			selectPass(0);
		}
	}catch( std::out_of_range& e ){
		WARNING_LOG(e.what());
	}
}

void TestStage::initTechniqueTabPage( size_t index )
{
}

void TestStage::selectPass(size_t _index )
{
	try{
		Ogre::MaterialManager& mm = Ogre::MaterialManager::getSingleton();
		Ogre::MaterialPtr mp = mm.getByName( mMatName );
		if( !mp.isNull() ){

			Ogre::Technique* tp = mp->getTechnique( mTechIndex );
			if( !tp )return;

			Ogre::Pass* ps = tp->getPass( _index );
			if( !ps )return;

			//初始化页面
			mMatDialog.checkItem( "pass_isprogrammable",ps->isProgrammable() );
			mMatDialog.checkItem( "pass_hasGeometryProgram",ps->hasGeometryProgram() );
			mMatDialog.checkItem( "pass_hasVertexProgram",ps->hasVertexProgram() );
			mMatDialog.checkItem( "pass_hasFragmentProgram",ps->hasFragmentProgram() );

			mMatDialog.checkItem( "pass_hasShadowReceiverVertexProgram",ps->hasShadowReceiverVertexProgram() );
			mMatDialog.checkItem( "pass_hasShadowCasterVertexProgram",ps->hasShadowCasterVertexProgram() );
			mMatDialog.checkItem( "pass_hasShadowReceiverFragmentProgram",ps->hasShadowReceiverFragmentProgram() );

			mMatDialog.checkItem( "pass_hasSeparateSceneBlending",ps->hasSeparateSceneBlending() );

			mMatDialog.setCaption<MyGUI::EditBox>( "pass_ambient",color2str(ps->getAmbient()) );
			mMatDialog.setCaption<MyGUI::EditBox>( "pass_diffuse",color2str(ps->getDiffuse()) );
			mMatDialog.setCaption<MyGUI::EditBox>( "pass_specular",color2str(ps->getSpecular()) );

			mMatDialog.setCaption<MyGUI::EditBox>( "pass_shininess",real2str(ps->getShininess(),2) );
			mMatDialog.setCaption<MyGUI::EditBox>( "pass_Self_Illumination",color2str(ps->getSelfIllumination()) );

			{ //Scene Blending
				string name[2] = {"_sceneblending","_sceneblendingAlpha"};
				string sceneBlending[5] = {"SBO_ADD","SBO_SUBTRACT",
					"SBO_REVERSE_SUBTRACT","SBO_MIN","SBO_MAX"};
				for( int i =0;i<2;++i )
				{
					mMatDialog.removeAllItem( name[i] );
					mMatDialog.addItem( name[i],sceneBlending,5 );
					Ogre::SceneBlendOperation sbo;
					if( i == 0 )
						sbo = ps->getSceneBlendingOperation();
					else
						sbo = ps->getSceneBlendingOperationAlpha();
					int index;
					switch( sbo )
					{
					case Ogre::SBO_ADD:
						index = 0;
						break;
					case Ogre::SBO_SUBTRACT:
						index = 1;
						break;
					case Ogre::SBO_REVERSE_SUBTRACT:
						index = 2;
						break;
					case Ogre::SBO_MIN:
						index = 3;
						break;
					case Ogre::SBO_MAX:
						index = 4;
						break;
					}
					mMatDialog.selectItem( name[i],index );
				}
			}

			{ //SceneBlendFactor
				string name[2] = {"_sceneBlendFactor","_desBlendFactor"};
				string cullingMode[10] = {"SBF_ONE","SBF_ZERO",
					"SBF_DEST_COLOUR","SBF_SOURCE_COLOUR",
					"SBF_ONE_MINUS_DEST_COLOUR","SBF_ONE_MINUS_SOURCE_COLOUR",
					"SBF_DEST_ALPHA","SBF_SOURCE_ALPHA",
					"SBF_ONE_MINUS_DEST_ALPHA","SBF_ONE_MINUS_SOURCE_ALPHA"};
				for( int i = 0;i < 2;++i )
				{
					mMatDialog.removeAllItem( name[i] );
					mMatDialog.addItem( name[i],cullingMode,10 );
					Ogre::SceneBlendFactor cm;
					if( i == 0 )
						cm = ps->getSourceBlendFactor();
					else
						cm = ps->getDestBlendFactor();
					int index;
					switch( cm )
					{
					case Ogre::SBF_ONE:
						index =0 ;
						break;
					case Ogre::SBF_ZERO:
						index =1 ;
						break;
					case Ogre::SBF_DEST_COLOUR:
						index =2 ;
						break;
					case Ogre::SBF_SOURCE_COLOUR:
						index =3 ;
						break;
					case Ogre::SBF_ONE_MINUS_DEST_COLOUR:
						index =4 ;
						break;
					case Ogre::SBF_ONE_MINUS_SOURCE_COLOUR:
						index =5 ;
						break;
					case Ogre::SBF_DEST_ALPHA:
						index =6 ;
						break;
					case Ogre::SBF_SOURCE_ALPHA:
						index =7 ;
						break;
					case Ogre::SBF_ONE_MINUS_DEST_ALPHA:
						index =8 ;
						break;
					case Ogre::SBF_ONE_MINUS_SOURCE_ALPHA:
						index =9 ;
						break;
					}
					mMatDialog.selectItem( name[i],index );
				}
			}

			{ //Culling Mode
				string name = "_cullingMode";
				string cullingMode[3] = {"CULL_NONE","CULL_CLOCKWISE",
					"CULL_ANTICLOCKWISE"};
				mMatDialog.removeAllItem( name );
				mMatDialog.addItem( name,cullingMode,3 );
				Ogre::CullingMode cm = ps->getCullingMode();
				int index;
				switch( cm )
				{
				case Ogre::CULL_NONE:
					index =0 ;
					break;
				case Ogre::CULL_CLOCKWISE:
					index =1 ;
					break;
				case Ogre::CULL_ANTICLOCKWISE:
					index =2 ;
					break;
				}
				mMatDialog.selectItem( name,index );
			}

			{ //Manual Culling Mode
				string name = "_manualCullingMode";
				string cullingMode[3] = {"MANUAL_CULL_NONE","MANUAL_CULL_BACK",
					"MANUAL_CULL_FRONT"};
				mMatDialog.removeAllItem( name );
				mMatDialog.addItem( name,cullingMode,3 );
				Ogre::ManualCullingMode cm = ps->getManualCullingMode();
				int index;
				switch( cm )
				{
				case Ogre::MANUAL_CULL_NONE:
					index =0 ;
					break;
				case Ogre::MANUAL_CULL_BACK:
					index =1 ;
					break;
				case Ogre::MANUAL_CULL_FRONT:
					index =2 ;
					break;
				}
				mMatDialog.selectItem( name,index );
			}

			{ //Compare Function
				string name = "_compareFunction";
				string compare[8] = {"CMPF_ALWAYS_FAIL","CMPF_ALWAYS_PASS",
					"CMPF_LESS","CMPF_LESS_EQUAL","CMPF_EQUAL","CMPF_NOT_EQUAL",
					"CMPF_GREATER_EQUAL","CMPF_GREATER"};
				mMatDialog.removeAllItem( name );
				mMatDialog.addItem( name,compare,8 );
				Ogre::CompareFunction cm = ps->getDepthFunction();
				int index;
				switch( cm )
				{
				case Ogre::CMPF_ALWAYS_FAIL:
					index =0 ;
					break;
				case Ogre::CMPF_ALWAYS_PASS:
					index =1 ;
					break;
				case Ogre::CMPF_LESS:
					index =2 ;
					break;
				case Ogre::CMPF_LESS_EQUAL:
					index =3;
					break;
				case Ogre::CMPF_EQUAL:
					index =4 ;
					break;
				case Ogre::CMPF_NOT_EQUAL:
					index =5 ;
					break;
				case Ogre::CMPF_GREATER_EQUAL:
					index =6 ;
					break;
				case Ogre::CMPF_GREATER:
					index =7 ;
					break;
				}
				mMatDialog.selectItem( name,index );
			}

			{ //ShadeOptions
				string name = "_shadeOptions";
				string compare[3] = {"SO_FLAT","SO_GOURAUD",
					"SO_PHONG" };
				mMatDialog.removeAllItem( name );
				mMatDialog.addItem( name,compare,3 );
				Ogre::ShadeOptions cm = ps->getShadingMode();
				int index;
				switch( cm )
				{
				case Ogre::SO_FLAT:
					index =0 ;
					break;
				case Ogre::SO_GOURAUD:
					index =1 ;
					break;
				case Ogre::SO_PHONG:
					index =2 ;
					break;
				}
				mMatDialog.selectItem( name,index );
			}

			{ //PolygonMode
				string name = "_polygonMode";
				string compare[3] = {"PM_POINTS","PM_WIREFRAME",
					"PM_SOLID" };
				mMatDialog.removeAllItem( name );
				mMatDialog.addItem( name,compare,3 );
				Ogre::PolygonMode cm = ps->getPolygonMode();
				int index;
				switch( cm )
				{
				case Ogre::PM_POINTS:
					index =0 ;
					break;
				case Ogre::PM_WIREFRAME:
					index =1 ;
					break;
				case Ogre::PM_SOLID:
					index =2 ;
					break;
				}
				mMatDialog.selectItem( name,index );
			}

			{ //FogMode
			}
		}
	}catch( std::out_of_range& e ){
		WARNING_LOG(e.what());
	}
}

string TestStage::color2str( const Ogre::ColourValue& c )
{
	return ( boost::format( "%02X%02X%02X")%(int)(c.r*255)%(int)(c.g*255)%(int)(c.b*255) ).str();
}

string TestStage::real2str( Ogre::Real r,int p )
{
	return (boost::format( "%0.2f")%r).str();
}

void TestStage::initPassTabPage( size_t index )
{
}
/*
*/
void TestStage::notifyMatListChange(MyGUI::ListBox* _sender, size_t _index)
{
	if( mMatDialog.IsWidget( _sender,"_matlist" ) )
		selectMaterial( _index );
	else if( mMatDialog.IsWidget( _sender,"_techlist" ) )
		selectTechnique( _index );
	else if( mMatDialog.IsWidget( _sender,"_passlist" ) )
		selectPass( _index );
}

void TestStage::resourceGroupScriptingStarted(const Ogre::String& groupName, size_t resourceCount)
{
	try{
		MyGUI::ProgressBar* pp = mUI["progress"]->castType<MyGUI::ProgressBar>(false);
		if( pp ){
			pp->setProgressRange(resourceCount);
			mProgress = 0;
		}
	}catch( out_of_range& e ){
		WARNING_LOG( e.what() );
	}
}

void TestStage::scriptParseStarted(const Ogre::String& scriptName, bool& skipThisScript)
{
	try{
		MyGUI::ProgressBar* pp = mUI["progress"]->castType<MyGUI::ProgressBar>(false);
		if( pp ){
			pp->setProgressPosition(++mProgress);
			Ogre::Root::getSingleton().renderOneFrame();
		}
	}catch( std::out_of_range& e ){
		WARNING_LOG( e.what() );
	}
}

void TestStage::scriptParseEnded(const Ogre::String& scriptName, bool skipped)
{
}

void TestStage::resourceGroupScriptingEnded(const Ogre::String& groupName)
{
}

void TestStage::resourceGroupLoadStarted(const Ogre::String& groupName, size_t resourceCount)
{
}

void TestStage::resourceLoadStarted(const Ogre::ResourcePtr& resource)
{
}

void TestStage::resourceLoadEnded(void)
{
}

void TestStage::worldGeometryStageStarted(const Ogre::String& description)
{
}

void TestStage::worldGeometryStageEnded(void)
{
}

void TestStage::resourceGroupLoadEnded(const Ogre::String& groupName)
{
}