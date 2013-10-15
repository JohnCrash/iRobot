#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

class Geometry:
	public Ogre::ManualResourceLoader,
	public Ogre::Singleton<Geometry>
{
public:
	const static int DEFAULT_DENSITY = 36;

	enum Type{
		UNKNOW,
		SPHERE, //球
		BOX, //盒子
		CYLINDER, //圆柱
		PULL, //拉长
		SPIN, //自旋
		PLANE //平面
	};

	typedef vector<Ogre::Vector3> tPath;

	Geometry();
	virtual ~Geometry();

	Ogre::MeshPtr createSphere( const string& name,Ogre::Real r,int des=DEFAULT_DENSITY);
	Ogre::MeshPtr createBox( const string& name,Ogre::Real volume[3],int des=DEFAULT_DENSITY);
	Ogre::MeshPtr createCylinder( const string& name,Ogre::Real r,Ogre::Real length,int des=DEFAULT_DENSITY);
	//path在xy平面上，沿着z轴拉伸。z轴和原点对称
	Ogre::MeshPtr createPull( const string& name,const tPath& p,Ogre::Real length,int des=DEFAULT_DENSITY);
	//path在yz平面上，绕z轴自旋
	//要求path的绕行方向是逆时针的
	Ogre::MeshPtr createSpin( const string& name,const tPath& p,int des=DEFAULT_DENSITY);

	void destroyAll();

	void destroyGemetry( Ogre::MeshPtr& pt );

	struct tSphere{
		tSphere(Ogre::Real r):mRadius(r){}
		Ogre::Real mRadius; //圆球的半径
	};
	struct tBox{
		Ogre::Real mVolume[3]; //长 宽 高
		tBox(Ogre::Real v[3]){
			mVolume[0] = v[0];
			mVolume[1] = v[1];
			mVolume[2] = v[2];
		}
	};
	struct tCylinder{
		tCylinder( Ogre::Real r,Ogre::Real l ):mRadius(r),mLength(l){}
		Ogre::Real mRadius; //圆柱的半径
		Ogre::Real mLength; //圆柱的长度
	};
	struct tSpin{
		tSpin(tPath p):mPath(p){}
		tPath mPath;
	};
	struct tPull{
		tPull(tPath p,Ogre::Real l):mPath(p),mLength(l){}
		tPath mPath;
		Ogre::Real mLength; //拉伸长度
	};
	struct Param{
		Type type;
		/*desity 代表线框的密度
			对于圆
			desity代表给一个圆分成多少分，例如desity=6代表从将圆分成6份
			根据具体的形体有不同的含义
		*/
		int density;

		//只能是tBox,tCylinder,tSpin,tPull中的一个
		boost::any data; 
	};
	//取出几何参数，同时也用它设置几何参数
	Geometry::Param* getGeometryParam( const Ogre::MeshPtr& mp );
protected:
	virtual void loadResource(Ogre::Resource* resource);
	void loadSphere( Ogre::Mesh* msh,Param& gp );
	void loadBox( Ogre::Mesh* msh,Param& gp );
	void loadCylinder( Ogre::Mesh* msh,Param& gp );
	void loadPull( Ogre::Mesh* msh,Param& gp );
	void loadSpin( Ogre::Mesh* msh,Param& gp );

	void UseDefaultFormat(Ogre::VertexDeclaration* pvdecl );
	void GeometryPoint(float*& pReal,const Ogre::Vector3& p,
							const Ogre::Vector3& n,
							const Ogre::Vector2& t );
	//法线自动被计算，法线是原点到p点的矢量。
	void GeometryPoint2(float*& pReal,const Ogre::Vector3& p,
							const Ogre::Vector2& t );
	//用于Cylinder，法线是从p点在z轴上的坐标点到p点的矢量。
	void GeometryPoint3(float*& pReal,const Ogre::Vector3& p,
							const Ogre::Vector2& t );
	void GeometryTriangle(unsigned short*& pindex,int p0,int p1,int p2);

	Ogre::Vector3 Normal2( const Ogre::Vector3& p0,
		const Ogre::Vector3& p1 );

	//返回是一个路径。每个点对应与p中的每一个点的法向量
	tPath PathNormal( const tPath& p );

	string mGroup;
	typedef map<Ogre::Resource*, Param> GeometryParamMap;
	GeometryParamMap mParams;
};

#endif