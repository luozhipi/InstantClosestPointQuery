# InstantClosestPointQuery

`Description`: query closest point on mesh, accelerated by resorting to AABB tree. First build AABB tree of mesh, when seach the tree, a sphere
centered by query point and with radius of specified maximum search distance is defined, used to check the intersection between AABB of current tree node.
if intersected, then recursive search the child.

`Assumption`: single-side triangle mesh, created in couter-clockwise order, and right-hand coordinate system.

`Dev environment`: Visual studio 2010, OpenGL, C++, Eigen (for data structure)

`Usage`: press Alt + T to translate the query point, the program will instantly return the closest point on mesh. 

`Visualization`: green point is the query point; red point is the closest point. arrow line pointing from query to closest point.

`Core files`: src/ClosestPointQuery.h  src/ClosestPointQuery.cpp   src/closestpoint_utils

`Screenshot`



