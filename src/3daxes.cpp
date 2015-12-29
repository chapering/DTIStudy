// ----------------------------------------------------------------------------
// 3daxes.cpp : a simple test of drawing a 3d CCS (Cartesian Coordinate System)
//
// Creation : Feb. 25th 2011
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#include "GLcppmoth.h"

using namespace std;

class CAxes: public CGLApp {
public:
	CAxes(int argc, char **argv) : CGLApp(argc, argv){
		setPrjInfo(45, 1.0, 10.0);
		setViewInfo(0, 0, 3.0);
		setGlutInfo(" a cartesian coordinate system ");
		setVerinfo("axes");
	}

	void onReshape(int w, int h) {
		CGLApp::onReshape(w, h);
	}

	void draw() {
		useCurrentColor();
		GLfloat origin[] = {0.0, 0.0, 0.0};

		glPushMatrix();
		glTranslatef(-0.5, -.5, -0.5);

		glBegin(GL_LINES);
			glVertex3fv(origin);
			glVertex3f(1.0, .0, .0);

			glVertex3fv(origin);
			glVertex3f(.0, 1.0, .0);

			glVertex3fv(origin);
			glVertex3f(.0, .0, 1.0);
		glEnd();
		glPopMatrix();
	}
};

int main(int argc, char** argv)
{
	CAxes axes(argc, argv);
	axes.run();
	return 0;
}

/* set ts=4 sts=4 tw=80 sw=4 */

