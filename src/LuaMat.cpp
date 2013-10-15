#include "stdheader.h"
#include "LuaManager.h"
#include "LuaExport.h"

/*
	Lua可以用3种方法创建一个Vec3向量
	v = mat.newVec3()
	v = mat.newVec3(x,y,z)
	v = mat.newVec3(v3)
*/
static int newVec3(lua_State* L)
{
	const char* meta = "mat.Vector3";
	if( lua_isnumber(L,1) )
	{
		lua_bindComplete(L,meta,new Ogre::Vector3(
			tonumber(1),tonumber(2),tonumber(3)));
	}
	else if( lua_isuserdata(L,1) )
	{
		Ogre::Vector3* v3 = (Ogre::Vector3*)lua_cast(L,1,"Vector3");
		if( v3 )
		{
			lua_bindComplete(L,meta,new Ogre::Vector3(*v3));
		}
	}
	else
	{
		lua_bindComplete(L,meta,new Ogre::Vector3(Ogre::Vector3::ZERO));
	}
	return 1;
}

/*
	mat.newMatrix3()
	mat.newMatrix3(mat3)
*/
static int newMat3(lua_State* L)
{
	const char* meta = "mat.Matrix3";
	if( lua_isuserdata(L,1) )
	{
		Ogre::Matrix3* m3 = (Ogre::Matrix3*)lua_cast(L,1,"Matrix3");
		if( m3 )
		{
			lua_bindComplete(L,meta,new Ogre::Matrix3(*m3));
		}
	}
	else
	{
		lua_bindComplete(L,meta,new Ogre::Matrix3(Ogre::Matrix3::IDENTITY));
	}
	return 1;
}

/*
	创建一个4元数
	mat.newQuaternion(x,y,z,w)
	mat.newQuaternion(mat3)
	mat.newQuaternion(axis,radian)
		axis是Vector3,radian是一个角度
	mat.newQuaternion()
*/
static int newQuaternion(lua_State* L)
{
	const char* meta = "mat.Quaternion";
	if( lua_isnumber(L,1) )
	{
		lua_bindComplete(L,meta,new Ogre::Quaternion(
			tonumber(1),tonumber(2),tonumber(3),tonumber(4)
			));
	}
	else if( lua_isuserdata(L,1) )
	{
		Ogre::Matrix3* m3 = (Ogre::Matrix3*)lua_isa(L,1,"Matrix3");
		if( m3 )
		{
			lua_bindComplete(L,meta,new Ogre::Quaternion(*m3));
		}
		else
		{
			Ogre::Vector3* v3 =  (Ogre::Vector3*)lua_cast(L,1,"Vector3");
			if( v3 )
			{
				lua_bindComplete(L,meta,new Ogre::Quaternion(Ogre::Radian(tonumber(2)),*v3));
			}
		}
	}
	else
	{
		lua_bindComplete(L,meta,new Ogre::Quaternion());
	}
	return 1;
}

static int toRadian(lua_State*L)
{
	pnumber(Ogre::Degree(tonumber(1)).valueRadians());
	return 1;
}

static int toDegree(lua_State*L)
{
	pnumber(Ogre::Radian(tonumber(1)).valueDegrees());
	return 1;
}

static const struct luaL_Reg matLibs[]=
{
	{"toRadian",toRadian},
	{"toDegree",toDegree},
	{"newVector3",newVec3},
	{"newMatrix3",newMat3},
	{"newQuaternion",newQuaternion},
	{nullptr,nullptr}
};

/*
	Ogre::Vector3
*/
#define METHOD( fun,T ) \
static int fun( lua_State* L )\
{\
	Ogre::T* self = (Ogre::T*)lua_cast( L,1,#T );\
	if( self == nullptr )return 0;

#define METHOD_END( ret ) \
	return ret; }

#define VEC3_METHOD( fun ) METHOD(fun,Vector3)

#define tovec3(n) get_vector3(L,n)
//处理两种情况，x,y,z和vec3
static Ogre::Vector3 get_vector3(lua_State* L,int n )
{
	if( lua_isnumber(L,n) )
	{
		return Ogre::Vector3(tonumber(n),tonumber(n+1),tonumber(n+2));
	}
	else
	{
		Ogre::Vector3* v3 = (Ogre::Vector3*)lua_cast(L,n,"Vector3");
		if( v3 )
			return *v3;
	}
	return Ogre::Vector3();
}

VEC3_METHOD(normalise)
pnumber(self->normalise());
METHOD_END(1)

VEC3_METHOD(crossProduct)
Ogre::Vector3 result = self->crossProduct(tovec3(2));
lua_bindComplete(L,"mat.Vector3",new Ogre::Vector3(result));
METHOD_END(1)

VEC3_METHOD(length)
pnumber(self->length());
METHOD_END(1)

VEC3_METHOD(distance)
pnumber(self->distance(tovec3(2)));
METHOD_END(1)

VEC3_METHOD(dotProduct)
pnumber(self->dotProduct(tovec3(2)));
METHOD_END(1)

VEC3_METHOD(midPoint)
Ogre::Vector3 result = self->midPoint(tovec3(2));
lua_bindComplete(L,"mat.Vector3",new Ogre::Vector3(result));
METHOD_END(1)

VEC3_METHOD(angleBetween)
pnumber(self->angleBetween(tovec3(2)).valueRadians());
METHOD_END(1)

VEC3_METHOD(reflect)
Ogre::Vector3 result = self->reflect(tovec3(2));
lua_bindComplete(L,"mat.Vector3",new Ogre::Vector3(result));
METHOD_END(1)

VEC3_METHOD(isZeroLength)
pbool(self->isZeroLength());
METHOD_END(1)

VEC3_METHOD(isNaN)
pbool(self->isNaN());
METHOD_END(1)

VEC3_METHOD(positionEquals)
pbool(self->positionEquals(tovec3(2)));
METHOD_END(1)

VEC3_METHOD(positionCloses)
pbool(self->positionCloses(tovec3(2)));
METHOD_END(0)

VEC3_METHOD(getRotationTo)
METHOD_END(0)

VEC3_METHOD(lua_Vec3tostring)
ostringstream os;
os<<self->x<<" "<<self->y<<" "<<self->z;
pstring(os.str().c_str());
METHOD_END(1)

VEC3_METHOD(addvec3)
lua_bindComplete(L,"mat.Vector3",new Ogre::Vector3(*self + tovec3(2)));
METHOD_END(1)

VEC3_METHOD(subvec3)
lua_bindComplete(L,"mat.Vector3",new Ogre::Vector3(*self - tovec3(2)));
METHOD_END(1)

VEC3_METHOD(mulvec3)
lua_bindComplete(L,"mat.Vector3",new Ogre::Vector3(*self*tonumber(2)));
METHOD_END(1)

VEC3_METHOD(divvec3)
lua_bindComplete(L,"mat.Vector3",new Ogre::Vector3(*self/ tonumber(2)));
METHOD_END(1)

VEC3_METHOD(eqvec3)
pbool(*self==tovec3(2));
METHOD_END(1)

//实现x,y,z的直接赋值__newindex的原型是newindex(obj,key,value)
VEC3_METHOD(newindexVec3)
	if( lua_isstring(L,2) )
	{
		const char* s = tostring(2);
		if( strcmp(s,"x")==0 )
		{
			self->x = tonumber(3);
		}
		else if( strcmp(s,"y")==0 )
		{
			self->y = tonumber(3);
		}
		else if( strcmp(s,"z")==0 )
		{
			self->z = tonumber(3);
		}
		else
		{
			ostringstream os;
			os << "'"<<s<<"' is not a member of Vector3";
			lua_errorstring(L,os.str());
		}
	}
	else
	{
		lua_errorstring(L,"Vector3 invalid index");
	}
METHOD_END(0)

/*
	加入__index为了实现直接取得x,y,z
	这略微增加处理时间
	__index函数原型是这样的index( obj,key )
*/
VEC3_METHOD(indexVec3)
	//先查原表
	if( lua_getmetatable(L,1) )
	{
		lua_pushvalue(L,2);
		lua_rawget(L,-2);
		lua_replace(L,-2);
		if( lua_isnil(L,-1) )
		{
			lua_pop(L,1);
			if( lua_isstring(L,2) )
			{
				const char* s = tostring(2);
				if( strcmp(s,"x")==0 )
				{
					pnumber(self->x);
				}
				else if( strcmp(s,"y")==0 )
				{
					pnumber(self->y);
				}
				else if( strcmp(s,"z")==0 )
				{
					pnumber(self->z);
				}
				else
					lua_pushnil(L);
			}
		}
	}
	else
	{
		lua_pushnil(L);
	}
METHOD_END(1)

//UString 回收函数
static int lua_gcVec3( lua_State* L )
{
	Ogre::Vector3* self = (Ogre::Vector3*)lua_cast(L,1,"Vector3");
	delete self;
	return 0;
}

static const struct luaL_Reg vec3Method[]=
{
	{"normalise",normalise},
	{"crossProduct",crossProduct},
	{"length",length},
	{"distance",distance},
	{"dotProduct",dotProduct},
	{"midPoint",midPoint},
	{"angleBetween",angleBetween},
	{"reflect",reflect},
	{"isZeroLength",isZeroLength},
	{"isNaN",isNaN},
	{"positionEquals",positionEquals},
	{"positionCloses",positionCloses},
	{"getRotationTo",getRotationTo},
	{"__gc",lua_gcVec3},
	{"__tostring",lua_Vec3tostring},
	{"__sub",subvec3},
	{"__add",addvec3},
	{"__mul",mulvec3},
	{"__div",divvec3},
	{"__eq",eqvec3},
	{"__newindex",newindexVec3},
	{"__index",indexVec3},
	{nullptr,nullptr}
};

/*
	Ogre::Matrix3
*/
#define MAT3_METHOD( fun ) METHOD(fun,Matrix3)

MAT3_METHOD(determinant)
pnumber(self->Determinant());
METHOD_END(1)

MAT3_METHOD(getColumn)
Ogre::Vector3 v3 = self->GetColumn(toint(2));
pnumber(v3.x);
pnumber(v3.y);
pnumber(v3.z);
METHOD_END(3)

MAT3_METHOD(setColumn)
self->SetColumn(toint(2),Ogre::Vector3(tonumber(3),tonumber(4),tonumber(5)));
METHOD_END(0)

MAT3_METHOD(hasScale)
pbool(self->hasScale());
METHOD_END(1)

MAT3_METHOD(inverse)
lua_bindComplete(L,"mat.Matrix3",new Ogre::Matrix3(self->Inverse()));
METHOD_END(1)

MAT3_METHOD( lua_gcmat3 )
	delete self;
METHOD_END(0)

MAT3_METHOD( lua_mat3tostring)
ostringstream os;
Ogre::Real* r[3];
r[0] = (*self)[0];
r[1] = (*self)[1];
r[2] = (*self)[2];
for( int i = 0;i<3;i++ )
{
	os << r[i][0] << " " <<r[i][1]<<" "<<r[i][2]<<" ";
}
pstring(os.str().c_str());
METHOD_END(1)

MAT3_METHOD( submat3)
Ogre::Matrix3* m3 = (Ogre::Matrix3*)lua_cast(L,2,"Matrix3");
if( m3 )
	lua_bindComplete(L,"mat.Matrix3",new Ogre::Matrix3((*self - *m3)));
METHOD_END(1)

MAT3_METHOD( addmat3)
Ogre::Matrix3* m3 = (Ogre::Matrix3*)lua_cast(L,2,"Matrix3");
if( m3 )
	lua_bindComplete(L,"mat.Matrix3",new Ogre::Matrix3((*self + *m3)));
METHOD_END(1)

MAT3_METHOD( mulmat3)
Ogre::Matrix3* m3 = (Ogre::Matrix3*)lua_isa(L,2,"Matrix3");
if( m3 )
	lua_bindComplete(L,"mat.Matrix3",new Ogre::Matrix3(((*self) * (*m3))));
else
{
	Ogre::Vector3* v3 = (Ogre::Vector3*)lua_isa(L,2,"Vector3");
	if( v3 )
		lua_bindComplete(L,"mat.Vector3",new Ogre::Vector3(((*self) * (*v3))));
	else
		lua_errorstring(L,"Matrix3 operator invalid argument");
}
METHOD_END(1)

MAT3_METHOD( eqmat3)
Ogre::Matrix3* m3 = (Ogre::Matrix3*)lua_cast(L,2,"Matrix3");
if( m3 )
	pbool(*self == *m3 );
METHOD_END(1)

MAT3_METHOD(identity)
*self = Ogre::Matrix3::IDENTITY;
METHOD_END(0)

static const struct luaL_Reg mat3Method[]=
{
	{"determinant",determinant},
	{"getColumn",getColumn},
	{"setColumn",setColumn},
	{"hasScale",hasScale},
	{"inverse",inverse},
	{"identity",identity},
	{"__gc",lua_gcmat3},
	{"__tostring",lua_mat3tostring},
	{"__sub",submat3},
	{"__add",addmat3},
	{"__mul",mulmat3},
	{"__eq",eqmat3},
	{nullptr,nullptr}
};

/*
	Ogre::Quaternion
*/
#define Q_METHOD( fun ) METHOD(fun,Quaternion)

Q_METHOD(getRoll)
pnumber(self->getRoll().valueRadians());
METHOD_END(1)

Q_METHOD(getPitch)
pnumber(self->getPitch().valueRadians());
METHOD_END(1)

Q_METHOD(getYaw)
pnumber(self->getYaw().valueRadians());
METHOD_END(1)

Q_METHOD(equals)
Ogre::Quaternion* q = (Ogre::Quaternion*)lua_cast(L,2,"Quaternion");
if( q )
pbool(self->equals(*q,Ogre::Radian(tonumber(3))));
METHOD_END(1)

Q_METHOD(q_isNaN)
pbool(self->isNaN());
METHOD_END(1)

Q_METHOD(q_inverse)
Ogre::Quaternion q = self->Inverse();
Ogre::Quaternion* qq = new Ogre::Quaternion();
qq->swap(q);
lua_bindComplete(L,"mat.Quaternion",qq );
METHOD_END(1)

Q_METHOD(q_normalise)
pnumber(self->normalise());
METHOD_END(1)

Q_METHOD(xAxis)
Ogre::Vector3 v = self->xAxis();
pnumber(v.x);
pnumber(v.y);
pnumber(v.z);
METHOD_END(3)

Q_METHOD(yAxis)
Ogre::Vector3 v = self->yAxis();
pnumber(v.x);
pnumber(v.y);
pnumber(v.z);
METHOD_END(3)

Q_METHOD(zAxis)
Ogre::Vector3 v = self->zAxis();
pnumber(v.x);
pnumber(v.y);
pnumber(v.z);
METHOD_END(3)

Q_METHOD(dot)
Ogre::Quaternion* q = (Ogre::Quaternion*)lua_cast(L,2,"Quaternion");
if( q )
pnumber(self->Dot(*q));
METHOD_END(1)

Q_METHOD(lua_gcquaternion)
delete self;
METHOD_END(0)

Q_METHOD(lua_qtostring)
METHOD_END(0)

Q_METHOD(subq)
Ogre::Quaternion* q = (Ogre::Quaternion*)lua_cast(L,2,"Quaternion");
if( q )
{
	lua_bindComplete(L,"mat.Quaternion",new Ogre::Quaternion((*self - *q).ptr()));
}
METHOD_END(1)

Q_METHOD(addq)
Ogre::Quaternion* q = (Ogre::Quaternion*)lua_cast(L,2,"Quaternion");
if( q )
{
	lua_bindComplete(L,"mat.Quaternion",new Ogre::Quaternion((*self + *q).ptr()));
}
METHOD_END(1)

Q_METHOD(mulq)
Ogre::Quaternion* q = (Ogre::Quaternion*)lua_isa(L,2,"Quaternion");
if( q )
{
	lua_bindComplete(L,"mat.Quaternion",new Ogre::Quaternion((*self * *q).ptr()));
}
else
{
	Ogre::Vector3* v = (Ogre::Vector3*)lua_isa(L,2,"Vector3");
	if( v )
		lua_bindComplete(L,"mat.Vector3",new Ogre::Vector3((*self * *v)));
}
METHOD_END(1)

Q_METHOD(eqq)
Ogre::Quaternion* q = (Ogre::Quaternion*)lua_cast(L,2,"Quaternion");
if( q )
	pbool( *self == *q );
METHOD_END(1)

Q_METHOD(newindexQ)
	if( lua_isstring(L,2) )
	{
		const char* s = tostring(2);
		if( strcmp(s,"x")==0 )
		{
			self->x = tonumber(3);
		}
		else if( strcmp(s,"y")==0 )
		{
			self->y = tonumber(3);
		}
		else if( strcmp(s,"z")==0 )
		{
			self->z = tonumber(3);
		}
		else if( strcmp(s,"w")==0 )
		{
			self->w = tonumber(4);
		}
		else
		{
			ostringstream os;
			os << "'"<<s<<"' is not a member of Quaternion";
			lua_errorstring(L,os.str());
		}
	}
	else
	{
		lua_errorstring(L,"Quaternion invalid index");
	}
METHOD_END(0)

//实现直接取的x,y,z,w
Q_METHOD(indexQ)
	if( lua_getmetatable(L,1) )
	{
		lua_pushvalue(L,2);
		lua_rawget(L,-2);
		lua_replace(L,-2);
		if( lua_isnil(L,-1) )
		{
			lua_pop(L,1);
			if( lua_isstring(L,2) )
			{
				const char* s = tostring(2);
				if( strcmp(s,"x")==0 )
				{
					pnumber(self->x);
				}
				else if( strcmp(s,"y")==0 )
				{
					pnumber(self->y);
				}
				else if( strcmp(s,"z")==0 )
				{
					pnumber(self->z);
				}
				else if( strcmp(s,"w")==0 )
				{
					pnumber(self->w);
				}
				else
					lua_pushnil(L);
			}
		}
	}
	else
	{
		lua_pushnil(L);
	}
METHOD_END(1)

static const struct luaL_Reg quatMethod[]=
{
	{"getRoll",getRoll},
	{"getPitch",getPitch},
	{"getYaw",getYaw},
	{"equals",equals},
	{"isNaN",q_isNaN},
	{"inverse",q_inverse},
	{"normalise",q_normalise},
	{"xAxis",xAxis},
	{"yAxis",yAxis},
	{"zAxis",zAxis},
	{"dot",dot},
	{"__gc",lua_gcquaternion},
	{"__tostring",lua_qtostring},
	{"__sub",subq},
	{"__add",addq},
	{"__mul",mulq},
	{"__eq",eqq},
	{"__newindex",newindexQ},
	{"__index",indexQ},
	{nullptr,nullptr}
};

void luaopen_mat( lua_State* L )
{
	LuaManager& lm=LuaManager::getSingleton();

	lm.registerGlobal("mat",matLibs);
	lm.registerClass("mat.Vector3",vec3Method);
	lm.registerClass("mat.Matrix3",mat3Method);
	lm.registerClass("mat.Quaternion",quatMethod);
}