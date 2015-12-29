// ----------------------------------------------------------------------------
// CGLoader : a simple C++ encapsulation that aims at offering general purpse
//			feature for loading graphical geometry in different formats.
//			Currently following formats are supported:
//			. tgdata - an output of tubegen, a fiber tracking program of
//				VisLab@brown, which describes merely vertices and colors that
//				are grouped in terms of meaning streamlines. More precisely, the
//				structure of file in this format can be symbolized as follows:
//				--------------------------------------------------------------
//				$N - line total
//				$n1 - number of points for line No.1
//				x1 y1 z1 r1 g1 b1
//				x2 y2 z2 r2 g2 b2
//				...........
//				xn1 yn1 zn1 rn1 gn1 bn1
//				$n2 - number of points for line No.2
//				x1 y1 z1 r1 g1 b1
//				x2 y2 z2 r2 g2 b2
//				...........
//				xn2 yn2 zn2 rn2 gn2 bn2
//				.	.			.
//				.	.			.
//				.	.			.
//				$nN - number of points for line No.N
//				x1 y1 z1 r1 g1 b1
//				x2 y2 z2 r2 g2 b2
//				...........
//				xnN ynN znN rnN gnN bnN
//				--------------------------------------------------------------
//			. sm - another ouput of tubegen, a fiber tracking program of 
//				VisLab@brown, which describes geometry of streamtube by
//				separated three sections, namely vertex section, face section
//				and color section. More precisely, the structure of file in this
//				format can be symbolized as follows:
//				--------------------------------------------------------------
//				$N - vertex total
//				x1 y1 z1
//				x2 y2 z2
//				...........
//				xN yN zN
//
//				$M - face total
//				vi1 vj1 vk1
//				vi2 vj2 vk2
//				...........
//				viM vjM vkM
//
//				0
//				0
//				#BEGIN COLORS (followed the same number of colors as vertices)
//				r1 g1 b1
//				r2 g2 b2
//				...........
//				rN gN bN
//				--------------------------------------------------------------
//				* there might be random blank lines or annotation lines spreaded
//				over in the file
//			. obj - the most common geometry format that actually is able to
//				fully describe a geometrical model, covering vertices, normals,
//				texture coordinates, colors, faces, and even materials
//
// Creation : Feb. 4th 2011
// revision : 
// @Feb. 8th 2011
//		1. add sm loader, a class dedicated to parsing .sm-formated file and
//		loading geometry of vertices and faces, possibly along with colors from
//		it.
// @Mar. 12th 2011
//		1. change the dump() logic to produce a file in the same format as the
//		source - precede color channel values with vertex coordinates
//		2. an extra parameter added to dump - edgeFlags, useful when dumping 
//		out partial rather than the holistic data loaded - if a pointer to the
//		flags is not NULL, it will be applied as such that corresponding
//		vertex will be skipped during dumping if the flag is GL_FALSE.
//		---- particularly, in CTgdataLoader::dump(), the edgeFlag filtering is
//		being applied at the streamline, not vertex, level due to the particular
//		symantics of doing so for DTI model
// @Mar. 13th 2011
//		1. when loading a mini-scale object, whose bounding box is within
//		[-0.5,-0.5] - [ 0.5,0.5 ], a bug is found in the CGLoader::CGLoader(const
//		string&) that is pertinent to binary complement : compiler will take
//		-0xffffffff as 1 when the variable assigned is defined as GLdouble !!
//		when changed to -0xffffffffff, the problem cured.
// @May 9th 2011
//		1.Add operator= for CGLoader, we only copy the geometries however, not
//		move the bulky stuff related to the "mmaped" memory blocks
// @May 16th 2011
//		1.correct the vertices statistics in the CTgdataLoader
//		2.fix bug: in operator=, before copy stuff from other object, release
//		the m_pbuf firstly to avoid accumulated memory leaks
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#if !defined (_GLOADER_H_)
#define _GLOADER_H_

#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#include <cstring>
#include <cassert>

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <utility>
#include <iterator>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>

#include <GL/glut.h>
#include <glrand.h>

using std::string;
using std::vector;
using std::map;
using std::cout;
using std::cerr;
using std::endl;
using std::setprecision;
using std::fixed;
using std::istream;
using std::istringstream;
using std::skipws;

typedef enum _gtype_t {
	GTYPE_OBJ = 0,
	GTYPE_TGDATA,
	GTYPE_SM,
	GTYPE_NONE
}gtype_t;

const static char* const g_gtypes[] = {
	"obj",
	"tgdata",
	"sm",
	"none"
};

typedef vector<GLboolean> edge_flag_t;

/*
 * the skeleton of a general geometry loader
 */ 
template <class _T = GLdouble>
class CGLoader {
public:
	CGLoader();
	CGLoader(const string& gtype);
	virtual ~CGLoader();

	CGLoader& operator = (const CGLoader& other);

	// load file into memory
	virtual int load(const string& fn);

	// dump memory for the file loaded into a disk file
	virtual int dump(const string& fn, const edge_flag_t* pedgeflags = NULL);

	// pick up an element from the buffer
	virtual _T& getElement(unsigned long idx);

	// query the size of the buffer
	virtual unsigned long getSize() const;

	// get the maximal coordinates among all vertices loaded
	virtual const GLdouble& getMaxCoord(int idx) const;

	// get the minimal coordinates among all vertices loaded
	virtual const GLdouble& getMinCoord(int idx) const;

	// get the radius of the bounding box engulfing all vertices loaded
	virtual const GLdouble getBoundRadius() const;

	// report about what was loaded
	virtual void report() const;

	// an utility used for verifying if the given is accessible(already exist
	// and can be visited
	static int checkFile(const string&);

	/*
	 * @brief an utility used for read a single item from an input stream; this
	 * functionality is quite common but with a particular: during the scanning,
	 * any empty and annotation lines will be skipped automatically
	 * TODO: support regular expression to indicate the filter while scanning
	 * @param is a reference to a input stream from which to read
	 * @param v a reference to a variable where the retrieved value will be
	 * stored
	 * @param strict a boolean indicating if forcing the valid line contains
	 * exactly designated number of values
	 * @return  0 - no desired value extracted, all lines scanned are filtered
	 *				out
	 *			-1 - exception encountered and bailed out in advance
	 *			positive integer - number of lines passed through before
	 *			reaching the desired line containing the expected value
	 */
	template <typename _targetT>
	static int bruteRead(istream& is, _targetT& v, bool strict=false);

	// a simply exented version of the single-value aimming bruteRead that
	// targets a series of values in a single specific line
	template <typename _targetT>
	static int bruteRead(istream& is, _targetT* pv, size_t n, bool strict=false);

protected:
	int m_gtype;			// geometry type
	string m_fn;			// data source file name
	map<string, int> m_typemap;	
							// map from literal representation of geometry type
							// to that of an enumeration item

	_T*    m_pbuf;			// a piece of memory either holding any of vertices,normals,
							// colors,texture coordinates, or serving as a interleaved
							// buffer
	char*	m_pdata;		// pointer to the memory created by mmap that holds the 
							// content of the source file
	off_t	m_fsize;		// size of the source file
	unsigned long m_numel;	// number of elements of type _T in the buffer

	GLdouble m_maxCoord[3];	// get the maximal X,Y and Z coordinate among all vertices
	GLdouble m_minCoord[3];	// get the minimal X,Y and Z coordinate among all vertices

	// mmap the source into memory and return the pointer to that memory and
	// save in m_pdata, it will also fill m_fsize
	int memmap();

	// release the mmap-ed memory
	int unmap();

private:
	// simply create the mapping from symbolic source file type to an integer
	// (macroes)
	void _initGtypeMap();
};

/////////////////////////////////////////////////////////////////////////////
//  the class implementation of CGLoader
template <class _T>
CGLoader<_T>::CGLoader() : 
	m_gtype(GTYPE_NONE),
	m_fn(""),
	m_pbuf(NULL),
	m_pdata(NULL),
	m_fsize(0),
	m_numel(0)
{
	for (int i=0;i<3;++i) {
		m_maxCoord[i] = -0x000000ffffffffff;
		m_minCoord[i] = 0x000000ffffffffff;
	}
	_initGtypeMap();
}

template <class _T>
CGLoader<_T>::CGLoader(const string& gtype) : 
	m_gtype(GTYPE_NONE),
	m_fn(""),
	m_pbuf(NULL),
	m_pdata(NULL),
	m_fsize(0),
	m_numel(0)
{
	for (int i=0;i<3;++i) {
		m_maxCoord[i] = -0x000000ffffffffff;
		m_minCoord[i] = 0x000000ffffffffff;
	}
	_initGtypeMap();
	if ( m_typemap.end() == m_typemap.find(gtype) ) {
		cerr << "unrecognized file type : " << gtype << endl;
		return;
	}
	m_gtype = m_typemap[ gtype ];
}

template <class _T>
CGLoader<_T>::~CGLoader()
{
	delete [] m_pbuf;
	m_pbuf = NULL;
	m_numel = 0;
}

template <class _T>
CGLoader<_T>& CGLoader<_T>::operator= (const CGLoader<_T>& other)
{
	if ( !other.m_pbuf ) {
		// no real geometry loaded in the source object, nothing needed further
		// since we are just to copy the geometries
		return *this;
	}

	if ( m_pbuf ) {
		delete [] m_pbuf;
	}

	m_numel = other.m_numel;
	m_pbuf = new _T[m_numel];

	// we need this seemingly trivial loop to cause calls to the operator= for type _T
	for (unsigned long i = 0; i < m_numel; i++) {
		m_pbuf[i] = other.m_pbuf[i];
	}
	for (int j = 0; j < 3; j++) {
		m_maxCoord[j] = other.m_maxCoord[j];
		m_minCoord[j] = other.m_minCoord[j];
	}
	return *this;
}

template <class _T>
int CGLoader<_T>::load(const string& fn)
{
	int ret = checkFile(fn);
	if ( 0 != ret ) {
		return ret;
	}

	m_fn = fn;

	ret = memmap();
	if ( 0 != ret ) {
		return ret;
	}

	return 0;
}

template <class _T>
int CGLoader<_T>::dump(const string& fn, const edge_flag_t* pedgeflags)
{
	return 0;
}

template <class _T>
_T& CGLoader<_T>::getElement(unsigned long idx)
{
	assert ( m_pbuf && idx >= 0 && idx < m_numel );
	return m_pbuf[ idx ];
}

template <class _T>
unsigned long CGLoader<_T>::getSize() const
{
	return m_numel;
}

template <class _T>
const GLdouble& CGLoader<_T>::getMaxCoord(int idx) const
{
	assert (idx >= 0 && idx <= 2);
	return m_maxCoord[idx];
}

template <class _T>
const GLdouble& CGLoader<_T>::getMinCoord(int idx) const
{
	assert (idx >= 0 && idx <= 2);
	return m_minCoord[idx];
}

template <class _T>
const GLdouble CGLoader<_T>::getBoundRadius() const
{
	return  magnitude( 
			(m_minCoord[0] + m_maxCoord[0])/2 - m_minCoord[0],
			(m_minCoord[1] + m_maxCoord[1])/2 - m_minCoord[1],
			(m_minCoord[2] + m_maxCoord[2])/2 - m_minCoord[2]
			);
}

template <class _T>
void CGLoader<_T>::report() const
{
	cout << "Loading finished : totally " << m_numel << " elements." << endl;
	cout << fixed << setprecision(6);
	cout << "X : " << m_minCoord[0] << " - " << m_maxCoord[0] << endl;
	cout << "Y : " << m_minCoord[1] << " - " << m_maxCoord[1] << endl;
	cout << "Z : " << m_minCoord[2] << " - " << m_maxCoord[2] << endl;
}

template <class _T>
int CGLoader<_T>::checkFile(const string& fn) 
{
	int ret = 0; 
	if ( fn.length() < 1 || 
		0 != (ret = access(fn.c_str(), F_OK|R_OK)) ) {
		cerr << "Can not access file : " << fn << endl;
	}

	return ret;
}

template <class _T>
template <typename _targetT>
int CGLoader<_T>::bruteRead(istream& is, _targetT& v, bool strict)
{
	if ( !is ) {
		return -1;
	}

	int nScannedLines = 0;

	string line, subline;
	is >> skipws;
	while ( is ) {
		line.clear();
		subline.clear();

		std::getline(is, line);
		nScannedLines ++;
		// skip empty line
		if ( line.empty() ) {
			continue;
		}

		std::istringstream iss(line);
		// hopping over the leading whitespaces
		iss >> skipws >> subline;

		// skip well-known leading annotation tags
		if ( subline.empty() ||
			 '#' == subline[0] || // annotation tag in python, bash, etc.
			 '%' == subline[0] || // annotation tag in matlab
			 0 == subline.compare(0,2,"/*") || // annotation tags in c/c++/php
			 0 == subline.compare(0,2,"//") ) {
			continue;
		}

		// skip unsatied lines in terms of fetching desired number of items
		std::istringstream subiss(subline);
		subiss >> skipws >> fixed >> setprecision(6) >> v;
		subiss.ignore(256, '\n');

		if (strict && subiss) { 
			// since we presume there is only a single valide value desired 
			// per line
			continue;
		}

		return nScannedLines;
	}

	return 0;
}

template <class _T>
template <typename _targetT>
int CGLoader<_T>::bruteRead(istream& is, _targetT* pv, size_t n, 
									bool strict)
{
	if ( !is || !pv || n <= 0) {
		return -1;
	}

	int nScannedLines = 0, icnt;

	string line;
	is >> skipws;
	while ( is ) {
		line.clear();

		std::getline(is, line);
		nScannedLines ++;
		// skip empty line
		if ( line.empty() ) {
			continue;
		}

		std::istringstream iss(line);

		// skip well-known leading annotation tags
		if ( line.empty() ||
			 '#' == line[0] || // annotation tag in python, bash, etc.
			 '%' == line[0] || // annotation tag in matlab
			 0 == line.compare(0,2,"/*") || // annotation tags in c/c++/php
			 0 == line.compare(0,2,"//") ) {
			continue;
		}

		// skip unsatied lines in terms of fetching desired number of items
		iss >> skipws >> fixed >> setprecision(6);

		for (icnt = 0; iss && icnt < (int)n; ++icnt) {
			iss >> pv[icnt];
		}
		iss.ignore(256, '\n');

		// no enough values extracted
		if ( icnt < (int)n ) {
			continue;
		}

		if (strict && iss ) { 
			// since we presume there are exactly $n valid values desired 
			// per line
			continue;
		}

		return nScannedLines;
	}

	return 0;
}

template <class _T>
int CGLoader<_T>::memmap()
{
	// in case of a very large source data, we choose mmap against the standard
	// file stream IO to take into the source geometry from a physical file
	int fd = open (m_fn.c_str(), O_RDONLY);
	if ( -1 == fd) {
		cerr << "FATAL: file opening failed - " << m_fn << "[ with error: " 
			<<  strerror(errno) << " ]" << endl;
		return -1;
	}

	struct stat sb;
	if ( -1 == fstat(fd, &sb) ) {
		cerr << "FATAL: failed at fstat - " << m_fn << "[ with error: "
			<< strerror(errno) << " ]" << endl;
		return -1;
	}

	// and map it into current process's address space
	char* pData = (char*)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if ( pData == MAP_FAILED ) {
		cerr << "FATAL: mmap failed [ with error: " << strerror(errno) << " ]" 
			<< endl;
		return -2;
	}

	close(fd);
	m_pdata = pData;
	m_fsize = sb.st_size;
	return 0;
}

template <class _T>
int CGLoader<_T>::unmap()
{
	int ret = munmap(m_pdata, m_fsize);
	if (-1 == ret) {
		cerr << "FATAL: munmap failed [ with error: " << strerror(errno) << " ]"
			<< endl;
	}
	m_pdata = NULL;
	m_fsize = 0;
	return ret;
}

template <class _T>
void CGLoader<_T>::_initGtypeMap()
{
	int sz = sizeof ( g_gtypes ) / sizeof ( g_gtypes[0] );
	for( int idx = 0 ; idx < sz ; ++idx ) {
		m_typemap[ g_gtypes[idx] ] = idx;
	}
}
	
/*
 * a more specific loader, loading geometry from a file in the "tgdata" format.
 *	in this format, vertices and colors are stored in the interleaved
 *	buffer, each being a vector holding all information available of points
 *	that constitute a polyline, and the m_pbuf will be the head pointer
 *	pointing to a head list with each element being a pointer to a vector
 */
class CTgdataLoader : public CGLoader< vector<GLfloat> > {
public:
	CTgdataLoader(bool = true);
	~CTgdataLoader();

	int load(const string& fn);
	int dump(const string& fn, const edge_flag_t* pedgeflags = NULL);

protected:
private:
	bool m_bWithColor;
};

/*
 * another specific loader, loading geometry from a file in the "sm" format.
 * in this format, vertices are listed for one 3D point per line firstly, then
 * all face indices and finally colors, each line in the color section gives
 * color information for each vertices in the vertex section one by one
 */
class CSmLoader : public CGLoader < GLfloat > {
public:
	CSmLoader(bool = true, bool=false);
	~CSmLoader();

	int load(const string& fn);
	int dump(const string& fn, const edge_flag_t* pedgeflags = NULL);
	int serializeToObj(const string& fnobj, const edge_flag_t* pedgeflags = NULL);

protected:
private:
	bool m_bWithColor;
	bool m_bNormalPerFace;

public:
	// stash of streamtube geometry 
	vector<GLdouble>		m_alltubevertices;
	vector<GLdouble>		m_alltubenormals;
	vector<GLdouble>		m_alltubecolors;
	vector<GLuint> 		m_alltubefaceIdxs;
};

/*
 * an subtype CGLoader specifically serving the most popular format - obj
 */
template <class _T = GLdouble>
class CObjLoader : public CGLoader<_T> {
public:
	CObjLoader();
	~CObjLoader();

	int load(const string& fn);
	int dump(const string& fn, const edge_flag_t* pedgeflags = NULL);

protected:
private:
};

#endif // _GLOADER_H_

/* set ts=4 sts=4 tw=80 sw=4 */

