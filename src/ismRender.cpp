// ----------------------------------------------------------------------------
// tubeRender.cpp : load geometry in the format of "tgdata" and then wrap each
//					each streamline with rings resembling tubes. 
//
//					Note that the tubes produced are not of any anatomical
//					meanings but simply a geometrical artifacts. So they are not
//					genuine streamtubes
//
//					this version of smRender supports box selection interaction
//
// Creation : Feb. 11th 2011
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#include "GLiboxmoth.h"
#include "GLoader.h"
#include "glrand.h"

using namespace std;

class CTubeRender: public CGLIBoxApp {
public:
	CTubeRender(int argc, char **argv) : CGLIBoxApp(argc, argv),
	m_bUseOrgColor(false),
	m_bVnormal(false),
    m_loader(true, false),
	m_strfnsrc(""),
	m_strfndst(""),
	m_strfnobj("")	{
		setVerinfo("smRender");

		addOption('f', true, "input-file-name", "the name of source file"
			   " containing geometry and in the format of \"sm\"");
		addOption('g', true, "output-file-name", "the name of target file"
			   " to store the geometry parsed and loaded");
		addOption('o', true, "obj-file-name", "the name of file storing geometry"
				" reorganized/calculated in the obj format");
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

	/* 
	 * read a file in the format of "sm", a .sm ouput from tubegen into a
	 * series of linear array as vectors 
	 */
	int loadGeometry() {
		if ( 0 != m_loader.load(m_strfnsrc) ) {
			m_cout << "Loading geometry failed - GLApp aborted abnormally.\n";
			return -1;
		}

		return 0;
	}

	void glInit(void) {
		CGLIBoxApp::glInit();

		// dis/en -abling primitive arrays
		glEnableClientState( GL_VERTEX_ARRAY );
		glDisableClientState( GL_COLOR_ARRAY );
		glEnableClientState( GL_NORMAL_ARRAY );
		glEnableClientState( GL_EDGE_FLAG_ARRAY );

		// force normalization when normals are designated
		glEnable(GL_NORMALIZE);
		glEnable(GL_RESCALE_NORMAL);

		glFrontFace(GL_CCW);
		//glEnable(GL_CULL_FACE);
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

		// by default, enable lighting and use smooth shading
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glShadeModel(GL_SMOOTH);
	}

	int handleOptions(int optv) {
		switch( optv ) {
			case 'f':
				m_strfnsrc = optarg;
				return 0;
			case 'g':
				m_strfndst = optarg;
				return 0;
			case 'o':
				m_strfnobj = optarg;
				return 0;
			default:
				return CGLIBoxApp::handleOptions( optv );
		}
		return 1;
	}

	void keyResponse(unsigned char key, int x, int y) {
		switch (key) {
			case 'c': // randomly seeding vertices
				m_bUseOrgColor = ! m_bUseOrgColor;
				if ( m_bUseOrgColor ) {
					glEnableClientState( GL_COLOR_ARRAY );
				}
				else {
					glDisableClientState( GL_COLOR_ARRAY );
				}
				break;
			case 'g': // generate different colors
				if (!m_bUseOrgColor) {
					genColors();
					glColor3fv(m_colors);
				}
				break;
			case 'v':
				m_bVnormal = ! m_bVnormal;
				break;
			default:
				return CGLIBoxApp::keyResponse(key, x, y);
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

		// write the loaded geometry into obj format
		if ( m_strfnobj.length() >= 1 ) {
			m_loader.serializeToObj( m_strfnobj );
		}

		setMinMax(m_loader.getMinCoord(0), 
				m_loader.getMinCoord(1), 
				m_loader.getMinCoord(2),
				m_loader.getMaxCoord(0), 
				m_loader.getMaxCoord(1), 
				m_loader.getMaxCoord(2));

		associateObj(&m_loader.m_alltubevertices, false,
						-( m_loader.getMinCoord(0) + m_loader.getMaxCoord(0) )/2,
						-( m_loader.getMinCoord(1) + m_loader.getMaxCoord(1) )/2,
						-( m_loader.getMinCoord(2) + m_loader.getMaxCoord(2) )/2);
		addBox();

		setPrjInfo(60.0f, 
				ABS(m_loader.getMinCoord(2))/4.0,
				(ABS(m_loader.getMaxCoord(2)) + ABS(m_loader.getMinCoord(2)))*4);
		
		setViewInfo(0.0, 0.0,
				( m_loader.getMinCoord(2) + m_loader.getMaxCoord(2) )*1.5,
				0.0, 0.0, 0.0,
				0, 1, 0);

		setGlutInfo(" DTI fiber tubes  ");

		return	CGLIBoxApp::mainstay();
	}

	void draw() {
		glPushMatrix();

		// move the local coordinate system so that the default origin is still
		// located at the center of the object coordinate system
		glTranslatef( -( m_loader.getMinCoord(0) + m_loader.getMaxCoord(0) )/2,
						-( m_loader.getMinCoord(1) + m_loader.getMaxCoord(1) )/2,
						-( m_loader.getMinCoord(2) + m_loader.getMaxCoord(2) )/2);

		/* load streamtube geometry and render
		 */
		//std::copy(m_edgeflags.begin(), m_edgeflags.end(), ostream_iterator<int> (cout, " "));
		glEdgeFlagPointer(0, &m_edgeflags[0]);

		glVertexPointer(3, GL_DOUBLE, 0, &m_loader.m_alltubevertices[0]);

		if ( m_bUseOrgColor ) {
			glColorPointer(3, GL_DOUBLE, 0, &m_loader.m_alltubecolors[0]);
		}

		if ( m_bVnormal ) {
			glNormalPointer(GL_DOUBLE, 0 ,&m_loader.m_alltubevertices[0]);
		}
		else {
			glNormalPointer(GL_DOUBLE, 0 ,&m_loader.m_alltubenormals[0]);
		}

		glDrawElements(GL_TRIANGLES, m_loader.m_alltubefaceIdxs.size(), 
				GL_UNSIGNED_INT, &m_loader.m_alltubefaceIdxs[0]);
		/*
		for (size_t c = 0; c < m_loader.m_alltubefaceIdxs.size()/3; c++) {
			glDrawElements(GL_POLYGON, 3, GL_UNSIGNED_INT, &m_loader.m_alltubefaceIdxs[0] + c*3);
		}
		*/

		glPopMatrix();
	}

private:
	GLfloat m_colors[3];
	GLboolean m_bUseOrgColor;
	GLboolean m_bVnormal;
	CSmLoader m_loader;

	string m_strfnsrc;
	string m_strfndst;
	string m_strfnobj;
};


int main(int argc, char** argv)
{
	CTubeRender ismr(argc, argv);
	//ismr.setFullWindow();
	ismr.run();
	return 0;
}

/* set ts=4 sts=4 tw=80 sw=4 */

