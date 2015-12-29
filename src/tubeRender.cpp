// ----------------------------------------------------------------------------
// tubeRender.cpp : load geometry in the format of "tgdata" and then wrap each
//					each streamline with rings resembling tubes. 
//
//					Note that the tubes produced are not of any anatomical
//					meanings but simply a geometrical artifacts. So they are not
//					genuine streamtubes
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

class CTubeRender: public CGLApp {
public:
	CTubeRender(int argc, char **argv) : CGLApp(argc, argv),
	m_bUseOrgColor(false),
    m_loader(),
	m_strfnsrc(""),
	m_strfnobj(""),
	m_lod(5),
	m_bVradius(true),
	m_fAdd(0.5),
	m_fRadius(0.25),
	m_fbdRadius(20.0),
	m_fMaxSegLen(-0xffffffff),
	m_fMinSegLen(0xffffffff) {
		for (int i=0;i<3;++i) {
			m_maxCoord[i] = -0xffffffff;
			m_minCoord[i] = 0xffffffff;
		}
		setVerinfo("tubeRender");

		addOption('f', true, "input-file-name", "the name of source file"
			   " containing geometry and in the format of tgdata");
		addOption('g', true, "output-file-name", "the name of target file"
			   " to store the geometry of streamtubes produced");
		addOption('r', true, "tube-radius", "fixed radius of the tubes"
				" to generate");
		addOption('l', true, "lod", "level ot detail controlling the tube"
				" generation, it is expected to impact the smoothness of tubes");
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

	// find the maximal and minimal streamline fragment length
	void findMaxMinSegLen() {
		unsigned long szTotal = m_loader.getSize();
		for (unsigned long idx = 0; idx < szTotal; ++idx) {
			const vector<GLfloat> & line = m_loader.getElement( idx );
			unsigned long szPts = static_cast<unsigned long>( line.size()/6 );
			GLfloat x1,x2,y1,y2,z1,z2;
			GLfloat segLen;   // length of streamline fragment
			szPts --; // szPts points contain szPts-1 line segments

			for (unsigned long idx1 = 0, idx2 = 1; idx1 < szPts; idx1++) {
				idx2 = idx1 + 1;  

				/* for each segment in a single streamline, try to interpolate
				 * a series of extraneous points to make the line segment look like a
				 * tube segment by contructing a ring around the line fragment
				 */
				x1 = line [ idx1*6 + 3 ], 
				y1 = line [ idx1*6 + 4 ], 
				z1 = line [ idx1*6 + 5 ];

				x2 = line [ idx2*6 + 3 ], 
				y2 = line [ idx2*6 + 4 ], 
				z2 = line [ idx2*6 + 5 ];

				segLen = magnitude(x1-x2, y1-y2, z1-z2);
				if (segLen > m_fMaxSegLen) {
					m_fMaxSegLen = segLen;
				}
				if (segLen < m_fMinSegLen) {
					m_fMinSegLen = segLen;
				}
			}
		}
	}

	/*
	 * MAKING artificial tubes by wrapping rings around a single streamline
	 * given by the index of the streamline storage, as is a linear structure
	 * created by loading the input geometry of streamlines
	 */
	void buildTubefromLine(unsigned long lineIdx) {
		const vector<GLfloat> & line = m_loader.getElement( lineIdx );
		unsigned long szPts = static_cast<unsigned long>( line.size()/6 );
		GLfloat x1,x2,y1,y2,z1,z2,dx,dy,dz;
		GLfloat rax, ray, raz; // for a vector acting as a rotating axis
		GLfloat angle, theta;
		GLfloat nx,ny,nz; // rotated normals
		GLfloat segLen;   // length of streamline fragment
		GLfloat scaleFactor; // scaling factor for generating quads

		// geometry generated for constructing the streamtube
		vector<GLfloat>		tube_vertices;
		vector<GLfloat>		tube_normals;
		vector<GLfloat>		tube_colors;
		vector<GLuint>		tube_faceIdxs;

		/* vertex array for each tube segment will contain all vertices needed to
		 * contruct the faces, tinted by the color originally for the start point
		 * of the source streamline segment,which is stored in the color array
		 */
		tube_vertices.resize ( szPts * m_lod * 3 );
		tube_normals.resize( szPts * m_lod * 3 );

		// szPts points contain szPts-1 line segments
		tube_colors.resize( (szPts-1) * 3 );
		tube_faceIdxs.resize( (szPts-1) * m_lod * 4 );

		for (unsigned long idx1 = 0, idx2 = 1; idx1 < szPts; idx1++) {
			idx2 = idx1 + 1;
			if ( szPts-1 == idx1 ) {
				idx2 = idx1 - 1;
			}
		    	
			/* for each segment in a single streamline, try to interpolate
			 * a series of extraneous points to make the line segment look like a
			 * tube segment by contructing a ring around the line fragment
			 */
			x1 = line [ idx1*6 + 3 ], 
			y1 = line [ idx1*6 + 4 ], 
			z1 = line [ idx1*6 + 5 ];

			x2 = line [ idx2*6 + 3 ], 
			y2 = line [ idx2*6 + 4 ], 
			z2 = line [ idx2*6 + 5 ];

			if ( m_bVradius ) {
				segLen = magnitude(x1-x2, y1-y2, z1-z2);
				// .002 and .01 are just certain magic numbers .... used for
				// tweaking the shape of tubes
				scaleFactor = (segLen - m_fMinSegLen)/(m_fMaxSegLen - m_fMinSegLen) 
								* (m_fbdRadius*.002 - m_fbdRadius*.01) +
								m_fbdRadius*.01;
			}
			else {
				scaleFactor = m_fRadius;
			}

			// take the color for the start point as the color for the associated tube
			// fragment
			if ( szPts-1 > idx1 ) {
				tube_colors [ idx1*3 + 0 ] = line [ idx1*6 + 0 ];
				tube_colors [ idx1*3 + 1 ] = line [ idx1*6 + 1 ];
				tube_colors [ idx1*3 + 2 ] = line [ idx1*6 + 2 ];
			}

			/* firstly project a third vector in between each pair of adjacent line
			 * fragments
			 */
			if ( 0 == idx1) {
				dx = x1 - x2, dy = y1 - y2, dz = z1 - z2;
			}
			else if ( szPts-1 == idx1 ) {
				dx = x2 - x1, dy = y2 - y1, dz = z2 - z1;
			}
			else { // idx1 >= 1
				GLfloat x0 = line [ (idx1-1)*6 + 3 ],
						y0 = line [ (idx1-1)*6 + 4 ],
						z0 = line [ (idx1-1)*6 + 5 ];
				GLfloat dx1 = x0 - x1, dy1 = y0 - y1, dz1 = z0 - z1,
						dx2 = x1 - x2, dy2 = y1 - y2, dz2 = z1 - z2;
				normalize(dx1, dy1, dz1);
				normalize(dx2, dy2, dz2);

				dx = m_fAdd*dx1 + dx2, dy = m_fAdd*dy1 + dy2, dz = m_fAdd*dz1 + dz2;
			}

			normalize(dx, dy, dz);

			/* decide the axis and angle used for rotating
			 * the angle between axis and the third vector interpolated above
			 * which is calculated as  "acos ( dotproduct( (0,0,1),(dx,dy,dz) ) )"
			 */
			angle = acos (  dz );
			/*
			 * the axis is simply the vector perpendicular both to these two vectors
			 * i.e. (rax,ray,raz) = crossproduct( (0,0,1),(dx,dy,dz) ).
			 */
			rax = -dy, ray = dx, raz = 0;

			normalize(rax, ray, raz);

			/* construct a series of unit normals in the xy plane, then rotate each of them
			 * around the axis by the angle above so as to contruct a multiple of ring that
			 * is expected to appear as a tube
			 */
			for (GLubyte l = 0; l < m_lod; ++l) {
				theta = 2 * 3.1415926 / m_lod * l;

				// rotate a unit normal ( cos(theta), sin(theta), 0 ) by the degree of
				// angle, around the axis above, then store the resulting normals
				tube_normals[ idx1*3 * m_lod + 3*l + 0 ] = nx = 
					( (1 - cos(angle)) * rax * rax + cos(angle) ) * cos(theta) +
					( (1 - cos(angle)) * rax * ray - sin(angle)*raz ) * sin(theta) ;
				
				tube_normals[ idx1*3 * m_lod + 3*l + 1 ] = ny =
					( (1 - cos(angle)) * rax * ray + sin(angle)*raz ) * cos(theta) +
					( (1 - cos(angle)) * ray * ray + cos(angle) ) * sin(theta);

				tube_normals[ idx1*3 * m_lod + 3*l + 2 ] = nz =
					( (1 - cos(angle)) * rax * raz - sin(angle)*ray ) * cos(theta) +
					( (1 - cos(angle)) * ray * raz + sin(angle)*rax ) * sin(theta) ;

				tube_vertices[ idx1*3 * m_lod + 3*l + 0 ] = scaleFactor*nx + line[idx1*6+3];
				tube_vertices[ idx1*3 * m_lod + 3*l + 1 ] = scaleFactor*ny + line[idx1*6+4];
				tube_vertices[ idx1*3 * m_lod + 3*l + 2 ] = scaleFactor*nz + line[idx1*6+5];

				// find the maximal and minimal coordinats among the new
				// vertices of the streamtubes
				for (int j=0; j<3; ++j) {
					if ( tube_vertices[ idx1*3 * m_lod + 3*l + j ] > m_maxCoord[j] ) {
						m_maxCoord[j] = tube_vertices[ idx1*3 * m_lod + 3*l + j ];
					}
					if ( tube_vertices[ idx1*3 * m_lod + 3*l + j ] < m_minCoord[j] ) {
						m_minCoord[j] = tube_vertices[ idx1*3 * m_lod + 3*l + j ];
					}
				}

				// the tube is finally established by a multiple of quads
				// and here we use vertex index to represent faces, each for a quad
				if ( szPts-1 > idx1 ) {
					tube_faceIdxs [ idx1*4 * m_lod + 4*l + 0 ] = idx1*m_lod + l;
					tube_faceIdxs [ idx1*4 * m_lod + 4*l + 1 ] = (idx1+1)*m_lod + l;
					tube_faceIdxs [ idx1*4 * m_lod + 4*l + 2 ] = (idx1+1)*m_lod + (l + 1)%m_lod;
					tube_faceIdxs [ idx1*4 * m_lod + 4*l + 3 ] = idx1*m_lod + (l + 1)%m_lod;
				}
			}
		}

		// save the streamtube geometry for current streamline
		m_alltubevertices[ lineIdx ] = tube_vertices;
		m_alltubenormals[ lineIdx ] = tube_normals;
		m_alltubecolors[ lineIdx ] = tube_colors;
		m_alltubefaceIdxs[ lineIdx ] = tube_faceIdxs;
	}

	/*
	 * serialize the streamtube geometry generated into a file in the format
	 * of WaveFront obj
	 */
	int serializeTubes(const string& fnobj) {
		ofstream ofs(fnobj.c_str());
		if ( ! ofs.is_open() ) {
			cerr << "failed to create file : " << fnobj << " for serializing." << endl;
			return -1;
		}

		m_cout << "Start serializing streamtube geometry .... ";
		fflush(stdout);

		// a brief even trivial header
		ofs << "# --------------------------- " << endl;
		ofs << "# start of the obj file " << endl;
		ofs << fixed << setprecision(6);

		unsigned long szTubes = static_cast<unsigned long> ( m_alltubevertices.size() );
		unsigned long szVertex, szNormal, szFace;
		// write each tube's geometry, one after another, throughout the tube
		// store
		for (unsigned long tIdx = 0; tIdx < szTubes; tIdx++) {
			ofs << "# Tube no." << tIdx << endl;

			// vertices of current tube
			szVertex = static_cast<unsigned long> ( m_alltubevertices[ tIdx ].size() ) / 3;
			for (unsigned long vIdx = 0; vIdx < szVertex; vIdx ++) {
				ofs << "v " << m_alltubevertices[ tIdx ][ vIdx*3 + 0 ] << " "
					 << m_alltubevertices[ tIdx ][ vIdx*3 + 1 ] << " "
					 << m_alltubevertices[ tIdx ][ vIdx*3 + 2 ] << endl;
			}

			// a blank line separating between each section of vertices, normals
			// and face indices
			ofs << endl;

			// normals for faces of current tube
			szNormal = static_cast<unsigned long> (m_alltubenormals[ tIdx ].size() ) / 3;
			for (unsigned long nIdx = 0; nIdx < szNormal; nIdx ++) {
				ofs << "vn " << m_alltubenormals[ tIdx ][ nIdx*3 + 0 ] << " "
							<< m_alltubenormals[ tIdx ][ nIdx*3 + 1 ] << " "
							<< m_alltubenormals[ tIdx ][ nIdx*3 + 2 ] << endl;
			}

			// a blank line separating between each section of vertices, normals
			// and face indices
			ofs << endl;

			// face indices, along with normal indices, for current tube
			szFace = static_cast<unsigned long> (m_alltubefaceIdxs[ tIdx ].size() ) / 4;
			for (unsigned long fIdx = 0; fIdx < szFace; fIdx ++) {
				ofs << "f ";
				for (int pIdx = 0; pIdx < 4; pIdx++) {
					// faces share normals with the first vertex, without text
					// coordinate information since it is not used here
					ofs << m_alltubefaceIdxs[ tIdx ][ fIdx*4 + pIdx ] << "//"
						<< m_alltubefaceIdxs[ tIdx ][ fIdx*4 + 0 ];
					if ( pIdx < 3 ) {
						ofs << " ";
					}
				}
				ofs << endl;
			}

			// two blank lines separating between each block of geometry for a
			// single tube 
			ofs << endl << endl;
		}

		// a brief even trivial tail
		ofs << "# end of the obj file " << endl;
		ofs << "# --------------------------- " << endl;

		ofs.close();

		m_cout << " finished.\n";
		return 0;
	}

	/* 
	 * read a file in the format of "tgdata", a .data ouput from tubegen into a
	 * series of linear array as vectors, each holding a streamline including
	 * its vertices and colors, then all these vectors are indexed in a headlist
	 * in which elements are pointers to these vectors respectively
	 *
	 * After the streamline geometry loaded, streamtube geometry was calculated
	 * by wrapping around each streamline a slew of rings that is expected to
	 * simulate a shape of tube only in the geometrical sense, not pertinent to
	 * visually encoding tensor attributes like the eigenvectors and eigenvalue
	 * in each voxel in the tensor field
	 */
	int loadGeometry() {
		if ( 0 != m_loader.load(m_strfnsrc) ) {
			m_cout << "Loading geometry failed - GLApp aborted abnormally.\n";
			return -1;
		}

		findMaxMinSegLen();

		m_fbdRadius = m_loader.getBoundRadius();

		// generate streamtube, each for a streamline
		unsigned long szTotal = m_loader.getSize();
		m_alltubevertices.resize( szTotal );
		m_alltubenormals.resize ( szTotal );
		m_alltubecolors.resize ( szTotal );
		m_alltubefaceIdxs.resize( szTotal );

		m_cout << "Generating streamtube meshes .... ";
		for (unsigned long idx = 0; idx < szTotal; ++idx) {
			buildTubefromLine( idx );
		}
		m_cout << " finished.\n";

		m_cout << "X : " << m_minCoord[0] << " - " << m_maxCoord[0] << "\n";
		m_cout << "Y : " << m_minCoord[1] << " - " << m_maxCoord[1] << "\n";
		m_cout << "Z : " << m_minCoord[2] << " - " << m_maxCoord[2] << "\n";

		return 0;
	}

	void glInit(void) {
		CGLApp::glInit();

		// dis/en -abling primitive arrays
		glEnableClientState( GL_VERTEX_ARRAY );
		glDisableClientState( GL_COLOR_ARRAY );
		glEnableClientState( GL_NORMAL_ARRAY );
		glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

		// force normalization when normals are designated
		glEnable(GL_NORMALIZE);
		glEnable(GL_RESCALE_NORMAL);

		glFrontFace(GL_CCW);
		glEnable(GL_CULL_FACE);
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
				m_strfnobj = optarg;
				return 0;
			case 'r':
				m_fRadius = strtof(optarg, NULL);
				if ( m_fRadius < 0.1 ) {
					cerr << "value for radius is illicit, should be >= .1\n";
					return -1;
				}
				m_bVradius = false;
				return 0;
			case 'l':
				{
					int lod = atoi(optarg);
					if ( lod >= 2 ) {
						m_lod = lod;
						return 0;
					}
					else {
						cerr << "value for lod is illict, should be >= 2.\n";
						return -1;
					}
				}
				break;
			default:
				return CGLApp::handleOptions( optv );
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

		// serialize the streamtube geometry if asked for
		if ( m_strfnobj.length() >= 1 ) {
			serializeTubes( m_strfnobj );
		}	

		setPrjInfo(60.0f, 
				ABS(m_minCoord[2])/4.0,
				(ABS(m_maxCoord[2]) + ABS(m_minCoord[2]))*4);
		
		setViewInfo(0.0, 0.0,
				( m_minCoord[2] + m_maxCoord[2] )*1.5,
				0.0, 0.0, 0.0,
				0, 1, 0);

		setGlutInfo(" DTI fiber tubes  ");

		return	CGLApp::mainstay();
	}

	void draw() {
		glPushMatrix();
		// move the local coordinate system so that the default origin is still
		// located at the center of the object coordinate system
		glTranslatef( -( m_minCoord[0] + m_maxCoord[0] )/2,
						-( m_minCoord[1] + m_maxCoord[1] )/2,
						-( m_minCoord[2] + m_maxCoord[2] )/2);
		/* load streamtube geometry and render
		 */
		unsigned long szTotal = m_loader.getSize();
		for (unsigned long idx = 0; idx < szTotal; ++idx) {
			glVertexPointer(3, GL_FLOAT, 0, &m_alltubevertices[idx][0]);
			if ( m_bUseOrgColor ) {
				glColorPointer(3, GL_FLOAT, 0, &m_alltubecolors[idx][0]);
			}
			glNormalPointer(GL_FLOAT, 0 ,&m_alltubenormals[idx][0]);

			glDrawElements(GL_QUADS, m_alltubefaceIdxs[idx].size(), 
					GL_UNSIGNED_INT, &m_alltubefaceIdxs[idx][0]);
		}
		glPopMatrix();
	}

private:
	GLfloat m_colors[3];
	GLboolean m_bUseOrgColor;
	CTgdataLoader m_loader;

	string m_strfnsrc;
	string m_strfnobj;

	// Level of Detail, the granularity of interpolation
	GLubyte				m_lod;

	// if use variant radius for tubes
	GLboolean			m_bVradius;

	// fantastic factor tunning the streamtube generation
	GLfloat m_fAdd;

	// tube radius and radius of the bounding box of the streamline model
	GLfloat m_fRadius, m_fbdRadius;

	// the maximal and minimal length over all streamline segments
	GLfloat m_fMaxSegLen, m_fMinSegLen;

	// stash of streamtube geometry for all streamlines 
	vector< vector<GLfloat> >		m_alltubevertices;
	vector< vector<GLfloat>	>		m_alltubenormals;
	vector< vector<GLfloat>	>		m_alltubecolors;
	vector< vector<GLuint> >		m_alltubefaceIdxs;

	GLdouble m_maxCoord[3];	// get the maximal X,Y and Z coordinate among all vertices
	GLdouble m_minCoord[3];	// get the minimal X,Y and Z coordinate among all vertices
};


int main(int argc, char** argv)
{
	CTubeRender tr(argc, argv);
	tr.run();
	return 0;
}

/* set ts=4 sts=4 tw=80 sw=4 */

