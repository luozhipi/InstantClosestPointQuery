#include "ClosestPointQuery.h"
#include "Mesh.h"
#include "closestpoint_utils.h"

RowVector3 closestpoint; ///closest point to mesh of query point.
ScalarType squaredMinDist; /// squared closest distance to mesh of query point.
bool do_closest;///if have closest point.

inline bool lessx(const My_triangle &t1, const My_triangle &t2)  
{
	return t1.m_pa[0]<t2.m_pa[0];  
} 
inline bool lessy(const My_triangle &t1, const My_triangle &t2)  
{  
	return t1.m_pa[1]<t2.m_pa[1];  
}
inline bool lessz(const My_triangle &t1, const My_triangle &t2)  
{  
	return t1.m_pa[2]<t2.m_pa[2];  
}

ClosestPointQuery::ClosestPointQuery(const Mesh & m_mesh)
{
	nbF = m_mesh.nbF;
	for(size_t i = 0; i<nbF; i++)
	{
		My_triangle triangle(m_mesh.V(i, 0), m_mesh.V(i, 1), m_mesh.V(i, 2));
		triangles.push_back(triangle);
	}
	m_root_node = new AABB_node[nbF-1]();
	if(m_root_node == NULL)
	{
		std::cerr << "Unable to allocate memory for AABB tree" << std::endl;
		triangles.clear();
		if( nbF > 1 ) {
			delete [] m_root_node;
		}
		m_root_node = NULL;
	}
	if(nbF>1)
		m_root_node->expand(triangles.begin(), triangles.end(), nbF);
}

RowVector3 ClosestPointQuery::operator()(const RowVector3 & query_point, ScalarType maxDist) const
{
	squaredMinDist = pow(maxDist, 2.0) + 2.0f;
	do_closest = false;
	closestpoint = RowVector3(0.f, 0.f, 0.f);
	switch(nbF)
	{
	case 0:
		do_closest = false;
		break;
	case 1:
		closestpoint = closestPointOnTriangle(triangles[0].m_pa, triangles[0].m_pb, triangles[0].m_pc, query_point);
		squaredMinDist = (closestpoint - query_point).squaredNorm();
		if(squaredMinDist < pow(maxDist, 2.0))
		{
			do_closest = true;
		}
		break;
	default:
		root_node()->traversal(query_point, maxDist, nbF);
	}
	///find a closest point within maxDist or not.
	do_closest = squaredMinDist <= pow(maxDist, 2.0);
	return closestpoint;
}

bool ClosestPointQuery::get_do_closest()
{
	return do_closest;
}

//! build the AABB tree
void AABB_node::expand(vector<My_triangle>::iterator first,
                      vector<My_triangle>::iterator beyond, size_t range)
{
	///compute the bounding box
	vector<RowVector3> triangles;
	for(vector<My_triangle>::iterator it = first; it!=beyond; it++)
	{
		triangles.push_back(it->m_pa); triangles.push_back(it->m_pb); triangles.push_back(it->m_pc);
	}
	RowVector3 bmin, bmax;
	compute_bbox(triangles, bmin, bmax);
	m_bbox.b_max = bmax; m_bbox.b_min = bmin; 
    ///sort triangles along longest AABB axis.
	ScalarType x = bmax[0] - bmin[0];
	ScalarType y = bmax[1] - bmin[1];
	ScalarType z = bmax[2] - bmin[2];
	IndexType sort_axis = max_three(x,y,z);
	vector<My_triangle>::iterator middle = first + (beyond - first)/2;
	switch(sort_axis)
	{
	case 0:
		std::nth_element(first, middle, beyond, lessx);
		break;
	case 1:
		std::nth_element(first, middle, beyond, lessy);
		break;
	case 2:
		std::nth_element(first, middle, beyond, lessz);
		break;
	default:
		std::cerr << "Unable to sort triangles along AABB" << std::endl;
	}
	switch(range)
	{
	case 2:
		m_p_left_child = &(*first);///pointing to input triangle.
		m_p_right_child = &(*(++first));
		break;
	case 3:
		m_p_left_child = &(*first);
		m_p_right_child = static_cast<AABB_node*>(this)+1; ///pointing to AABB_node.
		right_child().expand(first+1, beyond, 2);
		break;
	default:
		const size_t new_range = range / 2;
		m_p_left_child = static_cast<AABB_node*>(this) + 1;
		m_p_right_child = static_cast<AABB_node*>(this) + new_range;
		left_child().expand(first, first + new_range, new_range);
		right_child().expand(first + new_range, beyond, range - new_range);
	}
}

RowVector3 AABB_node::traversal(const RowVector3 & query_point, const ScalarType & maxDist,
                         const std::size_t nb_triangles) const
{
	// Recursive traversal
	RowVector3 tmp;
	switch(nb_triangles)
	{
	case 2:
		tmp = closestPointOnTriangle(left_data().m_pa, left_data().m_pb, left_data().m_pc, query_point);
		///closer point to mesh of query?
		///use squared distance, avoding sqrt().
		if(squaredMinDist >= (tmp - query_point).squaredNorm())
		{
			squaredMinDist = (tmp - query_point).squaredNorm();
			closestpoint = tmp;
		}
		else
		{
			tmp = closestPointOnTriangle(right_data().m_pa, right_data().m_pb, left_data().m_pc, query_point);
			if(squaredMinDist >= (tmp - query_point).squaredNorm())
			{
				squaredMinDist = (tmp - query_point).squaredNorm();
				closestpoint = tmp;
			}
		}
		break;
	case 3:
		tmp = closestPointOnTriangle(left_data().m_pa, left_data().m_pb, left_data().m_pc, query_point);
		if(squaredMinDist >= (tmp - query_point).squaredNorm())
		{
			squaredMinDist = (tmp - query_point).squaredNorm();
			closestpoint = tmp;
		}
		else
		{
			if(do_intersect(right_child().m_bbox.b_min, right_child().m_bbox.b_max, query_point, maxDist))
				right_child().traversal(query_point, maxDist, 2);
		}
		break;
	default:
		///first check whether the bounding box of child node intersect with the sphere which center is query point and radius is maxDist.
		///if so, recursive traversal of this child node.
		if(do_intersect(left_child().m_bbox.b_min, left_child().m_bbox.b_max, query_point, maxDist))
		{
			left_child().traversal(query_point, maxDist, nb_triangles/2);
			if(do_intersect(right_child().m_bbox.b_min, right_child().m_bbox.b_max, query_point, maxDist))
			{
				right_child().traversal(query_point, maxDist, nb_triangles - nb_triangles/2);
			}
		}
		else if(do_intersect(right_child().m_bbox.b_min, right_child().m_bbox.b_max, query_point, maxDist))
		{
			right_child().traversal(query_point, maxDist, nb_triangles - nb_triangles/2);
		}
	}
	return closestpoint;
}