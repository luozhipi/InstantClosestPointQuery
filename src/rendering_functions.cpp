#include "rendering_functions.h"
#include "common.h"
#include "Mesh.h"
#include "utils_functions.h"

const static objModel * MyGLSphere = new objModel("resources/sphere.obj");
const static objModel * MyGLCylinder = new objModel("resources/cylinder.obj");
const static objModel * MyGLCone = new objModel("resources/cone.obj");

void objModel::draw() const {

	glBegin(GL_TRIANGLES);
	for(int fi = 0 ; fi < F.rows() ; ++fi) {
		for(int j = 0 ; j < 3 ; ++j) {
			glNormal3d(N(F(fi,j),0),N(F(fi,j),1),N(F(fi,j),2));
			glVertexAttrib3d(1,N(F(fi,j),0),N(F(fi,j),1),N(F(fi,j),2));
			glVertex3d(V(F(fi,j),0),V(F(fi,j),1),V(F(fi,j),2));
		}
	}
	glEnd();

}

	//paints an arrow between from and to with a given radius
	void paintArrow(const RowVector3 & from, const RowVector3 & to, const double radius, const RowVector4 & color)
	{
		double length = (to - from).norm();
		RowVector3 axis = (to-from).normalized();
		if(length > 0.03) {
			paintCylinder(from, to-0.03*axis, radius,color);
			paintCone(to-0.03*axis, to, 2.0*radius,color);
		}
	}

	//paints a cylinder with radius r between points a and b
	void paintCylinder(const RowVector3 & a, const RowVector3 & b, const double radius, const RowVector4 & color)
	{
		glPushMatrix();

		// This is the default direction for the cylinders to face in OpenGL
		RowVector3 z = RowVector3(0,0,1);        
		// Get diff between two points you want cylinder along
		RowVector3 p = (a - b);                              
		// Get CROSS product (the axis of rotation)
		RowVector3 t = z.cross(p); 

		// Get angle. LENGTH is magnitude of the vector
		double angle = 180 / M_PI * acos ((z.dot(p)) / p.norm());

		glTranslatef(b[0],b[1],b[2]);
		glRotated(angle,t[0],t[1],t[2]);
		glScaled(radius,radius,p.norm()/2);
		glTranslated(0,0,1);
		
		glColor4f(color[0],color[1],color[2],color[3]);
		MyGLCylinder->draw();

		glPopMatrix();
	}

	//paints a cone given its bottom and top points and its bottom radius
	void paintCone(const RowVector3 & bottom, const RowVector3 & top, const double radius, const RowVector4 & color)
	{
		glPushMatrix();

		// This is the default direction for the cylinders to face in OpenGL
		RowVector3 z = Vector3(0,0,1);        
		// Get diff between two points you want cylinder along
		RowVector3 p = (top - bottom);                              
		// Get CROSS product (the axis of rotation)
		RowVector3 t = z.cross(p); 

		// Get angle. LENGTH is magnitude of the vector
		double angle = 180 / M_PI * acos ((z.dot(p)) / p.norm());

		glTranslatef(bottom[0],bottom[1],bottom[2]);
		glRotated(angle,t[0],t[1],t[2]);
		glScaled(radius,radius,p.norm()/2);
		glTranslated(0,0,1);
		
		glColor4f(color[0],color[1],color[2],color[3]);
		MyGLCone->draw();

		glPopMatrix();
	}

void paintPoint(const RowVector3 & a, const double radius, const RowVector4 & color)
{
	glPushMatrix();
	glTranslated(a[0],a[1],a[2]);
	glScaled(radius,radius,radius);
	glColor4f(color[0],color[1],color[2],color[3]);
	MyGLSphere->draw();
	glPopMatrix();
}

void paintCircle()
{
	int nside = 100;
	const double pi2 = 3.14159265 * 2.0;
	glBegin (GL_LINE_LOOP);
	for (double i = 0; i < nside; i++) {
		glNormal3d (cos (i * pi2 / nside), sin (i * pi2 / nside), 0.0);
		glVertex3d (cos (i * pi2 / nside), sin (i * pi2 / nside), 0.0);
	}
	glEnd ();
}