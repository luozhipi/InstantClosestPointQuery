#ifndef __MIS_COMMON_H
#define __MIS_COMMON_H

#include "MIS_inc.h"
namespace MIS {
	extern Config * m_config;
	extern Mesh * m_mesh;
	extern ClosestPointQuery * m_cpquery;
	extern RowVector3 closest_point;
	extern bool doQuery;
	extern float time_query;
	extern RowVector3 closestpoint; 
	bool isConfigLoaded();
	bool do_closest();
	extern ScalarType closestDist;
	void initialize();
	void uninitialize();
	void query_closest_point_on_mesh(const RowVector3 & query_point, const ScalarType & maxDist);
};
#endif