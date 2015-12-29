// ----------------------------------------------------------------------------
// selectFApoints : Across all input source geometry each for a DTI model in the
//					format of "tgdata", find a group of vertex pairs with each
//					pair of vertices sampled from each of the input geometry
//					respectively. As the result, each of the pairs of vertices
//					will meet following requirements:
//					. euclidean distance between two vertices of each pair is
//					in the same interval;
//					. color, in RGB space, distance between two vertices of each
//					pair is in the same interval as well.
//
//
// Creation : Mar. 15th 2011
//
// Revision:
//	@Mar. 17th
//		1.giving results by storing the index pairs, along with the answer about
//		which point has a higher FA (FA is positively proportional to the sum of
//		color channels), into a file named by corresponding input geometry file
//
//		2.add another option -u for distinctly searching solutions. When this
//		option is set, any solution that is similar to previous ones will be
//		skipped. For instance, if we already have "92 103" as the first point
//		for the 2nd source geometry, any further solution including this pair,
//		also as the first point for the same source geometry, will be ignored.
//
//	@Mar. 18th
//		1.practical use of the tool implies that the program should be able to
//		search the distance intervals in a meaningful range, since 
//		it is quite hard to determine the range for the user who might not know
//		what is a reasonable interval to search in
//		-- this drived the addition of m_bAutoDist which is automatically
//		assigned as true or false according to if the explicit values are given
//		by the user for m_distance[].
//		-- the color difference range can also be determined in this way;
//		implemented as well
//		-- so extra two options are added - "distance step size" and "color
//		difference step size".
// @Mar. 19th
//		1.restrain certain debug-level output and make them available only in
//		the Debug-compiled version
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#include "situbeRender.h"

#include <unistd.h>
#include <sys/signal.h>
#include <sys/wait.h>

using std::vector;
using std::string;
using std::cout;
using std::cerr;
using std::ostringstream;
using std::ofstream;

class CSelectFAPoints : public CApplication {
public:
	CSelectFAPoints(int argc, char **argv);
	virtual ~CSelectFAPoints();

	// overloadings to those in the parent class for customized actions
	int handleOptions(int optv);

	int mainstay();

private:
	// a set of source geometry files
	vector<string> m_strfnsrcs;

	// a set of renders, each for a single model, associated with a single
	// source geometry file and a DWI image directory, but they will share all
	// other command line arguments like lod, tube radius and number of
	// selection boxes, etc.
	vector<CSitubeRender*> m_renders;

	// a pointer list kept for recording the argument list each of the renders
	// to be fed with
	vector<char**>		m_allargvs;

	/* distance interval */
	GLfloat	m_distance[2];

	/* color difference interval */
	GLfloat	m_colordiff[2];

	// number of proprietary command line arguments
	int m_nparg;

	// for each solution, we save the index pairs for each input geometry into a
	// separate file, and then place this group of files into a newly created
	// directory named set${m_setnum}.
	int m_setnum;

	// switch of the distinct solutions
	bool m_bDistinct;

	// limit on the number of solutions to search
	int m_numsol;

	// first point indices of all source geometry
	vector< vector< _point_t<unsigned long> > > m_firstpts;

	// second point indices of all source geometry
	vector< vector< _point_t<unsigned long> > > m_secondpts;

	// if m_distance[] not explicitly specified, search in reasonable intervals
	// that are computed automatically, with a step size being decided by
	// m_stepsize
	bool m_bAutoDist;
	GLfloat m_diststep;
	// similar for the color difference
	bool m_bAutoColorDiff;
	GLfloat m_colordiffstep;

private:
	// extract from the original command line to form command line for each
	// render instance
	int _assign_argvs(int renderIdx);

	// just create instances of single itubeRender for each of the DTI models
	int _createAllRenders();

	// do everything each render needs to do before entering into the event loop 
	int _armRenders();

	// check if the given solution is similar to any previous ones
	bool _isDistinct( const vector< _point_t<unsigned long> >& firstpts,
					const vector< _point_t<unsigned long> >& secondpts);

	// save solution into text file, in different directories if needed
	int _saveToFile(const vector<string>& keys, 
					const vector< _point_t<unsigned long> >& firstpts,
					const vector< _point_t<unsigned long> >& secondpts);

	// look up all possible pairs: total indicates the intention to
	// search at most $total pairs, -1 for unlimited search
	int _lookupAllPairs(int total = -1);

	// exhaust in all possible intervals, both for the distance and color
	// difference
	int _automaticLookup(int total=-1);
};

///////////////////////////////////////////////////////////////////////////////
//
//	Implementation of the Class CSelectFAPoints
//
CSelectFAPoints::CSelectFAPoints(int argc, char **argv) : 
	CApplication(argc, argv),
	m_nparg(0),
	m_setnum(0),
	m_bDistinct(false),
	m_numsol(-1),
	m_bAutoDist(true),
	m_diststep(5.0f),
	m_bAutoColorDiff(true),
	m_colordiffstep(0.1)
{
	addOption('f', true, "input-files ", "the name of source file containing"
		   " geometry in the format of tgdata, each one given by a -f ");
	addOption('s', true, "small-distance ", "floor of the distance interval");
	addOption('l', true, "large-distance ", "ceiling of the distance interval");
	addOption('S', true, "Small-color diff ", "floor of the color difference interval");
	addOption('L', true, "Large-color diff ", "ceiling of the color difference interval");
	addOption('t', true, "total-limit", "search at most this number of solutions and stop");
	addOption('u', false, "distinct-solution", "only save distinct solutions");
	addOption('d', true, "dist-step", "while the distance range left unset, will "
			"search in an automatically determined range with this step");
	addOption('c', true, "colordiff-step", "while the color difference range left "
			"unset, will search in an automatically determined range with this step");

	m_distance[0] = m_distance[1] = 40.0;
	m_colordiff[0] = m_colordiff[1] = 0.5;
}

CSelectFAPoints::~CSelectFAPoints()
{
	for (size_t i = 0; i < m_allargvs.size(); ++i) {
		delete [] m_allargvs[i];
		m_allargvs[i] = NULL;
	}

	for (size_t i = 0; i < m_renders.size(); ++i) {
		delete m_renders[i];
		m_renders[i] = NULL;
	}
}

int CSelectFAPoints::handleOptions(int optv) 
{
	switch( optv ) {
		case 'f':
			m_strfnsrcs.push_back(optarg);
			return 0;
		case 's':
			m_bAutoDist = false;
			m_distance[0] = strtof(optarg, NULL);
			m_nparg +=2;
			return 0;
		case 'l':
			m_bAutoDist = false;
			m_distance[1] = strtof(optarg, NULL);
			m_nparg +=2;
			return 0;
		case 'S':
			m_bAutoColorDiff = false;
			m_colordiff[0] = strtof(optarg, NULL);
			m_nparg +=2;
			return 0;
		case 'L':
			m_bAutoColorDiff = false;
			m_colordiff[1] = strtof(optarg, NULL);
			m_nparg +=2;
			return 0;
		case 't':
			m_numsol = atoi(optarg);
			m_nparg +=2;
			return 0;
		case 'u':
			m_bDistinct = true;
			m_nparg ++;
			return 0;
		case 'd':
			m_bAutoDist = true;
			m_diststep = strtof(optarg, NULL);
			m_nparg += 2;
			return 0;
		case 'c':
			m_bAutoColorDiff = true;
			m_colordiffstep = strtof(optarg, NULL);
			m_nparg += 2;
			return 0;
		default:
			return CApplication::handleOptions( optv );
	}
	return 1;
}

int CSelectFAPoints::mainstay() 
{
	if ( 0 != _createAllRenders() ) {
		cerr << "FATAL : failed to create sitRenders.\n";
		return -1;
	}
	
	if ( 0 != _armRenders() ) {
		return -1;
	}

	if ( !m_bAutoColorDiff && !m_bAutoDist ) {
		_lookupAllPairs();
	}
	else {
		_automaticLookup();
	}

	return 0;
}

int CSelectFAPoints::_assign_argvs(int renderIdx)
{
	// number of common arguments
	int nItmes = m_argc - (int)m_strfnsrcs.size()*2 - m_nparg;
	// -f is compulsory
	nItmes +=2;

	m_allargvs[renderIdx] = new char*[nItmes];
	int cntf = 0, cntthis = 0;
	// scan the gross argv in m_argv and pick up to assign to each render
	for (int i = 0; i < m_argc; i++) {
		if ( 0 == strncmp( m_argv[i], "-f", 2 ) ) {
			if (renderIdx == cntf) {
				m_allargvs[renderIdx][cntthis++] = m_argv[i];
				m_allargvs[renderIdx][cntthis++] = m_argv[i+1];
			}
			i ++;
			cntf ++;
			continue;
		}
		if ( 0 == strncmp( m_argv[i], "-s", 2 ) ||
			 0 == strncmp( m_argv[i], "-l", 2 ) ||
			 0 == strncmp( m_argv[i], "-S", 2 ) ||
			 0 == strncmp( m_argv[i], "-L", 2 ) ||
			 0 == strncmp( m_argv[i], "-t", 2 ) ||
			 0 == strncmp( m_argv[i], "-d", 2 ) ||
			 0 == strncmp( m_argv[i], "-c", 2 ) 
			 ) {
			i++;
			continue;
		}

		if ( 0 == strncmp( m_argv[i], "-u", 2 ) ) {
			continue;
		}

		// others are all common arguments, just collect as ther are
		m_allargvs[renderIdx][cntthis++] = m_argv[ i ];
	}
	return nItmes;
}

int CSelectFAPoints::_createAllRenders()
{
	int nRender = static_cast<int>( m_strfnsrcs.size() );
	if ( nRender < 1 ) {
		// nothing to do, let it go
		return -1;
	}

	m_allargvs.resize ( nRender );
	m_allargvs.assign( nRender, (char**)NULL );

	/* prepare command line arguments for each render
	 * since "-d" is optional and "-f" is compulsory, we count the number of
	 * input arguments for "-f" and presume the same number of renders
	 * will be needed
	 */
	int nItmes;
	for (int i = 0; i < nRender; i++) {
		nItmes = _assign_argvs( i );
		if ( nItmes < 1 ) {
			return -1;
		}

		m_cout << "----- command line for process " << i << " -------\n";
		for (int j = 0; j < nItmes; j++) {
			m_cout << m_allargvs[i][j] << " ";
		}
		m_cout << "\n";

		m_renders.push_back( new CSitubeRender( nItmes, m_allargvs[ i ] ) );
	}

	m_firstpts.resize( nRender );
	m_secondpts.resize( nRender );

	return 0;
}

int CSelectFAPoints::_armRenders() 
{
	int nRender = (int)m_renders.size();
	if ( nRender < 1 ) {
		return -1;
	}

	for (int i = 0; i < nRender; i++) {
		m_renders[i]->suspend(true);
		if ( 0 != m_renders[i]->run() ) {
			return -1;
		}
	}

	return 0;
}

bool CSelectFAPoints::_isDistinct( const vector< _point_t<unsigned long> >& firstpts,
					const vector< _point_t<unsigned long> >& secondpts)
{
	for (size_t i = 0; i < m_firstpts.size(); ++i) {
		const vector< _point_t<unsigned long> >& curfirst = m_firstpts[i];
		for (size_t j = 0; j < curfirst.size(); ++j) {
			if ( firstpts[j] == curfirst[j] ) {
				return false;
			}
		}
	}

	for (size_t i = 0; i < m_secondpts.size(); ++i) {
		const vector< _point_t<unsigned long> >& cursecond = m_secondpts[i];
		for (size_t j = 0; j < cursecond.size(); ++j) {
			if ( secondpts[j] == cursecond[j] ) {
				return false;
			}
		}
	}

	return true;
}

int CSelectFAPoints::_saveToFile(const vector<string>& keys, 
					const vector< _point_t<unsigned long> >& firstpts,
					const vector< _point_t<unsigned long> >& secondpts)
{
	// create a new directory set${m_setnum} under current directory
	ostringstream ostrdir;
	ostrdir << "./set" << m_setnum;
	if ( 0 != mkdir( ostrdir.str().c_str(), S_IRWXU ) ) {
		cerr << "Fatal: failed to create directory " << ostrdir.str() << "\n";
		return -1;
	}
	
	// save indices of each two points into a file for a single source geometry
	for (size_t i = 0; i < m_strfnsrcs.size(); ++i) {
		// extract file name from the source full path
		size_t loc = m_strfnsrcs[i].rfind('/');
		string fnfile;
		if ( string::npos != loc ) {
			fnfile.assign(m_strfnsrcs[i].begin() + loc + 1, m_strfnsrcs[i].end());
		}
		
		// form a file name
		ostringstream ostrfullfile;
		ostrfullfile << ostrdir.str() << "/" << "FApts_" << fnfile;
		
		ofstream ofs( ostrfullfile.str().c_str() );
		ofs << keys[i] << "\n";
		ofs << firstpts[i].x << " " << firstpts[i].y << "\n";
		ofs << secondpts[i].x << " " << secondpts[i].y << "\n";
		ofs.close();
		cout << "one more solution saved to " << ostrfullfile.str() << "\n";
	}

	m_setnum ++;

	return 0;
}

int CSelectFAPoints::_lookupAllPairs(int total) 
{
	int nRender = (int)m_renders.size();
	unsigned long idxl1, idxp1, idxl2, idxp2;
	_point_t<GLfloat> v01, v02, v1, v2;
	GLfloat dx, dy, dz;
	GLfloat curdist, curcolordiff;

	// take render 0 as the base, from which the seeds are chosen
	dx = (m_renders[0]->m_maxCoord[0]+m_renders[0]->m_minCoord[0])/2;
	dy = (m_renders[0]->m_maxCoord[1]+m_renders[0]->m_minCoord[1])/2;
	dz = (m_renders[0]->m_maxCoord[2]+m_renders[0]->m_minCoord[2])/2;

	int distx, disty, distz, i, cnt = 0;
	vector< _point_t<unsigned long> > solutions_1st;
	vector< _point_t<unsigned long> > solutions_2nd;
	vector<string> keys;

	for (distx = 0; distx <= int(dx); distx ++) {
		for (disty = 0; disty <= int(dy); disty ++) {
			for (distz = 0; distz <= int(dz); distz ++) {
				v01.update( m_renders[0]->m_minCoord[0] + distx,
						m_renders[0]->m_minCoord[1] + disty,
						m_renders[0]->m_minCoord[2] + distz );

				v02.update( m_renders[0]->m_maxCoord[0] - distx,
						m_renders[0]->m_maxCoord[1] - disty,
						m_renders[0]->m_maxCoord[2] - distz );

				ostringstream ostrCur;
				solutions_1st.clear();
				solutions_2nd.clear();
				keys.clear();
				for (i = 0; i < nRender; i++) {

					m_renders[i]->_findNearest<GLfloat>(
							v01.x, v01.y, v01.z,
							v1.x, v1.y, v1.z, idxl1, idxp1);

					m_renders[i]->_findNearest<GLfloat>(
							v02.x, v02.y, v02.z,
							v2.x, v2.y, v2.z, idxl2, idxp2);

					curdist = v1.distanceTo(v2);
#ifdef DEBUG	
					cerr << "curdist = " << curdist << "\n";
#endif

					if ( curdist < m_distance[0] || curdist > m_distance[1] ) {
						break;
					}

					_point_t<GLfloat> color1(
							m_renders[i]->m_alltubecolors[idxl1][idxp1*3+0],
							m_renders[i]->m_alltubecolors[idxl1][idxp1*3+1],
							m_renders[i]->m_alltubecolors[idxl1][idxp1*3+2]);

					_point_t<GLfloat> color2(
							m_renders[i]->m_alltubecolors[idxl2][idxp2*3+0],
							m_renders[i]->m_alltubecolors[idxl2][idxp2*3+1],
							m_renders[i]->m_alltubecolors[idxl2][idxp2*3+2]);
					curcolordiff = color1.distanceTo(color2);

#ifdef DEBUG	
					cerr << "curcolordiff = " << curcolordiff << "\n";
#endif
					if ( curcolordiff < m_colordiff[0] || curcolordiff > m_colordiff[1] ) {
						break;
					}

					ostrCur << idxl1 << " " << idxp1 << "\n" << idxl2 << " " << idxp2 << "\n";
					solutions_1st.push_back( _point_t<unsigned long>(idxl1, idxp1) );
					solutions_2nd.push_back( _point_t<unsigned long>(idxl2, idxp2) );
					// according to current FA color mapping that R is always
					// 1.0, the larger the other two channels, the whiter the
					// color, and we take reder point as the point of greater FA
					// value. so we have following judgement
					keys.push_back( color1.magnitude() > color2.magnitude()? "B":"A" );
				}

				if ( i < nRender ) {
					continue;
				}

				// one pair found, check if it is necessary to save it
				if (m_bDistinct && !_isDistinct( solutions_1st, solutions_2nd )) {
#ifdef DEBUG	
					cerr << "a similar solution skipped.\n";
#endif
					continue;
				}

#ifdef DEBUG	
				cout << "solution found: " << ostrCur.str() << "\n";
#endif
				m_firstpts.push_back( solutions_1st );
				m_secondpts.push_back( solutions_2nd );
				
				// if need, save this solution
				if ( 0 != _saveToFile(keys, solutions_1st, solutions_2nd) ) {
					cout << "FATAL: failure in saving solution No." << cnt << "\n";
					return cnt;
				}

				cnt ++;
				// we might do not want too many solutions
				if ( -1 != total && cnt > total ) {
					cout << "reach total limit, abort here right now.\n";
					return total;
				}
			}
		}
#ifdef DEBUG	
		cerr << "cur distx= " << distx << ".....\n";
#endif
	}
	return cnt;
}

int CSelectFAPoints::_automaticLookup(int total)
{
	GLfloat maxDist = m_diststep;
	if ( m_bAutoDist ) {
		maxDist = _point_t<GLfloat> (
				m_renders[0]->m_maxCoord[0],
				m_renders[0]->m_maxCoord[1],
				m_renders[0]->m_maxCoord[2]).distanceTo(
					_point_t<GLfloat>(
				m_renders[0]->m_minCoord[0],
				m_renders[0]->m_minCoord[1],
				m_renders[0]->m_minCoord[2]) );
	}

	GLfloat maxColordiff = m_colordiffstep;
	if ( m_bAutoColorDiff ) {
		maxColordiff = _point_t<GLfloat> (1.0, 0.0, 0.0).distanceTo(
				_point_t<GLfloat> (1.0, 1.0, 1.0));
	}

	int num = 0, limit = total, sum = 0;

	for (GLfloat dist = 0; dist <= maxDist - m_diststep; dist += m_diststep) {
		if ( m_bAutoDist ) {
			m_distance[0] = dist;
			m_distance[1] = dist + m_diststep;
		}

		for (GLfloat colordiff = 0; colordiff <= maxColordiff - m_colordiffstep; 
				colordiff += m_colordiffstep ) {
			if ( m_bAutoColorDiff ) {
				m_colordiff[0] = colordiff;
				m_colordiff[1] = colordiff + m_colordiffstep;
			}

			cout << "Now search in distance:[" << m_distance[0] << "," << m_distance[1]
				<< "]; color difference:[" << m_colordiff[0] << "," << m_colordiff[1] << "].\n";
			sum += ( num = _lookupAllPairs(-1==total?-1: limit-num) );
		}
	}

	return sum;
}

int main(int argc, char** argv)
{
	CSelectFAPoints selfa(argc, argv);
	selfa.setVerinfo("selectFApoints");
	selfa.run();
	return 0;
}

/* set ts=4 sts=4 tw=80 sw=4 */

