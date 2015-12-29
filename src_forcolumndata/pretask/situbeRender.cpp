// ----------------------------------------------------------------------------
// itubeRender.cpp : load geometry in the format of "tgdata" and then wrap each
//					each streamline with rings resembling tubes. 
//
// Creation : Feb. 6th 2011
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#include "situbeRender.h"

using std::ostream_iterator;
using std::copy;
using std::ofstream;
using std::ends;

#ifdef __MAC
#include "mac_def.h"
#endif //__MAC


//////////////////////////////////////////////////////////////////////////////
//
//	implementation of the Class CSituberRender
//
#define _BLOCK_ALL_INPUT \
	if ( "" != m_strfntask && m_taskbox.iscovered() ) { \
		return; \
	}
const char* CSitubeRender::SITR_SHM_NAME = "SHM_SITR_INTERACTION";
CSitubeRender* CSitubeRender::m_psitInstance = NULL;
CSitubeRender::CSitubeRender(int argc, char **argv) : CGLIBoxApp(argc, argv),
	m_bUseOrgColor(false),
	m_bUseDirectionColor(true),
	m_bVnormal(false),
    m_loader(),
	m_strfnsrc(""),
	m_strfnobj(""),
#ifdef DWI_EMBEDDING
	m_strdwidir(""),
#endif
	m_strfntask(""),
	m_lod(5),
	m_nselbox(1),
	m_bVradius(true),
	m_fAdd(0.5),
	m_fRadius(0.25),
	m_fbdRadius(20.0),
	m_fMaxSegLen(-LOCAL_MAXDOUBLE),
	m_fMinSegLen(LOCAL_MAXDOUBLE),
	m_colorschemeIdx(CLSCHM_ANATOMY_SYMMETRIC),
#ifdef DWI_EMBEDDING
	m_bShowDWIImage(true),
#endif
	m_pIntInfo(NULL),
	m_bSync(false),
	m_nSiblings(0),
	m_bSuspended(false),
	m_bShowContext(true)
{
	for (int i=0;i<3;++i) {
		m_maxCoord[i] = -LOCAL_MAXDOUBLE;
		m_minCoord[i] = LOCAL_MAXDOUBLE;
	}

	addOption('f', true, "input-file-name", "the name of source file"
			" containing geometry and in the format of tgdata");
#ifdef DWI_EMBEDDING
	addOption('d', true, "dwi-b0-dir", "directory holding DWI b0 DICOM images");
#endif
	addOption('g', true, "output-file-name", "the name of target file"
			" to store the geometry of streamtubes produced");
	addOption('r', true, "tube-radius", "fixed radius of the tubes"
			" to generate");
	addOption('l', true, "lod", "level ot detail controlling the tube"
			" generation, it is expected to impact the smoothness of tubes");
	addOption('b', true, "box-num", "number of selection box"
			" which is 1 by default");
	addOption('t', true, "task-list", "a file containing a list of "
			"visualization tasks");

	// turn off gadget feature temporarily since we do not want the default X-Y-Z
	// coordinate system indicator anymore
	m_bGadgetEnabled = false;
	m_colormapper.setColorScheme( m_colorschemeIdx );
	m_pcmGadget = new CSphereColorMap();
	m_paxesGagdet = new CAnatomyAxis();
}

CSitubeRender::~CSitubeRender()
{
	if (NULL != m_pIntInfo) {
		_releaseSharedInfo();
	}
}

void CSitubeRender::genColors() 
{
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
void CSitubeRender::findMaxMinSegLen() 
{
	unsigned long szTotal = m_loader.getSize();
	for (unsigned long idx = 0; idx < szTotal; ++idx) {
		const vector<GLfloat> & line = m_loader.getElement( idx );
		unsigned long szPts = static_cast<unsigned long>( line.size()/6 );
		GLfloat x1,x2,y1,y2,z1,z2;
		GLfloat segLen;   // length of streamline fragment
		szPts --; // szPts points contain szPts-1 line segments

		for (unsigned long idx1 = 0, idx2 = 1; idx1 < szPts; idx1++) {
			idx2 = idx1 + 1;  

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
void CSitubeRender::buildTubefromLine(unsigned long lineIdx) 
{
	const vector<GLfloat> & line = m_loader.getElement( lineIdx );
	unsigned long szPts = static_cast<unsigned long>( line.size()/6 );
	GLfloat x1,x2,y1,y2,z1,z2,dx,dy,dz;
	GLfloat rax, ray, raz; // for a vector acting as a rotating axis
	GLfloat angle, theta;
	GLfloat nx,ny,nz; // rotated normals
	GLfloat segLen;   // length of streamline fragment
	GLfloat scaleFactor; // scaling factor for generating quads
	GLfloat r,g,b;		 // direction encoded color components

	// geometry generated for constructing the streamtube
	vector<GLfloat>		tube_vertices;
	vector<GLfloat>		tube_normals;
	vector<GLfloat>		tube_colors;
	vector<GLuint>		tube_faceIdxs;
	vector<GLfloat>		tube_encodedcolors;

	/* vertex array for each tube segment will contain all vertices needed to
	 * contruct the faces, tinted by the color originally for the start point
	 * of the source streamline segment,which is stored in the color array
	 */
	tube_vertices.resize ( szPts * m_lod * 3 );
	tube_normals.resize( szPts * m_lod * 3 );

	// szPts points contain szPts-1 line segments
	//tube_colors.resize( (szPts-1) * 3 );
	tube_colors.resize( szPts * m_lod * 3 );
	tube_faceIdxs.resize( (szPts-1) * m_lod * 4 );

	tube_encodedcolors.resize( szPts * m_lod * 3 );

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

		// map the streamline segment vector's coordinate to the RGB color
		// space
		if ( szPts-1 == idx1 ) {
			dx = x1 - x2, dy = y1 - y2, dz = z1 - z2;
		}
		else {
			dx = x2 - x1, dy = y2 - y1, dz = z2 - z1;
		}

		// take the color for the start point as the color for the associated tube
		// fragment
		m_colormapper.getColor( dx, dy, dz, r, g ,b);

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
			tube_colors [ idx1*3 * m_lod + 3*l + 0 ] = line [ idx1*6 + 0 ];
			tube_colors [ idx1*3 * m_lod + 3*l + 1 ] = line [ idx1*6 + 1 ];
			tube_colors [ idx1*3 * m_lod + 3*l + 2 ] = line [ idx1*6 + 2 ];
			tube_encodedcolors [ idx1*3 * m_lod + 3*l + 0 ] = r;
			tube_encodedcolors [ idx1*3 * m_lod + 3*l + 1 ] = g;
			tube_encodedcolors [ idx1*3 * m_lod + 3*l + 2 ] = b;

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
	m_encodedcolors[ lineIdx ] = tube_encodedcolors;
}

/*
 * serialize the streamtube geometry generated into a file in the format
 * of WaveFront obj
 */
int CSitubeRender::serializeTubes(const string& fnobj) 
{
	ofstream ofs(fnobj.c_str());
	if ( ! ofs.is_open() ) {
		cerr << "failed to create file : " << fnobj << 
			" for serializing." << endl;
		return -1;
	}

	m_cout << "Start serializing streamtube geometry .... ";
	fflush(stdout);

	// a brief even trivial header
	ofs << "# --------------------------- " << endl;
	ofs << "# start of the obj file " << endl;
	ofs << fixed << setprecision(6);

	unsigned long szTubes = static_cast<unsigned long> 
		( m_alltubevertices.size() );
	unsigned long szVertex, szNormal, szFace;
	// write each tube's geometry, one after another, throughout the tube
	// store
	for (unsigned long tIdx = 0; tIdx < szTubes; tIdx++) {
		ofs << "# Tube no." << tIdx << endl;

		// vertices of current tube
		szVertex = static_cast<unsigned long> 
			( m_alltubevertices[ tIdx ].size() ) / 3;
		for (unsigned long vIdx = 0; vIdx < szVertex; vIdx ++) {
			ofs << "v " << m_alltubevertices[ tIdx ][ vIdx*3 + 0 ] << " "
				<< m_alltubevertices[ tIdx ][ vIdx*3 + 1 ] << " "
				<< m_alltubevertices[ tIdx ][ vIdx*3 + 2 ] << endl;
		}

		// a blank line separating between each section of vertices, normals
		// and face indices
		ofs << endl;

		// normals for faces of current tube
		szNormal = static_cast<unsigned long> 
			(m_alltubenormals[ tIdx ].size() ) / 3;
		for (unsigned long nIdx = 0; nIdx < szNormal; nIdx ++) {
			ofs << "vn " << m_alltubenormals[ tIdx ][ nIdx*3 + 0 ] << " "
				<< m_alltubenormals[ tIdx ][ nIdx*3 + 1 ] << " "
				<< m_alltubenormals[ tIdx ][ nIdx*3 + 2 ] << endl;
		}

		// a blank line separating between each section of vertices, normals
		// and face indices
		ofs << endl;

		// face indices, along with normal indices, for current tube
		szFace = static_cast<unsigned long> 
			(m_alltubefaceIdxs[ tIdx ].size() ) / 4;
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
int CSitubeRender::loadGeometry() 
{
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
	m_encodedcolors.resize ( szTotal );

	m_cout << "Generating streamtube meshes .... ";
	for (unsigned long idx = 0; idx < szTotal; ++idx) {
		buildTubefromLine( idx );
	}
	m_cout << " finished.\n";

	m_cout << "X : " << m_minCoord[0] << " - " << m_maxCoord[0] << "\n";
	m_cout << "Y : " << m_minCoord[1] << " - " << m_maxCoord[1] << "\n";
	m_cout << "Z : " << m_minCoord[2] << " - " << m_maxCoord[2] << "\n";

	m_edgeflags.resize( szTotal );

	return 0;
}

bool CSitubeRender::isTubeInSelbox(unsigned long lineIdx) 
{
	//const vector<GLfloat> & line = m_loader.getElement( lineIdx );
	const vector<GLfloat> & line = m_alltubevertices[ lineIdx ];
	unsigned long szPts = static_cast<unsigned long>( line.size()/6 );

	for (unsigned long idx = 0; idx < szPts; idx++) {
		if (isPointInBox( 
					line [ idx*6 + 3 ] - (m_maxCoord[0] + m_minCoord[0])/2, 
					line [ idx*6 + 4 ] - (m_maxCoord[1] + m_minCoord[1])/2, 
					line [ idx*6 + 5 ] - (m_maxCoord[2] + m_minCoord[2])/2)) {
			return true;
		}
	}
	return false;
}

void CSitubeRender::glInit(void) 
{
	CGLIBoxApp::glInit();

	// dis/en -abling primitive arrays
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_COLOR_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

	// force normalization when normals are designated
	glEnable(GL_NORMALIZE);
	glEnable(GL_RESCALE_NORMAL);

	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// by default, enable lighting and use smooth shading
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);

	// when synchronous interaction among multiple processes is on, we use
	// glutIdleFunc callback as the monitor of the shared memory holding the
	// info block as the IPC here used
	if ( m_nSiblings >= 1 ) {
		glutIdleFunc(_onIdle);
		m_bSync = true;
	}
}

int CSitubeRender::handleOptions(int optv) 
{
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
		case 'b':
			{
				int nselbox = atoi(optarg);
				if ( nselbox >= 0 ) {
					m_nselbox = nselbox;
					return 0;
				}
				else {
					cerr << "value for number of selection box is illict, "
						"should be >= 0.\n";
					return -1;
				}
			}
			break;
#ifdef DWI_EMBEDDING
		case 'd':
			{
				m_strdwidir = optarg;
				return 0;
			}
			break;
#endif
		case 't':
			{
				m_strfntask = optarg;
				return 0;
			}
			break;
		default:
			return CGLIBoxApp::handleOptions( optv );
	}
	return 1;
}

void CSitubeRender::keyResponse(unsigned char key, int x, int y) 
{
	if ( 32 != key ) {
		_BLOCK_ALL_INPUT;
	}
	if ( m_bSync && m_nSiblings >= 1) {
		m_pIntInfo->bUpdated = true;
		m_pIntInfo->key = key;
		m_pIntInfo->x = x;
		m_pIntInfo->y = y;
		m_pIntInfo->event = IE_KEY_PRESSED;
		// wait for uniform synchronization across all sibling processes
		/* -- YES, I DO NOT KNOW WHO THEY ARE, BUT I JUST BROADCAST THE CHANGE
		 * IN INTERACTION INPUTS I TRIGGERED ---
		 */
		return;
	}
	_realkeyResponse(key, x, y);
}

void CSitubeRender::specialResponse(int key, int x, int y)
{
	_BLOCK_ALL_INPUT;
	/* following special keys are not suggested to broadcast to other processes
	 * if any and they are meant to effect for current process only
	 */
	switch (key) {
		case GLUT_KEY_F11:
		case GLUT_KEY_F9:
		case GLUT_KEY_F8:
		case GLUT_KEY_F7:
		case GLUT_KEY_F6:
		case GLUT_KEY_UP:
		case GLUT_KEY_DOWN:
		case GLUT_KEY_LEFT:
		case GLUT_KEY_RIGHT:
			return;
		case GLUT_KEY_F5:
			{
				if ( m_nSiblings >= 1 ) {
					m_bSync = !m_bSync;
					m_cout << "[PID: " << getpid() << 
						(m_bSync?" ] enter in ":" ] leave ") 
						<< " synchronization.\n";
				}
			}
			return;
		default:
			break;
	}

	if ( m_bSync && m_nSiblings >= 1) {
		m_pIntInfo->bUpdated = true;
		m_pIntInfo->key = key;
		m_pIntInfo->x = x;
		m_pIntInfo->y = y;
		m_pIntInfo->event = IE_SPECIAL_KEY;
		// wait for uniform synchronization across all sibling processes
		/* -- YES, I DO NOT KNOW WHO THEY ARE, BUT I JUST BROADCAST THE CHANGE
		 * IN INTERACTION INPUTS I TRIGGERED ---
		 */
		return;
	}

	_realSpecialResponse(key, x, y);
}

void CSitubeRender::mouseResponse(int button, int state, int x, int y)
{
	_BLOCK_ALL_INPUT;
	if ( m_bSync && m_nSiblings >= 1) {
		m_pIntInfo->bUpdated = true;
		m_pIntInfo->button = button;
		m_pIntInfo->state = state;
		m_pIntInfo->x = x;
		m_pIntInfo->y = y;
		m_pIntInfo->event = IE_MOUSE_CLICKED;
		// wait for uniform synchronization across all sibling processes
		/* -- YES, I DO NOT KNOW WHO THEY ARE, BUT I JUST BROADCAST THE CHANGE
		 * IN INTERACTION INPUTS I TRIGGERED ---
		 */
		return;
	}
	CGLIBoxApp::mouseResponse(button, state, x, y);
}

void CSitubeRender::mouseMotionResponse(int x, int y)
{
	_BLOCK_ALL_INPUT;
	if ( m_bSync && m_nSiblings >= 1) {
		m_pIntInfo->bUpdated = true;
		m_pIntInfo->x = x;
		m_pIntInfo->y = y;
		m_pIntInfo->event = IE_MOUSE_MOVE;
		// wait for uniform synchronization across all sibling processes
		/* -- YES, I DO NOT KNOW WHO THEY ARE, BUT I JUST BROADCAST THE CHANGE
		 * IN INTERACTION INPUTS I TRIGGERED ---
		 */
		return;
	}
	CGLIBoxApp::mouseMotionResponse(x, y);
}

void CSitubeRender::mousePassiveMotionResponse(int x, int y)
{
	_BLOCK_ALL_INPUT;
	if ( m_bSync && m_nSiblings >= 1) {
		m_pIntInfo->bUpdated = true;
		m_pIntInfo->x = x;
		m_pIntInfo->y = y;
		m_pIntInfo->event = IE_MOUSE_PASSIVE_MOVE;
		// wait for uniform synchronization across all sibling processes
		/* -- YES, I DO NOT KNOW WHO THEY ARE, BUT I JUST BROADCAST THE CHANGE
		 * IN INTERACTION INPUTS I TRIGGERED ---
		 */
		return;
	}
	CGLIBoxApp::mousePassiveMotionResponse(x, y);
}

void CSitubeRender::mouseWheelRollResponse(int wheel, int direction, int x, int y)
{
	_BLOCK_ALL_INPUT;
	if ( m_bSync && m_nSiblings >= 1) {
		m_pIntInfo->bUpdated = true;
		m_pIntInfo->wheel = wheel;
		m_pIntInfo->direction = direction;
		m_pIntInfo->x = x;
		m_pIntInfo->y = y;
		m_pIntInfo->event = IE_MOUSE_WHEEL_ROLL;
		// wait for uniform synchronization across all sibling processes
		/* -- YES, I DO NOT KNOW WHO THEY ARE, BUT I JUST BROADCAST THE CHANGE
		 * IN INTERACTION INPUTS I TRIGGERED ---
		 */
		return;
	}
	CGLIBoxApp::mouseWheelRollResponse(wheel, direction, x, y);
}

int CSitubeRender::mainstay() 
{
	// geometry must be loaded successfully in the first place in order the
	// openGL pipeline could be launched.
	if ( 0 != loadGeometry() ) {
		return -1;
	}

	// serialize the streamtube geometry if asked for
	if ( m_strfnobj.length() >= 1 ) {
		serializeTubes( m_strfnobj );
	}	

	setMinMax(m_minCoord[0], m_minCoord[1], m_minCoord[2],
			m_maxCoord[0], m_maxCoord[1], m_maxCoord[2]);

	// add a set of buttons for in-situ box interaction
	//addButton("+", "icons/1.bmp");
	addButton("+", "bicons/1.bmp");
	addButton("-", "bicons/2.bmp");
	addButton("&", "bicons/3.bmp","|"); // AND or OR
	addButton("a", "bicons/4.bmp","e"); // exclusive or active selection mode
	addButton("s", "bicons/5.bmp","h"); // show / hide
	addButton("c", "bicons/6.bmp","n"); // show context or no context

	// add initial number of selection boxes
	for (int i=0; i<m_nselbox; ++i) {
		addBox();
	}

	// add a color map sphere
	m_pcmGadget->setVertexCoordRange(
			( m_minCoord[0] + m_maxCoord[0] )/2.5,
			( m_minCoord[1] + m_maxCoord[1] )/2.5,
			( m_minCoord[2] + m_maxCoord[2] )/2.5);
	addGadget( m_pcmGadget );
	((CSphereColorMap*)m_pcmGadget)->setColorScheme( m_colorschemeIdx );
	//m_pcmGadget->switchTranslucent();

	// add an anatomical coordinate system gadget
	m_paxesGagdet->setVertexCoordRange(
			( m_minCoord[0] + m_maxCoord[0] )/2,
			( m_minCoord[1] + m_maxCoord[1] )/2,
			( m_minCoord[2] + m_maxCoord[2] )/2);
	addGadget( m_paxesGagdet );
	((CAnatomyAxis*)m_paxesGagdet)->setColorScheme( m_colorschemeIdx );

	// turn back on gadget feature 
	m_bGadgetEnabled = true;
	
	// associate with a task list if requested
	if ( "" != m_strfntask ) { // yes, requested
		// loading a task list is also optional
		if ( 0 < m_taskbox.loadFromFile( m_strfntask.c_str() ) ) {
			m_taskbox.setVertexCoordRange(
				( m_minCoord[0] + m_maxCoord[0] )/2,
				( m_minCoord[1] + m_maxCoord[1] )/2,
				( m_minCoord[2] + m_maxCoord[2] )/2);
			m_taskbox.setColor(1.0, 1.0, 1.0);
			m_taskbox.turncover(true);
			m_bIboxEnabled = false;
			m_bGadgetEnabled = false;
		}
	}

#ifdef DWI_EMBEDDING
	// get DWI embedding prepared if requested
	if ( "" != m_strdwidir ) {// yes, requested 
		if ( 0 != m_dcmexplorer.setDir( m_strdwidir.c_str() ) ) {
			cerr << "Failed in DWI image loading, aborted now.\n";
			return -1;
		}

		m_dcmexplorer.setVertexCoordRange(
				( m_minCoord[0] + m_maxCoord[0] )/2,
				( m_minCoord[1] + m_maxCoord[1] )/2,
				( m_minCoord[2] + m_maxCoord[2] )/2);
		if  (! m_dcmexplorer.start() ) {
			return -1;
		}
	}
#endif

	setPrjInfo(45.0f, 
			ABS(m_minCoord[2])/4.0,
			(ABS(m_maxCoord[2]) + ABS(m_minCoord[2]))*8);

	setViewInfo(0.0, 0.0,
			( m_minCoord[2] + m_maxCoord[2] )*3,
			0.0, 0.0, 0.0,
			0, 1, 0);

	m_psitInstance = this;
	// if multiple process support is enable
	if ( m_nSiblings >= 1 ) {
		_initSharedInfo();
		// catching signal SIGUSR1 to receive parent's post on the update of the
		// number of alive sibling processes
		signal(SIGUSR1, _on_sibling_exit);
	}

	signal (SIGTERM, _on_killed);
	signal (SIGINT, _on_killed);

	if ( m_bSuspended ) {
		m_cout << "Event loop entrance suspened.\n";
		return 0;
	}

	return	CGLIBoxApp::mainstay();
}

void CSitubeRender::draw() 
{
#ifdef DWI_EMBEDDING
	// draw the DWI B0 image
	if ( "" != m_strdwidir && m_bShowDWIImage ) {
		m_dcmexplorer.display();
	}
#endif

	// draw task text box if necessary
	if ( "" != m_strfntask ) {
		m_taskbox.display();
		if ( m_taskbox.iscovered() ) {
			// when a task is waiting for trigger, no draw afterwards is needed
			return;
		}
	}

	// show streamline model as context for selecting ROIs
	if ( m_bShowContext ) {
		glPushMatrix();
		glTranslatef( - (m_loader.getMaxCoord(0) + m_loader.getMinCoord(0))/2,
					  -	(m_loader.getMaxCoord(1) + m_loader.getMinCoord(1))/2,
					  -	(m_loader.getMaxCoord(2) + m_loader.getMinCoord(2))/2 );
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPushClientAttrib(GL_ALL_CLIENT_ATTRIB_BITS);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(.6,.6,.6,0.6f);
		glDisable(GL_LIGHTING);
		glDisable(GL_COLOR_MATERIAL);
		// always render the streamline geometry as the context for selection
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
		glPopClientAttrib();
		glPopAttrib();
		glPopMatrix();
	}

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
		if ( m_boxes.size() > 0 ) {
			if ( m_bOR ) {
			   if ( m_bRemovalbased == isTubeInSelbox(idx) ) {
				   /*
				    * flags of streamlines that will disappear is now set as GL_FALSE, 
				    * for the purpose of region dumping(saving)
				   */
				   m_edgeflags[idx] = GL_FALSE;
				   continue;
			   }
			}
			else if ( m_bRemovalbased == isLineInBox<GLfloat>( 
						&m_alltubevertices[idx][0], 
						m_alltubevertices[idx].size(),
						6, 3, 
						-(GLfloat)( m_minCoord[0] + m_maxCoord[0] )/2,
						-(GLfloat)( m_minCoord[1] + m_maxCoord[1] )/2,
						-(GLfloat)( m_minCoord[2] + m_maxCoord[2] )/2) ) {
				/*
				 * flags of streamlines that will disappear is now set as GL_FALSE, 
				 * for the purpose of region dumping(saving)
				*/
				m_edgeflags[idx] = GL_FALSE;
				continue;
			}
		}
		
		/* otherwise, for visible streamtubes, the corresponding streamlines
		 * should be set to GL_TRUE in their flags
		 */
		m_edgeflags[idx] = GL_TRUE;

		glVertexPointer(3, GL_FLOAT, 0, &m_alltubevertices[idx][0]);

		if ( m_bUseDirectionColor ) {
			glColorPointer(3, GL_FLOAT, 0, &m_encodedcolors[idx][0]);
		}
		else if ( m_bUseOrgColor ) {
			glColorPointer(3, GL_FLOAT, 0, &m_alltubecolors[idx][0]);
		}

		if ( m_bVnormal ) {
			glNormalPointer(GL_FLOAT, 0 ,&m_alltubevertices[idx][0]);
		}
		else {
			glNormalPointer(GL_FLOAT, 0 ,&m_alltubenormals[idx][0]);
		}

		glDrawElements(GL_QUADS, m_alltubefaceIdxs[idx].size(), 
				GL_UNSIGNED_INT, &m_alltubefaceIdxs[idx][0]);
	}

	glPopMatrix();

	/*
	 * since mostly an interaction input should bring about a need of redrawing,
	 * here at this point it should be the best place to say that one more
	 * interaction synchronization for current process has been finished
	 */
	if ( m_bSync && m_nSiblings >= 1 && m_pIntInfo->bUpdated ) {
		m_pIntInfo->nFinished ++;

		if ( m_pIntInfo->nFinished >= m_nSiblings ) {
			// ready for the next update of the interaction info sharing block
			m_pIntInfo->bUpdated = false;
			m_pIntInfo->nFinished = 0;
			m_cout << "OK, current synchronization finished.\n";
		}
	}
}

void CSitubeRender::onIdle(void)
{
	// monitoring the shared memory block for interaction synchronization
	if (!m_pIntInfo->bUpdated || !m_bSync) {
		return;
	}

	switch ( m_pIntInfo->event ) {
		case IE_KEY_PRESSED:
			{
				_realkeyResponse(m_pIntInfo->key, m_pIntInfo->x, m_pIntInfo->y);
			}
			break;
		case IE_SPECIAL_KEY:
			{
				_realSpecialResponse(m_pIntInfo->key, m_pIntInfo->x, m_pIntInfo->y);	
			}
			break;
		case IE_MOUSE_CLICKED:
			{
				CGLIBoxApp::mouseResponse(m_pIntInfo->button, m_pIntInfo->state, 
								m_pIntInfo->x, m_pIntInfo->y);
			}
			break;
		case IE_MOUSE_MOVE:
			{
				CGLIBoxApp::mouseMotionResponse(m_pIntInfo->x, m_pIntInfo->y);
			}
			break;
		case IE_MOUSE_PASSIVE_MOVE:
			{
				CGLIBoxApp::mousePassiveMotionResponse(m_pIntInfo->x, m_pIntInfo->y);
			}
			break;
		case IE_MOUSE_WHEEL_ROLL:
			{
				CGLIBoxApp::mouseWheelRollResponse(m_pIntInfo->wheel, 
						m_pIntInfo->direction, m_pIntInfo->x, m_pIntInfo->y);
			}
			break;
		case IE_NONE:
		default:
			m_cout << "unrecognized event...\n";
			return;
	}
}

void CSitubeRender::setNumberOfSiblings(int n)
{
	if ( n >= 0 ) {
		m_nSiblings = n;
	}
	else {
		m_nSiblings += n;
		m_cout << "POST from parent: one of siblings already died.\n";
		// when all siblings exit, no need to share interaction input with
		// others any more and no need of the idle function as well therefore
		if ( m_nSiblings < 1 ) {
			glutIdleFunc(NULL);
			m_cout << "no siblings to IPC with any more, "
				"liberated into single status.\n";
		}
	}
}

void CSitubeRender::suspend(bool bSuspend)
{
	m_bSuspended = bSuspend;
	if ( !bSuspend ) {
		CGLIBoxApp::mainstay();
	}
}

bool CSitubeRender::isSync() const
{
	return m_bSync;
}

bool CSitubeRender::switchhelp(bool bon)
{
	bool ret = m_bShowHelp;
	m_bShowHelp = bon;
	return ret;
}

int CSitubeRender::dumpRegions(const char* fnRegion)
{
	string strfn(fnRegion);
	if ("" == strfn) {
		// split directory and file name in the input file m_strfnsrc
		string fnsrc(m_strfnsrc), fndir("./");
		size_t loc = fnsrc.rfind('/');
		if ( string::npos != loc ) {
			fndir.assign(m_strfnsrc.begin(), m_strfnsrc.begin() + loc + 1);
			fnsrc.assign(m_strfnsrc.begin() + loc + 1, m_strfnsrc.end());
		}

		// automate the file name if not offered explicitly
		ostringstream ostrfn;
		ostrfn << fndir << "region_" << time(NULL) 
			<< "_" << fnsrc << ends;
		strfn = ostrfn.str();
	}

	if ( 0 != m_loader.dump(strfn.c_str(), &m_edgeflags) ) {
		cerr << "FATAL: error in dumping regions.\n";
		return -1;
	}
	m_cout << "regions selected dumped into " << strfn << ".\n";
	return 0;
}

int CSitubeRender::dumpFiberIdx(const char* fnFiberidx)
{
	string strfn(fnFiberidx);
	if ("" == strfn) {
		// split directory and file name in the input file m_strfnsrc
		string fnsrc(m_strfnsrc), fndir("./");
		size_t loc = fnsrc.rfind('/');
		if ( string::npos != loc ) {
			fndir.assign(m_strfnsrc.begin(), m_strfnsrc.begin() + loc + 1);
			fnsrc.assign(m_strfnsrc.begin() + loc + 1, m_strfnsrc.end());
		}

		// automate the file name if not offered explicitly
		ostringstream ostrfn;
		ostrfn << fndir << "fiberidx_" << time(NULL) 
			<< "_" << fnsrc << ends;
		strfn = ostrfn.str();
	}
	
	ofstream ofs(strfn.c_str());
	ostringstream ostr;
	size_t cnt = 0;
	for (size_t i = 0; i < m_edgeflags.size(); i++) {
		if ( m_edgeflags[i] == GL_TRUE ) {
			cnt ++;
			ostr << i << "\n";
		}
	}
	ofs << cnt << "\n";
	ofs << ostr.str();
	ofs.close();

	m_cout << "indices of fibers selected dumped into " << strfn << ".\n";
	return 0;
}

int CSitubeRender::dumpBoxpos(bool bTangential, const char* fnBoxpos)
{
	if ( m_boxes.size() < 1 ) {
		return -1;
	}

	string strfn(fnBoxpos);
	if ( "" == strfn ) {
		_formFilename(strfn, "tumorbox", m_strfnsrc, false);
	}

	ofstream ofs(strfn.c_str());
	ofs << (bTangential?"Y":"N") << "\n";
	m_boxes[0].reportself( ofs, true );
	ofs.close();

	m_cout << "box 0 measure dumped into " << strfn << ".\n";
	return 0;
}

void CSitubeRender::handleBtnEvents(int event)
{
	switch (event) {
		case 5: // show context or not
			m_bShowContext = !m_bShowContext;
			m_buttons[5].switchStatus();
			break;
		default:
			break;
	}
	CGLIBoxApp::handleBtnEvents(event);
}

int CSitubeRender::_initSharedInfo(bool bInit)
{
	// create or open the shared memory segment
	int fd = shm_open(SITR_SHM_NAME, O_RDWR|O_CREAT, S_IRWXU);
	if ( -1 == fd) {
		cerr << "FATAL: shared memory [" << SITR_SHM_NAME << 
			"] open failed - error code=" << strerror(errno) << "\n";
		return -1;
	}

	size_t sz = sizeof (interaction_info_t);
	if ( -1 == ftruncate(fd, sz) ) {
		cerr << "FATAL: shared memory [" << SITR_SHM_NAME << 
			"] truncating failed - error code=" << strerror(errno) << "\n";
		return -2;
	}

	// and map it into current process's address space
	m_pIntInfo = (interaction_info_t*)mmap(NULL, sz, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if ( m_pIntInfo == MAP_FAILED ) {
		cerr << "FATAL: shared memory [" << SITR_SHM_NAME << 
			"] mmap failed - error code=" << strerror(errno) << "\n";
		return -3;
	}

	if (bInit) {
		m_pIntInfo->key = -1;
		m_pIntInfo->x = -1;
		m_pIntInfo->y = -1;
		m_pIntInfo->button = -1;
		m_pIntInfo->state = -1;
		m_pIntInfo->wheel = -1;
		m_pIntInfo->direction = -1;
		m_pIntInfo->event = IE_NONE;
	}

	m_pIntInfo->bUpdated = false;
	m_pIntInfo->nFinished = 0;
	return 0;
}

int CSitubeRender::_releaseSharedInfo()
{
	size_t sz = sizeof (interaction_info_t);
	if ( -1 == munmap(m_pIntInfo, sz) ) {
		cerr << "FATAL: shared memory [" << SITR_SHM_NAME << 
			"] munmap failed - " << strerror(errno) << "\n";
		return -1;
	}

	shm_unlink(SITR_SHM_NAME);
	m_pIntInfo = NULL;
	return 0;
}

void CSitubeRender::_realkeyResponse(unsigned char key, int x, int y)
{
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
		case 'v':
			m_bVnormal = ! m_bVnormal;
			break;
		case 'd':
			m_bUseDirectionColor = ! m_bUseDirectionColor;
			if ( m_bUseDirectionColor ) {
				glEnableClientState( GL_COLOR_ARRAY );
				m_bGadgetEnabled = true;
			}
			else {
				glDisableClientState( GL_COLOR_ARRAY );
				m_bGadgetEnabled = false;
			}
			break;
		case 'm':
			{
				m_colorschemeIdx = ( m_colorschemeIdx + 1 ) % getNumberOfColorSchemes();
				m_colormapper.setColorScheme( m_colorschemeIdx );
				((CSphereColorMap*)m_pcmGadget)->setColorScheme( m_colorschemeIdx );
				((CAnatomyAxis*)m_paxesGagdet)->setColorScheme( m_colorschemeIdx );
				m_cout << "coloring scheme switched to : " << m_colormapper.getName() << "\n";

				m_cout << "Regenerating streamtube meshes .... ";
				unsigned long szTotal = m_loader.getSize();
				for (unsigned long idx = 0; idx < szTotal; ++idx) {
					buildTubefromLine( idx );
				}
				m_cout << " finished.\n";
			}
			break;
#ifdef DWI_EMBEDDING
		case 'p':
			if ( "" != m_strdwidir && m_bShowDWIImage ) {
				m_dcmexplorer.prev();
			}
			break;
		case 'n':
			if ( "" != m_strdwidir && m_bShowDWIImage ) {
				m_dcmexplorer.next();
			}
			break;
		case 'o':
			m_bShowDWIImage = !m_bShowDWIImage;
			m_cout << "DWI Image embedding " << 
				(m_bShowDWIImage?"on":"off") << "\n";
			break;
#endif
		case '0': // possibly we have already occupied the idlefunc callback!
			return;
		case 27: // from the interaction standpoint, 
				 // ESC is not a good choice for exiting, we use PAGE_DOWN
				 // instead
			if ( m_bSync && m_nSiblings < 1 ) {
				CGLIBoxApp::keyResponse(key, x, y);
			}
			return;
		case 32: // Space bar to trigger next task if any
			if ( "" != m_strfntask ) {
				if ( m_taskbox.iscovered() && !m_taskbox.isPrompt() ) {
					m_bGadgetEnabled = true;
					m_taskbox.turncover(false);
					m_cout << "task started...\n";
				}
				else {
					m_taskbox.turncover(true);
					if ( m_taskbox.next() ) {
						m_bIboxEnabled = false;
						m_bGadgetEnabled = false;
					}
					else { // all task listed out already
						cleanup();
						exit(EXIT_SUCCESS);
						m_bIboxEnabled = true;
						m_bGadgetEnabled = true;
						m_taskbox.turncover(false);
					}
				}
				break;
			}
			CGLIBoxApp::keyResponse(key, x, y);
			return;
		default:
			CGLIBoxApp::keyResponse(key, x, y);
			return;
	}
	glutPostRedisplay();
}

void CSitubeRender::_realSpecialResponse(int key, int x, int y)
{
	switch( key ) {
		case GLUT_KEY_F11: 
			// this key is only available in sampling process where
			// regions are selected and saved as references by
			// medical doctor and will be used to compare with
			// during the post-study data analysis
			{
				// now what is visible on the screen is exactly what the
				// user selected if he/she has actually done so using
				// selection box(es); to save the result, just to dump
				// at this point
				if (m_bIboxEnabled && m_boxes.size() > 0) {
					// since box is currently the solitary selection
					// means, it is meaningful to save the selected
					// regions only if the use does use selection
					// box(es)
					dumpRegions();
				}
			}
			return;
		case GLUT_KEY_F12:
			{
				if (m_bIboxEnabled && m_boxes.size() > 0) {
					dumpFiberIdx();
				}
			}
			return;
		case GLUT_KEY_PAGE_UP:
			{
				if (m_bIboxEnabled && m_boxes.size() > 0) {
					dumpBoxpos(true);
				}
			}
			return;
		case GLUT_KEY_PAGE_DOWN:
			{
				if (m_bIboxEnabled && m_boxes.size() > 0) {
					dumpBoxpos(false);
				}
			}
			return;
		default:
			CGLIBoxApp::specialResponse(key, x, y);
			return;
	}
	glutPostRedisplay();
}

void CSitubeRender::_on_sibling_exit(int sig)
{
	if ( SIGUSR1 != sig ) {
		// no, we never expect to handle other signals than SIGUSR1
		return;
	}
	if (m_psitInstance) {
		m_psitInstance->setNumberOfSiblings(-1);
	}
}

void CSitubeRender::_on_killed(int sig)
{
	if ( SIGTERM != sig && SIGINT != sig) {
		// no, we never expect to handle other signals than SIGTERM and SIGINT
		return;
	}
	if (m_psitInstance) {
		cout << "istubeRender killed.\n";
		m_psitInstance->cleanup();
		exit(EXIT_SUCCESS);
	}
}

int CSitubeRender::_formFilename(std::string& fnout, const std::string& prefix,
			const std::string& fnsrc, bool usetime, const char* extension)
{
	// split directory and file name in the input file fnsrc
	string fnfile(fnsrc), fndir("./"), strext(extension);
	size_t loc = fnfile.rfind('/');
	if ( string::npos != loc ) {
		fndir.assign(fnsrc.begin(), fnsrc.begin() + loc + 1);
		fnfile.assign(fnsrc.begin() + loc + 1, fnsrc.end());
	}

	// retrieve extension from fnsrc if needed
	string strname(fnfile);
	if ( "" != strext ) { 
		loc = fnfile.rfind('.');
		if ( string::npos != loc ) {
			strname.assign(fnfile.begin(), fnfile.begin() + loc + 1);
			strname += strext;
		}
	}

	ostringstream ostrfn;
	ostrfn << fndir << prefix << "_";
	if ( usetime ) {
		ostrfn << time(NULL) << "_";
	}
	ostrfn << strname << ends;
	fnout = ostrfn.str();
	return 0;
}

/* set ts=4 sts=4 tw=80 sw=4 */

