// ----------------------------------------------------------------------------
// CGLoader : the class implementation
//
// Creation : Feb. 4th 2011
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#include "GLoader.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//  the class implementation of CTgdataLoader 
CTgdataLoader::CTgdataLoader(bool bLoadColor) :
	CGLoader < vector< GLfloat > > ("tgdata"),
	m_bWithColor(bLoadColor)
{
}

CTgdataLoader::~CTgdataLoader()
{
}

int CTgdataLoader::load(const string& fn)
{
	int ret = CGLoader< vector< GLfloat> >::load(fn);
	if ( 0 != ret ) {
		return ret;
	}

	unsigned long nLine = 0, icntLine = 0;
	unsigned long nPoints = 0, icntPoint = 0, nPtTotal = 0;
	int nStride = m_bWithColor? 6:3;

	GLfloat d;
	unsigned long icurFline = 0;
	istringstream istr(m_pdata, istringstream::in);
	istr >> nLine;
	icurFline ++;

	if ( nLine < 1 ) {
		cerr << "empty source data." << endl;
		return -3;
	}

	try {
		m_pbuf = new vector<GLfloat> [ nLine ];
	}
	catch (std::bad_alloc & e) {
		cerr << "FATAL: alloc failed for the head list - " << e.what() << endl;
		return -4;
	}

	/**** WE PRESUME THERE IS NO EMPTY LINE IN THE SOURCE FILE ********/

	// iterate through all polylines
	while (istr.good() && icntLine < nLine) {
		istr >> nPoints;
		icurFline ++;
		icntPoint = 0;
		m_pbuf [ icntLine ].resize(nStride*nPoints);
		nPtTotal += nPoints;

		// iterate through all points for a single polyline
		while (istr.good() && icntPoint < nPoints) {

			// read info for a single point once
			for (int icnt = 0, pos = 0; istr && icnt < nStride; icnt++) {
				pos = icnt + icntPoint*nStride;
				if ( m_bWithColor ) {
					pos += (icnt>2?-3:3);
				}

				istr >> fixed >> setprecision(6) >> d;
				m_pbuf [ icntLine ][ pos ] = d;
				icurFline ++;

				if ( icnt < 3 ) {
					if( m_maxCoord[ icnt ] < d ) {
						m_maxCoord[ icnt ] = d;
					}
				    if( m_minCoord[ icnt ] > d ) {
						m_minCoord[ icnt ] = d;
				    } 
				}
			}

			if ( ! istr ) {
				goto err_parse;
			}

			icntPoint ++;
		}

		if ( icntPoint < nPoints ) {
			goto err_parse;
		}

		icntLine ++;
	}

	if ( icntLine < nLine ) {
		goto err_parse;
	}

	m_numel = nLine;
	report();
	cout << nPtTotal << " vertices totally." << endl;
	unmap();
	return 0;

err_parse:
	cerr << "FATAL: parsing source failed at Line No." << icurFline << 
		" -- File not compatible with designated format: " << 
		g_gtypes[m_gtype] << endl;
	unmap();
	m_numel = 0;
	delete [] m_pbuf;
	m_pbuf = NULL;
	return -1;
}

int CTgdataLoader::dump(const string& fn, const edge_flag_t* pedgeflags)
{
	ofstream ofs(fn.c_str());
	if ( ! ofs.is_open() ) {
		cerr << "failed to create file : " << fn << " for serializing." << endl;
		return -1;
	}

	ofs << fixed << setprecision(6);

	unsigned long szTotal = getSize(), szLine;
	unsigned long nStride = m_bWithColor? 6:3;
	unsigned long offset = m_bWithColor? 3:0;

	unsigned long szToDump = 0;
	if ( pedgeflags ) {
		for (size_t i=0; i<pedgeflags->size(); ++i) {
			szToDump += ( GL_TRUE == (*pedgeflags)[i] ? 1:0 );
		}
	}

	ofs << (pedgeflags?szToDump:szTotal)
		/*
		<< " lines, with" << (m_bWithColor?"":"out")
		<< " colors"	<< endl 
		*/
		<< endl;

	for (unsigned long idx = 0; idx < szTotal; ++idx) {
		// apply edge Flags (here it is streamline flag in fact) as filters
		if ( pedgeflags && GL_FALSE == (*pedgeflags)[idx] ) {
			continue;
		}
		vector<GLfloat> & curLine = getElement( idx );
		szLine = static_cast<unsigned long> ( curLine.size() );
		
		ofs << 
			/*
			"curline No." << idx << " : [" << szLine << 
			" elements, " << 
			*/
			( szLine/nStride )	//<< " points ]" 
			<< endl;
		
		for (unsigned long j = 0; j < szLine; j += nStride) {
			ofs << curLine[j+offset+0] <<  " " << 
				curLine[j+offset+1] << " " << curLine[j+offset+2];
			if ( m_bWithColor ) {
				ofs << " " << curLine[j] <<  " " << 
					curLine[j+1] << " " << curLine[j+2];
			}
			ofs << endl;
		}
	}

	ofs.close();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//  the class implementation of CSmLoader
CSmLoader::CSmLoader(bool bLoadColor, bool bNormalPerFace) :
	CGLoader< GLfloat> ("sm"),
	m_bWithColor(bLoadColor),
	m_bNormalPerFace(bNormalPerFace)
{
}

CSmLoader::~CSmLoader()
{
	m_alltubevertices.clear();
	m_alltubenormals.clear();
	m_alltubecolors.clear();
	m_alltubefaceIdxs.clear();
}

int CSmLoader::load(const string& fn)
{
	int ret = CGLoader< GLfloat >::load(fn);
	if ( 0 != ret ) {
		return ret;
	}

	unsigned long nFaces = 0, icntFace = 0;
	unsigned long nPoints = 0, icntPoint = 0;
	unsigned long nColors = 0, icntColor = 0;

	unsigned long icurFline = 0;
	unsigned int vofs[3]; // vertex index for face
	GLdouble pcs[3]; // point coordinates or color channels

	GLdouble nx, ny, nz; // normals

	istringstream istr(m_pdata, istringstream::in);

	// read vertices in the first place
	ret = bruteRead(istr, nPoints, true);
	if ( 0 >= ret ) {
		goto err_parse;
	}
	icurFline += ret;

	if ( nPoints < 1 ) {
		cerr << "empty source data." << endl;
		return -3;
	}

	m_alltubevertices.resize( nPoints * 3 );

	// iterate through all vertices 
	while (istr.good() && icntPoint < nPoints) {

		// read info for a single point once
		ret = bruteRead(istr, pcs, 3, true);
		if ( 0 >= ret ) {
			goto err_parse;
		}
		icurFline += ret;

		for (int icnt = 0; icnt < 3; icnt++) {
			m_alltubevertices[ icntPoint*3 + icnt ] = pcs[icnt];

			if( m_maxCoord[ icnt ] < pcs[icnt] ) {
				m_maxCoord[ icnt ] = pcs[icnt];
			}
			if( m_minCoord[ icnt ] > pcs[icnt] ) {
				m_minCoord[ icnt ] = pcs[icnt];
			} 
		}

		icntPoint ++;
	}

	if ( icntPoint < nPoints ) {
		goto err_parse;
	}

	// read faces in the second  place
	ret = bruteRead(istr, nFaces, true);
	if ( 0 >= ret ) {
		goto err_parse;
	}
	icurFline += ret;

	if ( nFaces < 1 ) {
		cerr << "no face data." << endl;
		return -3;
	}

	m_alltubefaceIdxs.resize( nFaces * 3 );
	m_alltubenormals.resize( m_bNormalPerFace? nFaces*3 : nPoints*3 );

	// iterate through all faces 
	while (istr.good() && icntFace < nFaces) {

		// read info for a single face once
		ret = bruteRead(istr, vofs, 3, true);
		if ( 0 >= ret ) {
			goto err_parse;
		}
		icurFline += ret;

		for (int icnt = 0; icnt < 3; icnt++) {
			if (vofs[icnt] > nPoints) {
				cerr << "FATAL: invalid vertex index for face!\n";
				goto err_parse;
			}
			m_alltubefaceIdxs[ icntFace*3 + icnt ] = vofs[icnt];
		}

		// calcvofsate normals for each face
		crossproduct(nx,
					ny,
					nz,

					m_alltubevertices[ vofs[0]*3 + 0 ] -
					m_alltubevertices[ vofs[1]*3 + 0 ],
					m_alltubevertices[ vofs[0]*3 + 1 ] -
					m_alltubevertices[ vofs[1]*3 + 1 ],
					m_alltubevertices[ vofs[0]*3 + 2 ] -
					m_alltubevertices[ vofs[1]*3 + 2 ],

					m_alltubevertices[ vofs[1]*3 + 0 ] -
					m_alltubevertices[ vofs[2]*3 + 0 ],
					m_alltubevertices[ vofs[1]*3 + 1 ] -
					m_alltubevertices[ vofs[2]*3 + 1 ],
					m_alltubevertices[ vofs[1]*3 + 2 ] -
					m_alltubevertices[ vofs[2]*3 + 2 ]);

		normalize(nx, ny, nz);

		if ( m_bNormalPerFace ) {
			m_alltubenormals[ icntFace*3 + 0 ] = nx;
			m_alltubenormals[ icntFace*3 + 1 ] = ny;
			m_alltubenormals[ icntFace*3 + 2 ] = nz;
		}
		else { // assign this face normal to all vertices of the face
			for (int icnt = 0; icnt < 3; icnt++) {
				m_alltubenormals[ vofs[icnt]*3 + 0 ] = nx;
				m_alltubenormals[ vofs[icnt]*3 + 1 ] = ny;
				m_alltubenormals[ vofs[icnt]*3 + 2 ] = nz;
			}
		}

		icntFace ++;
	}

	if ( icntFace < nFaces ) {
		goto err_parse;
	}

	// finally read color information conditionally
	if ( m_bWithColor ) {
		nColors = nPoints;

		m_alltubecolors.resize( nColors * 3 );

		// iterate through all colors 
		while (istr.good() && icntColor < nColors ) {

			// read info for a single color once
			ret = bruteRead(istr, pcs, 3, true);
			if ( 0 >= ret ) {
				goto err_parse;
			}
			icurFline += ret;

			for (int icnt = 0; icnt < 3; icnt++) {
				m_alltubecolors[ icntColor*3 + icnt ] = pcs[icnt];
			}

			icntColor ++;
		}

		if ( icntColor < nColors ) {
			goto err_parse;
		}
	}

	m_numel = nPoints;

	report();
	cout << icurFline << " lines in all, and " 
		<< nFaces << " faces totally." << endl;
	unmap();
	return 0;

err_parse:
	cerr << "FATAL: parsing source failed at Line No." << icurFline << 
		" -- File not compatible with designated format: " << 
		g_gtypes[m_gtype] << endl;
	unmap();
	m_numel = 0;
	return -1;
}

int CSmLoader::dump(const string& fn, const edge_flag_t* pedgeflags)
{
	ofstream ofs(fn.c_str());
	if ( ! ofs.is_open() ) {
		cerr << "failed to create file : " << fn << " for serializing." << endl;
		return -1;
	}

	unsigned long szPoints = static_cast<unsigned long> (m_alltubevertices.size())/3;
	unsigned long szFaces = static_cast<unsigned long> (m_alltubefaceIdxs.size())/3;
	unsigned long szColors = static_cast<unsigned long> (m_alltubecolors.size())/3;

	ofs << szPoints << " vertices,  " << szFaces << " faces, with"
		<< (m_bWithColor?"":"out") << " colors"	<< endl << endl;

	// dump vertices
	ofs << szPoints << endl;
	ofs << fixed << setprecision(6);
	for (unsigned long idx = 0; idx < szPoints; idx++) {
		ofs << m_alltubevertices[idx*3 + 0] << " "
			<< m_alltubevertices[idx*3 + 1] << " "
			<< m_alltubevertices[idx*3 + 2] << endl;
	}

	ofs << endl;

	// dump faces 
	ofs << szFaces << endl;
	for (unsigned long idx = 0; idx < szFaces; idx++) {
		ofs << m_alltubefaceIdxs[idx*3 + 0] << " "
			<< m_alltubefaceIdxs[idx*3 + 1] << " "
			<< m_alltubefaceIdxs[idx*3 + 2] << endl;
	}

	// dump colors conditionally
	if (m_bWithColor) {
		ofs << endl;
		ofs << "0\n0\n# BEGIN COLORS" << endl;
		for (unsigned long idx = 0; idx < szColors; idx++) {
			ofs << m_alltubecolors[idx*3 + 0] << " "
				<< m_alltubecolors[idx*3 + 1] << " "
				<< m_alltubecolors[idx*3 + 2] << endl;
		}
	}

	ofs.close();
	return 0;
}

/*
 * serialize the streamtube geometry generated into a file in the format
 * of WaveFront obj
 */
int CSmLoader::serializeToObj(const string& fnobj, const edge_flag_t* pedgeflags) 
{
	ofstream ofs(fnobj.c_str());
	if ( ! ofs.is_open() ) {
		cerr << "failed to create file : " << fnobj << " for serializing." << endl;
		return -1;
	}

	cout << "Start serializing streamtube geometry .... ";
	fflush(stdout);

	// a brief even trivial header
	ofs << "# --------------------------- " << endl;
	ofs << "# start of the obj file " << endl << endl;
	ofs << "# *** Normal per " << (m_bNormalPerFace?"Face":"Vertex") 
		<< " ***" << endl << endl;
	ofs << fixed << setprecision(6);

	unsigned long szVertex, szNormal, szFace;

	// traverse all vertices
	szVertex = static_cast<unsigned long> ( m_alltubevertices.size() ) / 3;
	for (unsigned long vIdx = 0; vIdx < szVertex; vIdx ++) {
		ofs << "v " << m_alltubevertices[ vIdx*3 + 0 ] << " "
			 << m_alltubevertices[ vIdx*3 + 1 ] << " "
			 << m_alltubevertices[ vIdx*3 + 2 ] << endl;
	}

	// a blank line separating between each section of vertices, normals
	// and face indices
	ofs << endl;

	// normals 
	szNormal = static_cast<unsigned long> ( m_alltubenormals.size() ) / 3;
	// in the case of "Normal per Vertex", normal for each vertex of a face
	// has been stored repeatedly in the store, all being the face normal
	for (unsigned long nIdx = 0; nIdx < szNormal; nIdx ++) {
		ofs << "vn " << m_alltubenormals[ nIdx*3 + 0 ] << " "
					<< m_alltubenormals[ nIdx*3 + 1 ] << " "
					<< m_alltubenormals[ nIdx*3 + 2 ] << endl;
	}

	// a blank line separating between each section of vertices, normals
	// and face indices
	ofs << endl;

	// face indices, along with normal indices
	szFace = static_cast<unsigned long> ( m_alltubefaceIdxs.size() ) / 3;
	for (unsigned long fIdx = 0; fIdx < szFace; fIdx ++) {
		ofs << "f ";
		for (int pIdx = 0; pIdx < 3; pIdx++) {
			if ( m_bNormalPerFace ) {
				// each face has a normal, shared by all of its vertices
				ofs << m_alltubefaceIdxs[ fIdx*3 + pIdx ] << "//"
					<< fIdx;
			}
			else { // Normal per Vertex
				// Vertices share normals with that of the face it is on.
				// no texture coordinate since it is not used here
				ofs << m_alltubefaceIdxs[ fIdx*3 + pIdx ] << "//"
					<< m_alltubefaceIdxs[ fIdx*3 + 0 ];
			}

			if ( pIdx < 2 ) {
				ofs << " ";
			}
		}
		ofs << endl;
	}

	// a brief even trivial tail
	ofs << endl << endl;
	ofs << "# end of the obj file " << endl;
	ofs << "# --------------------------- " << endl;

	ofs.close();

	cout << " finished.\n";
	return 0;
}

/* set ts=4 sts=4 tw=80 sw=4 */

