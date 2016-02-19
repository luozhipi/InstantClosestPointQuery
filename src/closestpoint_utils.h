#ifndef __UTILS_CP_H
#define __UTILS_CP_H

#include "EIGEN_inc.h"
#include "utils_functions.h"
#include <math.h> 

//!find the closest point of query point [p] to mesh.
//!implementation of the method of "David Eberly's Distance Between Point and Triangle in 3D".
//! a triangle (v0, v1, v2) is created in a right-hand coordinate system, and in a counter-clockwise order.
inline RowVector3 closestPointOnTriangle(const RowVector3 & v0, const RowVector3 & v1,
	                                     const RowVector3 & v2, const RowVector3 & p)
{
	RowVector3 e0 = v1 - v0;
	RowVector3 e1 = v2 - v0;
	RowVector3 p0 = v0 - p;

	ScalarType a = e0.dot(e0);
	ScalarType b = e0.dot(e1);
	ScalarType c = e1.dot(e1);
	ScalarType d = e0.dot(p0);
	ScalarType e = e1.dot(p0);

	ScalarType det = a*c - pow(b,2.0);
	ScalarType s = b*e - c*d;
	ScalarType t = b*d - a*e;

	if((s+t) < det)
	{
		if(s<0.f)
		{
			if(t<0.f)
			{
				if(d<0.f)
				{
					s = clamp(-d/a, 0.f, 1.f);
					t = 0.f;
				}
				else
				{
					s = 0.f;
					t = clamp(-e/c, 0.f, 1.f);
				}
			}
			else
			{
				s = 0.f;
				t = clamp(-e/c, 0.f, 1.f);
			}
		}
		else if(t<0.f)
		{
			s = clamp(-d/a, 0.f, 1.f);
			t = 0.f;
		}
		else
		{
			ScalarType invDet = 1.f/det;
			s *= invDet;
			t *= invDet;
		}
	}
	else
	{
		if(s<0.f)
		{
			ScalarType tmp0 = b + d;
			ScalarType tmp1 = c + e;
			if(tmp1 > tmp0)
			{
				ScalarType numer = tmp1 - tmp0;
				ScalarType denom = a - 2*b + c;
				s = clamp(numer/denom, 0.f, 1.f);
				t = 1-s;
			}
			else
			{
				t = clamp(-e/c, 0.f, 1.f);
				s = 0.f;
			}
		}
		else if(t < 0.f)
		{
			if((a+d) > (b+e))
			{
				ScalarType numer = c + e - b - d;
				ScalarType denom = a - 2*b + c;
				s = clamp(numer/denom, 0.f, 1.f);
				t = 1-s;
			}
			else
			{
				s = clamp(-e/c, 0.f, 1.f);
				t = 0.f;
			}
		}
		else
		{
			ScalarType numer = c + e - b - d;
			ScalarType denom = a - 2*b + c;
			s = clamp(numer/denom, 0.f, 1.f);
			t = 1.f-s;
		}
	}
	return v0 + s*e0 + t*e1;
}
//!intersection of sphere with query point as center and maximum search distance as radius, and AABB.
inline bool do_intersect(const RowVector3 & b_min, const RowVector3 & b_max, 
	                     const RowVector3 & sph_center, const ScalarType & sph_radius)
{
	RowVector3 maxSphereAABB = RowVector3(b_min[0]>sph_center[0]?b_min[0]:sph_center[0],
		                                  b_min[1]>sph_center[1]?b_min[1]:sph_center[1],
										  b_min[2]>sph_center[2]?b_min[2]:sph_center[2]);
	RowVector3 closestPointInAABB = RowVector3(maxSphereAABB[0]<b_max[0]?maxSphereAABB[0]:b_max[0],
	                                           maxSphereAABB[1]<b_max[1]?maxSphereAABB[1]:b_max[1],
											   maxSphereAABB[2]<b_max[2]?maxSphereAABB[2]:b_max[2]);
	////!squared distance, avoiding sqrt()
	ScalarType dist = (sph_center - closestPointInAABB).squaredNorm();
	return dist<pow(sph_radius, 2.0);
}

//!compute the bounding box of a set of triangles.
inline void compute_bbox(const vector<RowVector3> & triangles, RowVector3 & bmin, RowVector3 & bmax)
{
	bmin = bmax = triangles[0];
	IndexType nbV = triangles.size();
	for(int i = 1; i < nbV; i++) {
		bmin = bmin.cwiseMin(triangles[i]);
		bmax = bmax.cwiseMax(triangles[i]);    
	}
}

//! find the max of three values.
inline IndexType max_three(ScalarType x, ScalarType y, ScalarType z)
{
	ScalarType max = x>y?x:y;
	max = max>z?max:z;
	if(max==x) return 0;
	if(max==y) return 1;
	if(max==z) return 2;
	return 0;
}

#endif
