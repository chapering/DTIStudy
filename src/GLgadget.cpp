// ----------------------------------------------------------------------------
// GLgadget.cpp : a class dedicated exclusively to describe a gadget that is
//				expected to take part in an openGL driven visualization
//
// Creation : Feb. 25th 2011
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#include "GLgadget.h"
#include "colorscheme.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
//
// implementation of the CGLGadget class
//
CGLGadget::CGLGadget()
{
	// set a set of default parameters
	setParams();
	m_bTranslucent = false;
	m_nViewRatio = 5.0;
	m_fvoy = 25.0f;
}

CGLGadget::~CGLGadget()
{
}

void CGLGadget::setParams(bool enabled, 
						int left, int bottom, 
						int width, int height,
						bool bTranslate, bool bRotate)
{
	m_bEnabled = enabled;
	m_left = left, m_bottom = bottom;
	m_width = width, m_height = height;
	m_bTranslate = bTranslate;
	m_bRotate = bRotate;

	m_bUseStaticWidth = (-1 != m_width);
	m_bUseStaticHeight = (-1 != m_height);
}

void CGLGadget::setVertexCoordRange(GLdouble dx, GLdouble dy, GLdouble dz)
{
	m_dx = dx, m_dy = dy, m_dz = dz;
}

void CGLGadget::draw()
{
}

GLboolean CGLGadget::switchTranslucent()
{
	return (m_bTranslucent = !m_bTranslucent);
}

void CGLGadget::display()
{
	if ( ! m_bEnabled ) {
		return;
	}

	GLint viewport[4];
	GLdouble color[4];
	GLfloat mvmat[16];
	GLdouble maxdist = max( max(m_dx, m_dy), m_dz );
	GLdouble mindist = min( min(m_dx, m_dy), m_dz );

	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_CURRENT_COLOR, color);
	glGetFloatv(GL_MODELVIEW_MATRIX, mvmat);
	for (int i=0;i<4;i++) {
		for (int j=0;j<4;j++) {
			// if only want the rotation
			if ( !m_bTranslate ) {
				if (j==3)  {
					mvmat[j*4+i] = 0.0; // remove the translation
				}
			}
			else { 
				// the translation vector is 
				// actually holding the viewing transformation information
				mvmat[14] = .0;
			}

			// if only want the translation
			if ( !m_bRotate) { 
				if (i==j) {
					mvmat[j*4+i] = 1.0; // remove the rotation 
				}
				else if (i<3 && j<3) {
					mvmat[j*4+i] = 0.0; // remove the rotation 
				}
			}
		}
	}

	// set defaults if necessary
	if ( !m_bUseStaticWidth ) {
		m_width = viewport[2] / m_nViewRatio;
	}
	if ( !m_bUseStaticHeight ) {
		m_height = m_width / ( (GLdouble)viewport[2]/(GLdouble)viewport[3] ); 
	}

	/* just this statement rescued me out of the long frustration ...
	 * ignornance of that the bottom row of the modelview matrix should always
	 * be (0,0,0,1) has been miring me for more than 6 hours!!
	 */
	mvmat[15] = 1;

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

	//cout << "USED: (" << m_left << "," << m_bottom << ") - (" << m_width << "," << m_height << ")\n";
	glViewport(m_left, m_bottom, m_width, m_height);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective( m_fvoy /* 2*atan2(m_dy, m_dz*8 )*180/M_PI */,
			(GLdouble)m_width / (GLdouble)m_height,
			mindist/8.0, 4.0*maxdist); 

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	gluLookAt(0, 0, maxdist*2.0, 0, 0, 0, 0, 1, 0);

	glMultMatrixf(mvmat);
	//glMultTransposeMatrixf(mvmat);

	if (m_bTranslucent) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else {
		glDisable(GL_BLEND);
	}

	draw();

	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();

	glColor4dv(color);
}

void CGLGadget::setViewportRatio(int ratio)
{
	m_nViewRatio = ratio;
}

void CGLGadget::setfvoy(GLfloat fvoy)
{
	m_fvoy = fvoy;
}

///////////////////////////////////////////////////////////////////////////////
//
// implemenation of the CGLAxis class
//
CGLAxis::CGLAxis()
	:CGLGadget()
{
	setColor(0, 1.0, 0.0, 0.0);
	setColor(1, 0.0, 1.0, 0.0);
	setColor(2, 0.0, 0.0, 1.0);
	setParams(true, 0, 0, -1, -1, false, true);
}

CGLAxis::~CGLAxis()
{
}

void CGLAxis::setColor(int idx, GLclampf r, GLclampf g, GLclampf b, GLclampf a)
{
	if ( -1 == idx ) {
		for (int i = 0; i < 3; i++) {
			setColor(i, r, g, b, a);
		}
		return;
	}

	if ( idx >= 0 && idx <= 2 ) {
		m_colors[idx][0] = r;
		m_colors[idx][1] = g;
		m_colors[idx][2] = b;
		m_colors[idx][3] = a;
	}
}

// draw the axes in the bottom-left corner of the screen to orient the user
// so as to facilitate interaction especially in the cases of complex
// objects
void CGLAxis::draw()
{
	GLfloat origin[] = {0.0, 0.0, 0.0};
	glLineWidth(1.0);
	glEnable(GL_LINE_SMOOTH);

	GLdouble d = min( min(m_dx, m_dy), m_dz )/2.0;
	//glTranslatef(-m_dx/2.0, -m_dy/2.0, -m_dz/2.0);
	glBegin(GL_LINES);
		// x-axis
		glColor4fv(m_colors[0]);
		glVertex3fv(origin);
		glVertex3f(d, .0, .0);

		// y-axis
		glColor4fv(m_colors[1]);
		glVertex3fv(origin);
		glVertex3f(.0, d, .0);

		// z-axis
		glColor4fv(m_colors[2]);
		glVertex3fv(origin);
		glVertex3f(0.0, 0.0, d);
	glEnd();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);

	// arrow at the end of x-axis 
	glColor4fv(m_colors[0]);
	_drawArrow(point_t(0,0,0), point_t(d, 0, 0), 3);
	printText("X", d + 1, 0, 0, 
			m_colors[0][0], m_colors[0][1], m_colors[0][2], m_colors[0][3]);

	// arrow at the end of y-axis 
	glColor4fv(m_colors[1]);
	_drawArrow(point_t(0,0,0), point_t(0, d, 0), 3);
	printText("Y", 0, d + 1, 0,
			m_colors[1][0], m_colors[1][1], m_colors[1][2], m_colors[1][3]);

	// arrow at the end of z-axis 
	glColor4fv(m_colors[2]);
	_drawArrow(point_t(0,0,0), point_t(0, 0, d), 3);
	printText("Z", 0, 0, d + 1,
			m_colors[2][0], m_colors[2][1], m_colors[2][2], m_colors[2][3]);
}

//void CGLAxis::_drawArrow(const point_t& p1, const point_t& p2, GLdouble sz, GLdouble angle)
void CGLAxis::_drawArrow(const point_t& p1, const point_t& p2, GLdouble sz)
{
	point_t vn = (p2 - p1).normalize();
	point_t l(-vn.y, vn.x, 0), r(vn.y, -vn.x, 0);
	if ( fabs(vn.z) > 1e-6 ) {
		//l.update(0, -vn.z, vn.y);
		//r.update(0, vn.z, -vn.y);
		l.update(-vn.z, 0, vn.y);
		r.update(vn.z, 0, -vn.y);
	}

	l = (vn+l).normalize();
	r = (vn+r).normalize();

	l = p2-l*sz;
	r = p2-r*sz;
	
	glBegin(GL_POLYGON);
		glVertex3f(p2.x, p2.y, p2.z);
		glVertex3f(l.x, l.y, l.z);
		glVertex3f(r.x, r.y, r.z);
	glEnd();

}

///////////////////////////////////////////////////////////////////////////////
//
// implemenation of the CAnatomyAxis class
//
#define RESCALE_COLOR(idx,negative)							\
	r = m_colors[idx][0] * (negative?-1:);					\
	g = m_colors[idx][1] * (negative?-1:1);					\
	b = m_colors[idx][2] * (negative?-1:1);					\
	r = fabs(r);											\
	g = (1+g)/2.0;											\
	b = (1+b)/2.0;											

#define COLORING(x,y,z)										\
	cm.getColor(x, y, z, r, g, b);                          \
	glColor4f( r, g, b, 0.6f );

CAnatomyAxis::CAnatomyAxis() :
	CGLAxis(),
	m_colorschemeIdx(CLSCHM_CUSTOM)
{
}

CAnatomyAxis::~CAnatomyAxis()
{
}

void CAnatomyAxis::draw()
{
	GLfloat origin[] = {0.0, 0.0, 0.0};
	glLineWidth(1.0);
	glEnable(GL_LINE_SMOOTH);

	GLdouble d = min( min(m_dx, m_dy), m_dz )/2.0;
	CColorMapper<GLfloat> cm(m_colorschemeIdx);
	GLfloat r,g,b;	

	glBegin(GL_LINES);
		// right-axis
		//glColor4fv(m_colors[0]);
		COLORING(d, .0, .0);
		glVertex3fv(origin);
		glVertex3f(d, .0, .0);

		// left-axis
		//glColor4fv(m_colors[0]);
		COLORING(-d, .0, .0);
		glVertex3fv(origin);
		glVertex3f(-d, .0, .0);

		// superior-axis
		//glColor4fv(m_colors[1]);
		COLORING(.0, d, .0);
		glVertex3fv(origin);
		glVertex3f(.0, d, .0);

		// inferior-axis
		//glColor4fv(m_colors[1]);
		COLORING(.0, -d, .0);
		glVertex3fv(origin);
		glVertex3f(.0, -d, .0);

		// anterior-axis
		//glColor4fv(m_colors[2]);
		COLORING(.0, .0, d);
		glVertex3fv(origin);
		glVertex3f(0.0, 0.0, d);

		// posterior-axis
		//glColor4fv(m_colors[2]);
		COLORING(.0, .0, -d);
		glVertex3fv(origin);
		glVertex3f(0.0, 0.0, -d);
	glEnd();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);

	GLdouble labelOffset = 2.0;
	// arrow at the end of right-axis 
	//glColor4fv(m_colors[0]);
	COLORING(d, .0, .0);
	_drawArrow(point_t(0,0,0), point_t(d, 0, 0), 3);
	printText("R", d + labelOffset, 0, 0, r, g, b);

	// arrow at the end of left-axis 
	//glColor4fv(m_colors[0]);
	COLORING(-d, .0, .0);
	_drawArrow(point_t(0,0,0), point_t(-d, 0, 0), 3);
	printText("L", -(d + labelOffset), 0, 0, r, g, b);

	// arrow at the end of superior-axis 
	//glColor4fv(m_colors[1]);
	COLORING(.0, d, .0);
	_drawArrow(point_t(0,0,0), point_t(0, d, 0), 3);
	printText("A", 0, d + labelOffset, 0, r, g, b);

	// arrow at the end of inferior-axis 
	//glColor4fv(m_colors[1]);
	COLORING(.0, -d, .0);
	_drawArrow(point_t(0,0,0), point_t(0, -d, 0), 3);
	printText("P", 0, -(d + labelOffset), 0, r, g, b);

	// arrow at the end of anterior-axis 
	//glColor4fv(m_colors[2]);
	COLORING(.0, .0, d);
	_drawArrow(point_t(0,0,0), point_t(0, 0, d), 3);
	printText("S", 0, 0, d + labelOffset, r, g, b);

	// arrow at the end of anterior-axis 
	//glColor4fv(m_colors[2]);
	COLORING(.0, .0, -d);
	_drawArrow(point_t(0,0,0), point_t(0, 0, -d), 3);
	printText("I", 0, 0, -(d + labelOffset), r, g, b);
}

void CAnatomyAxis::setColorScheme(int nScheme)
{
	if ( nScheme != m_colorschemeIdx ) {
		m_colorschemeIdx = nScheme;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
// implemenation of the CSphereColorMap class
//
CSphereColorMap::CSphereColorMap(GLdouble radius, GLint slices, GLint stacks) :
	CGLGadget(),
	m_pData(NULL),
	m_sz(0),
	m_radius(radius),
	m_slices(slices),
	m_stacks(stacks),
	m_colorschemeIdx(0)
{
	setParams(true, 0, 0, -1, -1, false, true);
}

CSphereColorMap::~CSphereColorMap()
{
	delete [] m_pData;
	m_pData = NULL;
	m_sz = 0;
}

void CSphereColorMap::setColorScheme(int nScheme)
{
	if ( nScheme != m_colorschemeIdx ) {
		m_colorschemeIdx = nScheme;
		_calcShpereGeometry();
	}
}

void CSphereColorMap::updateSphereParams(GLdouble radius, GLint slices, GLint stacks)
{
	bool bRecalc = m_radius != radius ||
					m_slices != slices ||
					m_stacks != stacks;

	m_radius = radius;
	m_slices = slices;
	m_slices = stacks;

	// when things change, recomputation becomes a necessity
	if ( bRecalc ) {
		_calcShpereGeometry();
	}
}

void CSphereColorMap::draw()
{
	if ( !m_pData ) { // take as where to initialize here
		_calcShpereGeometry();
	}

	glShadeModel( GL_SMOOTH );
	glEnable    ( GL_DEPTH_TEST );

	/*
	GLfloat d = m_dz*4.0;
	GLfloat pos[][4] = {
		{0, 0, d, 1},
		{0, 0, -d, 1},
		{d, 0, 0, 1},
		{-d, 0, 0, 1},
		{0, d, 0, 1},
		{0, -d, 0, 1}
	};

	glEnable(GL_LIGHTING);
	for (GLenum i = 0; i < 6; ++i) {
		glEnable(GL_LIGHT0 + i);
		glLightfv(GL_LIGHT0 + i, GL_POSITION, pos[i]);
	}
	*/

	glPushClientAttrib( GL_ALL_CLIENT_ATTRIB_BITS );
#ifdef _PATCH_CRACK
	glPointSize(1.0);
#endif

	/*
	glEnableClientState( GL_NORMAL_ARRAY );
	glEnable(GL_NORMALIZE);
	glEnable(GL_RESCALE_NORMAL);
	glNormalPointer(GL_FLOAT, 0, m_pData);
	*/
	glInterleavedArrays(GL_C3F_V3F, 0, m_pData);
#ifdef _PATCH_CRACK
	glDrawArrays(GL_POINTS, 0, m_sz/6);
#else
	glDrawArrays(GL_LINES, 0, m_sz/6);
#endif

	glPopClientAttrib();
}

int CSphereColorMap::_calcShpereGeometry()
{
	if ( m_pData ) {
		delete [] m_pData;
		m_pData = NULL;
	}
	
	// decide the correct value of either m_slices or m_stacks
	//
	if ( m_slices < 0 ) {
		// make an semi-unit slicing
		m_slices = static_cast<int> ( ceil( m_dx*20 ) );
	}

	if ( m_stacks < 0 ) {
		/// make a semi-unit stacking
		m_stacks = static_cast<int> ( ceil( m_dz*20 ) );
	}

	GLdouble d = min( min(m_dx, m_dy), m_dz )/3.0;
#ifdef _PATCH_CRACK
	m_slices = m_stacks = 2*d/.025;
#else
	m_slices = m_stacks = 2*d/.1;
#endif

	if ( m_radius < 1 ) {
		// make a minimal sphere inscribed in the bounding cube defined by m_dx,
		// m_dy and m_dz
		m_radius = d;
	}

	/*
	cout << "m_radius = " << m_radius << 
			" m_slices = " << m_slices <<
			" m_stacks = " << m_stacks << "\n";
	*/

#ifdef _PATCH_CRACK
	m_sz = ( (2 + m_slices) * (2 + m_stacks) * 3 ) * 6;
#else
	m_sz = ( (2 + m_slices) * (2 + m_stacks) * 2 ) * 6;
#endif

	// reclaim memory to hold vertices and color information needed
	try {
		m_pData = new GLfloat [ m_sz ];
	}
	catch (std::bad_alloc & e) {
		cerr << "FATAL : failed to allocate memory in generating sphere geometry.\n";
		return -1;
	}

	GLfloat xstep = 2*d / m_slices;
	GLfloat zstep = 2*d / m_stacks;
	GLfloat x = -d, y = .0, z = -d;

	// instead of a triangulation or subdivision approach, here a simple
	// construction of sphere by precisely plane mapping using the standard
	// sphere equation x^2 + y^2 + z^2 = radius^2
	GLuint nVertices = 0;
	//for (x = -d; (int)x <= d; x += xstep ) {
	for (GLint i=0; i<m_slices+2; i++) {

		// for the sake of imperfect floating precision, neither x nor y would never
		// extactly reach 0 throughout the loop above
		if ( fabs(x) <= 1e-6 ) {
			x = .0;
		}

		if ( fabs( x - d ) <= 1e-6 ) {
			x = d;
		}
		//for (z = -d; (int)z <= d; z += zstep ) {
		z = -d;
		for (GLint j=0; j<m_stacks+2; j++) {
			if ( fabs(z) <= 1e-6 ) {
				z = .0;
			}
			if ( fabs( z - d ) <= 1e-6 ) {
				z = d;
			}
			//cout << "x= " << x << ", z=" << z << "\n";

			if ( x*x + z*z > m_radius*m_radius ) {
				z += zstep;
				continue;
			}
			if ( m_radius*m_radius - x*x - z*z < 0 ) {
				cout << "invalid parameters for the sphere, aborted.\n";
				return -1;
			}

			// always place the sphere centered at the origin
			y = sqrt ( m_radius*m_radius - x*x - z*z );
			if ( fabs(y) <= 1e-6 ) {
				y = .0;
			}

			_place_a_vertex(nVertices++, x, y, z);
			_place_a_vertex(nVertices++, x, -y, z);

			z += zstep;
		}
		x += xstep;
	}

	// there might be crack in the xz plane, thus here goes following two
	// patching geometry generation

#ifdef _PATCH_CRACK
	x = d, y = .0;
	for (GLint i=0; i<m_slices+2; i++) {
		z = sqrt ( m_radius*m_radius - x*x - y*y );

		_place_a_vertex(nVertices++, x, y, z);
		_place_a_vertex(nVertices++, x, y, -z);

		x-=xstep;
	}

	z = d, y = .0;
	for (GLint i=0; i<m_stacks+2; i++) {
		x = sqrt ( m_radius*m_radius - z*z - y*y );

		_place_a_vertex(nVertices++, x, y, z);
		_place_a_vertex(nVertices++, -x, y, z);

		z-=zstep;
	}

	y = -min(10.0,d), z = -d;
	for (GLint i=0; i< (int)(2*min(10.0,d)/xstep); i++) {

		// for the sake of imperfect floating precision, neither x nor y would never
		// extactly reach 0 throughout the loop above
		if ( fabs(y) <= 1e-6 ) {
			y = .0;
		}

		if ( fabs( y - d ) <= 1e-6 ) {
			y = d;
		}
		//for (z = -d; (int)z <= d; z += zstep ) {
		z = -d;
		for (GLint j=0; j<m_stacks+2; j++) {
			if ( fabs(z) <= 1e-6 ) {
				z = .0;
			}
			if ( fabs( z - d ) <= 1e-6 ) {
				z = d;
			}
			//cout << "x= " << x << ", z=" << z << "\n";

			if ( y*y + z*z > m_radius*m_radius ) {
				z += zstep;
				continue;
			}
			if ( m_radius*m_radius - y*y - z*z < 0 ) {
				cout << "invalid parameters for the sphere, aborted.\n";
				return -1;
			}

			// always place the sphere centered at the origin
			x = sqrt ( m_radius*m_radius - y*y - z*z );
			if ( fabs(x) <= 1e-6 ) {
				x = .0;
			}

			_place_a_vertex(nVertices++, -x, y, z);
			_place_a_vertex(nVertices++, x, y, z);

			z += zstep;
		}
		y += xstep;
	}
#endif

	cerr << nVertices << " vertices produced for the Color Map Sphere [ " << 
		(m_sz - nVertices*6) * sizeof (GLfloat) << " bytes wasted ]. \n";

	/*
	GLsizei sz_cut = m_sz - nVertices*6;
	GLfloat* mp = m_pData + nVertices*6;
	for (GLsizei i = 0; i < sz_cut; ++i) {
		delete  mp;
		mp += i;
	}
	*/
	m_sz = nVertices*6;

	return nVertices;
}

int CSphereColorMap::_place_a_vertex(int vertexIndex, GLfloat x, GLfloat y, GLfloat z)
{
	CColorMapper<GLfloat> cm(m_colorschemeIdx);
	GLfloat r, g, b;
	cm.getColor(x, y, z, r, g, b);

	m_pData[vertexIndex * 6 + 0] = r;
	m_pData[vertexIndex * 6 + 1] = g;
	m_pData[vertexIndex * 6 + 2] = b;
	m_pData[vertexIndex * 6 + 3] = x;
	m_pData[vertexIndex * 6 + 4] = y;
	m_pData[vertexIndex * 6 + 5] = z;

	/*
	cout << "(" << x << "," << y << "," << z << "), ("
		<< r << "," << g << "," << b << ").\n";
	*/

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// implementation of the CGLTextbox class
//
CGLTextbox::CGLTextbox():
	CGLGadget(),
	m_font(GLUT_BITMAP_8_BY_13),
	m_tw(-1),
	m_th(-1),
	DEFAULT_FONT_HEIGHT(13),
	m_rasterPosOffx(0),
	m_rasterPosOffy(0)
{
	setColor(1.0, 1.0, 1.0);
	setParams(true, 0, 0, -2, -2, false, false);
	m_fvoy = 45;
	//m_nViewRatio = 2.0;
}

CGLTextbox::~CGLTextbox()
{
}

void CGLTextbox::addLine(const string& line)
{
	m_lines.push_back( line );
}

void CGLTextbox::clear()
{
	m_lines.clear();
}

int CGLTextbox::loadFromFile(const char* fn)
{
	if ( 0 != access ( fn, F_OK|R_OK ) ) { 
		cerr << "Can not access to file " << fn << "\n";
		return -1;
	}

	ifstream ifs(fn);
	string line;
	int icnt = 0;
	while ( ifs.good() ) {
		std::getline(ifs, line);
		// skip the annotated but the empty lines
		if (0 == line.compare(0, 1, "#") ||
			0 == line.compare(0, 2, "//") ||
			0 == line.compare(0, 2, "/*") ) {
			continue;
		}
		addLine(line);
		icnt ++;
	}

	return icnt;
}

void CGLTextbox::setColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a)
{
	m_color[0] = r, m_color[1] = g, m_color[2] = b;
}

void CGLTextbox::setFont(void* font)
{
	m_font = font;
}

void CGLTextbox::setRasterPosOffset(int offx, int offy)
{
	m_rasterPosOffx = offx;
	m_rasterPosOffy = offy;
}

void CGLTextbox::display()
{
	if ( -1 == m_th && -1 == m_tw ) {
		_calcViewport();
	}
	else {
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		m_bottom = viewport[3] - m_th;
		if ( m_bottom < 0 ) {
			m_bottom = 0;
		}
	}
	CGLGadget::display();
}

void CGLTextbox::draw()
{
	GLdouble dmin = min(m_dx, min(m_dy, m_dz));
	GLdouble ystep = 2*dmin / (m_lines.size()+2),
		y = dmin + m_rasterPosOffy,
		x = -dmin - m_rasterPosOffx;
	for (size_t i = 0; i < m_lines.size(); ++i) {
		/* the sloppy way by swapping can cause flickering effect during scene
		 * updating, as is quite undesirable.
		 */
		//swap(m_color[0], m_color[2]);
		printText(m_lines[i].c_str(), x, y, 0, 
				m_color[i%2==0?2:0], m_color[1], 
				m_color[i%2==0?0:2], m_color[3], 
				m_font);
		y -= 1+ystep;
	}
}

void CGLTextbox::_calcViewport()
{
	/* take the maximal length among all lines as the width, and 
	 * sum of height as the height, of the final viewport
	 */
	int maxw = 0, curw;
	for (size_t i = 0; i < m_lines.size(); ++i) {
		curw = glutBitmapLength(m_font, (const unsigned char*)m_lines[i].c_str());
		if ( curw > maxw ) {
			maxw = curw;
		}
	}

	// --- TOO MUCH TIME WASTED ON TWEAKING THE DEFAULT INITIAL POSITION:
	// --- AT THE UPPER LEFT CORNER OF THE WINDOW, MORE THAN 3 HOURS!
	// --- IT IS STILL IMPERFECT, SOMETHING MUST BE UNCLEAR TO ME.
	/* vertical border size = font height*2 */
	m_th = DEFAULT_FONT_HEIGHT * ( m_lines.size() + 2 );

	/* horizontal border size = font width */
	m_tw = maxw + glutBitmapWidth(m_font, 'W')*2;

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	m_th = min( viewport[3], m_th );
	m_tw = min( viewport[2], m_tw );
	/*
	int dmax = 2*int(m_dx);
	m_tw = max(dmax, m_tw);
	*/
	setParams(true, 0, viewport[3] - m_th, m_tw, m_th, false, false);

	/*
	cout << "(" << m_left << "," << m_bottom << ") - (" << m_width << "," << m_height << ")\n";
	cout << "(" << viewport[0]<< "," << viewport[1]<< ") - (" << viewport[2]<< "," << viewport[3]<< ")\n";
	*/
}

///////////////////////////////////////////////////////////////////////////////
//
// implementation of the CGLTaskbox class
//
CGLTaskbox::CGLTaskbox() :
	CGLTextbox(),
	m_bShowCover(true),
	m_curTaskId(-1)
{
	setFont( GLUT_BITMAP_TIMES_ROMAN_24);
	setColor(1.0, 1.0, 1.0);
	DEFAULT_FONT_HEIGHT = 24;
	m_fvoy = 40;
}

CGLTaskbox::~CGLTaskbox()
{
}

void CGLTaskbox::addLine(const string& line)
{
	if ( -1 == m_curTaskId ) {
		// for the first task
		m_curTaskId = 0;
		addTask(CGLTextbox());
	}
	else {
		// each blank line will initiate a new task, i.e. the next task in the
		// task store m_tasks
		if ( "" == line ) {
			addTask(CGLTextbox());
			m_curTaskId ++;
		}
	}
	m_tasks[m_curTaskId].addLine(line);
}

void CGLTaskbox::clear()
{
	for (size_t i = 0; i < m_tasks.size(); ++i) {
		m_tasks[i].clear();
	}
	m_tasks.clear();
}

int CGLTaskbox::loadFromFile(const char* fn)
{
	return CGLTextbox::loadFromFile(fn);
}

void CGLTaskbox::setColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a)
{
	CGLTextbox::setColor(r,g,b,a);
	for (size_t i = 0; i < m_tasks.size(); ++i) {
		m_tasks[i].setColor(r,g,b,a);
	}
}

void CGLTaskbox::setFont(void* font)
{
	CGLTextbox::setFont(font);
	for (size_t i = 0; i < m_tasks.size(); ++i) {
		m_tasks[i].setFont(font);
	}
}

void CGLTaskbox::setRasterPosOffset(int offx, int offy)
{
	CGLTextbox::setRasterPosOffset(offx, offy);
	for (size_t i = 0; i < m_tasks.size(); ++i) {
		m_tasks[i].setRasterPosOffset(offx, offy);
	}
}

void CGLTaskbox::display()
{
	if ( -1 == m_th && -1 == m_tw ) {
		_calcViewport();
	}

	if ( -1 == m_curTaskId ) {
		return;
	}

	// draw the cover if needed
	if ( m_bShowCover ) {
		_drawcover();
	}

	setParams(true, m_tasks[m_curTaskId].m_left, m_tasks[m_curTaskId].m_bottom,
			m_tasks[m_curTaskId].m_width, m_tasks[m_curTaskId].m_height, 
			false, false);
	CGLGadget::display();
}

void CGLTaskbox::draw()
{
	if ( -1 == m_curTaskId ) {
		return;
	}
	// then show the text box describing the task
	m_tasks[m_curTaskId].draw();
}

void CGLTaskbox::addTask(const CGLTextbox& task)
{
	m_tasks.push_back( task );
}

bool CGLTaskbox::next()
{
	if ( m_curTaskId >= int(m_tasks.size()) - 2 ) {
		return false;
	}

	m_curTaskId ++;
	return true;
}

void CGLTaskbox::turncover(bool bon)
{
	m_bShowCover = bon;
}

bool CGLTaskbox::iscovered() const
{
	return m_bShowCover;
}

bool CGLTaskbox::isPrompt() const
{
	// first task is actually a prompt for starting, not a real task
	return 0 == m_curTaskId;
}

void CGLTaskbox::_calcViewport()
{
	// here it is to anchor each of the CGLTextbox object associated with each
	// of the tasks included
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	setColor(m_color[0], m_color[1], m_color[2], m_color[3]);
	setFont(m_font);
	setRasterPosOffset(m_rasterPosOffx, m_rasterPosOffy);
	for (size_t i = 0; i < m_tasks.size(); ++i) {
		m_tasks[i].DEFAULT_FONT_HEIGHT = DEFAULT_FONT_HEIGHT;

		// all tasks share the same vertex coordinate range to make it easy to
		// position the task text box
		m_tasks[i].setVertexCoordRange(m_dx, m_dy, m_dz);

		m_tasks[i]._calcViewport();
		m_tasks[i].setParams(true, 
				(viewport[2] - m_tasks[i].m_tw*2)/2.0, 0, 
				m_tasks[i].m_tw, m_tasks[i].m_th, false, false);
		/*
		cout << "for task " << i << ": m_left= " << m_tasks[i].m_left << 
			" m_bottom = " << m_tasks[i].m_bottom << " m_width= " << m_tasks[i].m_width
			<< " m_height= " << m_tasks[i].m_height << "\n";
		*/
	}

	// simply indicate that we have already calculate the viewport for all tasks
	m_th = viewport[2];
	m_tw = viewport[3];

	// ready to start, from the first task
	if ( m_tasks.size() >= 1 ) {
		m_curTaskId = 0;
	}
}

void CGLTaskbox::_drawcover()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	// the cover is simply a rectangle covering the whole screen
	GLfloat clrcolor[4] = {0.5, 0.1, 0.2, 0.6};
	glGetFloatv(GL_COLOR_CLEAR_VALUE, clrcolor);

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glColor4fv(clrcolor);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_POLYGON);
		glVertex3f(-m_tw, -m_th, 0);
		glVertex3f(m_tw, -m_th, 0);
		glVertex3f(m_tw, m_th, 0);
		glVertex3f(0, m_th, 0);
	glEnd();

	glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////////
//
// implemenation of the CRectColorMap class
//
CRectColorMap::CRectColorMap():
	CGLGadget(),
	m_pData(NULL),
	m_sz(0),
	m_colorschemeIdx(CLSCHM_FA),
	m_ticknum(10),
	m_tickmin(0),
	m_tickmax(1000),
	m_ticklen(5.0f),
	m_markoffset(2.0f)
{
	setParams(true, 0, 0, -1, -1, false, false);
	m_fvoy = 40;
	setColor(0.5f, 1.0f, 0.0f);
	setFont(GLUT_BITMAP_TIMES_ROMAN_10);
}

CRectColorMap::~CRectColorMap()
{
	delete [] m_pData;
	m_pData = NULL;
	m_sz = 0;
}

void CRectColorMap::updateTickParams(GLint ticknum, GLdouble tickmin, 
		GLdouble tickmax, GLdouble ticklen, GLdouble markoffset)
{
	bool bRecalc = m_ticknum != ticknum ||
				m_tickmin != tickmin ||
				m_tickmax != tickmax ||
				m_ticklen != ticklen ||
				m_markoffset != markoffset;
	if ( bRecalc ) {
		_calcRectGeometry();
	}
}

void CRectColorMap::setColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a)
{
	m_color[0] = r, m_color[1] = g, m_color[2] = b;
}

void CRectColorMap::setFont(void* font)
{
	m_font = font;
}

void CRectColorMap::setColorScheme(int nScheme)
{
	if ( nScheme != m_colorschemeIdx ) {
		m_colorschemeIdx = nScheme;
		_calcRectGeometry();
	}
}

void CRectColorMap::display() 
{
	if ( !m_pData ) { // take as where to initialize here
		_calcRectGeometry();

		// set the desirable viewport
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		setParams(true, viewport[2]*8.0/10.0, viewport[3]*1.0/6.0, 
				viewport[2]*1.0/10.0, viewport[3]*2.0/3.0, 
				false, false);
	}

	CGLGadget::display();
}

void CRectColorMap::draw()
{
	glShadeModel( GL_SMOOTH );
	glEnable    ( GL_DEPTH_TEST );

	glPushClientAttrib( GL_ALL_CLIENT_ATTRIB_BITS );
	glInterleavedArrays(GL_C3F_V3F, 0, m_pData);
	glDrawArrays(GL_POINTS, 0, m_sz/6);
	//glDrawArrays(GL_LINES, 0, m_sz/6);
	glPopClientAttrib();

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	_drawticks();
	glPopAttrib();
}

int CRectColorMap::_calcRectGeometry()
{
	if ( m_pData ) {
		delete [] m_pData;
		m_pData = NULL;
	}
	
	GLdouble d = min( min(m_dx, m_dy), m_dz );

	char mark[64];
	mark [ sprintf(mark, "%f", m_tickmax) ] = 0;
	GLint tickmarkwidth = glutBitmapLength(m_font, (const unsigned char*)mark);
	tickmarkwidth += ( m_ticklen*16 + m_markoffset );

	m_sz = 2*(int)(d+1)*10 * (int)((d+1)*2-tickmarkwidth)*10 * 6;

	// reclaim memory to hold vertices and color information needed
	try {
		m_pData = new GLfloat [ m_sz ];
	}
	catch (std::bad_alloc & e) {
		cerr << "FATAL : failed to allocate memory in generating rectangular geometry.\n";
		return -1;
	}

	GLuint nVertices = 0;
	GLdouble x = -d+m_ticklen*14, y = -d;
	GLdouble tickv;
	for (GLint i=0; i<(int)((d+1)*2-tickmarkwidth)*10; i++) {

		// for the sake of imperfect floating precision, neither x nor y would never
		// extactly reach 0 throughout the loop above
		if ( fabs(x) <= 1e-6 ) {
			x = .0;
		}

		if ( fabs( x - d ) <= 1e-6 ) {
			x = d;
		}
		y = -d;
		tickv = m_tickmin;
		for (GLint j=0; j<(int)(d+1)*2*10; j++) {
			if ( fabs(y) <= 1e-6 ) {
				y = .0;
			}
			if ( fabs( y - d ) <= 1e-6 ) {
				y = d;
			}

			_place_a_vertex(nVertices++, x, y, 0.0f, tickv);

			y += .1;
			tickv += ( m_tickmax - m_tickmin ) / int( (d+1)*2*10 );
		}
		x += .1;
	}

	cerr << nVertices << " vertices produced for the Color Map Rectangle[ " << 
		(m_sz - nVertices*6) * sizeof (GLfloat) << " bytes wasted ]. \n";

	m_sz = nVertices*6;

	return nVertices;
}

int CRectColorMap::_place_a_vertex(int vertexIndex, GLfloat x, GLfloat y, GLfloat z, GLdouble tickv)
{
	/*
	CColorMapper<GLfloat> cm(m_colorschemeIdx);
	GLfloat r, g, b;
	if ( 0 != cm.getColor(x, tickv, z, r, g, b) ) {
		r = g = b = 0.5;
	}
	*/

	m_pData[vertexIndex * 6 + 0] = tickv/((m_tickmax-m_tickmin)/255.0)/255.0;
	m_pData[vertexIndex * 6 + 1] = 0;
	m_pData[vertexIndex * 6 + 2] = 0;
	m_pData[vertexIndex * 6 + 3] = x;
	m_pData[vertexIndex * 6 + 4] = y;
	m_pData[vertexIndex * 6 + 5] = z;

	return 0;
}

void CRectColorMap::_drawticks()
{
	if ( m_ticknum <= 0 ) {
		return;
	}

	char mark[64];
	mark [ sprintf(mark, "%f", m_tickmax) ] = 0;
	GLint markwidth = glutBitmapLength(m_font, (const unsigned char*)mark);

	// we presume the FA range is in [1,1000]
	GLdouble d = min( min(m_dx, m_dy), m_dz );
	GLdouble x = d - m_ticklen - m_markoffset - markwidth, 
			 y = -d, ystep = 2*(d+1)/m_ticknum;
	GLdouble tickv = m_tickmin;

	glColor3f( 1.0, 1.0, 1.0 );
	glBegin(GL_LINES);
		glVertex3f(x + m_ticklen, y, 0);
		glVertex3f(x += m_ticklen, y+ystep*m_ticknum, 0);
	glEnd();
	for (GLint i = 0; i <= m_ticknum; i++) {
		// draw the tick
		glBegin(GL_LINES);
			glVertex3f(x, y, 0);
			glVertex3f(x + m_ticklen, y, 0);
		glEnd();

		// draw the mark
		mark[ sprintf(mark, "%.0f", tickv) ] = '\0';
		printText(mark, x + m_ticklen + m_markoffset, y, 0, 
				m_color[0], m_color[1], m_color[2], m_color[3],
				m_font);

		y += ystep;
		tickv += (m_tickmax - m_tickmin)/m_ticknum;
	}
}

/* set ts=4 sts=4 tw=80 sw=4 */

