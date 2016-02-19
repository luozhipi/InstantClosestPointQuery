#include "config.h"
#include "Mesh.h"

	Config::Config(const string _meshfile) {
		meshfile = _meshfile;
		meshRotation.w() =1.0f; meshRotation.x() =0.0f;meshRotation.y() =0.0f;meshRotation.z() =0.0f;
		string filename = getConfigfile();
		if(exists(filename)) {
			ifstream fin;
			fin.open(filename,std::ios::in);
			if(fin.fail()) {
				cout << "I/O error" << endl;
			}
			else {
				while(!fin.eof()) {
					string tag;
					fin >> tag;
					if(tag == "mesh_rotation") {
						fin >> meshRotation.w() >> meshRotation.x() >> meshRotation.y() >> meshRotation.z();
					}
				}
			}
			fin.close();  
		}
	}
	void Config::saveConfig(const string _filename) {
		ofstream fout;
		fout.open(_filename, std::ios::out);
		if(fout.fail()) {
		   cout << "I/O error" << endl;
		}
		else {
			fout << "mesh_rotation " << meshRotation.w() << " " << meshRotation.x() << " " << meshRotation.y() << " " << meshRotation.z() << endl;
		}
		fout.close();
	}
	Config::~Config() {}