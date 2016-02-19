#ifndef __MIS_INCLUDE_H
#define __MIS_INCLUDE_H

#include "STL_inc.h"
#include "EIGEN_inc.h"
#include "GL_inc.h"
#include "timer.h"

#include <omp.h>


class ClosestPointQuery;
class Mesh;
class Config;

#undef min
#undef max
#include "utils_functions.h"
const static IndexType INDEX_NULL = 999999;
#define DEFAULT_FLATTEN_THRESHOLD 0.02
#define DEFAULT_ANGLE_OBJ 0.0

#define MESH_DIR "mesh/"
#define PRINT_DIR "print/"
#define MESH_EXT_LENGTH 4
#define OBJ_EXT ".obj"
#define CONFIG_EXT ".mis"

inline void removeExt(string & s) {
	s.erase(s.cend()-MESH_EXT_LENGTH,s.cend());
}
#endif