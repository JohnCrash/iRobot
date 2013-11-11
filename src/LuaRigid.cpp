#include "stdheader.h"
#include "LuaManager.h"
#include "LuaExport.h"
#include "Geometry.h"
#include "GeometryObject.h"
#include "ObjectFactory.h"
#include "RigidManager.h"
#include "RigidToolManager.h"

//meta元表名称
#define METHOD( fun,T ) \
static int fun( lua_State* L )\
{	boost::shared_ptr<T> self = cast_shared_ptr<T>(L,1,#T);\
	if( !self )return 0;

#define METHOD_END( ret ) \
	return ret; }\

static void bindVisualObject(lua_State*L,const VisualObjectPtr& obj)
{
	lua_bindSharedPtr<VisualObject>(L,"geo.VisualObject",obj);
}

static void bindRigid(lua_State*L,const RigidPtr& obj)
{
	lua_bindSharedPtr<Rigid>(L,"geo.Rigid",obj);
}

/*
	开始模拟
	rigid.simulation(true)
*/
static int simulation(lua_State*L)
{
	if( lua_isboolean(L,2) )
		RigidManager::getSingleton().goSimulationSingle(tobool(1));
	else
		RigidManager::getSingleton().goSimulation(tobool(1));
	return 0;
}

static int toDesity(lua_State* L,int n)
{
	if( lua_isnumber(L,n) )
		return toint(n);
	else
		return Geometry::DEFAULT_DENSITY;
}

static bool toHasGeomID(lua_State* L,int n )
{
	if( lua_isboolean(L,n) )
		return lua_toboolean(L,n)?true:false;
	else
		return false;
}

/*
	创建对象还可以使用ObjectFactory,然后使用load来构架。
*/
static int createSphere(lua_State*L)
{
	VisualObjectPtr obj;
	obj = VisualObjectPtr( new VisualObject( 
			GeometryObjectPtr( new Sphere(tonumber(1),toDesity(L,2),toHasGeomID(L,3) ))));
	bindVisualObject(L,obj);
	return 1;
}
 
static int createBox(lua_State*L)
{
	VisualObjectPtr obj;
	Ogre::Real v[3];
	v[0] = tonumber(1);
	v[1] = tonumber(2);
	v[2] = tonumber(3);

	obj = VisualObjectPtr( new VisualObject( 
			GeometryObjectPtr( new Box(v,toDesity(L,4),toHasGeomID(L,5) ))));

	bindVisualObject(L,obj);
	return 1;
}

static int createCylinder(lua_State*L)
{
	VisualObjectPtr obj;

	obj = VisualObjectPtr( new VisualObject( 
			GeometryObjectPtr( new Cylinder(
			tonumber(1),tonumber(2),toDesity(L,3),toHasGeomID(L,4) ))));

	bindVisualObject(L,obj);
	return 1;
}

static int createPlane(lua_State*L)
{
	VisualObjectPtr obj;
	Ogre::Plane plane(tonumber(1),tonumber(2),tonumber(3),tonumber(4));

	obj = VisualObjectPtr( new VisualObject( 
			GeometryObjectPtr( new Quad(
			plane,tonumber(5),tonumber(6),toint(7),toint(8),toHasGeomID(L,9) ))));

	bindVisualObject(L,obj);
	return 1;
}

static int createRigidSphere(lua_State*L)
{
	RigidPtr obj;
	obj = RigidPtr( new Rigid(
			GeometryObjectPtr( new Sphere(tonumber(1),toDesity(L,2),true ))));
	bindRigid(L,obj);
	return 1;
}

static int createRigidBox(lua_State*L)
{
	RigidPtr obj;
	Ogre::Real v[3];
	v[0] = tonumber(1);
	v[1] = tonumber(2);
	v[2] = tonumber(3);

	obj = RigidPtr( new Rigid( 
			GeometryObjectPtr( new Box(v,toDesity(L,4),true ))));

	bindRigid(L,obj);
	return 1;
}

static int createRigidCylinder(lua_State*L)
{
	RigidPtr obj;

	obj = RigidPtr( new Rigid( 
			GeometryObjectPtr( new Cylinder(
			tonumber(1),tonumber(2),toDesity(L,3),true ))));

	bindRigid(L,obj);
	return 1;
}

static int isSimulation(lua_State*L)
{
	pbool(RigidManager::getSingleton().isSimulation());
	return 1;
}

static int removeNode(lua_State*L)
{
	VisualObjectPtr obj = cast_shared_ptr<VisualObject>(L,1,"VisualObject");

    RigidManager::getSingleton().removeNode(obj);
    
	return 0;
}

static int addNode(lua_State*L)
{
	VisualObjectPtr obj = cast_shared_ptr<VisualObject>(L,1,"VisualObject");

    RigidManager::getSingleton().addNode(obj);
    
	return 0;
}

static int clearAllNode(lua_State*L)
{
	RigidManager::getSingleton().clearAllNode();
	return 0;
}

static int stopControl(lua_State*L)
{
	RigidToolManager::getSingleton().stopControl();
	return 0 ;
}

static int moveControl(lua_State*L)
{
	VisualObjectPtr obj = cast_shared_ptr<VisualObject>(L,1,"VisualObject");
	if( lua_isstring(L,2) )
	{
		string s = tostring(2);
		if( s=="local" || s=="LOCAL" )
			RigidToolManager::getSingleton().moveControl( obj,Ogre::Node::TS_LOCAL );
		else if( s=="world" || s=="WORLD" )
			RigidToolManager::getSingleton().moveControl( obj,Ogre::Node::TS_WORLD );
	}
	else
		RigidToolManager::getSingleton().moveControl( obj,Ogre::Node::TS_WORLD );
	return 0 ;
}

static int move2dControl(lua_State*L)
{
	VisualObjectPtr obj = cast_shared_ptr<VisualObject>(L,1,"VisualObject");
	RigidToolManager::getSingleton().move2dControl(obj);
	return 0 ;
}

static int rotateControl(lua_State*L)
{
	VisualObjectPtr obj = cast_shared_ptr<VisualObject>(L,1,"VisualObject");
	RigidToolManager::getSingleton().rotateControl(obj);
	return 0 ;
}

static int modifyControl(lua_State*L)
{
	VisualObjectPtr obj = cast_shared_ptr<VisualObject>(L,1,"VisualObject");
	RigidToolManager::getSingleton().modifyControl(obj);
	return 0 ;
}

//返回x,y点的对象
static int pickObject(lua_State*L)
{
	VisualObjectPtr o = RigidManager::getSingleton().pickObject(toint(1),toint(2));
	if( o )
		bindVisualObject(L,o);
	else
		lua_pushnil(L);
	return 1;
}

int loadScene(lua_State*L)
{
	if( RigidManager::getSingleton().loadSceneFromFile(tostring(1)) )
		lua_pushboolean(L,1);
	else
		lua_pushboolean(L,0);
	return 1;
}

int saveScene(lua_State*L)
{
	if( RigidManager::getSingleton().saveSceneFromFile(tostring(1)) )
		lua_pushboolean(L,1);
	else
		lua_pushboolean(L,0);
	return 1;
}

static int createJointBall(lua_State*L)
{
    lua_bindSharedPtr<Joint>(L,"geo.JointBall",JointPtr(new JointBall()));
    return 1;
}

static int createJointHinge(lua_State*L)
{
    lua_bindSharedPtr<Joint>(L,"geo.JointHinge",JointPtr(new JointHinge()));
    return 1;
}

static int createJointSlider(lua_State*L)
{
    lua_bindSharedPtr<Joint>(L,"geo.JointSlider",JointPtr(new JointSlider()));
    return 1;
}
static int createFramework(lua_State*L)
{
    lua_bindSharedPtr<Framework>(L,"geo.Framework",FrameworkPtr(new Framework()));
    return 1;
}

static const struct luaL_Reg rigidGlobalMethod[]=
{
	{"createSphere",createSphere},
	{"createBox",createBox},
	{"createCylinder",createCylinder},
	{"createPlane",createPlane},

	{"createRigidSphere",createRigidSphere},
	{"createRigidBox",createRigidBox},
	{"createRigidCylinder",createRigidCylinder},

    {"createFramework",createFramework},
    {"createJointBall",createJointBall},
    {"createJointHinge",createJointHinge},
    {"createJointSlider",createJointSlider},
    
	{"loadScene",loadScene},
	{"saveScene",saveScene},

	{"simulation",simulation},
	{"isSimulation",isSimulation},

	{"addNode",addNode},
	{"removeNode",removeNode},
	{"clearAllNode",clearAllNode},

	{"stopControl",stopControl},
	{"moveControl",moveControl},
	{"move2dControl",move2dControl},
	{"rotateControl",rotateControl},
	{"modifyControl",modifyControl},

	{"pickObject",pickObject},
	{nullptr,nullptr}
};

/*
    Object
 */
static int load(lua_State* L)
{
    ObjectPtr obj = cast_shared_ptr<Object>(L,1,"Object");
    if( obj )
    {
        MyGUI::xml::Element* elp = (MyGUI::xml::Element*)lua_cast( L,2,"Element" );
        if( elp )
        {
            obj->load(elp);
        }
    }
    return 0;
}

static int save(lua_State* L)
{
    ObjectPtr obj = cast_shared_ptr<Object>(L,1,"Object");
    if( obj )
    {
        MyGUI::xml::Element* elp = (MyGUI::xml::Element*)lua_cast( L,2,"Element" );
        if( elp )
        {
            obj->save(elp);
        }
    }
    return 0;
}

static int gcObject(lua_State* L)
{
	boost::shared_ptr<Object>* pobj = (boost::shared_ptr<Object>*)lua_cast( L,1,"Object" );
	delete pobj;
	return 0;
}

static const struct luaL_Reg objectMethod[]=
{
    {"load",load},
    {"save",save},
	{"__gc",gcObject},    
    {nullptr,nullptr},
};

/*
	VisualObject
*/
#define OBJECT_METHOD(fun) METHOD(fun,VisualObject)

OBJECT_METHOD(updatePhysica)
self->updatePhysica();
METHOD_END(0)

OBJECT_METHOD(getPosition)
Ogre::Vector3 v = self->getPosition();
pnumber(v.x);
pnumber(v.y);
pnumber(v.z);
METHOD_END(3)

OBJECT_METHOD(setPosition)
self->setPosition(Ogre::Vector3(tonumber(2),tonumber(3),tonumber(4)));
METHOD_END(0)

OBJECT_METHOD(getLocalAxis)
//Ogre::Matrix3 m3 = self->getLocalAxis();
METHOD_END(0)

OBJECT_METHOD(translate)
self->translate(Ogre::Vector3(tonumber(2),tonumber(3),tonumber(4)));
METHOD_END(0)

OBJECT_METHOD(rotate)
self->rotate(Ogre::Vector3(tonumber(2),tonumber(3),tonumber(4)),tonumber(5));
METHOD_END(0)

OBJECT_METHOD(Intersect)
//
METHOD_END(0)

OBJECT_METHOD(setCastShadows)
self->setCastShadows(tobool(2));
METHOD_END(0)

OBJECT_METHOD(getGeometryObject)
METHOD_END(0)

OBJECT_METHOD(setMaterialName)
self->setMaterialName(tostring(2));
METHOD_END(0)

OBJECT_METHOD(getMaterialName)
pstring(self->getMaterialName().c_str());
METHOD_END(1)

OBJECT_METHOD(showBoundingBox)
self->showBoundingBox(tobool(2));
METHOD_END(0)

static const struct luaL_Reg visualObjectMethod[]=
{
	{"updatePhysica",updatePhysica},
	{"getPosition",getPosition},
	{"getLocalAxis",getLocalAxis},
	{"setPosition",setPosition},
	{"translate",translate},
	{"rotate",rotate},
	{"Intersect",Intersect},
	{"setCastShadows",setCastShadows},
	{"getGeometryObject",getGeometryObject},
	{"setMaterialName",setMaterialName},
	{"getMaterialName",getMaterialName},
	{"showBoundingBox",showBoundingBox},
	{nullptr,nullptr}
};

/*
	Rigid
*/
#define RIGID_METHOD(fun) METHOD(fun,Rigid)

RIGID_METHOD(updateRigid)
self->updateRigid();
METHOD_END(0)

static const struct luaL_Reg rigidMethod[]=
{
	{"updateRigid",updateRigid},
	{nullptr,nullptr}
};


/*
	Joint
*/
static int breakRigid(lua_State* L)
{
    JointPtr obj = cast_shared_ptr<Joint>(L,1,"Joint");
    if(obj)
    {
        RigidPtr rigp = cast_shared_ptr<Rigid>(L, 2, "Rigid");
        if( rigp )
        {
            obj->breakRigid(rigp);
        }
    }
    return 0;
}
static int linkRigid(lua_State* L)
{
    JointPtr obj = cast_shared_ptr<Joint>(L,1,"Joint");
    if(obj)
    {
        RigidPtr r1 = cast_shared_ptr<Rigid>(L,2,"Rigid");
        RigidPtr r2 = cast_shared_ptr<Rigid>(L,3,"Rigid");
        if( r1 && r2 )
        {
            obj->linkRigid(r1, r2);
        }
    }
    return 0;
}
static int enableJoint(lua_State* L)
{
    JointPtr obj = cast_shared_ptr<Joint>(L,1,"Joint");
    if(obj)
    {
        if( ( lua_isboolean(L, 2) && !lua_toboolean(L, 2) )||
            lua_isnil(L,2) )
            obj->Disable();
        else
            obj->Enable();
    }
    return 0;
}
static int isEnabled(lua_State* L)
{
    JointPtr obj = cast_shared_ptr<Joint>(L,1,"Joint");
    if(obj)
    {
        lua_pushboolean(L,obj->isEnabled());
    }
    return 1;
}
static const struct luaL_Reg jointMethod[]=
{
    {"breakRigid",breakRigid},
    {"linkRigid",linkRigid},
    {"enable",enableJoint},
    {"isEnabled",isEnabled},
    {nullptr,nullptr}
};
static int setBallAnchor(lua_State* L)
{
    JointBallPtr jbp = cast_shared_ptr<JointBall>(L,1,"JointBall");
    if(jbp)
    {
        Ogre::Vector3* v3 = (Ogre::Vector3*)lua_cast(L,2,"Vector3");
        if( v3 )
        {
            jbp->setBallAnchor(*v3);
        }
    }
    return 0;
}
static int getBallAnchor(lua_State* L)
{
    JointBallPtr jbp = cast_shared_ptr<JointBall>(L,1,"JointBall");
    if(jbp)
    {
        int idx = 0;
        if( lua_isnumber(L, 2) )
            idx = lua_tointeger(L, 2);
        lua_bindComplete(L, "mat.Vector3", new Ogre::Vector3(jbp->getBallAnchor(idx)));
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}
static const struct luaL_Reg joinBallMethod[]=
{
    {"setBallAnchor",setBallAnchor},
    {"getBallAnchor",getBallAnchor},
    {nullptr,nullptr}
};
static int setHingeAnchor(lua_State* L)
{
    JointHingePtr jbp = cast_shared_ptr<JointHinge>(L,1,"JointHinge");
    if(jbp)
    {
        Ogre::Vector3* v3 = (Ogre::Vector3*)lua_cast(L,2,"Vector3");
        if( v3 )
        {
            jbp->setHingeAnchor(*v3);
        }
    }
    return 0;
}
static int getHingeAnchor(lua_State* L)
{
    JointHingePtr jbp = cast_shared_ptr<JointHinge>(L,1,"JointHinge");
    if(jbp)
    {
        lua_bindComplete(L, "mat.Vector3", new Ogre::Vector3(jbp->getHingeAnchor()));
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}
static int setHingeAxis(lua_State* L)
{
    JointHingePtr jbp = cast_shared_ptr<JointHinge>(L,1,"JointHinge");
    if(jbp)
    {
        Ogre::Vector3* v3 = (Ogre::Vector3*)lua_cast(L,2,"Vector3");
        if( v3 )
        {
            jbp->setHingeAxis(*v3);
        }
    }
    return 0;
}
static int getHingeAxis(lua_State* L)
{
    JointHingePtr jbp = cast_shared_ptr<JointHinge>(L,1,"JointHinge");
    if(jbp)
    {
        lua_bindComplete(L, "mat.Vector3", new Ogre::Vector3(jbp->getHingeAxis()));
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}
static const struct luaL_Reg joinHingeMethod[]=
{
    {"setHingeAnchor",setHingeAnchor},
    {"getHingeAnchor",getHingeAnchor},
    {"setHingeAxis",setHingeAxis},
    {"getHingeAxis",getHingeAxis},
    {nullptr,nullptr}
};
static const struct luaL_Reg joinSliderMethod[]=
{
    {nullptr,nullptr}
};
/*
 Framework
 */
static int addJoint(lua_State* L)
{
    FrameworkPtr fwk = cast_shared_ptr<Framework>(L,1,"Framework");
    if(fwk)
    {
        JointPtr jp = cast_shared_ptr<Joint>(L, 2, "Joint");
        if( jp )
        {
            fwk->addJoint(jp);
        }
    }
    return 0;
}
static int removeJoint(lua_State* L)
{
    FrameworkPtr fwk = cast_shared_ptr<Framework>(L,1,"Framework");
    if(fwk)
    {
        JointPtr jp = cast_shared_ptr<Joint>(L, 2, "Joint");
        if( jp )
        {
            fwk->removeJoint(jp);
        }
    }
    return 0;
}
static int removeAllJoint(lua_State* L)
{
    FrameworkPtr fwk = cast_shared_ptr<Framework>(L,1,"Framework");
    if(fwk)
    {
        fwk->removeAllJoint();
    }
    return 0;
}
static const struct luaL_Reg frameworkMethod[]=
{
    {"addJoint",addJoint},
    {"removeJoint",removeJoint},
    {"removeAllJoint",removeAllJoint},
    {nullptr,nullptr}
};

void luaopen_rigid( lua_State* L )
{
	LuaManager& lm=LuaManager::getSingleton();

    lm.registerClass("geo.Object",objectMethod);
	lm.registerClass("geo.VisualObject",objectMethod,visualObjectMethod);
	lm.registerClass("geo.Rigid",objectMethod,visualObjectMethod,rigidMethod);

    lm.registerClass("geo.Joint",objectMethod,jointMethod);
    lm.registerClass("geo.JointBall",objectMethod,jointMethod,joinBallMethod);
    lm.registerClass("geo.JointHinge",objectMethod,jointMethod,joinHingeMethod);
    lm.registerClass("geo.JointSlider",objectMethod,jointMethod,joinSliderMethod);
    
    lm.registerClass("geo.Framework",objectMethod,frameworkMethod);
	lm.registerGlobal("geo",rigidGlobalMethod);
}
