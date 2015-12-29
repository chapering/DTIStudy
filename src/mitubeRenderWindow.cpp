// ----------------------------------------------------------------------------
// mitubeRenderWindow : a multiple-window itubeRender capsule, loading multiple
//					DTI voxel model each assigned with a separate window using
//					multiple process run-time modality
//
//					parallel multiple windows, purely GLUT based
//
// Creation : Mar. 2nd 2011
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#include "mitubeRenderWindow.h"

using std::cout;
using std::cerr;
using std::ostringstream;

///////////////////////////////////////////////////////////////////////////////
//
//	Implementation of the Class CMitubeRenderWindow
//
CMitubeRenderWindow* CMitubeRenderWindow::m_pmitInstance = NULL;
CMitubeRenderWindow::CMitubeRenderWindow(int argc, char **argv) : 
	CSitubeRender(argc, argv),
	m_bSync(true)
{
	addOption('f', true, "input-files", "the name of source file containing"
		   " geometry in the format of tgdata, each one given by a -f ");
	addOption('d', true, "dwi-b0-dirs", "directory holding DWI b0 DICOM images"
		   ",each one given by a -d ");
}

CMitubeRenderWindow::~CMitubeRenderWindow()
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

int CMitubeRenderWindow::handleOptions(int optv) 
{
	switch( optv ) {
		case 'f':
			m_strfnsrcs.push_back(optarg);
			return 0;
		case 'd':
			m_strdwidirs.push_back( optarg );
			return 0;
		case 'b':
		case 'l':
		case 'g':
		case 'r':
		case 'p':
		case 't':
			return 0;
		default:
			return CSitubeRender::handleOptions( optv );
	}
	return 1;
}

void CMitubeRenderWindow::keyResponse(unsigned char key, int x, int y) 
{
	switch (key) {
		case 27:
		case '0':
			return;
		default:
			break;
	}

	// when not in the synchronization mode, we only take care of the focused
	// window
	if ( !m_bSync ) {
		m_renders[ glutGetWindow() - 1 ]->keyResponse(key, x, y);
		glutPostRedisplay();
		return;
	}

	for (int i = 0; i < (int)m_renders.size(); i++) {
		glutSetWindow(i+1);
		m_renders[i]->keyResponse(key, x, y);
		glutPostRedisplay();
	}
}

void CMitubeRenderWindow::specialResponse(int key, int x, int y)
{
	switch (key) {
		case GLUT_KEY_INSERT:
			m_cout << "quit.\n";
			cleanup();
			exit (EXIT_SUCCESS);
		case GLUT_KEY_F5:
			m_bSync = !m_bSync;
			return;
		default:
			break;
	}

	// when not in the synchronization mode, we only take care of the focused
	// window
	if ( !m_bSync ) {
		m_renders[ glutGetWindow() - 1 ]->specialResponse(key, x, y);
		glutPostRedisplay();
		return;
	}

	for (int i = 0; i < (int)m_renders.size(); i++) {
		glutSetWindow(i+1);
		m_renders[i]->specialResponse(key, x, y);
		glutPostRedisplay();
	}
}

void CMitubeRenderWindow::mouseResponse(int button, int state, int x, int y)
{
	// when not in the synchronization mode, we only take care of the focused
	// window
	if ( !m_bSync ) {
		m_renders[ glutGetWindow() - 1 ]->mouseResponse(button, state, x, y);
		glutPostRedisplay();
		return;
	}

	for (int i = 0; i < (int)m_renders.size(); i++) {
		glutSetWindow(i+1);
		m_renders[i]->mouseResponse(button, state, x, y);
		glutPostRedisplay();
	}
}

void CMitubeRenderWindow::mouseMotionResponse(int x, int y)
{
	// when not in the synchronization mode, we only take care of the focused
	// window
	if ( !m_bSync ) {
		m_renders[ glutGetWindow() - 1 ]->mouseMotionResponse(x, y);
		glutPostRedisplay();
		return;
	}

	for (int i = 0; i < (int)m_renders.size(); i++) {
		glutSetWindow(i+1);
		m_renders[i]->mouseMotionResponse(x, y);
		glutPostRedisplay();
	}
}

void CMitubeRenderWindow::mousePassiveMotionResponse(int x, int y)
{
	// when not in the synchronization mode, we only take care of the focused
	// window
	if ( !m_bSync ) {
		m_renders[ glutGetWindow() - 1 ]->mousePassiveMotionResponse(x, y);
		glutPostRedisplay();
		return;
	}

	for (int i = 0; i < (int)m_renders.size(); i++) {
		glutSetWindow(i+1);
		m_renders[i]->mousePassiveMotionResponse(x, y);
		glutPostRedisplay();
	}
}

void CMitubeRenderWindow::mouseWheelRollResponse(int wheel, int direction, int x, int y)
{
	// when not in the synchronization mode, we only take care of the focused
	// window
	if ( !m_bSync ) {
		m_renders[ glutGetWindow() - 1 ]->mouseWheelRollResponse(wheel, direction, x, y);
		glutPostRedisplay();
		return;
	}

	for (int i = 0; i < (int)m_renders.size(); i++) {
		glutSetWindow(i+1);
		m_renders[i]->mouseWheelRollResponse(wheel, direction, x, y);
		glutPostRedisplay();
	}
}

void CMitubeRenderWindow::onReshape( GLsizei w, GLsizei h )
{
	/*
	cout << "in Reshape:: this windows is : " << glutGetWindow() << "\n";
	*/
	int idx = glutGetWindow();
	if ( idx - 1 >= 0 && idx - 1 < (int)(m_pmitInstance->m_renders.size()) ) {
		m_pmitInstance->m_renders[idx-1]->onReshape(w, h);
	}
}

void CMitubeRenderWindow::display( void )
{
	/*
	cout << "in display:: this windows is : " << glutGetWindow() << "\n";
	*/
	int idx = glutGetWindow();
	if ( idx - 1 >= 0 && idx - 1 < (int)(m_pmitInstance->m_renders.size()) ) {
		// dispatching drawing process to window-associated render
		m_pmitInstance->m_renders[idx-1]->display();
	}
}

int CMitubeRenderWindow::mainstay() 
{
	if ( 0 != _createAllRenders() ) {
		cerr << "FATAL : failed to create sitRenders.\n";
		return -1;
	}

	if ( 0 != _armRenders() ) {
		return -1;
	}

	m_pInstance = this;
	m_pmitInstance = this;

	glutInit (&m_argc, m_argv);
	glutInitDisplayMode	( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
	glInit();

	_createAllWindows();
	glutMainLoop();

	return 0;
}

/*
 * we set a layout in which each row holds at most two subwindows, then
 * followed by other rows if needed
 */
int CMitubeRenderWindow::_createAllWindows()
{
	int nRender = (int)m_renders.size();
	if ( nRender < 1 ) {
		return -1;
	}

	int w, h, x = 0, y = 0;
	CGLApp::getScreenSize(&w, &h);

	w -= (nRender-1)*5;
	h -= (nRender-1)*20;

	w /= (nRender<=1?1:2);
	h /= nRender<=1?1:(int)(1.0*nRender/2+.5); 
	glutInitWindowPosition ( x, y );
	glutInitWindowSize ( w, h );
	for (int i = 0; i < nRender; i++) {
		if ( i % 2 == 0 ) {
			x = 0;
		}
		else {
			// need to take into account the window borders
			x = (w+5);
		}

		if ( i != 0 && i % 2 == 0 ) {
			y += (h+25);
		}

		ostringstream ostrTitle;
		ostrTitle << " DTI voxel model render - " << i;
		m_renders[i]->setGlutInfo(ostrTitle.str().c_str(),
				x, y, w, h);

		glutCreateWindow ( ostrTitle.str().c_str() );
		glutPositionWindow(x, y);

		/* all windows will all share the same interaction input */
		glutMouseFunc(_mouseResponse);
		glutMotionFunc(_mouseMotionResponse);
		glutPassiveMotionFunc(_mousePassiveMotionResponse);
		//glutMouseWheelFunc(_mouseWheelRollResponse);
		glutKeyboardFunc(_keyResponse);
		glutSpecialFunc(_specialResponse);

		/* but not the drawing stuff */
		glutReshapeFunc(_onReshape);
		glutDisplayFunc(_display);

		m_renders[i]->glInit();
	}

	return 0;
}

int CMitubeRenderWindow::_assign_argvs(int renderIdx)
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
				m_allargvs[renderIdx][cntthis++] = m_argv[i];
				m_allargvs[renderIdx][cntthis++] = m_argv[i+1];
			}
			i ++;
			cntf ++;
			continue;
		}

		if ( 0 == strncmp( m_argv[i], "-d", 2 ) ) {
			if (renderIdx == cntd) {
				m_allargvs[renderIdx][cntthis++] = m_argv[i];
				m_allargvs[renderIdx][cntthis++] = m_argv[i+1];
			}
			i ++;
			cntd ++;
			continue;
		}

		// others are all common arguments, just collect as ther are
		m_allargvs[renderIdx][cntthis++] = m_argv[ i ];
	}
	return nItmes;
}

int CMitubeRenderWindow::_createAllRenders()
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

	return 0;
}

int CMitubeRenderWindow::_armRenders() 
{
	int nRender = (int)m_renders.size();
	if ( nRender < 1 ) {
		return -1;
	}

	for (int i = 0; i < nRender; i++) {
		m_renders[i]->suspend(true);
		// user never need repeated block of help prompt
		m_renders[i]->switchhelp(0 == i);
		m_renders[i]->run();
	}

	return 0;
}

/* set ts=4 sts=4 tw=80 sw=4 */

