#ifndef _MATH3D_H_
#define _MATH3D_H_

namespace Math3d{
	typedef pair<bool,Ogre::Vector3> CalcResult;

	//计算在A上的一个点，它连接垂直与B的线是最短的
	CalcResult CalcNearPoint( const Ogre::Ray& A,const Ogre::Ray& B );

	//计算射线A和平面P的交点
	CalcResult CalcRayPlanePoint( const Ogre::Ray& A,const Ogre::Plane& P );

	//计算射线上的一点，该点距离射线原点的距离是t
	Ogre::Vector3 RayPoint( const Ogre::Ray& A,Ogre::Real t );
	//已知直线，求点的参数
	Ogre::Real RayParameter( const Ogre::Ray& A,const Ogre::Vector3& v );

	//不对A的方向进行正则化
	Ogre::Vector3 RayPoint2( const Ogre::Ray& A,Ogre::Real t );
	/*计算一个射线与球面的交点的比较近的那个
	*/
	std::vector<Ogre::Real> CalcRaySpherePoint( 
		const Ogre::Ray& A,const Ogre::Sphere& S );

	/*如果射线相交返回较为近的交点，如果不相交返回较近的点
		b = true表示取离A原点较近的点
	*/
	CalcResult CalcRaySphereNearFarPoint( 
		const Ogre::Ray& A,const Ogre::Sphere& S,bool b );

	//计算射线和三角形a b c的交点，包括在射线原点背面的那个。原始的Ogre::Math不包括背面的
	std::pair<bool, Ogre::Real> intersects(const Ogre::Ray& ray, const Ogre::Vector3& a,
        const Ogre::Vector3& b, const Ogre::Vector3& c,
        bool positiveSide, bool negativeSide);
}

#endif