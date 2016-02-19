#include "Mesh.h"
#include "config.h"
#include "readMesh.h"

	Mesh::Mesh(string filename) {
		readMeshfile(filename,vertices,faces);
		nbV = vertices.rows();
		nbF = faces.rows();
		InitNeighboringData();
		ComputeBoundingBox();
		corner_threshold = 64;
		ComputeNormals();
	}

	Mesh::~Mesh() {
	}

	void Mesh::InitNeighboringData() {
		vertex_to_faces.clear();
		vertex_to_faces.resize(nbV);
		vertex_to_vertices.clear();
		vertex_to_vertices.resize(nbV);
		for(int fi = 0; fi < nbF; fi++) {
			for(IndexType j = 0 ; j < 3 ; ++j) { 
				vertex_to_faces[F(fi,j)].push_back(fi);
				IndexType v0 = F(fi,j);
				IndexType v1 = F(fi,(j+1)%3);
				IndexType v2 = F(fi,(j+2)%3);
				if(!inTheList(vertex_to_vertices[v0],v1)) vertex_to_vertices[v0].push_back(v1);
				if(!inTheList(vertex_to_vertices[v0],v2)) vertex_to_vertices[v0].push_back(v2);
			}
		}
		face_to_faces.setConstant(nbF,3,INDEX_NULL);
		for(int fi = 0 ; fi < nbF ; ++fi) {
			for(int i = 0 ; i < 3 ; ++i) {
				IndexType v0 = faces(fi,i);
				IndexType v1 = faces(fi,(i+1)%3);
				for(unsigned int j = 0 ; j < vertex_to_faces[v0].size() ; ++j) {
					IndexType fj = vertex_to_faces[v0][j];
					if(fj != fi && inTheList(vertex_to_faces[v1],fj)) {
						face_to_faces(fi,i) = fj;
						break;
					}
				}
			}
		}
	}
	void Mesh::ComputeNormals() {
		FNormals.setZero(nbF,3);
		#pragma omp parallel for
		for(int fi = 0; fi < nbF; ++fi) {
			const RowVector3 & p0 = V(faces(fi,0));
			const RowVector3 & p1 = V(faces(fi,1));
			const RowVector3 & p2 = V(faces(fi,2));		
			FNormals.row(fi) = ((p1-p0).cross(p2-p0)).normalized();
		}
		VNormals.setZero(nbV,3);
		#pragma omp parallel for
		for(int vi = 0; vi < nbV; ++vi) {
			RowVector3 n = RowVector3::Zero();
			for(IndexType j = 0; j < vfNbNeighbors(vi); ++j) {
				n += FNormals.row(vfNeighbor(vi,j));
			}
			VNormals.row(vi) = n.normalized();
		}
		ComputeCornerNormals();
	}
	void Mesh::ComputeCornerNormals() {
		double t = cos(clamp(corner_threshold,0.0,180.0)*M_PI/180.0);
		CNormals.setZero(nbF*3,3);
		#pragma omp parallel for
		for(int fi = 0 ; fi < nbF ; ++fi) {
			for(int j = 0 ; j < 3 ; ++j) {
				IndexType vj = faces(fi,j);
				RowVector3 n = FNormals.row(fi);
				for(int k = 0; k < vfNbNeighbors(vj); ++k) {
					IndexType fk = vfNeighbor(vj,k);
					if(fk != fi && FNormals.row(fi).dot(FNormals.row(fk)) >= t)
						n += FNormals.row(fk);
				}
				CNormals.row(3*fi+j) = n.normalized();
			}
		}
	}
	void Mesh::ComputeBoundingBox() {
		bmin = bmax = vertices.row(0);	
		for(int i=1; i < nbV; i++) {
			bmin = bmin.cwiseMin(vertices.row(i));
			bmax = bmax.cwiseMax(vertices.row(i));    
		}
		RowVector3 length = (bmax - bmin).cwiseInverse();
		ScalarType min_scale = 0.95*length.minCoeff();
		scale = 0.98*length.minCoeff();
		center = (bmin+bmax)/2.0;
		for(int i=0; i < nbV; i++) {
			vertices.row(i) = min_scale*(vertices.row(i)-center) + RowVector3(0.5,0.5,0.5);
		}
		bmin = bmax = vertices.row(0);	
		for(int i=1; i < nbV; i++) {
			bmin = bmin.cwiseMin(vertices.row(i));
			bmax = bmax.cwiseMax(vertices.row(i));    
		}
	}
	void Mesh::draw()
	{
		for(IndexType fi = 0; fi < nbF ; ++fi) {
			glBegin(GL_TRIANGLES);
			for(IndexType j = 0; j < 3; ++j)
			{
				IndexType vj = F(fi,j);
				const RowVector3 & p = V(vj);
				const RowVector3 & n = cN(fi,j);
				glNormal3f(n[0],n[1],n[2]);
				glVertexAttrib3f(1,n[0],n[1],n[2]); 
				glVertex3f(p[0],p[1],p[2]);
			}
			glEnd();
		}
	}
	void Mesh::render()
	{
		glLineWidth((GLfloat)1.0f);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0, 1.0);
		draw();
		glDisable(GL_POLYGON_OFFSET_FILL);
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
		glColor3f (0.0, 0.0, 0.0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		draw();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
	}