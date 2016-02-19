#ifndef __MIS_PLUGIN_H
#define __MIS_PLUGIN_H
#include "PreviewPlugin.h"
#include "PluginManager.h"
#include "common.h"

enum MouseMode { NOTHING, TRANSLATE};
class MISPlugin : public PreviewPlugin
{
public:
	MISPlugin()
	{
		CreatePluginManager();
		PluginManager().register_plugin(this);
		cfg_meshfile = 0;
		resetVars();
	}

	void resetVars() {
		mouse_down = false;
		m_mouse_mode = NOTHING;
		maxDist = 1.0f;
	}

	~MISPlugin() {
	}
	void init(Preview3D *preview);
	void updateAfterConfigLoading();

	bool keyDownEvent(unsigned char key, int modifiers, int mouse_x, int mouse_y);
	
	bool keyUpEvent(unsigned char key, int modifiers, int mouse_x, int mouse_y);

	bool mouseDownEvent(int mouse_x, int mouse_y, int button, int modifiers);
	
	bool mouseUpEvent(int mouse_x, int mouse_y, int button, int modifiers);
	
	bool mouseMoveEvent(int mouse_x, int mouse_y);
	
	bool mouseScrollEvent(int mouse_x, int mouse_y, float delta);

	void get_translation(const RowVector3 & pos0, int mouse_x, int mouse_y, int from_x, int from_y, RowVector3 & translation);

	void preDraw(int currentTime);
	void postDraw(int currentTime);
	void resize(int w, int h);
	void resetCamera();
	void close();
	void takeScreenshot();

	void setOpenglLight();
	void setOpenglMatrices();
	void initOpenglMatricesForLight();
	void initOpenglMatricesForCamera(bool forceSquared = false);
	void initOpenglMatricesForWorld();

	string getMeshfile() const { return objFiles[cfg_meshfile]; }
	int cfg_meshfile;
	int log_nbF;
	int previousTime;
protected:

	MouseMode m_mouse_mode;
	bool mouse_down;
	RowVector3 old_query_point, query_point; 
	int from_x, from_y;
	ScalarType maxDist;
	RowVector3 translation;

	TwBar *mybar;
	TwBar *stats;
	Preview3D::KeyModifier m_key_modifier;
	float mesh_color[4];
	int m_viewport[4];
	double m_projMatrix[16];
	double m_projSquaredMatrix[16];
	double m_cameraViewMatrix[16];
	double m_lightViewMatrix[16];
	double m_modelViewMatrix[16];
	double m_fullViewMatrix[16];
	vector<string> objFiles;

	void drawPerText(const char * text, int len, int x, int y)
	{
		glMatrixMode(GL_PROJECTION); 
		 double matrix[16]; 
		 glGetDoublev(GL_PROJECTION_MATRIX, matrix); 
		 glLoadIdentity(); 
		 glOrtho(-300, 300, -300, 300, -5, 5); 
		 glMatrixMode(GL_MODELVIEW); 
		 glLoadIdentity();
		 glPushMatrix(); 
		 glLoadIdentity(); 
		 glRasterPos2i(x, y); 
		 for(int i=0; i<len; i++){
			 glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, (int)text[i]);
		 }
		 glPopMatrix(); 
		 glMatrixMode(GL_PROJECTION); 
		 glLoadMatrixd(matrix);
		 glMatrixMode(GL_MODELVIEW);
	}
	void drawText()
	{
		static int timeBase = 0;
		int t = glutGet(GLUT_ELAPSED_TIME);
		if (t - timeBase > 500) {
			timeBase = t;		
		}
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);
		glColor3f(1, 0, 0);
		stringstream statistics;
		statistics <<"#Triangles" << "  " << log_nbF;
		statistics<< "  query point  ("<<query_point[0]<<" "<<query_point[1]<<" "<<query_point[2]<<")";
		statistics<<" maxDist "<<maxDist;
		string str_statis = statistics.str();
		drawPerText(str_statis.data(), str_statis.size(), -250, 280);
		
		glColor3f(0, 1, 0);
		stringstream performance;
		performance <<"query time  "<<MIS::time_query;
		if(MIS::do_closest())
		{
			performance<<"  closest point ("<< MIS::closest_point[0]<<" "<<MIS::closest_point[1]<<" "<<MIS::closest_point[2]<<")";
			performance<<" closestDist "<<MIS::closestDist;
		}
		else
			performance<<"  NO closest point";
		string str_performance = performance.str();
		drawPerText(str_performance.data(), str_performance.size(), -250, 250);
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
	}
};

#endif
