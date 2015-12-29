// ----------------------------------------------------------------------------
// GLiboxmoth.cpp: an extension to CGLApp that supports multiple box selection
//			 on the basis of the simplest collision detection 
//
// Creation : Feb. 12th 2011
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#include "GLiboxmoth.h"
#include "bitmap.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////
// class implementation of CGLIBoxApp
//
CGLIBoxApp::CGLIBoxApp() : CGLApp(),
	m_curselboxidx(-1),
	m_curhlboxidx(-1),
	m_pvertices(NULL),
	m_bIboxEnabled(true),
	m_bIboxCulling(true),
	m_bRemovalbased(false),
	m_bOR(false),
	m_bBtnEnabled(false),
	m_curselbtnidx(-1),
	m_curhlbtnidx(-1),
	m_btnMenu(-1),
	m_bMouseOnlyForBox(false)
{
}

CGLIBoxApp::CGLIBoxApp(int argc, char **argv) : CGLApp(argc, argv),
	m_curselboxidx(-1),
	m_curhlboxidx(-1),
	m_pvertices(NULL),
	m_bIboxEnabled(true),
	m_bIboxCulling(true), 
	m_bRemovalbased(false),
	m_bOR(false),
	m_bBtnEnabled(true),
	m_curselbtnidx(-1),
	m_curhlbtnidx(-1),
	m_btnMenu(-1),
	m_bMouseOnlyForBox(false)
{
	// in selection box application, gadgets are often needed since
	// application in this catalog mostly involves 3D interaction
	m_bGadgetEnabled = true;
	m_cout.switchtime(true);
}

CGLIBoxApp::~CGLIBoxApp()
{
	// we may have to release data ever allocated for button icon images
	for (size_t i = 0; i < m_buttons.size(); ++i) {
		if ( m_buttons[i].pIconImgData != NULL ) {
			//XFree( m_buttons[i].pIconImgData );
			free( m_buttons[i].pIconImgData );
			m_buttons[i].pIconImgData = NULL;
		}
		if ( m_buttons[i].pIconImgData2 != NULL ) {
			free( m_buttons[i].pIconImgData2 );
			m_buttons[i].pIconImgData2 = NULL;
		}
	}
}

int CGLIBoxApp::associateObj(vector<GLdouble>* pvertices, bool bcalminmax,
						GLdouble dx, GLdouble dy, GLdouble dz)
{
	if ( !pvertices ) {
		return -1;
	}

	size_t szVertex = pvertices->size()/3;

	if ( szVertex <= 0 ) { // guard against the trivial request
		return 0;
	}

	m_pvertices = pvertices;

	// establish edge flags for vertices so that to ease the visibility
	// controls and initialize it as such that all vertices are visible
	m_edgeflags.resize ( szVertex );
	m_edgeflags.assign ( szVertex, GL_TRUE );

	m_dx = dx, m_dy = dy, m_dz = dz;
	updateGadgets();

	// if the clientele did not call setMinMax before, it might want calculate
	// the bounding corners here
	if ( !bcalminmax ) {
		return 0;
	}

	// C++ does not support array of references, which sounds reasonable
	// however. so we only to employ array of pointers again
	GLdouble *mins[3] = { &m_minx, &m_miny, &m_minz },
			*maxs[3] = { &m_maxx, &m_maxy, &m_maxz };
	for (size_t idx = 0; idx < szVertex; ++idx) {
		for (size_t j = 0; j < 3; ++j) {
			if ( (*pvertices)[ idx*3 + j ] < *mins[ j ] ) {
				*mins [ j ] = (*pvertices)[ idx*3 + j ];
			}
			if ( (*pvertices)[ idx*3 + j ] > *maxs[ j ] ) {
				*maxs [ j ] = (*pvertices)[ idx*3 + j ];
			}
		}
	}

	return 0;
}

void CGLIBoxApp::drawBoxes()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	//glEnable(GL_LINE_SMOOTH);
	//glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	size_t szBoxes = m_boxes.size();
	for (size_t idx = 0; idx < szBoxes; idx++) {
		m_boxes[idx].drawFrame(m_bBtnEnabled);
	}
	glPopMatrix();
	glPopAttrib();

	// draw the beacon, the box centric point projected to the ground
	if (m_curselboxidx != -1) {
		drawBeacon();
	}
}

int CGLIBoxApp::addBox(const CIselbox& selbox)
{
	if ( !m_bIboxEnabled ) {
		return -1;
	}
	// from the programmability standpoint, any number of selection box is
	// acceptable but in actuality, too much, over 10 for instance, should be
	// mischievous and declined therefore
	if ( m_boxes.size() >= 10 ) {
		m_cout << "Too many selection boxes while Upper limit is 10.\n";
		return -1;
	}
	m_boxes.push_back( selbox );
	// the responsiblity of setting the layout for the new selection box is
	// attributed to the user who creates the instance selbox
	return 0;
}

int CGLIBoxApp::addBox()
{
	if ( !m_bIboxEnabled ) {
		return -1;
	}
	CIselbox selbox;
	if ( m_cout.isswitchon() ) {
		m_cout << "debug output switched on\n";
		selbox.m_cout.switchoff(false);
		selbox.m_cout.switchtime(true);
	}
	else {
		selbox.m_cout.switchoff(true);
	}
	selbox.associateObj(m_pvertices, &m_edgeflags, 
			m_buttons.size()>=1?&m_buttons:NULL,
			m_dx, m_dy, m_dz);
	addBox( selbox );
	return _updateDefaultLayout();
}

int CGLIBoxApp::removeBox(int idx)
{
	if ( !m_bIboxEnabled ) {
		return -1;
	}
	// trivial check against invalid index
	if (idx < 0 || idx >= static_cast<int>(m_boxes.size()) ) {
		if ( -1 == idx ) {
			if ( -1 != m_curhlboxidx ) {
				idx = m_curhlboxidx;
			}
			else if ( -1 != m_curselboxidx ) {
				idx = m_curselboxidx;
			}
			else if (m_boxes.size() >= 1) {
				idx = int(m_boxes.size()) - 1;
			}
			else {
				return -1;
			}
		}
		else {
			return -1;
		}
	}

	m_boxes.erase( m_boxes.begin() + idx );
	// now since the box store has been changed, index to it will also need
	// changing - elements after idx have been moved forwards by just one
	// slot, and "This invalidates all iterator and references to elements after
	// position or first."
	if ( m_curselboxidx >= idx ) {
		m_curselboxidx --;
	}
	else if ( m_curselboxidx == idx ) {
		m_curselboxidx = -1;
	}

	if ( m_curhlboxidx >= idx ) {
		m_curhlboxidx --;
	}
	else if ( m_curhlboxidx == idx ) {
		m_curhlboxidx = -1;
	}

	// when all boxes are remove, we need a in-situ way (avoid all keyborad
	// interactivity) to add back a box
	
	if ( m_bBtnEnabled && 0 == m_boxes.size() ) {
		if ( -1 == m_btnMenu ) {
			_createRCMenu();
		}
		glutAttachMenu(GLUT_RIGHT_BUTTON);
	}

	return 0;
}

int CGLIBoxApp::duplicateBox(GLdouble dx, GLdouble dy, GLdouble dz)
{
	if ( !m_bIboxEnabled ) {
		return -1;
	}
	if ( m_boxes.size() < 1 ) {
		return addBox();
	}

	int idx = -1;
	if ( -1 != m_curhlboxidx ) {
		idx = m_curhlboxidx;
	}
	else if ( -1 != m_curselboxidx ) {
		idx = m_curselboxidx;
	}
	else if (m_boxes.size() >= 1) {
		idx = int(m_boxes.size()) - 1;
	}

	if (idx >= 0 && idx < static_cast<int>(m_boxes.size()) ) {
		return addBox( m_boxes[idx].clone(dx, dy, dz) );
	}

	return idx;
}

int CGLIBoxApp::getSelectedBtn(GLdouble x, GLdouble y)
{
	if ( !m_bBtnEnabled || m_buttons.size() < 1 ||
		-1 == m_curhlboxidx || !m_bIboxEnabled ) {
		return -1;
	}

	int szBtn = (int)m_buttons.size();
	int ret = -1;
	for (int idx = 0; idx < szBtn; idx++) {
		if ( m_buttons[idx].isInButton(x,y) ) {
			ret = idx;
			break;
		}
	}

	return ret;
}

bool CGLIBoxApp::isPointInBox(GLdouble x, GLdouble y, GLdouble z)
{
	if (!m_bIboxCulling) {
		return true;
	}

	// judge if (x,y,z) in any box
	//
	size_t szBoxes = m_boxes.size();
	for (size_t idx = 0; idx < szBoxes; idx++) {
		if (m_boxes[idx].isInside(x, y, z)) {
			return true;
		}
	}
	return false;
}

void CGLIBoxApp::glInit(void)
{
	CGLApp::glInit();
}

void CGLIBoxApp::addButton(const string& btntext, const char* fnIconImage,
							const string& btntext2, const char* fnIconImage2)
{
	if ( !m_bBtnEnabled ) {
		return;
	}

	if ( strlen(fnIconImage) < 1 ) {
		m_buttons.push_back( button_t( btntext,btntext2 ) );
		return;
	}

	// ready the data of image for button icon
	button_t btn(btntext,btntext2);
	// we never bear an image as big as more than 4K for just an icon
	/* unsigned char _bmpdata[4096]; */
	/*
	int ret = XReadBitmapFileData(fnIconImage, 
				&btn.iconW, &btn.iconH, 
				&btn.pIconImgData,
				&btn.iconX, &btn.iconY);
	if ( BitmapSuccess != ret ) {
		m_cout << "Failed to read image for button icon - " << 
			fnIconImage << " -> disregarded [err: " << ret << "].\n";
	}
	*/
	/*
	else {
		try {
			btn.pIconImgData = new unsigned char [btn*iconW * btn.iconH];
			memcpy(btn.pIconImgData, _bmpdata, 
					btn.iconW * btn.iconH * sizeof(unsigned char));
		}
		catch (std::bad_alloc & e) {
			m_cout << "Failed to allocate memory to hold an icon image in " <<
				fnIconImage << " -> dropped.\n";
			// resume as if the button were never fed an image as icon for it
			btn.pIconImgData = NULL;
			btn.iconW = btn.iconH = btn.iconX = btn.iconY = 0;
		}
	}
	*/
	
	BITMAPINFO *pBmpInfo;
	if ( NULL == ( btn.pIconImgData = LoadDIBitmap(fnIconImage, &pBmpInfo) ) ) {
		m_cout << "Failed to read image for button icon - " << 
			fnIconImage << " -> disregarded .\n";
	}
	else {
		btn.iconW = pBmpInfo->bmiHeader.biWidth;
		btn.iconH = pBmpInfo->bmiHeader.biHeight;
		//cout << "bitcount=" << pBmpInfo->bmiHeader.biBitCount << ", size=" << pBmpInfo->bmiHeader.biSizeImage << "\n";
		//cout << "planes=" << pBmpInfo->bmiHeader.biPlanes << ", #colors=" << pBmpInfo->bmiHeader.biClrUsed << "\n";
		free(pBmpInfo);
	}

	// load image data for the button icon for the secondary status
	if ( strlen(fnIconImage2) >= 1 ) {
		if ( NULL == ( btn.pIconImgData2 = LoadDIBitmap(fnIconImage2, &pBmpInfo) ) ) {
			m_cout << "Failed to read secondary image for button icon - " << 
				fnIconImage2 << " -> disregarded .\n";
		}
		else {
			btn.iconW2 = pBmpInfo->bmiHeader.biWidth;
			btn.iconH2 = pBmpInfo->bmiHeader.biHeight;
			//cout << "bitcount=" << pBmpInfo->bmiHeader.biBitCount << ", size=" << pBmpInfo->bmiHeader.biSizeImage << "\n";
			//cout << "planes=" << pBmpInfo->bmiHeader.biPlanes << ", #colors=" << pBmpInfo->bmiHeader.biClrUsed << "\n";
			free(pBmpInfo);
		}
	}

	m_buttons.push_back( btn );
}

void CGLIBoxApp::keyResponse(unsigned char key, int x, int y)
{
	switch (key) {
		case 'b':
			if ( !m_bIboxEnabled ) {
				return;
			}
			_genBoxColors();
			glutPostRedisplay();
			return;
		case 't':
			m_bBtnEnabled = !m_bBtnEnabled;
			glutPostRedisplay();
			return;
		case 'r':
			if ( !m_bIboxEnabled ) {
				return;
			}
			m_bRemovalbased = ! m_bRemovalbased;
			m_cout << "Enter into " << 
				(m_bRemovalbased?"removal":"selection") << " mode.\n";
			glutPostRedisplay();
			return;
		default:
			break;
	}
	CGLApp::keyResponse(key, x, y);
}

void CGLIBoxApp::specialResponse(int key, int x, int y)
{
	switch (key) {
		case GLUT_KEY_F10:
			m_bIboxEnabled = !m_bIboxEnabled;
			glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
			m_cout << "selection box " << 
				(m_bIboxEnabled?"Enabled":"Disabled") << "\n";
			glutPostRedisplay();
			return;
		case GLUT_KEY_F9:
			if ( !m_bIboxEnabled ) {
				return;
			}
			m_bIboxCulling = !m_bIboxCulling;
			m_cout << "box culling " << 
				(m_bIboxCulling?"on":"off") << "\n";
			glutPostRedisplay();
			return;
		case GLUT_KEY_F8:
			if ( !m_bIboxEnabled ) {
				return;
			}
			for (size_t idx = 0; idx < m_boxes.size(); idx++) {
				m_boxes[idx].switchLimit();
			}
			return;
		case GLUT_KEY_F7:
			if ( !m_bIboxEnabled ) {
				return;
			}
			for (size_t idx = 0; idx < m_boxes.size(); idx++) {
				m_boxes[idx].switchHint();
			}
			glutPostRedisplay();
			return;
		case GLUT_KEY_F4: // remove current selection box
			if ( !m_bIboxEnabled ) {
				return;
			}
			if (0 == removeBox() ) {
				m_cout << "Box removed.\n";
			}
			glutPostRedisplay();
			return;
		case GLUT_KEY_F3: // duplicate current selection box
			if ( !m_bIboxEnabled ) {
				return;
			}
			if (0 == duplicateBox(10,10,10)) {
				m_cout << "A box duplicated and added.\n";
			}
			glutPostRedisplay();
			return;
		case GLUT_KEY_F2:
			if ( !m_bIboxEnabled ) {
				return;
			}
			m_bOR = !m_bOR;
			m_cout << "Changed into " << 
				(m_bOR?"OR":"AND") << " associative pattern.\n";
			glutPostRedisplay();
			return;
		default:
			break;
	}
	CGLApp::specialResponse(key, x, y);
}

void CGLIBoxApp::mouseResponse(int button, int state, int x, int y)
{
	if ( !m_bIboxEnabled || m_boxes.size() < 1 ) {
		m_curselbtnidx = -1;
		if ( !m_bMouseOnlyForBox ) {
			CGLApp::mouseResponse(button, state, x, y);
		}
		return;
	}

	/*
	// check if right click menu needs to pop up
	if ( GLUT_RIGHT_BUTTON == button && GLUT_DOWN == state ) {
		for (size_t idx = 0; idx < szBoxes; idx++) {
			if ( -1 != m_boxes[idx].getSelectedVertex(x,y) ) {
				glutAttachMenu(GLUT_RIGHT_BUTTON);
			}
		}
	}
	*/

	int curret = -1, maxret = -1;
	m_curselboxidx = -1;
	switch (state) {
	case GLUT_DOWN: 
		{
			// the first selection box that accepts the mouse event is regarded
			// as the currently focused
			size_t szBoxes = m_boxes.size();
			for (size_t idx = 0; idx < szBoxes; idx++) {
				curret = m_boxes[idx].onMouseClicked(button, state, x, y);
				if ( curret != 0 && curret != 2 && curret != 4) {
					continue;
				}
				if ( curret > maxret ) {
					m_curselboxidx = idx;
					maxret = curret;
				}
			}

			if ( -1 == m_curhlboxidx ) {
				break;
			}

			m_curselbtnidx = getSelectedBtn(x,y);
			if ( GLUT_LEFT_BUTTON == button ) {
				if ( -1 != m_curselbtnidx ) {
					// we desire a single response, rather than duplex behaviour, to
					// a single interaction
					m_buttons[m_curselbtnidx].setPressed(true);
					if ( 0 == m_curselbtnidx ) {
						// add by coping a box and start anchoring the new box
						// seamlessly
						if (0 == duplicateBox(10,10,10)) {
							m_cout << "A box duplicated and added.\n";
						}
						glutDetachMenu(GLUT_RIGHT_BUTTON);
						m_curselboxidx = m_boxes.size() - 1;
						m_boxes[m_curselboxidx].onMouseClicked(
								GLUT_RIGHT_BUTTON, GLUT_DOWN, 
								x+15, y+15);
						return;
					}
					glutPostRedisplay();
					return;
				}
			}
			/*
			else {
				glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
			}
			*/
		}
		break;
	case GLUT_UP:
		{
			if ( -1 != m_curselboxidx ) {
				m_boxes[ m_curselboxidx ].onMouseClicked( 
						button, state, 0,0);
				return;
			}

			// respond to box button if possible
			// to follow the convention of mouse interaction, only if the mouse
			// is pressed and bounced up at a same button, we then deem the user
			// really wants to hit it 
			//
			// ------- an exception is the button for "adding a box", which is
			// specialized because we want a more handy operation for this
			// function
			if ( -1 != m_curselbtnidx ) {
				m_buttons[m_curselbtnidx].setPressed(false);
				if ( 0 != m_curselbtnidx && m_curselbtnidx == getSelectedBtn(x,y)) {
					handleBtnEvents( m_curselbtnidx );
				}
				else {
					m_buttons[m_curselbtnidx].setSelected(false);
				}
				m_curselbtnidx = -1;
				// we desire a single response, rather than duplex behaviour, to
				// a single interaction
				glutPostRedisplay();
				return;
			}
		}
		break;
	default:
		break;
	}

	if ( !m_bMouseOnlyForBox ) {
		CGLApp::mouseResponse(button, state, x, y);
	}
	return;
}

void CGLIBoxApp::mouseMotionResponse(int x, int y)
{
	if ( !m_bIboxEnabled || m_boxes.size() < 1 ) {
		if ( !m_bMouseOnlyForBox ) {
			CGLApp::mouseMotionResponse(x, y);
		}
		return;
	} 

	if ( -1 != m_curselbtnidx && 0 != m_curselbtnidx ) {
		return;
	}

	// forward the mouse input to the selected box
	if ( -1 != m_curselboxidx && 
		0 == m_boxes[ m_curselboxidx ].onMouseMove( x, y ) ) {
		glutPostRedisplay();
		return;
	}

	if ( !m_bMouseOnlyForBox ) {
		CGLApp::mouseMotionResponse(x, y);
	}
}

void CGLIBoxApp::mousePassiveMotionResponse(int x, int y)
{
	size_t szboxes = m_boxes.size();

	/*
	if ( !m_bIboxEnabled ) {
		CGLApp::mousePassiveMotionResponse(x, y);
		return;
	}
	*/
	if ( !m_bIboxEnabled || szboxes < 1 ) {
		// the right click menu is only available for a click that falls within
		// a narrow region around the window center
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

		int regW = viewport[2]/5;
		int regH = viewport[3]/5;
		if ( x >= viewport[0] + (viewport[2] -regW)/2 && 
			 x <= viewport[0] + viewport[2] - (viewport[2] - regW)/2 &&
			 y >= viewport[1] + (viewport[3] -regH)/2 && 
			 y <= viewport[1] + viewport[3] - (viewport[3] - regH)/2 ) {
			if ( -1 == m_btnMenu ) {
				_createRCMenu();
			}
			glutAttachMenu(GLUT_RIGHT_BUTTON);
			glutSetCursor(GLUT_CURSOR_WAIT);
		}
		else {
			glutDetachMenu(GLUT_RIGHT_BUTTON);
			glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
		}

		if ( !m_bMouseOnlyForBox ) {
			CGLApp::mousePassiveMotionResponse(x, y);
		}
		return;
	}

	int orghlidx = m_curhlboxidx;
	m_curhlboxidx = -1;
	// the first selection box that accepts the mouse event is regarded
	// as the currently focused
	int ret = -1, _ret = -1;
	for (size_t idx = 0; idx < szboxes; idx++) {
		if ( 0 == (ret = m_boxes[idx].onMousePassiveMove(x, y)) % 2)  {
			m_curhlboxidx = idx;

			// we intend to take the last selected as the currently focused
			//break;
			if ( -1 == _ret && (2 == ret || 4 == ret || 6 == ret) ) {
				_ret = ret;
			}
		}
	}

	if ( 2 == _ret ) {
		if ( -1 == m_btnMenu ) {
			_createRCMenu();
		}
		glutAttachMenu(GLUT_RIGHT_BUTTON);
	}
	else {
		if ( 4 != _ret && 6 != _ret ) {
			glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
		}
		if ( m_boxes.size() >= 1) {
			glutDetachMenu(GLUT_RIGHT_BUTTON);
		}
	}

	if ( orghlidx != m_curhlboxidx ) {
		glutPostRedisplay();
	}

	if ( -1 != m_curhlboxidx ) {
		int btnidx = getSelectedBtn(x,y);
		if ( btnidx == m_curhlbtnidx ) {
			return;
		}
		else {
			glutPostRedisplay();
		}

		if ( -1 != m_curhlbtnidx ) {
			m_buttons[m_curhlbtnidx].setSelected( false );
		}
		m_curhlbtnidx = btnidx;
		if ( -1 != m_curhlbtnidx ) {
			m_buttons[m_curhlbtnidx].setSelected( true );
			m_cout << "button " << btnidx << " hightlighted.\n";
		}
	}
}

int CGLIBoxApp::mainstay()
{
	return CGLApp::mainstay();
}

void CGLIBoxApp::onReshape( GLsizei w, GLsizei h )
{
	// boxes should also be reset 
	if (m_bFirstReshape) {
		if ( m_bIboxEnabled ) {
			_updateDefaultLayout();
		}
	}
	CGLApp::onReshape(w, h);
}

void CGLIBoxApp::display ( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	draw();
	if ( m_bIboxEnabled ) {
		drawBoxes();
	}

	if ( m_bGadgetEnabled ) {
		drawGadgets();
	}

	// draw help text box if necessary
	if ( "" != m_strfnhelp && m_bShowHelp ) {
		m_helptext.display();
	}

	trigger();
}

int CGLIBoxApp::_updateDefaultLayout()
{
	GLdouble w = ( m_maxx - m_minx );
	//int h = ( m_maxy - m_miny );
	int szbox = static_cast<int> ( m_boxes.size() );
	GLdouble dx = w/szbox;
	//glfloat	dy = (h-8*szbox)/szbox;

	if ( 0 >= szbox ) {
		return -1;
	}

	GLdouble tx = - (m_minx + m_maxx)/2,
			 ty = -(m_miny + m_maxy)/2,
			 tz = -(m_minz + m_maxz)/2;

	for (int idx = 0; idx < szbox; ++ idx) {
		m_boxes[idx].setMinMax( 
				m_minx + idx*dx + tx, m_miny + ty, m_minz + tz,
				m_minx + (idx+1)*dx + tx, m_maxy + ty, m_maxz + tz);
		//m_boxes[idx].setFrameColor(0.1*((idx+1)%10), 0.2*((1+idx)%5), 1.0/((idx+1)%10));
		m_boxes[idx].setFrameColor(0.1, .1, 1.0);
	}

	return 0;
}

void CGLIBoxApp::_genBoxColors()
{
	glrand::genFloatvector<GLfloat> (3, m_boxcolors, 
			ARRAY_SIZE(m_boxcolors), 0, 1);
	if ( m_cout.isswitchon() ) {
		cout << "box colors: ";
		copy (m_boxcolors, m_boxcolors + ARRAY_SIZE(m_boxcolors), 
				ostream_iterator<GLfloat> (cout, " "));
		cout << "\n";
	}
	int szbox = static_cast<int> ( m_boxes.size() );
	for (int idx = 0; idx < szbox; ++ idx) {
		m_boxes[idx].setFrameColor(m_boxcolors[0], m_boxcolors[1], m_boxcolors[2]);
	}
}

void CGLIBoxApp::_handleBtnEvents(int event)
{
	((CGLIBoxApp*)m_pInstance)->handleBtnEvents(event);
}

void CGLIBoxApp::_createRCMenu()
{
	// create a right-click menu for in-situ box interaction
	//
	size_t szbtn = m_buttons.size();
	if ( szbtn < 1 ) {
		return;
	}
	m_btnMenu = glutCreateMenu(_handleBtnEvents);
	for (size_t idx = 0; idx < szbtn; idx++) {
		string curtext = m_buttons[idx].isPrimary?m_buttons[idx].text:
						m_buttons[idx].text2;
		glutAddMenuEntry(curtext.c_str(), idx);
	}
}

void CGLIBoxApp::handleBtnEvents(int event)
{
	switch( event ) {
		case 0: // "new" means "add a box"
			specialResponse(GLUT_KEY_F3, 0, 0);
			break;
		case 1: // "delete this ox"
			specialResponse(GLUT_KEY_F4, 0, 0);
			break;
		case 2: // "and" or "or" mode
			specialResponse(GLUT_KEY_F2, 0, 0);
			m_buttons[2].switchStatus();
			// need to update the menu since button text changed
			m_btnMenu = -1;
			break;
		case 3:  // "removal" or "selection" pattern
			keyResponse('r', 0, 0);
			m_buttons[3].switchStatus();
			// need to update the menu since button text changed
			m_btnMenu = -1;
			break;
		case 4: // hide & disable boxes
			specialResponse(GLUT_KEY_F10, 0, 0);
			m_buttons[4].switchStatus();
			// need to update the menu since button text changed
			m_btnMenu = -1;
			if ( m_bIboxEnabled ) {
				break;
			}

			// have to provide a way for enabling boxes back again
			// and that is right-click menu
			if ( -1 == m_btnMenu ) {
				_createRCMenu();
			}
			glutAttachMenu(GLUT_RIGHT_BUTTON);
			return;
		default:
			break;
	}

	if ( -1 != m_btnMenu && m_boxes.size() >= 1 && m_bIboxEnabled) {
		glutDetachMenu(GLUT_RIGHT_BUTTON);
	}

	return;
}

void CGLIBoxApp::drawBeacon()
{
	GLfloat mvmat[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mvmat);

	GLfloat w = 1;
	_point_t<GLfloat> offset( (m_minx + m_maxx)/2, (m_miny+m_maxy)/2,
					(m_minz + m_maxz)/2 );

	point_t _center = m_boxes[m_curselboxidx].getCenter();
	_point_t<GLfloat> boxCenter(_center.x, _center.y, _center.z);

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib(GL_ALL_CLIENT_ATTRIB_BITS);

	glColor3f(0.0, 1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// transpose the model view matrix
	GLfloat tmp;
	for (int i=0;i<4;i++) {
		for (int j=0;j<4;j++) {
			if ( i < j ) {
				tmp = mvmat[i*4+j];
				mvmat[i*4+j] = mvmat[i+j*4];
				mvmat[i+j*4] = tmp;
			}
		}
	}

	transpoint(mvmat, boxCenter.x, boxCenter.y, boxCenter.z, w);

	/*
	GLfloat psRange[2];
	glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, psRange);
	glPointSize(psRange[1]);
	*/
	glPointSize(10);
	glBegin(GL_POINTS);
		glVertex3f( boxCenter.x, boxCenter.y, boxCenter.z);
		glVertex3f( boxCenter.x, -offset.y - mvmat[13] + 1, boxCenter.z);
	glEnd();

	/*
	// manually interpolate vertices between the two end points since we have
	// already taken over the model view transformation 
	GLfloat initY = boxCenter.y, a,b,c,d;
	a = 0;
	d = 5;
	c = 0;
	transpoint(mvmat, a, d, c, w);

	glBegin(GL_POINTS);
		while ( initY >= -offset.y - mvmat[13] ) {
			w = 1;
			a = boxCenter.x;
			b = initY;
			c = boxCenter.z;
			transpoint(mvmat, a, b, c, w);
			glVertex3f(boxCenter.x, b, boxCenter.z);
			initY -= d;
		}
	glEnd();
	*/

	glPopMatrix();
	glPopClientAttrib();
	glPopAttrib();
}

/* set ts=4 sts=4 tw=80 sw=4 */

