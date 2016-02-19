#ifndef __CLOSESTPOINTQUERY_H
#define __CLOSESTPOINTQUERY_H

#include "MIS_inc.h"

/// custom triangle type
struct My_triangle {
    RowVector3 m_pa;
    RowVector3 m_pb;
    RowVector3 m_pc;
	My_triangle(){};
    My_triangle(RowVector3 pa,
        RowVector3 pb,
        RowVector3 pc)
        : m_pa(pa), m_pb(pb), m_pc(pc) {}
};

//! custom bouding box
struct My_bbox {
    RowVector3 b_min;
    RowVector3 b_max;
	My_triangle triangle;
};

/**
 * @class AABB_node
 *
 *
 */
class AABB_node
{
public:
  /// Constructor
  AABB_node()
    : m_bbox()
    , m_p_left_child(NULL)
    , m_p_right_child(NULL)      { };

  /// Destructor
  /// Do not delete children because the tree hosts and delete them
  ~AABB_node() { };

  /// Returns the bounding box of the node
  const My_bbox& bbox() const { return m_bbox; }

  const AABB_node& left_child() const { return *static_cast<AABB_node*>(m_p_left_child); }
  const AABB_node& right_child() const { return *static_cast<AABB_node*>(m_p_right_child); }

  ///leaf node pointing to input triangle, so can get the triangle vertices.
  const My_triangle& left_data() const
                     { return *static_cast<My_triangle*>(m_p_left_child); }
  const My_triangle& right_data() const
                     { return *static_cast<My_triangle*>(m_p_right_child); }

  AABB_node& left_child() { return *static_cast<AABB_node*>(m_p_left_child); }
  AABB_node& right_child() { return *static_cast<AABB_node*>(m_p_right_child); }
  My_triangle& left_data() { return *static_cast<My_triangle*>(m_p_left_child); }
  My_triangle& right_data() { return *static_cast<My_triangle*>(m_p_right_child); }
  
  /**
   * @brief Builds the tree by recursive expansion.
   * @param first the first triangle to insert
   * @param last the last triangle to insert
   * @param range the number of triangle of the range
   *
   * [first,last[ is the range of primitives to be added to the tree.
   */
  void expand(vector<My_triangle>::iterator first,
              vector<My_triangle>::iterator beyond, size_t range);

  /**
   * @brief General traversal query point
   * @param query_point the query point
   * @param radius the radius of the sphere defined by maximum search distance
   * @param nb_triangles the number of triangles
   */
  RowVector3 traversal(const RowVector3 & query_point, const ScalarType& maxDist,
                 const std::size_t nb_triangles) const;

private:
  /// node bounding box
  My_bbox m_bbox;

  /// children nodes, either pointing towards children (if children are not leaves),
  /// or pointing toward input triangles (if children are leaves).
  void *m_p_left_child;
  void *m_p_right_child;

};  // end class AABB_node


/**
 * @class ClosestPointQuery
 *
 *
 */
class ClosestPointQuery
{

private:
	AABB_node * m_root_node;
	size_t nbF;
	vector<My_triangle> triangles;
public:

	ClosestPointQuery(const Mesh & m_mesh);
	//!return the closest point to mesh within the specified maximum search distance.
	RowVector3 operator()(const RowVector3 & queryPoint, ScalarType maxDist) const;

	//! whether find closest point on mesh within the specified maximum search distance.
	bool get_do_closest();

    const AABB_node* root_node() const {
      return m_root_node;
    }

}; // end class ClosestPointQuery

#endif