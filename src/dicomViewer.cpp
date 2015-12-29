#include "GLcppmoth.h"

#include "gdcmBitmap.h"
#include "gdcmImageReader.h"
#include "gdcmImage.h"
#include "gdcmPixmap.h"

using namespace gdcm;
using namespace std;

class CDICOMView : public CGLApp {
public:
	CDICOMView(int argc, char **argv) : CGLApp(argc, argv),
		m_dims(NULL),
		m_buffer(NULL) {

		setGlutInfo("Viewing DICOM image in openGL");
		setVerinfo("DICOMViewr");
		addOption('f', true, "input-file-name", "the name of source file"
			   " containing DICOM image");
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

	int mainstay() {
		ImageReader reader;
		reader.SetFileName ( m_strfn.c_str() );
		if ( !reader.Read() ) {
			cerr << "failed to read " << m_strfn << "\n";	
			return -1;
		}

		const Image& image = reader.GetImage();
		const File& file = reader.GetFile();

		const gdcm::Pixmap& pmap = reader.GetPixmap();

		pmap.Print( cout );
		unsigned long sz = image.GetBufferLength();
		m_buffer = new char[ sz ];
		if ( !pmap.GetBuffer( (char*)m_buffer ) ) {
			cerr << "failed to get raw buffer.\n";
			return -1;
		}
		cout << "Buffer retrieved successfully [sz = " << sz << " bytes].\n";

		unsigned int nDim = image.GetNumberOfDimensions();
		if ( nDim  < 2 ) {
			m_cout << "only " << nDim  << " dimensions, wrong source image.\n";
			return -1;
		}

		const unsigned int *dims = image.GetDimensions();
		m_dims = new unsigned int [ nDim ];
		memcpy(m_dims, dims, nDim * sizeof (unsigned int));

		m_cout << "Dimension is " << m_dims[0] << " x " << m_dims[1] << "\n";

		setViewInfo(0, 0, max(m_dims[0],m_dims[1])/2.0);
		setPrjInfo(100.0f, 1, max(m_dims[0],m_dims[1]));

		/*
		for (int i=0;i<m_dims[0];i++) {
			for (int j=0;j<m_dims[1];j++) {
				printf("0x%02d ", m_buffer[i*m_dims[1] + j + 0 ]);
				printf("0x%02d ", m_buffer[i*m_dims[1] + j + 1 ]);
			}
		}
		*/

		return	CGLApp::mainstay();
	}

	void draw() {
		glPushMatrix();

		glTranslatef( - (m_dims[0]/4.0), - (m_dims[1]/4.0), .0 );
		//glRectd(0, 0, 256, 256);

		glRasterPos2i(0, 0);
		//glDrawPixels(m_dims[0], m_dims[1], GL_ALPHA, GL_BITMAP, (short*)m_buffer);
		glDrawPixels(m_dims[0], m_dims[1], GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4_REV, m_buffer);
		//glDrawPixels(m_dims[0], m_dims[1], GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, m_buffer);
		//glDrawPixels(m_dims[0], m_dims[1], GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, m_buffer);
		//glDrawPixels(m_dims[0], m_dims[1], GL_RGB, GL_UNSIGNED_SHORT_5_6_5_REV, m_buffer);
		//glDrawPixels(m_dims[0], m_dims[1], GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, m_buffer);
		//glDrawPixels(m_dims[0], m_dims[1], GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, m_buffer);

		glPopMatrix();
	}

private:
	string m_strfn;
	unsigned int *m_dims;
	char *m_buffer;
};

int main(int argc, char** argv)
{
	CDICOMView dcmv(argc, argv);
	dcmv.run();

	return 0;
}

/* set ts=4 sts=4 tw=80 sw=4 */

