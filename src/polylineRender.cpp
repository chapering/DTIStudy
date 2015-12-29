// ----------------------------------------------------------------------------
// tubeRender.cpp : load geometry in the format of "tgdata" and render directly
// through the legacy openGL pipeline
//
// ----------- when being imposed with modeling transformations, stay in the
// fixed grand coordinate system.
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

class CpolylineRender: public CGLApp {
public:
	CpolylineRender(int argc, char **argv) : CGLApp(argc, argv),
	m_bUseOrgColor(true),
    m_loader(),
	m_strfn("")	{
		setVerinfo("polylineRender");

		addOption('f', true, "input-file-name", "the name of source file"
			   " containing geometry and in the format of tgdata");
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

	void glInit(void) {
		CGLApp::glInit();

		// dis/en -abling primitive arrays
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

		// force normalization when normals are designated
		glEnable(GL_NORMALIZE);
	}

	int handleOptions(int optv) {
		switch( optv ) {
			case 'f':
				m_strfn = optarg;
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

	int loadGeometry() {
		if ( 0 != m_loader.load(m_strfn) ) {
			m_cout << "Loading geometry failed - GLApp aborted abnormally.\n";
			return -1;
		}
		return 0;
	}

	int mainstay() {
		// geometry must be loaded successfully in the first place in order the
		// openGL pipeline could be launched.
		if ( 0 != loadGeometry() ) {
			return -1;
		}

		setPrjInfo(60.0f,
				30,
				//ABS(m_loader.getMaxCoord(2)) + ABS(m_loader.getMinCoord(2)),
				//( ABS(m_loader.getMaxCoord(2)) + ABS(m_loader.getMinCoord(2)) )*2 );
				(ABS(m_loader.getMaxCoord(2)) + ABS(m_loader.getMinCoord(2)))*4 + 30);
		
		setViewInfo(
				(m_loader.getMaxCoord(0) + m_loader.getMinCoord(0))/2,
				(m_loader.getMaxCoord(1) + m_loader.getMinCoord(1))/2,
				//m_loader.getMaxCoord(2) + 10.0,
				( ABS(m_loader.getMaxCoord(2)) + ABS(m_loader.getMinCoord(2)) )*2 + 40 ,
				(m_loader.getMaxCoord(0) + m_loader.getMinCoord(0))/2,
				(m_loader.getMaxCoord(1) + m_loader.getMinCoord(1))/2,
				(m_loader.getMaxCoord(2) + m_loader.getMinCoord(2))/2,
				//0, 
				0, 1, 0);

		setGlutInfo(" DTI fiber polylines ");

		return	CGLApp::mainstay();
	}

	void draw() {

		/* load polyline by means of vertex buffer one by one, assuming the
		 * loader has already got the vertices and colors all well-prepared
		 */
		unsigned long szTotal = m_loader.getSize();
		for (unsigned long idx = 0; idx < szTotal; ++idx) {
			vector<GLfloat> & curLine = m_loader.getElement( idx );
			
			/*
			cout << "curline No." << idx << " : [" << curLine.size() << " elements, " << 
			  ( curLine.size()/6 )	<< " points ]" << endl;
			copy(curLine.begin(), curLine.end(), ostream_iterator< GLfloat > (cout, " "));
			cout << endl;
			*/
			
			if ( m_bUseOrgColor ) {
				glInterleavedArrays(GL_C3F_V3F, 0, &curLine[0]);
				//glEnableClientState( GL_VERTEX_ARRAY );
				//glVertexPointer(3, GL_FLOAT, 0, &curLine[0]);
				//ofs << (curLine.size()/3) << fixed << setprecision(6) << endl;
				/*
				glBegin(GL_LINE_STRIP);
				for (int j = 0; j<curLine.size();j+=3) {
					glVertex3f( curLine[j], curLine[j+1], curLine[j+2] );
					//ofs << curLine[j] <<  " " << curLine[j+1] << " " << curLine[j+2] << endl;
				}
				glEnd();
				*/
			}
			else {
				glInterleavedArrays(GL_V3F, 6*sizeof(GLfloat), &curLine[3]);
			}

			glDrawArrays(GL_LINE_STRIP, 0, curLine.size()/6);
		}
		//ofs.close();
	}
private:
	GLfloat m_colors[3];
	GLboolean m_bUseOrgColor;
	CTgdataLoader m_loader;

	string m_strfn;
};

int main(int argc, char** argv)
{
	CpolylineRender pr(argc, argv);
	pr.run();
	return 0;
}

/* set ts=4 sts=4 tw=80 sw=4 */

