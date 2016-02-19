#include "plugin.h"
#include <windows.h>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <map>
#include "Mesh.h"
#include "config.h"
#include "trackball.h"
#include "rendering_functions.h"
#include "timer.h"
#include "SOIL.h"

MISPlugin MISPluginInstance;
void TW_CALL SetMeshFileCB(const void *value, void *clientData)
{
	MISPluginInstance.cfg_meshfile = *(const int *)value;
	MISPluginInstance.close();
	MIS::m_config = new Config(MISPluginInstance.getMeshfile());
	MIS::m_mesh = new Mesh(MIS::m_config->getMeshfile());
	MISPluginInstance.updateAfterConfigLoading();
}

void TW_CALL GetMeshFileCB(void *value, void *clientData)
{ 
	*(int *)value = MISPluginInstance.cfg_meshfile;
}

void TW_CALL SetCornerThresCB(const void *value, void *clientData)
{ 
	if(!MIS::isConfigLoaded()) return;

	MIS::m_mesh->corner_threshold = *(const ScalarType *)value;
	MIS::m_mesh->ComputeCornerNormals();
}

void TW_CALL GetCornerThresCB(void *value, void *clientData)
{ 
	if(MIS::isConfigLoaded())
		*(ScalarType *)value = MIS::m_mesh->corner_threshold;
	else
		*(ScalarType *)value = 20.0;
}

void TW_CALL CopyStdStringToClient(std::string& destinationClientString, const std::string& sourceLibraryString)
{
  destinationClientString = sourceLibraryString;
}

void MISPlugin::init(Preview3D *preview)
{
	cout << "LOADING MIS PLUGIN..." << endl;
	findFiles(MESH_DIR,OBJ_EXT,objFiles);
	PreviewPlugin::init(preview);
	mybar = TwNewBar("ClosestPoint");
	TwDefine(" ClosestPoint size='280 200' text=light alpha=255 color='40 40 40' position='3 3'");
	TwCopyStdStringToClientFunc(CopyStdStringToClient);

	std::stringstream ssEnumMeshFile;
	ssEnumMeshFile << objFiles[0];
	for(unsigned int i = 1 ; i < objFiles.size() ; ++i)
		ssEnumMeshFile << "," << objFiles[i];
	TwType meshFileType = TwDefineEnumFromString("MeshFileType", ssEnumMeshFile.str().c_str());
    TwAddVarCB(mybar, "meshFile", meshFileType, SetMeshFileCB, GetMeshFileCB, this, " visible=true label='mesh file :' ");	
	TwAddVarRW(mybar, "maxDist", TW_TYPE_SCALARTYPE, &maxDist, "visible=true min=0.0 max=100.0 step=0.1 ");

	mesh_color[0] = 158.0/255.0;
	mesh_color[1] = 189.0/255.0;
	mesh_color[2] = 248.0/255.0;
	mesh_color[3] = 74.0/255.0;
	TwAddVarRW(m_preview->bar, "mesh color", TW_TYPE_COLOR4F, mesh_color, " group='Transparency'");
	TwAddVarCB(m_preview->bar, "Corner threshold", TW_TYPE_SCALARTYPE, SetCornerThresCB, GetCornerThresCB, this, " min=0 max=180 step=2 ");

	cout << "\t [done]" << endl;
}

void MISPlugin::updateAfterConfigLoading() {
	log_nbF = MIS::m_mesh->nbF;
	get_scale_and_shift_to_fit_mesh(MIS::m_mesh->vertices,m_preview->zoom,m_preview->mesh_center);
	m_preview->radius = 1/m_preview->zoom;
	m_preview->shift = RowVector3(0,-0.1,0);
	old_query_point = MIS::m_mesh->V(0);
	MIS::initialize();
	resetCamera();
}

bool MISPlugin::keyDownEvent(unsigned char key, int modifiers, int mouse_x, int mouse_y) {

	if(key == 2) {resetCamera();}
	if(!MIS::isConfigLoaded()) return false;	
	if(key == 100) {
		m_preview->g_RotationAngle -= 0.1;
	}
	if(key == 102) {
		m_preview->g_RotationAngle += 0.1;
	}
	if(MIS::isConfigLoaded())
	{
		if( modifiers == (Preview3D::ALT) ) {
			translation = RowVector3::Zero();
			if(key == 't')
			{
				m_mouse_mode = TRANSLATE;
				return true;
			}
			}
	}
	if(key=='p')
		takeScreenshot();
	if(key=='0')
	{
		MIS::doQuery = false;
		m_mouse_mode = NOTHING;
	}
	return false;
}
	
bool MISPlugin::keyUpEvent(unsigned char key, int modifiers, int mouse_x, int mouse_y) {
	return false;
}

bool MISPlugin::mouseDownEvent(int mouse_x, int mouse_y, int button, int modifiers) { 
	if(!MIS::isConfigLoaded()) return false;
	mouse_down = true;
	from_x = mouse_x;
	from_y = mouse_y;
	return false;
}
	
bool MISPlugin::mouseUpEvent(int mouse_x, int mouse_y, int button, int modifiers) {
	if(!MIS::isConfigLoaded()) return false;
	mouse_down = false;
	MIS::doQuery = false;
	translation = RowVector3::Zero();
	old_query_point = query_point;
	return false;
};
	
bool MISPlugin::mouseMoveEvent(int mouse_x, int mouse_y) {
	if(!MIS::isConfigLoaded()) return false;
	if(mouse_down) {
		if(m_mouse_mode == TRANSLATE)
		{
			get_translation(old_query_point,
				mouse_x,mouse_y, from_x, from_y, translation);
			query_point = old_query_point + translation;
			MIS::doQuery = true;
			return true;
		}
		else
			return false;
	}
	if(m_mouse_mode == TRANSLATE)
		return true;
	else
		return false;
}
	
bool MISPlugin::mouseScrollEvent(int mouse_x, int mouse_y, float delta) {
	return false;
}

	const static RowVector4 green(0,1,0,1);
	const static RowVector4 red(1,0,0,1);
	const static RowVector4 blue(0,0,1,1);
	const static RowVector4 yellow(1,1,0,1);
	const static RowVector4 white(1,1,1,1);
	const static RowVector4 black(0,0,0,1);

	void MISPlugin::setOpenglLight() {
		float v[4]; 
		glEnable(GL_LIGHT0);
		v[0] = v[1] = v[2] = m_preview->g_LightMultiplier*0.4f; v[3] = 1.0f;
		glLightfv(GL_LIGHT0, GL_AMBIENT, v);
		v[0] = v[1] = v[2] = m_preview->g_LightMultiplier*1.0f; v[3] = 1.0f;
		glLightfv(GL_LIGHT0, GL_DIFFUSE, v);	
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMultMatrixd(m_cameraViewMatrix);
		v[0] = - m_preview->g_LightDistance*m_preview->g_LightDirection[0];
		v[1] = - m_preview->g_LightDistance*m_preview->g_LightDirection[1];
		v[2] = - m_preview->g_LightDistance*m_preview->g_LightDirection[2];
		v[3] = 1.0f;
		glLightfv(GL_LIGHT0, GL_POSITION, v);
	}

	void MISPlugin::setOpenglMatrices() {		
		double mat[16];
		glViewport(300, 0, m_preview->width-300, m_preview->height);
		glGetIntegerv(GL_VIEWPORT, m_viewport);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		double fH = tan( m_preview->view_angle / 360.0 * M_PI ) * m_preview->dnear;
		double fW = fH * (double)(m_preview->width-300)/(double)m_preview->height;
		glFrustum( -fW, fW, -fH, fH, m_preview->dnear, m_preview->dfar);
		glGetDoublev(GL_PROJECTION_MATRIX, m_projMatrix);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glFrustum( -fH, fH, -fH, fH, m_preview->dnear, m_preview->dfar);
		glGetDoublev(GL_PROJECTION_MATRIX, m_projSquaredMatrix);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(m_preview->eye[0], m_preview->eye[1], m_preview->eye[2], m_preview->center[0], m_preview->center[1], m_preview->center[2], m_preview->up[0], m_preview->up[1], m_preview->up[2]);	
		glScaled(m_preview->g_Zoom, m_preview->g_Zoom, m_preview->g_Zoom);
		glScaled(m_preview->zoom, m_preview->zoom, m_preview->zoom);	
		glTranslated(m_preview->shift[0],m_preview->shift[1],m_preview->shift[2]);
		QuaternionToMatrix44(m_preview->g_Rotation,mat);
		glMultMatrixd(mat);
		glGetDoublev(GL_MODELVIEW_MATRIX, m_cameraViewMatrix);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		Quaternion q0 = MIS::m_config->getMeshRotation();
		QuaternionToMatrix44(q0,mat);
		glMultMatrixd(mat);
		Matrix33 mat2 = q0.conjugate().toRotationMatrix();
		RowVector3 g = (mat2 * Vector3(0,-1,0));	
		Quaternion q(AngleAxis(m_preview->g_RotationAngle,g.normalized()));
		QuaternionToMatrix44(q,mat);
		glMultMatrixd(mat);
		glTranslated(-m_preview->mesh_center[0],-m_preview->mesh_center[1],-m_preview->mesh_center[2]);
		glGetDoublev(GL_MODELVIEW_MATRIX, m_modelViewMatrix);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		RowVector3 l_LookAt = m_preview->mesh_center;
		RowVector3 l_Pos = l_LookAt - m_preview->g_LightDistance*RowVector3(m_preview->g_LightDirection[0],m_preview->g_LightDirection[1],m_preview->g_LightDirection[2]);
		gluLookAt(l_Pos[0], l_Pos[1], l_Pos[2], l_LookAt[0], l_LookAt[1], l_LookAt[2], 1, 0, 0);
		glGetDoublev(GL_MODELVIEW_MATRIX, m_lightViewMatrix);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMultMatrixd(m_cameraViewMatrix);
		glMultMatrixd(m_modelViewMatrix);
		glGetDoublev(GL_MODELVIEW_MATRIX, m_fullViewMatrix);
	}

	void MISPlugin::initOpenglMatricesForLight() {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMultMatrixd(m_projMatrix);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMultMatrixd(m_lightViewMatrix);
		glMultMatrixd(m_modelViewMatrix);
	}

	void MISPlugin::initOpenglMatricesForCamera(bool forceSquared) {
		glViewport(300,0,m_preview->width-300,m_preview->height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		if(forceSquared) {
			glMultMatrixd(m_projSquaredMatrix);
		}
		else {
			glMultMatrixd(m_projMatrix);
		}
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMultMatrixd(m_fullViewMatrix);
	}

	void MISPlugin::initOpenglMatricesForWorld() {
		glViewport(300,0,m_preview->width-300,m_preview->height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMultMatrixd(m_projMatrix);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMultMatrixd(m_cameraViewMatrix);
	}

	void MISPlugin::resize(int w, int h) {}

	void MISPlugin::preDraw(int currentTime) { 
		if(!MIS::isConfigLoaded()) return;
		if(MIS::doQuery) 
		{
			MIS::query_closest_point_on_mesh(query_point, maxDist);
		}
		previousTime = currentTime;
		setOpenglMatrices();
		initOpenglMatricesForWorld();
		glGetIntegerv(GL_VIEWPORT, m_preview->m_viewport);
		glGetDoublev(GL_PROJECTION_MATRIX, m_preview->m_projection_matrix);
		glGetDoublev(GL_MODELVIEW_MATRIX, m_preview->m_modelview_matrix);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_preview->g_MatAmbient);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_preview->g_MatDiffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_preview->g_MatSpecular);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m_preview->g_MatShininess);
		return;
	}


	void MISPlugin::postDraw(int currentTime) {
		if(!MIS::isConfigLoaded()) return;
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_NORMALIZE);
		glEnable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
		glDisable(GL_COLOR_MATERIAL);
		glDepthMask( GL_TRUE );
		glEnable(GL_TEXTURE_2D);
		glPushAttrib(GL_ENABLE_BIT);	
		setOpenglLight();
		initOpenglMatricesForCamera();
		glUseProgram(m_preview->shader_id);
		glUseProgram(0);
		glEnable(GL_LIGHTING);
		glUseProgram(m_preview->shader_id);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_preview->g_MatAmbient);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_preview->g_MatDiffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_preview->g_MatSpecular);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m_preview->g_MatShininess);
		glColor4f(mesh_color[0],mesh_color[1],mesh_color[2],mesh_color[3]);
		MIS::m_mesh->render();
		glUseProgram(0);
		glUseProgram(m_preview->shader_id);
		if(MIS::do_closest())
		{
			const RowVector3 & a = MIS::closest_point;
			paintPoint(a, 0.01, RowVector4(1.0f, 0.0f, 0.0f,1.0f));
			paintPoint(query_point, 0.01, RowVector4(0.0f, 1.0f, 0.0f,1.0f));
			paintArrow(query_point, a, 0.006, RowVector4(0.5f, 0.5f, 0.5f,1.0f));
		}
		glUseProgram(0);
		drawText();
		glPopAttrib();
		return;
	}
	
	void MISPlugin::resetCamera() {
		m_preview->g_Rotation.setIdentity();
	}

void MISPlugin::close() {
	if(!MIS::isConfigLoaded()) return;
	TwDefine(" ClosestPoint/meshFile readwrite ");;
	resetVars();
	MIS::uninitialize();
}

void MISPlugin::get_translation(const RowVector3 & pos0, int mouse_x, int mouse_y, int from_x, int from_y, RowVector3 & translation)
{
	GLdouble winX, winY, winZ;
	gluProject(pos0[0],pos0[1],pos0[2],m_fullViewMatrix,m_projMatrix,m_viewport, &winX, &winY, &winZ);

	double x,y,z;
	RowVector3 p1, p2;
	winX = from_x;
	winY = m_viewport[3] - from_y;
	gluUnProject(winX, winY, winZ, m_fullViewMatrix,m_projMatrix,m_viewport, &x,&y,&z);
	p1 << x,y,z;
	winX = mouse_x;
	winY = m_viewport[3] - mouse_y;
	gluUnProject(winX, winY, winZ, m_fullViewMatrix,m_projMatrix,m_viewport, &x,&y,&z);
	p2 << x,y,z;

	translation = p2-p1;
}
void MISPlugin::takeScreenshot()
{
	static int imageID = 0;
	std::stringstream s;
	s  << "screenshot/"<<imageID << ".bmp";
	SOIL_save_screenshot
	(
		s.str().c_str(),
		SOIL_SAVE_TYPE_BMP,
		300, 0, glutGet(GLUT_WINDOW_WIDTH)-300, glutGet(GLUT_WINDOW_HEIGHT)
	);
	cout <<"saving a screenshot ... "<<"screenshot/"<<imageID<<".bmp"<<endl;
	imageID++;
}
