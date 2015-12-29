// ----------------------------------------------------------------------------
// DCMviewer.h : a class dedicated exclusively to act as a DICOM viewer, either
//				for a single DICOM image or a set of DICOM images in a directory
//
// Creation : Mar. 1st 2011
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------

#include "DCMviewer.h"

#include <sys/types.h>
#include <dirent.h>

#include <algorithm>

using std::cerr;
using std::cout;
using std::ostringstream;
using std::max;
using std::min;

//////////////////////////////////////////////////////////////////////////////
//
//	Implementation of the Class CSingleDcmViewer
//
CSingleDcmViewer::CSingleDcmViewer(const char* pfn) :
	CGLGadget(),
	m_strfn( pfn ),
	m_dims(NULL),
	m_buffer(NULL),
	m_imgbuf(NULL),
	m_bLoaded(false),
	m_imgz(0.0f)
{
}

CSingleDcmViewer::~CSingleDcmViewer()
{
	_destroy_mem();
}

int CSingleDcmViewer::setFile(const char* pfn)
{
	if ( m_strfn == pfn ) {
		return 0;
	}

	if ( 0 != access ( pfn, F_OK|R_OK ) ) {
		cerr << "Can not access to file " << pfn << "\n";
		return -1;
	}

	// a new file ready to be loaded
	m_strfn = pfn;
	m_bLoaded = false;
	_destroy_mem();
	return 0;
}

int CSingleDcmViewer::load()
{
	if ( m_bLoaded ) {
		return 0;
	}

	ImageReader reader;
	reader.SetFileName ( m_strfn.c_str() );
	if ( !reader.Read() ) {
		cerr << "failed to read " << m_strfn << "\n";	
		return -1;
	}

	const Image& image = reader.GetImage();
	const gdcm::Pixmap& pmap = reader.GetPixmap();

	pmap.Print( cout );
	unsigned long sz = image.GetBufferLength();
	m_buffer = new char[ sz ];
	if ( !pmap.GetBuffer( (char*)m_buffer ) ) {
		cerr << "failed to read raw image data.\n";
		return -1;
	}
	cerr << "Buffer retrieved successfully [sz = " << sz << " bytes].\n";

	unsigned int nDim = image.GetNumberOfDimensions();
	if ( nDim  < 2 ) {
		cerr << "only " << nDim  << " dimensions, wrong source image.\n";
		return -1;
	}

	const unsigned int *dims = image.GetDimensions();
	m_dims = new unsigned int [ nDim ];
	memcpy(m_dims, dims, nDim * sizeof (unsigned int));

	// try to convert the image format as far as possible
	if ( image.GetPhotometricInterpretation() == 
			gdcm::PhotometricInterpretation::MONOCHROME2 &&
		image.GetPixelFormat() == gdcm::PixelFormat::UINT16 ) {
		_convert_to_rgb();
	}

	m_bLoaded = true;
	cerr << "DICOM IMAGE loaded: " << m_strfn << "\n";

	return 0;
}

/*
 * for the specific use of the embedded DWI image,
 * it is usually necessary to place the image at such a position 
 * that it is cutting through the XY plane, so we need use a same
 * viewing, besides the modeling, transformation as that for the primary DTI
 * model. This is why we need to overload the display routine of CGLGadget
 */
void CSingleDcmViewer::display()
{
	if ( ! m_bEnabled ) {
		return;
	}

	draw();
	/*
	m_dx = m_dims[0],  m_dy = m_dims[1], m_dz = 1;
	CGLGadget::display();
	*/
}

void CSingleDcmViewer::draw()
{
	if ( !m_bLoaded && 0 != load() ) {
		return;
	}

	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glDisable( GL_CULL_FACE );
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);

	/*
	glTranslatef( - (m_dims[0]/2.0), - (m_dims[1]/2.0), .0 );
	glRectd(0, 0, m_dims[0], m_dims[1]);
	glRasterPos2i(0, 0);
	glDrawPixels(m_dims[0], m_dims[1], GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4_REV, m_buffer);
	*/

	/* CREATE TEXTURE FROM DICOM image*/
	GLuint texture_id;
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	if ( m_imgbuf ) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 
					m_dims[0], m_dims[1],
					0, GL_LUMINANCE, 
					GL_UNSIGNED_BYTE, m_imgbuf);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
					m_dims[0], m_dims[1],
					0, GL_RGBA, 
					GL_UNSIGNED_SHORT_4_4_4_4_REV, m_buffer);
	}

	//glTranslatef( - m_dx, - m_dy, .0 );
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); 
		glVertex3f(-m_dx, -m_dy, m_imgz);
		
		//glTexCoord2f(m_dims[0], 0.0); 
		glTexCoord2f(1.0, 0.0); 
		//glVertex3f( m_dims[0], 0.0, 0.0);
		glVertex3f( m_dx, -m_dy, m_imgz);
		
		//glTexCoord2f(m_dims[0], m_dims[1]); 
		glTexCoord2f(1.0, 1.0); 
		//glVertex3f( m_dims[0], m_dims[1], 0.0);
		glVertex3f( m_dx, m_dy, m_imgz);

		//glTexCoord2f(0.0, m_dims[1]); 
		glTexCoord2f(0.0, 1.0); 
		//glVertex3f(0.0, m_dims[1], 0.0);
		glVertex3f(-m_dx, m_dy, m_imgz);
	glEnd(); 

	glPopAttrib();
	glPopMatrix();
}

void CSingleDcmViewer::updateImageZ(GLdouble imgz)
{
	m_imgz = imgz;
}

void CSingleDcmViewer::_destroy_mem()
{
	delete [] m_buffer;
	m_buffer = NULL;

	delete [] m_dims;
	m_dims = NULL;

	delete [] m_imgbuf;
	m_imgbuf = NULL;
}

void CSingleDcmViewer::_convert_to_rgb()
{
	if (!m_buffer) {
		return;
	}

	// we assume the photometric interpretation is MONOCHROME2
	// and pixel format is UINT16
	
	m_imgbuf = new unsigned char[ m_dims[0]*m_dims[1] ];
	unsigned short *porgbuf = (unsigned short*)m_buffer;
	unsigned int cnt = 0;
	for (unsigned int i = 0; i < m_dims[0]; ++i) {
		for (unsigned int j = 0; j < m_dims[1]; ++j) {
			//cout << (*porgbuf) << " ";
			m_imgbuf[ cnt ++ ] = *porgbuf;
			porgbuf ++;
		}
	}

	cout << " Image data converted.\n ";
	delete [] m_buffer;
	m_buffer = NULL;
}

//////////////////////////////////////////////////////////////////////////////
//
//	Implementation of the Class CDcmExplorer
//
CDcmExplorer::CDcmExplorer(const char* pdn) :
	CGLGadget(),
	m_strdir(pdn),
	m_curfnIdx(-1),
	m_bLoaded(false)
{
	// be default, DICOM viewer holds the whole viewport as the primary object
	// (DTI Model,say) while it is working as a gadget
	m_nViewRatio = 1;
	m_fvoy = 100.0f;
	m_bTranslate = true;
	m_bRotate = true;
}

CDcmExplorer::~CDcmExplorer()
{
}

int CDcmExplorer::setDir(const char* pdn)
{
	if (m_strdir == pdn) {
		return 0;
	}

	if ( 0 != access ( pdn, F_OK|R_OK|X_OK ) ) { 
		// to list a directory, x permission must be available
		cerr << "Can not access to directory " << pdn << "\n";
		return -1;
	}

	// now a new directory is ready to read
	m_strdir = pdn;
	m_fns.clear();
	m_bLoaded = false;
	m_curfnIdx = -1;
	return 0;
}

int CDcmExplorer::load()
{
	if ( m_bLoaded ) {
		return 0;
	}

	DIR *pdir = opendir( m_strdir.c_str() );
	if ( !pdir ) {
		cerr << "Failed to open " << m_strdir << " as a directory.\n";
		return -1;
	}

	dirent *pent;
	while ( NULL != (pent = readdir(pdir)) ) {
		// skip "." and "..", the current and parent directory,respectively
		if ( 0 == strncmp(pent->d_name, ".", 1) ||
			 0 == strncmp(pent->d_name, "..", 2) ) {
			continue;
		}

		ostringstream osfulldir;
		osfulldir << m_strdir << "/" << pent->d_name;
		m_fns.push_back( osfulldir.str() );
		cout << "\nLoading " << osfulldir.str() << " -- OK.";
	}

	std::sort(m_fns.begin(), m_fns.end());

	closedir (pdir);

	cout << "\nTotally " << m_fns.size() << " files found in " << m_strdir << "\n";

	// an empty directory will also be able to undermine the procedure
	// afterwards, so it is still set as "not loaded" as a precaution if it is
	// the case
	m_bLoaded = (0 < m_fns.size());

	// always starting from the first file
	m_curfnIdx = m_bLoaded?0:-1;

	// now if the directory has been read successfully, get the agent ready
	/* WE USE THE SAME CGLGadget properties (its data member values) for all the
	 * DICOM images in a specific directory by default, and any change on the
	 * the gadget property wil not be effectual until the directory is updated
	 */
	if ( m_curfnIdx >= 0 ) {
		// just relay all the following actions to the CSingleDcmViewer agent
		m_dcmAgent.setParams(m_bEnabled, 
				m_left, m_bottom,
				m_width, m_height, m_bTranslate, m_bRotate);

		m_dcmAgent.setViewportRatio(m_nViewRatio);
		m_dcmAgent.setfvoy(m_fvoy);
		m_dcmAgent.setVertexCoordRange(m_dx, m_dy, m_dz);

		// starting from the central image in terms of the file name order
		m_curfnIdx = (int)m_fns.size()/2;
	}

	return m_bLoaded?0:-1;
}

void CDcmExplorer::display()
{
	if ( !m_bLoaded || m_curfnIdx < 0 ) {
		return;
	}

	GLdouble imgz = (m_curfnIdx - (int)m_fns.size()/2) * (m_dz*2)/m_fns.size();
	m_dcmAgent.updateImageZ( imgz );
	m_dcmAgent.display();
}

void CDcmExplorer::draw()
{
	m_dcmAgent.draw();
}

bool CDcmExplorer::switchTranslucent()
{
	return m_dcmAgent.switchTranslucent();
}

bool CDcmExplorer::start()
{
	if ( !m_bLoaded && 0 != load() ) {
		return false;
	}

	if ( m_curfnIdx < 0 || m_curfnIdx >= (int)m_fns.size() ) {
		return false;
	}

	if ( 0 != m_dcmAgent.setFile( m_fns[ m_curfnIdx ].c_str() ) ) {
		return false;
	}

	return 0 == m_dcmAgent.load();
}

bool CDcmExplorer::next()
{
	// we adapt an alternating viewing by default
	m_curfnIdx = (m_curfnIdx + 1) % (int)m_fns.size();
	
	return start();
}

bool CDcmExplorer::prev()
{
	// we adapt an alternating viewing by default
	if ( 0 == m_curfnIdx ) {
		m_curfnIdx = (int)m_fns.size() - 1;
	}
	else {
		m_curfnIdx --;
	}
	
	return start();
}

const char* CDcmExplorer::getCurFileName() const
{
	if ( m_curfnIdx < 0 || m_curfnIdx >= (int)m_fns.size() ) {
		return "~N/A~";
	}
	return m_fns[ m_curfnIdx ].c_str();
}

/* set ts=4 sts=4 tw=80 sw=4 */

