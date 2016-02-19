#ifndef __CONFIGURATION_H
#define __CONFIGURATION_H

#include "MIS_inc.h"

class Config {

private:
	string filename;
	string meshfile;
	Quaternion meshRotation;
public:
	Config(const string _meshfile);
	~Config();
	void saveConfig(const string _filename);
	string getConfigfile() const { 
		string configfile = string(PRINT_DIR).append(meshfile);
		removeExt(configfile);
		configfile.append(CONFIG_EXT);
		return configfile; }

	string getMeshName() const {return meshfile;}
	string getMeshfile() const { return string(MESH_DIR).append(meshfile); }
	const Quaternion & getMeshRotation() const { return meshRotation; }
	void setMeshRotation(const Quaternion & quat) {
		meshRotation = quat;
	}
};


#endif