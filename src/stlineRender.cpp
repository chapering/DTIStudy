// ----------------------------------------------------------------------------
// tubeRender.cpp : load geometry in the format of "tgdata" and render directly
// through the legacy openGL pipeline
//
// ----------- when being imposed with modeling transformations, operate
// in the local coordinate system.
//
// Creation : Feb. 6th 2011
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#include "GLcppmoth.h"
#include "GLoader.h"
#include "glrand.h"

using namespace std;

class CstlineRender: public CGLApp {
public:
	CstlineRender(int argc, char **argv) : CGLApp(argc, argv),
	m_bUseOrgColor(true),
    m_loader(),
	m_strfnsrc(""),
	m_strfndst("")	{
		setVerinfo("stlineRender");

		addOption('f', true, "input-file-name", "the name of source file"
			   " containing geometry and in the format of tgdata");
		addOption('g', true, "output-file-name", "the name of target file"
			   " to store the geometry parsed and loaded");
	}

	void genColors() {
		glrand::genFloatvector<GLfloat> (3, m_colors, 
				ARRAY_SIZE(m_colors), 0, 1);
		if ( m_cout.isswitchon() ) {
			cout << "colors: ";
			copy (m_colors, m_colors + ARRAY_SIZE(m_colors), 
					ostream_iterator<GLfloat> (cout, " "));
			cout << "\n";
		}
	}

	int loadGeometry() {
		if ( 0 != m_loader.load(m_strfnsrc) ) {
			m_cout << "Loading geometry failed - GLApp aborted abnormally.\n";
			return -1;
		}
		return 0;
	}

	void glInit(void) {
		CGLApp::glInit();

		// dis/en -abling primitive arrays
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glEnableClientState( GL_NORMAL_ARRAY );
		glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

		// force normalization when normals are designated
		glEnable(GL_NORMALIZE);
		glEnable(GL_RESCALE_NORMAL);
	}

	int handleOptions(int optv) {
		switch( optv ) {
			case 'f':
				m_strfnsrc = optarg;
				return 0;
			case 'g':
				m_strfndst = optarg;
				return 0;
			default:
				return CGLApp::handleOptions( optv );
		}
		return 1;
	}

	void keyResponse(unsigned char key, int x, int y) {
		switch (key) {
			case 'c': // randomly seeding vertices
				m_bUseOrgColor = ! m_bUseOrgColor;
				break;
			case 'g': // generate different colors
				if (!m_bUseOrgColor) {
					genColors();
					glColor3fv( m_colors );
				}
				break;
			default:
				return CGLApp::keyResponse(key, x, y);
		}
		glutPostRedisplay();
	}

	int mainstay() {
		// geometry must be loaded successfully in the first place in order the
		// openGL pipeline could be launched.
		if ( 0 != loadGeometry() ) {
			return -1;
		}

		// serialize the loaded geometry if asked for
		if ( m_strfndst.length() >= 1 ) {
			m_loader.dump ( m_strfndst );	
		}

		setPrjInfo(60.0f, 
				ABS(m_loader.getMinCoord(2))/4.0,
				(ABS(m_loader.getMaxCoord(2)) + ABS(m_loader.getMinCoord(2)))*4);

		setViewInfo(0.0, 0.0,
				( m_loader.getMaxCoord(2) + m_loader.getMinCoord(2) )*1.5,
				0.0, 0.0, 0.0,
				0, 1, 0);
		
		setGlutInfo(" DTI fiber streamlines ");

		return	CGLApp::mainstay();
	}

	void display(void) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glPushMatrix();

		// move the local coordinate system so that the default origin is still
		// located at the center of the object coordinate system
		glTranslatef( - (m_loader.getMaxCoord(0) + m_loader.getMinCoord(0))/2,
					  -	(m_loader.getMaxCoord(1) + m_loader.getMinCoord(1))/2,
					  -	(m_loader.getMaxCoord(2) + m_loader.getMinCoord(2))/2 );

		/* load polyline by means of vertex buffer one by one, assuming the
		 * loader has already got the vertices and colors all well-prepared
		 */
		unsigned long szTotal = m_loader.getSize();
		for (unsigned long idx = 0; idx < szTotal; ++idx) {
			vector<GLfloat> & curLine = m_loader.getElement( idx );
			
			if ( m_bUseOrgColor ) {
				glInterleavedArrays(GL_C3F_V3F, 0, &curLine[0]);
			}
			else {
				glInterleavedArrays(GL_V3F, 6*sizeof(GLfloat), &curLine[3]);
			}

			// using vertices as normals
			glNormalPointer( GL_FLOAT, 6*sizeof(GLfloat), &curLine[3]);

			glDrawArrays(GL_LINE_STRIP, 0, curLine.size()/6);
		}
		glPopMatrix();
		trigger();
	}

private:
	GLfloat m_colors[3];
	GLboolean m_bUseOrgColor;
	CTgdataLoader m_loader;

	string m_strfnsrc;
	string m_strfndst;
};

int main(int argc, char** argv)
{
	CstlineRender slr(argc, argv);
	slr.run();
	return 0;
}

/* set ts=4 sts=4 tw=80 sw=4 */

