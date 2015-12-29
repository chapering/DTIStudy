// ----------------------------------------------------------------------------
// multipleitr.cpp : a multiple-window itubeRender capsule, loading multiple
//					DTI voxel model each assigned with a separate window using
//					multiple process implementation;
//
//					this capsule provides centralized interaction events
//					dispatching, thus offering a sort of synchronous interaction
//					response
//
// Creation : Mar. 2nd 2011
//
// Revision:
//	@Mar. 3rd
//		.Change the modality from multiple-process to multiple-thread, since
//		fork() just copies its parent's memory, this is going against our needs
//		that the sub processes should do with different models
//
//		--- multiple threading modality failed, due to, at least, glut does not
//		permit multiple initialization, there might be many other things that
//		are only allowed at the process level, that is there must be
//		independent/separate piece of memory to work in.
//
//		--- so go back to multiple process mode, and find the real problem (the
//		problem may not be as guessed above.
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#include "itubeRender.h"

#include <unistd.h>
#include <signal.h>
#include <pthread.h>

using std::vector;
using std::string;
using std::cout;
using std::cerr;

class CMitubeRender: public CGLIBoxApp {
public:
	CMitubeRender(int argc, char **argv) : CGLIBoxApp(argc, argv) {
		setVerinfo("mitubeRender");

		addOption('f', true, "input-file-name", "the name of source file"
			   " containing geometry and in the format of tgdata");
		addOption('g', true, "output-file-name", "the name of target file"
			   " to store the geometry of streamtubes produced");
		addOption('r', true, "tube-radius", "fixed radius of the tubes"
				" to generate");
		addOption('l', true, "lod", "level ot detail controlling the tube"
				" generation, it is expected to impact the smoothness of tubes");
		addOption('b', true, "box-num", "number of selection box"
				" which is 1 by default");
		addOption('d', true, "dwi-b0-dir", "directory holding DWI b0 DICOM images");

	}

	~CMitubeRender()
	{
		_killallchildren();
		for (size_t i = 0; i < m_allargvs.size(); ++i) {
			delete [] m_allargvs[i];
			m_allargvs[i] = NULL;
		}

		for (size_t i = 0; i < m_renders.size(); ++i) {
			delete m_renders[i];
			m_renders[i] = NULL;
		}
	}

	int handleOptions(int optv) {
		switch( optv ) {
			case 'f':
				m_strfnsrcs.push_back(optarg);
				return 0;
			case 'd':
				m_strdwidirs.push_back( optarg );
				return 0;
			case 'b':
			case 'l':
			case 'r':
				return 0;
			default:
				return CGLIBoxApp::handleOptions( optv );
		}
		return 1;
	}

	void keyResponse(unsigned char key, int x, int y) {
		switch (key) {
			default:
				return CGLIBoxApp::keyResponse(key, x, y);
		}
		glutPostRedisplay();
	}

	int mainstay() {
		setGlutInfo(" multiple DTI model visualization");

		if ( 0 != _createchildren() ) {
			cerr << "Failed to create multiple tasks.\n";
			return -1;
		}

		startchildren();

		//return CGLIBoxApp::mainstay();

		for (size_t i = 0; i < m_allchildpids.size(); ++i) {
			pthread_join( m_allchildpids[i], (void**)NULL );
		}

		return 0;
	}

	static void* _threadfunc(void* pData) 
	{
		CSitubeRender* pRender = (CSitubeRender*)pData;
		pRender->run();

		pthread_exit(0);
	}

	int startchildren() {
		// start all processes, each for a single model render
		for (size_t i = 0; i < m_renders.size(); ++i) {
			if ( 0 != pthread_create(&m_allchildpids[i], NULL, _threadfunc,
					   m_renders[i]) )	{
				cerr << "Failed to start process for render No." << i << "\n";
				_killallchildren();
				return -1;
			}

			cout << "one more instance of TubeRender started, pid = " << m_allchildpids[i] << ". \n";
		}

		return 0;
	}

	void _realcopy(char*& dst, char* src) {
		dst = new char[ strlen(src) + 1 ];
		strncpy(dst, src, strlen(src)+1 );
	}

	int _assign_argvs(int renderIdx)
	{
		// number of common arguments
		int nItmes = m_argc - (int)m_strfnsrcs.size()*2 - (int)m_strdwidirs.size()*2;
		// -f is compulsory
		nItmes +=2;
		// -d is optional, the dwi dirs will be assigned from left to right when
		// it is exhausted, the later render will have not got this argument 
		if ( renderIdx < (int)m_strdwidirs.size() ) {
			nItmes+=2;
		}

		m_allargvs[renderIdx] = new char*[nItmes];
		int cntf = 0, cntd = 0, cntthis = 0;
		// scan the gross argv in m_argv and pick up to assign to each render
		for (int i = 0; i < m_argc; i++) {
			if ( 0 == strncmp( m_argv[i], "-f", 2 ) ) {
				if (renderIdx == cntf) {
					_realcopy( m_allargvs[renderIdx][cntthis++], m_argv[i]);
					_realcopy( m_allargvs[renderIdx][cntthis++], m_argv[i+1]);
				}
				i ++;
				cntf ++;
				continue;
			}

			if ( 0 == strncmp( m_argv[i], "-d", 2 ) ) {
				if (renderIdx == cntd) {
					_realcopy( m_allargvs[renderIdx][cntthis++], m_argv[i]);
					_realcopy( m_allargvs[renderIdx][cntthis++], m_argv[i+1]);
				}
				i ++;
				cntd ++;
				continue;
			}

			// others are all common arguments, just collect as ther are
			_realcopy( m_allargvs[renderIdx][cntthis++], m_argv[ i ]);
		}
		return nItmes;
	}

	int _createchildren()
	{
		int nRender = static_cast<int>( m_strfnsrcs.size() );
		if ( nRender < 1 ) {
			// nothing to do, let it go
			return -1;
		}

		m_allargvs.resize ( nRender );
		m_allargvs.assign( nRender, (char**)NULL );

		m_allchildpids.resize ( nRender );
		m_allchildpids.assign( nRender, -1 );
		
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

			for (int j = 0; j < nItmes; j++) {
				cout << m_allargvs[i][j] << " ";
			}
			cout << "\n";

			m_renders.push_back( new CSitubeRender( nItmes, m_allargvs[ i ] ) );
		}

		return 0;
	}

	int _killallchildren()
	{
		int ret = 0;
		for (size_t i = 0; i < m_allchildpids.size(); ++i) {
			if ( m_allchildpids[i] > 0 ) {
				pthread_cancel(m_allchildpids[i]);
				ret ++;
			}
		}
		return ret;
	}

private:
	// a set of source geometry files
	vector<string> m_strfnsrcs;
	// a set of DWI image directories
	vector<string> m_strdwidirs;

	// a set of renders, each for a single model, associated with a single
	// source geometry file and a DWI image directory, but they will share all
	// other command line arguments like lod, tube radius and number of
	// selection boxes, etc.
	vector<CSitubeRender*> m_renders;

	vector<char**>		m_allargvs;
	vector<pthread_t> 	m_allchildpids;

	/*
	int _createchildren();
	int _killallchildren();
	*/
};

int main(int argc, char** argv)
{
	CMitubeRender mit(argc, argv);
	mit.run();
	return 0;
}

/* set ts=4 sts=4 tw=80 sw=4 */

