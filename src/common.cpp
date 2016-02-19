#include "common.h"
#include "Mesh.h"
#include "config.h"
#include "ClosestPointQuery.h"
#include "timer.h"

namespace MIS {
	Config * m_config = 0;
	Mesh * m_mesh = 0;
	ClosestPointQuery * m_cpquery = 0;
	RowVector3 closest_point(0,0,0);
	ScalarType closestDist = 0.f;
	bool doQuery = false;
	float time_query=0.0;
	bool isConfigLoaded() { return m_config != 0; }
	void initialize()
	{
		closest_point = m_mesh->V(0);
		m_cpquery = new ClosestPointQuery(*m_mesh);
	}
	void query_closest_point_on_mesh(const RowVector3 & query_point, const ScalarType & maxDist)
	{
		igl::Timer timer = igl::Timer();
		timer.start();
		closest_point = (*m_cpquery)(query_point, maxDist);
		timer.stop();
		time_query = timer.getElapsedTimeInSec();
		closestDist = (query_point - closest_point).norm();
	}
	bool do_closest()
	{
		return m_cpquery->get_do_closest();
	}
	void uninitialize() {		
		if(m_mesh) { delete m_mesh; m_mesh = 0; }
		if(m_config) { delete m_config; m_config = 0; }
		if(m_cpquery){delete m_cpquery;}
	}
};