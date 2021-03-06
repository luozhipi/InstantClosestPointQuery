#ifndef __READMESH_H
#define __READMESH_H

#include "readOBJ.h"
#include "MIS_inc.h"

inline void readMeshfile(string filename, PointMatrixType & vertices, FaceMatrixType & faces)
{
	size_t last_dot = filename.rfind('.');
	if(last_dot == std::string::npos)
		printf("Error: No file extension found in %s.\n",filename);

	string extension = filename.substr(last_dot+1);
	if(extension == "obj" || extension =="OBJ") {
		if(!igl::readOBJ(filename, vertices, faces))
			printf("Error: Unreadable mesh file.\n");
	}
	else {
		printf("Error: %s is not a recognized file type.\n",extension.c_str());
	}
}

#endif