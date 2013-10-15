#include "stdheader.h"
#include "Geometry.h"
#include <math.h>

SINGLETON(Geometry)

Ogre::MeshPtr Geometry::createSphere( const string& name,Ogre::Real r,int des ){
	Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createManual( name,mGroup,this );
	Param gp;
	gp.type = SPHERE;
	gp.density = des;
	gp.data = tSphere(r);
	mParams[mesh.getPointer()] = gp;
	mesh->load();
	return mesh;
}

void Geometry::loadResource(Ogre::Resource* resource){
	Ogre::Mesh* msh = static_cast<Ogre::Mesh*>(resource);
	GeometryParamMap::iterator it = mParams.find(resource);
	if( it == mParams.end() ){
		ERROR_LOG("loadResource"<<" invalid parameter");
		return;
	}
	Param& gp = it->second;
	switch( gp.type ){
		case SPHERE:
			loadSphere( msh,gp );
			break;
		case BOX:
			loadBox( msh,gp );
			break;
		case CYLINDER:
			loadCylinder( msh,gp );
			break;
		case PULL:
			loadPull( msh,gp );
			break;
		case SPIN:
			loadSpin( msh,gp );
			break;
		default:
			ERROR_LOG("loadResource"<<" unknown geometry:"<<it->first );
	}
}

Ogre::MeshPtr Geometry::createBox( const string& name,Ogre::Real volume[3],int des){
	Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createManual( name,mGroup,this );
	Param gp;
	gp.type = BOX;
	gp.density = des;
	gp.data = tBox(volume);
	mParams[mesh.getPointer()] = gp;
	mesh->load();
	return mesh;
}

Ogre::MeshPtr Geometry::createCylinder( const string& name,Ogre::Real r,Ogre::Real length,int des){
	Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createManual( name,mGroup,this );
	Param gp;
	gp.type = CYLINDER;
	gp.density = des;
	gp.data = tCylinder(r,length);
	mParams[mesh.getPointer()] = gp;
	mesh->load();
	return mesh;
}

void Geometry::loadBox( Ogre::Mesh* msh,Param& gp ){
	Ogre::Real v[3];
	int density;
	try{
		tBox box=boost::any_cast<tBox>(gp.data);
		v[0] = box.mVolume[0]/2;
		v[1] = box.mVolume[1]/2;
		v[2] = box.mVolume[2]/2;
	}catch( boost::bad_any_cast& e ){
		ERROR_LOG( "loadBox"<<" invalid paramter:"<<e.what() );
		return;
	}
	Ogre::VertexData* pvd = OGRE_NEW Ogre::VertexData();
	msh->sharedVertexData = pvd;
	
	Ogre::VertexDeclaration* pvdecl = pvd->vertexDeclaration;

	UseDefaultFormat( pvd->vertexDeclaration );
	
	density = (int)(gp.density/4);
	//注意这里density是取其值的1/4
	//用在有弧形边缘的方盒上
	if( density > 90 || density < 1 ){
		WARNING_LOG( "loadBox"<<" invalid density:"<<density )
		density = 9;
		WARNING_LOG( "loadBox"<<" reset density:"<<density )
	}
	//目前简化的盒子不具有弧形边缘
	pvd->vertexCount = 14;
	//创建顶点缓存
	Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().
		createVertexBuffer(pvdecl->getVertexSize(0),
		pvd->vertexCount,Ogre::HardwareBuffer::HBU_DYNAMIC,
		false );

	pvd->vertexBufferBinding->setBinding(0,vbuf);

	float * pReal = static_cast<float*>(vbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));
	if( pReal ){
		Ogre::Vector3 p;
		Ogre::Vector2 t;
		Ogre::Real w = 2*(v[2]+v[0]);
		Ogre::Real h = 2*v[2]+v[1];

		p = Ogre::Vector3(v[0],v[1],v[2]);
		t = Ogre::Vector2(0,(v[2]+v[1])/h);
		GeometryPoint2(pReal,p,t); //0
		p = Ogre::Vector3(-v[0],v[1],v[2]);
		t = Ogre::Vector2(v[0]/w,(v[2]+v[1])/h);
		GeometryPoint2(pReal,p,t); //1
		p = Ogre::Vector3(-v[0],-v[1],v[2]);
		t = Ogre::Vector2(v[0]/w,v[2]/h);
		GeometryPoint2(pReal,p,t); //2
		p = Ogre::Vector3(v[0],-v[1],v[2]);
		t = Ogre::Vector2(0,v[2]/h);
		GeometryPoint2(pReal,p,t); //3

		p = Ogre::Vector3(-v[0],v[1],-v[2]);
		t = Ogre::Vector2((v[0]+v[2])/w,(v[2]+v[1])/h);
		GeometryPoint2(pReal,p,t); //4
		p = Ogre::Vector3(v[0],v[1],-v[2]);
		t = Ogre::Vector2((2*v[0]+v[2])/w,(v[2]+v[1])/h);
		GeometryPoint2(pReal,p,t); //5
		p = Ogre::Vector3(v[0],-v[1],-v[2]);
		t = Ogre::Vector2((2*v[0]+v[2])/w,v[2]/h);
		GeometryPoint2(pReal,p,t); //6
		p = Ogre::Vector3(-v[0],-v[1],-v[2]);
		t = Ogre::Vector2((v[0]+v[2])/w,v[2]/h);
		GeometryPoint2(pReal,p,t); //7

		p = Ogre::Vector3(v[0],v[1],v[2]);
		t = Ogre::Vector2(v[0]/w,1);
		GeometryPoint2(pReal,p,t); //8 对应与0
		p = Ogre::Vector3(v[0],v[1],-v[2]);
		t = Ogre::Vector2((v[0]+v[2])/w,1);
		GeometryPoint2(pReal,p,t); //9 对应于5
		p = Ogre::Vector3(v[0],-v[1],v[2]);
		t = Ogre::Vector2(v[0]/w,0);
		GeometryPoint2(pReal,p,t); //10 对应于3
		p = Ogre::Vector3(v[0],-v[1],-v[2]);
		t = Ogre::Vector2((v[0]+v[2])/w,0);
		GeometryPoint2(pReal,p,t); //11 对应于6

		p = Ogre::Vector3(v[0],v[1],v[2]);
		t = Ogre::Vector2(1,(v[2]+v[1])/h);
		GeometryPoint2(pReal,p,t); //12 对应于0 / 8
		p = Ogre::Vector3(v[0],-v[1],v[2]);
		t = Ogre::Vector2(1,v[2]/h);
		GeometryPoint2(pReal,p,t); //13 对应于 3 / 10

		vbuf->unlock();
	}
	Ogre::SubMesh* psub = msh->createSubMesh();
	psub->useSharedVertices = true;
	psub->indexData->indexCount = 3*2*6;
	psub->indexData->indexBuffer = Ogre::HardwareBufferManager::getSingleton().
		createIndexBuffer(Ogre::HardwareIndexBuffer::IT_16BIT,
		psub->indexData->indexCount,Ogre::HardwareBuffer::HBU_DYNAMIC,false );
	unsigned short* pindex = static_cast<unsigned short*>
		(psub->indexData->indexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD));

	if( pindex ){

		GeometryTriangle( pindex,0,1,2 ); //逆时针绕
		GeometryTriangle( pindex,0,2,3 );

		GeometryTriangle( pindex,1,4,7 );
		GeometryTriangle( pindex,1,7,2 );

		GeometryTriangle( pindex,1,8,9 );
		GeometryTriangle( pindex,1,9,4 );

		GeometryTriangle( pindex,2,7,11 );
		GeometryTriangle( pindex,2,11,10 );

		GeometryTriangle( pindex,4,5,6 );
		GeometryTriangle( pindex,4,6,7 );

		GeometryTriangle( pindex,5,12,13 );
		GeometryTriangle( pindex,5,13,6 );
		psub->indexData->indexBuffer->unlock();
	}
	//设置包围盒与包围球
	msh->_setBounds(Ogre::AxisAlignedBox(Ogre::Vector3(-v[0],-v[1],-v[2]),Ogre::Vector3(v[0],v[1],v[2])),true);
	msh->_setBoundingSphereRadius(sqrtf(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]));
}

void Geometry::loadCylinder( Ogre::Mesh* msh,Param& gp ){
	Ogre::Real r,l;
	try{
		tCylinder tc = boost::any_cast<tCylinder>(gp.data);
		r = tc.mRadius;
		l = tc.mLength;
	}catch( boost::bad_any_cast& e ){
		ERROR_LOG( "loadCylinder"<<" invalid paramter:"<<e.what() );
		return;
	}
	Ogre::VertexData* pvd = OGRE_NEW Ogre::VertexData();
	msh->sharedVertexData = pvd;
	
	Ogre::VertexDeclaration* pvdecl = pvd->vertexDeclaration;

	UseDefaultFormat( pvd->vertexDeclaration );

	if( gp.density > 360 || gp.density < 3 ){
		WARNING_LOG( "loadCylinder"<<" invalid density(3~360):"<<gp.density )
		gp.density = DEFAULT_DENSITY;
		WARNING_LOG( "loadCylinder"<<" reset density:"<<gp.density )
	}

	//顶点个数，加上上下中心点
	pvd->vertexCount = 4*(gp.density+1)+2;
	//创建顶点缓存
	Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().
		createVertexBuffer(pvdecl->getVertexSize(0),
		pvd->vertexCount,Ogre::HardwareBuffer::HBU_DYNAMIC,
		false );

	pvd->vertexBufferBinding->setBinding(0,vbuf);

	float * pReal = static_cast<float*>(vbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));
	if( pReal ){
		//计算柱面上面的点，上下两个圆
		int i;
		Ogre::Vector3 p,n;
		Ogre::Vector2 t;
		Ogre::Real sinv,cosv,d;
		
		p = Ogre::Vector3( 0,0,0.5f*l );
		n = Ogre::Vector3::UNIT_Z;
		t = Ogre::Vector2(0.5f,0.5f);
		GeometryPoint( pReal,p,n,t );
		p = Ogre::Vector3( 0,0,-0.5f*l );
		n = Ogre::Vector3::NEGATIVE_UNIT_Z;
		t = Ogre::Vector2(0.5f,0.5f);
		GeometryPoint( pReal,p,n,t );
		for( i=0;i < gp.density+1;++i ){
			d =  (float)(2.0*i*Ogre::Math::PI/gp.density);
			cosv = cosf( d );
			sinv = sinf( d );
			p = Ogre::Vector3( r*cosv,r*sinv,0.5f*l );
			n = Ogre::Vector3::UNIT_Z;
			t = Ogre::Vector2(0.5f*cosv+0.5f,0.5f*sinv+0.5f);
			//上圆面
			GeometryPoint( pReal,p,n,t );
			//下圆面
			p = Ogre::Vector3( r*cosv,r*sinv,-0.5f*l );
			n = Ogre::Vector3::NEGATIVE_UNIT_Z;
			GeometryPoint( pReal,p,n,t );
			//侧面上边
			p = Ogre::Vector3( r*cosv,r*sinv,0.5f*l );
			t = Ogre::Vector2((Ogre::Real)(d/(2.0*Ogre::Math::PI)),0);
			GeometryPoint3( pReal,p,t );
			//侧面下边
			p = Ogre::Vector3( r*cosv,r*sinv,-0.5f*l );
			t = Ogre::Vector2((Ogre::Real)(d/(2.0*Ogre::Math::PI)),1);
			GeometryPoint3( pReal,p,t );
		}
		vbuf->unlock();
	}
	Ogre::SubMesh* psub = msh->createSubMesh();
	//使用共享数据，就是Mesh中的数据
	psub->useSharedVertices = true;
	psub->indexData->indexCount = 12*gp.density;

	psub->indexData->indexBuffer = Ogre::HardwareBufferManager::getSingleton().
		createIndexBuffer(Ogre::HardwareIndexBuffer::IT_16BIT,
		psub->indexData->indexCount,Ogre::HardwareBuffer::HBU_DYNAMIC,false );

	unsigned short* pindex = static_cast<unsigned short*>
		(psub->indexData->indexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD));

	if( pindex ){
		int offset1 = 2; //前两个点是上下圆原点

		for( int i=0;i < gp.density;++i ){
			//上圆面
			GeometryTriangle(pindex,0,i*4+offset1,offset1+(i+1)*4);
			//下圆面
			GeometryTriangle(pindex,1,offset1+(i+1)*4+1,i*4+offset1+1);
			//侧面的圆柱面
			GeometryTriangle(pindex,i*4+2+offset1,i*4+3+offset1,(i+1)*4+2+offset1);

			GeometryTriangle(pindex,i*4+3+offset1,(i+1)*4+3+offset1,(i+1)*4+2+offset1);
		}
		psub->indexData->indexBuffer->unlock();
	}
	//设置包围盒与包围球
	msh->_setBounds(Ogre::AxisAlignedBox(Ogre::Vector3(-r,-r,-l/2),Ogre::Vector3(r,r,l/2)),true);
	msh->_setBoundingSphereRadius(sqrtf(r*r+l*l/4));
}

void Geometry::loadSphere( Ogre::Mesh* msh,Param& gp ){
	Ogre::Real r;
	try{
		r = boost::any_cast<tSphere>(gp.data).mRadius;
	}catch( boost::bad_any_cast& e ){
		ERROR_LOG( "loadSphere"<<" invalid paramter:"<<e.what() );
		return;
	}
	Ogre::VertexData* pvd = OGRE_NEW Ogre::VertexData();
	msh->sharedVertexData = pvd;
	
	Ogre::VertexDeclaration* pvdecl = pvd->vertexDeclaration;

	UseDefaultFormat( pvd->vertexDeclaration );

	if( gp.density > 360 || gp.density < 6 ){
		WARNING_LOG( "loadSphere"<<" invalid density:(6~360)"<<gp.density )
		gp.density = DEFAULT_DENSITY;
		WARNING_LOG( "loadSphere"<<" reset density:"<<gp.density )
	}

	//顶点个数
	pvd->vertexCount = (gp.density+1) * ((size_t)(gp.density/2)+1);
	//创建顶点缓存
	Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().
		createVertexBuffer(pvdecl->getVertexSize(0),
		pvd->vertexCount,Ogre::HardwareBuffer::HBU_DYNAMIC,
		false );

	pvd->vertexBufferBinding->setBinding(0,vbuf);

	float * pReal = static_cast<float*>(vbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));
	if( pReal ){
		//计算球上面的点
		Ogre::Vector3 p,n;
		Ogre::Vector2 t;
		int N = (int)(gp.density/2)+1;

		//从球的两个极点形成的半圆线，绕一周形成球面
		for( int i = 0; i < gp.density+1; ++i ){
			float d = (float)(2.0*i*Ogre::Math::PI/gp.density);
			float sinv = sinf( d );
			float cosv = cosf( d );
			for( int j = 0; j < N;++j ){
				float s = (float)(j*Ogre::Math::PI/(N-1));
				float sins = sinf(s);
				float coss = cosf(s);
				p = Ogre::Vector3(r*cosv*sins,r*sinv*sins,r*coss);
				n = Ogre::Vector3(cosv*sins,sinv*sins,coss);
				t = Ogre::Vector2((Ogre::Real)(d/(2.0*Ogre::Math::PI)),(Ogre::Real)(s/Ogre::Math::PI));
				GeometryPoint(pReal,p,n,t);
			}
		}
		vbuf->unlock();
	}
	Ogre::SubMesh* psub = msh->createSubMesh();
	//使用共享数据，就是Mesh中的数据
	psub->useSharedVertices = true;
	psub->indexData->indexCount = 6*gp.density*((int)(gp.density/2)-1);
	psub->indexData->indexBuffer = Ogre::HardwareBufferManager::getSingleton().
		createIndexBuffer(Ogre::HardwareIndexBuffer::IT_16BIT,
		psub->indexData->indexCount,Ogre::HardwareBuffer::HBU_DYNAMIC,false );
	//组织球面上的三角形
	unsigned short* pindex = static_cast<unsigned short*>
		(psub->indexData->indexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD));

	if( pindex ){
		int N = (int)(gp.density/2)+1;
		
		for( int i = 0; i < gp.density;++i ){
			for( int j = 0; j < N-1;++j ){
				if( j != N-2 ) //最后一个不要下半区
					GeometryTriangle(pindex,i*N + j,i*N + j + 1,(i+1)*N + j + 1); //逆时针绕
				if( j != 0 ) //第一个不要上半区
					GeometryTriangle(pindex,i*N + j,(i+1)*N + j + 1,(i+1)*N + j);
			}
		}

		psub->indexData->indexBuffer->unlock();
	}
	//设置包围盒与包围球
	msh->_setBounds(Ogre::AxisAlignedBox(Ogre::Vector3(-r,-r,-r),Ogre::Vector3(r,r,r)),true);
	msh->_setBoundingSphereRadius(r);
}

void Geometry::UseDefaultFormat(Ogre::VertexDeclaration* pvdecl ){
	size_t offset = 0;

	pvdecl->addElement( 0,offset,Ogre::VET_FLOAT3,Ogre::VES_POSITION );
	offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
	//法线
	pvdecl->addElement( 0,offset,Ogre::VET_FLOAT3,Ogre::VES_NORMAL );
	offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
	//纹理坐标
	pvdecl->addElement( 0,offset,Ogre::VET_FLOAT2,Ogre::VES_TEXTURE_COORDINATES );
}

void Geometry::GeometryPoint(float*& pReal,const Ogre::Vector3& p,
							const Ogre::Vector3& n,const Ogre::Vector2& t ){
	*pReal++ = p.x;
	*pReal++ = p.y;
	*pReal++ = p.z;
	*pReal++ = n.x;
	*pReal++ = n.y;
	*pReal++ = n.z;
	*pReal++ = t.x;
	*pReal++ = t.y;
}

void Geometry::GeometryPoint2(float*& pReal,const Ogre::Vector3& p,
							const Ogre::Vector2& t ){
	Ogre::Vector3 n(p);
	n.normalise();
	*pReal++ = p.x;
	*pReal++ = p.y;
	*pReal++ = p.z;
	*pReal++ = n.x;
	*pReal++ = n.y;
	*pReal++ = n.z;
	*pReal++ = t.x;
	*pReal++ = t.y;
}

void Geometry::GeometryPoint3(float*& pReal,const Ogre::Vector3& p,
	const Ogre::Vector2& t ){
	Ogre::Vector3 n(p.x,p.y,0);
	n.normalise();
	*pReal++ = p.x;
	*pReal++ = p.y;
	*pReal++ = p.z;
	*pReal++ = n.x;
	*pReal++ = n.y;
	*pReal++ = n.z;
	*pReal++ = t.x;
	*pReal++ = t.y;
}

void Geometry::GeometryTriangle(unsigned short*& pindex,int p0,int p1,int p2){
	*pindex++ = p0;
	*pindex++ = p1;
	*pindex++ = p2;
}

Geometry::Geometry():mGroup(Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME){
//	Ogre::ResourceGroupManager::getSingleton().createResourceGroup(mGroup);
}

Geometry::~Geometry(){
//	Ogre::ResourceGroupManager::getSingleton().destroyResourceGroup(mGroup);
}

void Geometry::destroyAll(){
	for( GeometryParamMap::iterator it = mParams.begin();
		it!=mParams.end();++it ){
			Ogre::MeshManager::getSingleton().remove( it->first->getName() );
	}
	mParams.clear();
}

void Geometry::destroyGemetry( Ogre::MeshPtr& pt ){
	if( !pt.isNull() ){
		BOOST_AUTO( it,mParams.find(pt.getPointer()) );
		if( it != mParams.end() ){
			mParams.erase( it );
		}
		Ogre::MeshManager::getSingleton().remove( pt->getName() );
	}
}

Geometry::Param* Geometry::getGeometryParam( const Ogre::MeshPtr& mp ){
	BOOST_AUTO(it,mParams.find( mp.getPointer() ));
	if( it!=mParams.end() ){
		return &(it->second);
	}
	return nullptr;
}

Ogre::MeshPtr Geometry::createPull( const string& name,const tPath& p,Ogre::Real length,int des){
	Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createManual( name,mGroup,this );
	Param gp;
	gp.type = PULL;
	gp.density = des;
	gp.data = tPull(p,length);
	mParams[mesh.getPointer()] = gp;
	mesh->load();
	return mesh;
}

Ogre::MeshPtr Geometry::createSpin( const string& name,const tPath& p,int des){
	Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createManual( name,mGroup,this );

	Param gp;
	gp.type = SPIN;
	gp.density = des;
	gp.data = tSpin(p);
	mParams[mesh.getPointer()] = gp;

	mesh->load();

	return mesh;
}

void Geometry::loadPull( Ogre::Mesh* msh,Param& gp ){
	tPath path;
	Ogre::Real length;
	int count;

	try{
		//重新构造一个path
		path = boost::any_cast<tPull>(gp.data).mPath;
		length = boost::any_cast<tPull>(gp.data).mLength;
	}catch( boost::bad_any_cast& e ){
		ERROR_LOG( "loadPull"<<" invalid paramter:"<<e.what()<<" name:"<<msh->getName() );
		return;
	}
	count = path.size();

	if( count < 1 ){
		ERROR_LOG( "loadPull"<<" invalid paramter:"<<"path size less than 1" <<" name:"<<msh->getName());
		return;
	}

	Ogre::VertexData* pvd = OGRE_NEW Ogre::VertexData();
	msh->sharedVertexData = pvd;
	
	Ogre::VertexDeclaration* pvdecl = pvd->vertexDeclaration;

	UseDefaultFormat( pvd->vertexDeclaration );

	if( gp.density > 360 || gp.density < 1 ){
		WARNING_LOG( "loadPull"<<" invalid density:(1~360)"<<gp.density )
		gp.density = DEFAULT_DENSITY;
		WARNING_LOG( "loadPull"<<" reset density:"<<gp.density )
	}

	//顶点个数
	pvd->vertexCount = count*(gp.density+1);
	//创建顶点缓存
	Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().
		createVertexBuffer(pvdecl->getVertexSize(0),
		pvd->vertexCount,Ogre::HardwareBuffer::HBU_DYNAMIC,
		false );

	pvd->vertexBufferBinding->setBinding(0,vbuf);

	//计算法向量
	Ogre::AxisAlignedBox box;
	tPath norm = PathNormal( path );
	float * pReal = static_cast<float*>(vbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));
	if( pReal ){
		Ogre::Vector3 p,n;
		Ogre::Vector2 t;

		for( int i = 0;i < (gp.density+1);++i ){
			for( int j = 0;j < count;++j ){
				p = path[j];
				p.z += length*(Ogre::Real)i/(Ogre::Real)(gp.density+1);
				n = norm[j];
				t = Ogre::Vector2((Ogre::Real)i/(Ogre::Real)gp.density,(Ogre::Real)j/(Ogre::Real)(count-1));
				box.merge(p);
				GeometryPoint( pReal,p,n,t );
			}
		}
		vbuf->unlock();
	}
	Ogre::SubMesh* psub = msh->createSubMesh();
	//使用共享数据，就是Mesh中的数据
	psub->useSharedVertices = true;
	psub->indexData->indexCount = 2*3*(count-1)*gp.density;
	psub->indexData->indexBuffer = Ogre::HardwareBufferManager::getSingleton().
		createIndexBuffer(Ogre::HardwareIndexBuffer::IT_16BIT,
		psub->indexData->indexCount,Ogre::HardwareBuffer::HBU_DYNAMIC,false );

	unsigned short* pindex = static_cast<unsigned short*>
		(psub->indexData->indexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD));

	if( pindex ){
		for( int i = 0;i < gp.density;++i ){
			for( int j = 0;j < count-1;++j ){
				GeometryTriangle(pindex,i*count+j,(i+1)*count+j+1,i*count+j+1 ); //顺时针绕
				GeometryTriangle(pindex,i*count+j,(i+1)*count+j,(i+1)*count+j+1 );
			}
		}
		psub->indexData->indexBuffer->unlock();
	}
	//设置包围盒与包围球
	msh->_setBounds(box,true);
	Ogre::Vector3 minv = box.getMinimum();
	Ogre::Vector3 maxv = box.getMaximum();
	msh->_setBoundingSphereRadius(max(minv.length(),maxv.length()));
}

Ogre::Vector3 Geometry::Normal2( const Ogre::Vector3& p0,
								const Ogre::Vector3& p1 ){
	Ogre::Vector3 e1,e2,n;

	e1 = p1.crossProduct(p0);
	e2 = p1-p0;
	n=e2.crossProduct(e1);
	n.normalise();
	return n;
}

Geometry::tPath Geometry::PathNormal( const tPath& p ){
	int i,count = p.size();
	Ogre::Vector3 v;
	tPath n(count),nn(count);

	//先算两点之间线段的法线
	for( i = 0;i < count-1;++i )
		n.push_back( Normal2( p[i],p[i+1] ) );

	for( i = 0;i < count;++i ){
		if( i==0 )
			nn.push_back(n[i]); //取第一段法向量
		else if(i==count-1)
			nn.push_back( n[i-1] ); //最后一段法向量
		else{
			v = n[i-1]+n[i];
			v.normalise(); //求中间那个向量
			nn.push_back( v );
		}
	}
	return nn;
}

void Geometry::loadSpin( Ogre::Mesh* msh,Param& gp ){
	tPath path;
	int count;
	try{
		//重新构造一个path
		path = boost::any_cast<tSpin>(gp.data).mPath;
	}catch( boost::bad_any_cast& e ){
		ERROR_LOG( "loadSpin"<<" invalid paramter:"<<e.what()<<" name:"<<msh->getName() );
		return;
	}
	count = path.size();

	if( count < 1 ){
		ERROR_LOG( "loadSpin"<<" invalid paramter:"<<"path size less than 1" <<" name:"<<msh->getName());
		return;
	}

	Ogre::VertexData* pvd = OGRE_NEW Ogre::VertexData();
	msh->sharedVertexData = pvd;
	
	Ogre::VertexDeclaration* pvdecl = pvd->vertexDeclaration;

	UseDefaultFormat( pvd->vertexDeclaration );

	if( gp.density > 360 || gp.density < 3 ){
		WARNING_LOG( "loadSpin"<<" invalid density:(3~360)"<<gp.density )
		gp.density = DEFAULT_DENSITY;
		WARNING_LOG( "loadSpin"<<" reset density:"<<gp.density )
	}

	//顶点个数
	pvd->vertexCount = count*(gp.density+1);
	//创建顶点缓存
	Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().
		createVertexBuffer(pvdecl->getVertexSize(0),
		pvd->vertexCount,Ogre::HardwareBuffer::HBU_DYNAMIC,
		false );

	pvd->vertexBufferBinding->setBinding(0,vbuf);

	//计算法向量
	Ogre::AxisAlignedBox box;
	tPath norm = PathNormal( path );
	float * pReal = static_cast<float*>(vbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));
	if( pReal ){
		Ogre::Vector3 p,n;
		Ogre::Vector2 t;
		Ogre::Real d;

		for( int i = 0;i < (gp.density+1);++i ){
			d =  (float)(2.0*i*Ogre::Math::PI/gp.density);
			Ogre::Quaternion rot( Ogre::Radian(d),Ogre::Vector3::UNIT_Z );
			for( int j = 0;j < count;++j ){
				p = rot*path[j];
				n = rot*norm[j];
				t = Ogre::Vector2(d/(Ogre::Real)(2.0*Ogre::Math::PI),(Ogre::Real)j/(Ogre::Real)(count-1));
				box.merge(p);
				GeometryPoint( pReal,p,n,t );
			}
		}
		vbuf->unlock();
	}
	Ogre::SubMesh* psub = msh->createSubMesh();
	//使用共享数据，就是Mesh中的数据
	psub->useSharedVertices = true;
	psub->indexData->indexCount = 2*3*(count-1)*gp.density;
	psub->indexData->indexBuffer = Ogre::HardwareBufferManager::getSingleton().
		createIndexBuffer(Ogre::HardwareIndexBuffer::IT_16BIT,
		psub->indexData->indexCount,Ogre::HardwareBuffer::HBU_DYNAMIC,false );

	unsigned short* pindex = static_cast<unsigned short*>
		(psub->indexData->indexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD));

	if( pindex ){
		for( int i = 0;i < gp.density;++i ){
			for( int j = 0;j < count-1;++j ){
				GeometryTriangle(pindex,i*count+j,i*count+j+1,(i+1)*count+j+1 ); //逆时针绕
				GeometryTriangle(pindex,i*count+j,(i+1)*count+j+1,(i+1)*count+j );
			}
		}
		psub->indexData->indexBuffer->unlock();
	}
	//设置包围盒与包围球
	msh->_setBounds(box,true);
	Ogre::Vector3 minv = box.getMinimum();
	Ogre::Vector3 maxv = box.getMaximum();
	msh->_setBoundingSphereRadius(max(minv.length(),maxv.length()));
}
