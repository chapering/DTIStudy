// ----------------------------------------------------------------------------
// DCMviewer.h : a class dedicated exclusively to act as a DICOM viewer, either
//				for a single DICOM image or a set of DICOM images in a directory
//
//				the viewer of DICOM is still being modeled as a gadget in terms
//				of the class design, hence an inherited from CGLGadget
//
//				Dependencies : libGDCM
//
// Creation : Mar. 1st 2011
//
// Revisions:
//	@Mar. 2nd 2011
//		.make the first workable version using Pixmap texturing and placement
//		calibrating
//		.add prev() in the CDcmExplorer to make it easy to navigate
//		throughout images in a volume
//		.by the DTI convention, the initial position of DWI image is that of
//		Z=0.0 and the image used preliminarily is the central one within the b0
//		volume
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#ifndef _DCMVIEWER_H_
#define _DCMVIEWER_H_

#include "gdcmBitmap.h"
#include "gdcmImageReader.h"
#include "gdcmImage.h"
#include "gdcmPixmap.h"

#include "GLgadget.h"

using gdcm::ImageReader;
using gdcm::Pixmap;
using gdcm::Image;

using std::vector;
using std::string;

/*
 * this class aims at parsing a SINGLE DICOM file and then draw it in openGL
 * viewport by pixmap to texture mapping
 *
 * the parsing procedure depends on the popular, thus robuts hopefully, DICOM
 * library GDCM to work, which entails an unwieldy load of libraries to build
 * with
 */
class CSingleDcmViewer : public CGLGadget {
public:
	CSingleDcmViewer(const char* = "");
	~CSingleDcmViewer();

	// update to a new file 
	int setFile(const char*);

	// read DICOM and prepare the buffer
	int load();

	// rendering the pixels from the buffer prepared
	void display();
	void draw();

	// update Z coordinate of the image
	void updateImageZ(GLdouble imgz);

private:
	/* the name of the file associated currently */
	string m_strfn;
	/* record the size of each dimension */
	unsigned int *m_dims;
	/* buffer holding the raw image data */
	char *m_buffer;
	/* spawned buffer holding the converted data */
	unsigned char* m_imgbuf;
	/* record the status on if the loading has been completed */
	bool m_bLoaded;

	/* Z coordinate in terms of the image position */
	GLdouble m_imgz;

	void _destroy_mem();

	void _convert_to_rgb();
};

/*
 * in order to facilitate alternating DICOM images in a directory, within a DWI
 * volume for example, this class provides a simple explorer of DICOM images by
 * embedding a CSingleDcmViewer object instance as the DICOM handling agent, and
 * just focuses on maintaining the directory and keep track a focus image being
 * viewed
 */
class CDcmExplorer : public CGLGadget {
public:
	CDcmExplorer(const char* = "");
	~CDcmExplorer();

	// update to a new dir
	int setDir(const char*);

	// read the directory and prepare the file list
	int load();

	// just relay the drawing action to the CSingleDcmViewer 
	// agent for the selected image
	void display();
	void draw();
	// also for the translucency switching
	bool switchTranslucent();

	// say this is an explorer
	bool start();
	bool next();
	bool prev();

	// the caller might need this
	const char* getCurFileName() const;

private:
	/* the name of the directory to explore */
	string	m_strdir;
	/* record the current selected file */
	int		m_curfnIdx;
	/* the list of file names */
	vector<string> m_fns;

	/* record the current status */
	bool	m_bLoaded;

	/* the agent charging of a single DICOM parsing */
	CSingleDcmViewer m_dcmAgent;
};

#endif // _DCMVIEWER_H_

/* set ts=4 sts=4 tw=80 sw=4 */

