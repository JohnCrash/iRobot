#include "stdheader.h"
#include <math.h>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include "Math3d.h"

namespace Math3d
{
	/*一个简单的构造函数，将一个直线用两个线性方程表示
	*/
	static void Ray2Plane( const Ogre::Ray& ray,
					int i,int j,
				   boost::numeric::ublas::matrix<Ogre::Real>& m,
				   boost::numeric::ublas::vector<Ogre::Real>& v ){
		Ogre::Vector3 ao =ray.getOrigin();
		Ogre::Vector3 ad = ray.getDirection();
		//这么写是为了防止如下面的方向(0,0,1)或者(1,0,0)等
		//这样将导致0=0的方程
		if( std::abs(ad.x) > abs(ad.y) && std::abs(ad.x) > std::abs(ad.z) ){
			m(i,j) = ad.y; m(i,j+1) = -ad.x; v(i) = ao.x*ad.y - ao.y*ad.x;
			m(i+1,j) = ad.z; m(i+1,j+2) = -ad.x; v(i+1) = ao.x*ad.z - ao.z*ad.x;
		}else if( std::abs(ad.y) > abs(ad.x) && std::abs(ad.y) > std::abs(ad.z) ){
			m(i,j) = ad.y; m(i,j+1) = -ad.x; v(i) = ao.x*ad.y - ao.y*ad.x;
			m(i+1,j+1) = ad.z; m(i+1,j+2) = -ad.y; v(i+1) = ao.y*ad.z - ao.z*ad.y;
		}else{
			m(i,j+1) = ad.z; m(i,j+2) = -ad.y; v(i) = ao.y*ad.z - ao.z*ad.y;
			m(i+1,j) = ad.z; m(i+1,j+2) = -ad.x; v(i+1) = ao.x*ad.z - ao.z*ad.x;
		}
	}

	Ogre::Vector3 RayPoint( const Ogre::Ray& A,Ogre::Real t ){
		Ogre::Vector3 a = A.getDirection();
		a.normalise();
		return (t*a + A.getOrigin());
	}

	Ogre::Vector3 RayPoint2( const Ogre::Ray& A,Ogre::Real t ){
		Ogre::Vector3 a = A.getDirection();
		return (t*a + A.getOrigin());
	}
	
	Ogre::Real RayParameter( const Ogre::Ray& A,const Ogre::Vector3& v ){
		Ogre::Vector3 o = A.getOrigin();
		Ogre::Vector3 p = v - o;
		if( A.getDirection().dotProduct(p) > 0 )
			return p.length();
		else
			return -p.length();
	}

	//直线A与直线B，返回A上到B最近的点
	CalcResult
	CalcNearPoint( const Ogre::Ray& A,const Ogre::Ray& B ){
		using namespace boost::numeric::ublas;
		matrix<Ogre::Real> m(6,6);
		boost::numeric::ublas::vector<Ogre::Real> v(6);
		permutation_matrix<size_t> P(6); 

		for( int i = 0;i < 6;++i ){
			for( int j = 0;j < 6;++j )
				m(i,j)=0;
		}
		//将第一条直线方程放入矩阵
		Ray2Plane( A,0,0,m,v );

		//将第二条直线方程放入矩阵
		Ray2Plane( B,2,3,m,v );

		Ogre::Vector3 ao =A.getOrigin();
		Ogre::Vector3 ad = A.getDirection();
		Ogre::Vector3 bo =B.getOrigin();
		Ogre::Vector3 bd = B.getDirection();
		//矢量和A垂直同时和B垂直，且矢量的起点和终点在A和B上。使用点积有两个方程
		m(4,0) = ad.x; m(4,1) = ad.y; m(4,2) = ad.z; m(4,3) = -ad.x; m(4,4) = -ad.y; m(4,5) = -ad.z; v[4] = 0;
		m(5,0) = bd.x; m(5,1) = bd.y; m(5,2) = bd.z; m(5,3) = -bd.x; m(5,4) = -bd.y; m(5,5) = -bd.z; v[5] = 0;

		try{
			//解由m v构成的线性方程组
			lu_factorize(m,P);
			lu_substitute(m,P,v);
			return CalcResult(true,Ogre::Vector3(v[0],v[1],v[2]));
		}catch( singular& ){
			//预料中的无解
		}catch( internal_logic& ){
			//?有待研究
		}
		return CalcResult(true,Ogre::Vector3());
	}

	//该代码从Ogre::Math::intersects拷贝而来
	std::vector<Ogre::Real> CalcRaySpherePoint(
		const Ogre::Ray& ray,const Ogre::Sphere& sphere ){
		using namespace Ogre;
		std::vector<Ogre::Real> result;

		const Vector3& raydir = ray.getDirection();
		// Adjust ray origin relative to sphere center
		const Vector3& rayorig = ray.getOrigin() - sphere.getCenter();
		Real radius = sphere.getRadius();

		// Check origin inside first
		if (rayorig.squaredLength() <= radius*radius )
		{
			return result;
		}

		// Mmm, quadratics
		// Build coeffs which can be used with std quadratic solver
		// ie t = (-b +/- sqrt(b*b + 4ac)) / 2a
		Real a = raydir.dotProduct(raydir);
		Real b = 2 * rayorig.dotProduct(raydir);
		Real c = rayorig.dotProduct(rayorig) - radius*radius;

		// Calc determinant
		Real d = (b*b) - (4 * a * c);
		if (d < 0)
		{
			// No intersection
			return result;
		}
		else
		{
			// BTW, if d=0 there is one intersection, if d > 0 there are 2
			// But we only want the closest one, so that's ok, just use the 
			// '-' version of the solver
			Real t1 = ( -b - Math::Sqrt(d) ) / (2 * a);
			Real t2 = ( -b + Math::Sqrt(d) ) / (2 * a);
			
			result.push_back( min(t1,t2) );
			result.push_back( max(t1,t2) );
			return result;
		}
	}

	//b表示远近,true表示近点离射线原点,false远点
	CalcResult CalcRaySphereNearFarPoint(
		const Ogre::Ray& ray,const Ogre::Sphere& sphere,bool b ){
	/*返回射线距离球心最近点和球的面的交点
	*/
		using namespace boost::numeric::ublas;

		std::vector<Ogre::Real> result = CalcRaySpherePoint( ray,sphere );
		if( !result.empty() ){
			if( b )
				return CalcResult(true,RayPoint( ray,result[0]));
			else
				return CalcResult(true,RayPoint( ray,result[1]));
		}
		//如果没有交点求一个球面上离射线的最近点
		//这等价与求一个线性方程
		matrix<Ogre::Real> m(3,3);
		boost::numeric::ublas::vector<Ogre::Real> v(3);
		permutation_matrix<size_t> P(3); 
		//将矩阵的元素都设置为0
		for( int i = 0;i < 3;++i ){
			for( int j = 0;j < 3;++j )
				m(i,j)=0;
		}
		//构造射线的两个平面方程
		Ogre::Vector3 n = ray.getDirection();
		Ogre::Vector3 o = sphere.getCenter();
		Ray2Plane( ray,0,0,m,v );
		//特别注意这里的v，是在方程的右边的向量
		//a*x + b*y + c*z = v
		m(2,0) = n.x; m(2,1) = n.y; m(2,2) = n.z; v(2) = n.dotProduct(o);
		try{
			//解由m v构成的线性方程组
			lu_factorize(m,P);
			lu_substitute(m,P,v);
			Ogre::Vector3 p(v[0],v[1],v[2]);
			Ogre::Vector3 l = p-o;
			l.normalise();
			//线性差值点
			return CalcResult(true,o+l*sphere.getRadius() );
		}catch( singular& ){
			//预料中的无解
		}catch( internal_logic& ){
			//?有待研究
		}
		return CalcResult(false,Ogre::Vector3());
	}

	//计算射线A和平面P的交点
	CalcResult CalcRayPlanePoint( 
		const Ogre::Ray& A,const Ogre::Plane& plane
		){
		using namespace boost::numeric::ublas;
		//这等价与求一个线性方程
		matrix<Ogre::Real> m(3,3);
		boost::numeric::ublas::vector<Ogre::Real> v(3);
		permutation_matrix<size_t> P(3); 
		//将矩阵的元素都设置为0
		for( int i = 0;i < 3;++i ){
			for( int j = 0;j < 3;++j )
				m(i,j)=0;
		}
		//构造射线的两个平面方程
		Ray2Plane( A,0,0,m,v );
		m(2,0) = plane.normal.x; m(2,1) = plane.normal.y; m(2,2) = plane.normal.z;
		//特别注意这里的v，是在方程的右边的向量
		//a*x + b*y + c*z = v
		v(2) = -plane.d;
		try{
			//解由m v构成的线性方程组
			lu_factorize(m,P);
			lu_substitute(m,P,v);
			return CalcResult(true,Ogre::Vector3(v[0],v[1],v[2]));
		}catch( singular& ){
			//预料中的无解
		}catch( internal_logic& ){
			//?有待研究
		}
		return CalcResult(false,Ogre::Vector3());
	}

	/*代码来只有Ogre::Math求射线和三角形的交点
	*/
	std::pair<bool, Ogre::Real> intersects(const Ogre::Ray& ray, const Ogre::Vector3& a,
        const Ogre::Vector3& b, const Ogre::Vector3& c, const Ogre::Vector3& normal,
        bool positiveSide, bool negativeSide)
    {
		using namespace Ogre;
        //
        // Calculate intersection with plane.
        //
        Real t;
        {
            Real denom = normal.dotProduct(ray.getDirection());

            // Check intersect side
            if (denom > + std::numeric_limits<Real>::epsilon())
            {
                if (!negativeSide)
                    return std::pair<bool, Real>(false, 0);
            }
            else if (denom < - std::numeric_limits<Real>::epsilon())
            {
                if (!positiveSide)
                    return std::pair<bool, Real>(false, 0);
            }
            else
            {
                // Parallel or triangle area is close to zero when
                // the plane normal not normalised.
                return std::pair<bool, Real>(false, 0);
            }

            t = normal.dotProduct(a - ray.getOrigin()) / denom;

			/*
			拷贝Ogre::Math修改的原因所在，我的代码要求的到背面的交点
            if (t < 0)
            {
                // Intersection is behind origin
               return std::pair<bool, Real>(false, 0);
            }
			*/
        }

        //
        // Calculate the largest area projection plane in X, Y or Z.
        //
        size_t i0, i1;
        {
            Real n0 = Math::Abs(normal[0]);
            Real n1 = Math::Abs(normal[1]);
            Real n2 = Math::Abs(normal[2]);

            i0 = 1; i1 = 2;
            if (n1 > n2)
            {
                if (n1 > n0) i0 = 0;
            }
            else
            {
                if (n2 > n0) i1 = 0;
            }
        }

        //
        // Check the intersection point is inside the triangle.
        //
        {
            Real u1 = b[i0] - a[i0];
            Real v1 = b[i1] - a[i1];
            Real u2 = c[i0] - a[i0];
            Real v2 = c[i1] - a[i1];
            Real u0 = t * ray.getDirection()[i0] + ray.getOrigin()[i0] - a[i0];
            Real v0 = t * ray.getDirection()[i1] + ray.getOrigin()[i1] - a[i1];

            Real alpha = u0 * v2 - u2 * v0;
            Real beta  = u1 * v0 - u0 * v1;
            Real area  = u1 * v2 - u2 * v1;

            // epsilon to avoid float precision error
            const Real EPSILON = 1e-6f;

            Real tolerance = - EPSILON * area;

            if (area > 0)
            {
                if (alpha < tolerance || beta < tolerance || alpha+beta > area-tolerance)
                    return std::pair<bool, Real>(false, 0);
            }
            else
            {
                if (alpha > tolerance || beta > tolerance || alpha+beta < area-tolerance)
                    return std::pair<bool, Real>(false, 0);
            }
        }

        return std::pair<bool, Real>(true, t);
    }
    
	//代码拷贝来自于Ogre::Math
	std::pair<bool, Ogre::Real> intersects(const Ogre::Ray& ray, const Ogre::Vector3& a,
        const Ogre::Vector3& b, const Ogre::Vector3& c,
        bool positiveSide, bool negativeSide)
    {
		using namespace Ogre;
		Vector3 normal = Ogre::Math::calculateBasicFaceNormalWithoutNormalize(a, b, c);
        return intersects(ray, a, b, c, normal, positiveSide, negativeSide);
    }
}