// ----------------------------------------------------------------------------
// iselbox.h : an independent class encapsulating functions needed for a
//				selection box in the interactive graphics application.
//
// Creation : Feb. 11th 2011
// revision : 
//  @Feb. 12th 2011 
//			adjust into an independent class from the
//			original CGLApp inheritance for the purpose of the support of
//			multiple-box selection
//	@Feb. 19th 2011
//			evolution into intuitive user-oriented box selection, with
//			resistance to modeling and viewing transformations
//	@Feb. 21th 2011
//			1.upgrade the front-facing judgement algorithm to what is suggested
//			in the redBook of openGL, and make the decision more correct and
//			reliable
//			2.interaction for the face movement on the selection box perfected
//			3.whole selection box moving interaction added, following the same
//			idea for the same interaction applied towards the whole object:
//			 - mouse dragging with right button for translation along X/Y axis
//			 - combined with CTRL+Right_Mouse_Button for translating along Z-axis,
//	@Feb. 22th 2011
//			1. add hint for the currently front facing faces of the selection
//			box
//			2. when mouse moving over a selectable (front facing) face, further
//			cueing the user by filling the polygon translucently.
//			3. fix the glitch in the selection stretching : while face 1,3
//			rotated to be upwards or downwards, or face 2,4 to be leftwards or
//			rightwards,the movement of mouse does not match the desirable
//			stretching of the faces intuitively. Fixing by following the same
//			idea for face 0 and 5.
//			4. add limit on the selection stretching : forbidding a movement
//			intending to push or pull a face over its opposite one, which is
//			meaningless in most cases and absurd in the box selection
//			interaction especially.
//	@Feb. 23th 2011
//			1. Make an uniform and smoother movement in the selection box
//			stretching by calculating the difference of object coordinates
//			"Unprojected" from the mouse window coordinate rather than the
//			original mouse vector length as used before
//			2. the very initial selection stretching is changed to be driven by
//			the movement along the Y axis rathen than X axis as was before, this
//			should perfect the interaction by making it further intuitive
//			3. add limit on the selection stretching : forbidding a movement
//			that will lead the focused face to go beyond the window
//			4. add a switch for the selection box sketching/moving limit
//			control, when it is off, the abrupt, maybe faster in other users'
//			view, movement will be resumed by using the mouse vector length
//			again.
//			5. add a switch for the front facing hint. When users prefer to
//			better performance, they might want to turn off this gaudy feature
// @Feb. 25th 2011
//			1. add a default axis orientation gadget
//			2. change the front facing hint filling color to grey
// @Feb. 26th 2011
//			1. place a more rigid limit control on the box stretching by
//			detecting if the focused face has any of its vertices in window
//			coordinate got outside the viewport, rather than simply checking if
//			the mouse click goes beyond the viewport as previously adopted
// @Mar. 8th 2011
//			1.in order to make the behaviour of m_pcout controllable as m_cout
//			in its hosting container CGLIBoxApp, change this pointer to a plain
//			MyCout instance; the properties of it will be force to conformed
//			to that of CGLIBoxApp::m_cout when a CIselbox object is added to its
//			container, a CGLIBoxApp object
// @Mar. 10th 2011
//			1.Extra member CIselbox::clone() is added to support run-time
//			addition or removal of selection box; cloning a selection box is to
//			duplicate a new box with the same size but with a deviation in terms
//			of position to help user discern it from the source; for this
//			purpose, copy constructor of CIselbox ensues
// @Mar. 11th 2011
//			1.For correct cloning of CIselbox, default trivial operator = is not
//			sufficiently secure so is also needed, thus added as well; for this
//			purpose, copy constructor and operator = are both added to _face_t
//			as well in the consideration of pointer safety
// @Mar. 12th 2011
//			1.Fix a seemingly odd, really inveterate, bug: when wanting to
//			delete an non-tail box, the operation actually causes all boxes
//			after it removed;  Long reckoning leads to the bug found in
//			CIselbox::operator=. this operator will be called indirectly by
//			CGLIBoxApp::removeBox.
// @Mar. 17th 2011
//			1.reportself gains another parameter bthin for offering flexibility
//			in that only the concise corner information will be reported.
// @April 12th 2011
//			1.add box corner stretching and hint by changing mouse cursor to
//			crosshair;
//			this operation can improve the efficiency of box sketching since it
//			will change size in three dimensions simultaneously
//			---- lots of changes made, all of which can be traced by the
//			reference of the added variables "m_selectedVid, m_highlightedVid,
//			_strechFace" etc
//			2.add box contraction/expansion, as will change size in all edges of
//			the box and still target facilitating box stretching, by 
//			Shift + Right Mouse Button
// @April 13th 2011
//			1.add a group of buttons for in-situ interaction; the buttons are
//			always floating over the currently highlighted front-facing side
// @April 15th 2011
//			1.two days of effort ends up with giving up on tackling with the
//			button text mapping in 3D; so move to the substitution by image icon 
//			through texture mapping; an extra member of button_t is thus added -
//			for holding the icon image data (Achieved until 17th, flaundering
//			with failures of texture mapping just stemming from the miss of the
//			requirement of texturing - a GL_FILL polygon mode!!!!)
// @April 18th 2011 (12 days to the submission due...hurry much more!)
//			1.add box button event handling - highlighting & clicking
//			2.since box button interaction is only responded when button is
//			shown, and we handle box button events uniformly in the hosting
//			class (CGLIBoxApp in our case), an extra member in _button_t is
//			added to make possible the hosting class to recognize if button is
//			shown at the time the event is about to be handled, the name is
//			"isShown".
//			--------- this is mainly for individual box button control;
//			 in order to control all box buttons together, the variable
//			 CGLIBoxApp::m_curhlboxidx can tell if we need to handle box button
//			 events - simply see if m_curhlboxidx != -1
// @April 23th
//			For a full range of box resizing functionalities:
//			1.A sphere will be shown at a vertex when mouse cursor closes to it,
//			as a hint/feedback to the point-dragging interaction.
//			2.Add box edge dragging, when mouse cursor closes to an edge, the
//			edge will be highlighted; dragging the edge can drag two adjacent
//			faces concurrently.
// @April 25th
//			1.Add extra button text/icon for the support of binary button, i.e the
//			button whose status can be clicked to be alternated within two states
//			2.as data members of button_t, iconX and iconY are removed because
//			of no avail
//			3.in button_t::getNearestEidx(), fix the bug that when the mouse
//			cursor is actually close to the mathematically straightline passing
//			through a box edge, the line-dragging becomes ready; To correct to
//			implement a detection against a near distance to a straightline segment ,
//			just add a checking against the point's being outside the bounding
//			box of the edge before calculating the distance from the point to
//			corresponding straightline 
//
// @April 26th
//			1.Remove the reliance on key (Control) to move the box along Z axis
//			The way: right dragging the edge triggers moving along Z axis
//
//			2. Remove the reliance on key (Shift) to do equal-proportion scaling
//			The way: when the cursor falls into a narrow rectangular region 
//			around the face center, right dragging triggers this
//			equal-proportion scaling
// @May 3rd 2011
//			1.enlightened by the solution to precise global translation for all objects in the scene,
//			ameliorate the inveterate problem of imprecise movement imposed upon selection boxes,
//			including moving, resizing (by vertex, edge and face).
//			~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//			IT WORKS, benefitting from the basic math of openGL
// @May 4th 2011
//			1.Tried another type of front face hint : difference in alpha (with GL_BLEND enabled).
//			  No good effect, so gave up
//			2.Use the current projection matrix record, make a constant size of yellow sphere as the 
//			  vertex dragging hint
//			3.Fix a bug: when the widgets' location is uninitialized, isInButton() might return incorrect 
//			  decision on the inclusion judgement
//			4.Still use projection matrix, make adaptable widget alignment/size so that it is more 
//			  reasonable to use - appear in appropriate size on demand
// @May 9th 2011
//			1.add CIselbox::getCenter for the implementation of the beacon
//			metaphor in CGLIBoxApp.
// @May 18th 2011
//			1.add visual feedback for the readiness of congruent scaling ( in
//			case the cursor change might not be supported in certain platforms )
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#ifndef _ISELBOX_H_
#define _ISELBOX_H_

#include <iostream>
#include <vector>

#include <GL/glut.h>
#include "glrand.h"
#include "point.h"
#include "cppmoth.h"

class CGLIBoxApp;

using std::vector;
using std::ostream;
using std::cout;
using std::string;

// distance threshold for corner touch judgement
const GLdouble CORNER_DIST_THRESHOLD = 3.50;
const GLdouble EDGE_DIST_THRESHOLD = 4.50;
typedef _point_t<int>	edge_idx_t;

// struct describing a polygon, as is also holding a 2D face
typedef struct  _face_t {
	vector<GLubyte> vidxs; // index to the vertex sequence
	vector<point_t>* pvs; // pointer to a sequence of vertices
	vector<point_t>* wps; // pointer to 2D points of window coordinate 
						 // corresponding to obj vertices pointed by pvs
	bool	isSelected;  // if this face is currently focused

	_face_t() {
		pvs = wps = NULL;
		isSelected = false;
	}

	_face_t(const _face_t& other);
	_face_t& operator = (const _face_t& other);

	void setSelected(bool bSelected = false);
	point_t getNormal() const;
	point_t getNormalInWincoord() const;
	bool isUserOriented();
	bool isFrontFacing();
	bool isInFace(GLdouble x, GLdouble y);
	void drawFrame();
	int  updateVertices(GLdouble dx = 0, GLdouble dy = 0, GLdouble dz = 0);
	GLdouble getNearestZ(GLdouble x, GLdouble y);
	int getNearestVidx(GLdouble x, GLdouble y);
	// the edge is represented by the indices of its two end vertices
	int getNearestEidx(GLdouble x, GLdouble y, edge_idx_t& eIdx);
	bool atCenter(GLdouble x, GLdouble y, int proportion=5);
	point_t getCenter(bool bIn2D = false) const;
}face_t, *pface_t;

// struct describing a button
typedef struct _button_t {
	// two points defining the bounding rectangle : in object coordinates
	// used to draw button frame and print button text
	point_t leftbottom, righttop;
	// two points defining the bounding rectangle : in window coordinates
	// used to detect mouse hit
	vector<point_t> winVertices;

	// where the button text will start to display, not pondering on the length
	// of the text string
	point_t textpos;
	string text;

	bool isSelected;	// if this button is currently focused
	bool	isShown;	// if this button is currently shown
	bool isPressed;		// if the button is currently pressed down

	unsigned char *pIconImgData;	// graphic image as alternative to the button text
	unsigned int iconW, iconH;		// width and height of the icon image

	bool isPrimary;
	string text2;		// for binary-status button, the text for another status
	unsigned char *pIconImgData2;	// graphic image for another status of the button
	unsigned int iconW2, iconH2;		// width and height of the secondary icon image

	_button_t(const string& btntext=" ", const string& btntext2="") {
		winVertices.resize(4);
		text = btntext;
		isSelected = false;
		isShown	 = false;
		isPressed = false;
		pIconImgData = NULL;
		iconW = iconH = 0;

		isPrimary = true;

		text2 = btntext2;
		pIconImgData2 = NULL;
		iconW2 = iconH2 = 0;
	}

	_button_t(const _button_t& other);
	_button_t& operator = (const _button_t& other);

	void setSelected(bool bSelected = false);
	void setPressed(bool bPressed = false);
	void switchStatus();
	bool isInButton(GLdouble x, GLdouble y);
	void draw();
	// convert button location from object coordinates to window coordinates
	int  _updateBtnWinCoords(const vector<point_t>& btnFrameVertices);
}button_t, *pbutton_t;

class CIselbox {
public:
	friend class CGLIBoxApp;

	CIselbox();
	CIselbox(const CIselbox& other);
	~CIselbox();

	CIselbox& operator = (const CIselbox& other);

	// tell the position and size of the selection box, as is simply by
	// indicating the two opposite corners, i.e. (minx, miny, minz) and 
	// (maxx, maxy, maxz), the box is always rectangular
	void setMinMax(GLdouble minx, GLdouble miny, GLdouble minz,
					GLdouble maxx, GLdouble maxy, GLdouble maxz);

	// designate preferrable color to use while drawing the box frame
	void setFrameColor(GLfloat r=0.2, GLfloat g=0.5, GLfloat b=0.4);

	// trigger of frame drawing
	void drawFrame(bool bBtnEnabled = true);

	/*
	 * @brief associate vertices of object with the instance of CIselbox
	 * @param pvertices pointer to the client vertex store
	 * @param pedgeflags pointer to the client edge flag store
	 * @return 0 for success and -1 otherwise
	 */
	int associateObj(vector<GLdouble>* pvertices, 
					vector<GLboolean>* pedgeflags,
					vector<button_t>* pbuttons = NULL,
					GLdouble dx = 0, GLdouble dy = 0, GLdouble dz = 0);

	/*
	 * @brief the core function of this class - update the edge flags of the
	 * client according to current box scale, the simple working is this:
	 *	EdgeFlag[ i ] = ( Vertex[ i ] is inside this box )
	 *	currently we use the brute-force algorithm,i.e. traverse throughout the
	 *	whole vertex store...
	 * @return -1 if error occurred, number of vertices that has been culled out
	 *		otherwise
	 */
	int refreshEdgeFlags();

	// mostly used for debugging
	ostream& reportself(ostream& os, bool bthin=false);

	// judege if the single vertex, not moust point, (x,y,z) is inside this
	// box or not using the simplest collision detection 
	bool isInside(GLdouble x, GLdouble y, GLdouble z);

	// transform all the eight box vertices according to the current matrices at
	// the top of each of the matrix stacks of  modelview and projection
	// transformation
	int transformBoxVertices();

	// examine which face is currently hit with mouse coordinate (x,y)
	int getSelectedFace(int x, int y);

	// examine which of the eight vertices is currently hit
	int getSelectedVertex(int x, int y);

	// examine which of the twelve edges is currently hit
	edge_idx_t getSelectedEdge(int x, int y);

	// obtain current centric point of the box
	const point_t getCenter() const;

	// alternate the control of limit on the selection box
	// stretching/moving
	// return the update status
	bool switchLimit();

	// alternate the selection hint for the front facing faces
	// return the update status
	bool switchHint();

	// duplicate a selection box, possibly with shifts
	// (0==dx && 0==dy && 0==dz) indicates using the largest dimension of the
	// source box as the displacement
	CIselbox clone(GLdouble dx=0, GLdouble dy=0, GLdouble dz=0);

	// dimension, i.e. the distance from minor corner to the major corner
	GLdouble getDimension() const;

	// proprietary for density study task 4 - the lesion locating task
	// -- get the distance from mouse click to the selected face center
	GLdouble getClickDist(int x, int y) const;
protected:
	/* 
	 * since the interaction handling is the major charge of this class, it
	 * might need hand over the user input received by the client, as is mostly
	 * the GL application
	 *
	 * returns:
	 *	0 indicates that things has been tackled and the event flow terminates
	 *	  therein
	 *	1 indicates a need for relaying event back tot the caller
	 *	-1 for errors
	 */
	int onMouseClicked(int button, int state, int x, int y);
	int onKeyPressed(unsigned char key);
	int onSpecialKeyPressed(int key);
	int onMouseMove(int x, int y);
	int onMousePassiveMove(int x, int y);

private:
	// two opposite corners of the box
	GLdouble m_minx, m_miny, m_minz;
	GLdouble m_maxx, m_maxy, m_maxz;

	// storage of the box vertices
	vector<point_t>		m_boxvertices;

	// the window coordinate counterparts to m_boxvertices
	vector<point_t>		m_winboxvertices;

	// all faces of the selection box
	vector<face_t>		m_faces;

	// color of the box wire frame
	GLdouble m_fcr, m_fcg, m_fcb;

	/* record which mouse button is being pressed, -1 indicates none */
	int m_pressedbtn;

	/* record recently selected face index */
	int m_selectedFid;

	/* whenever a mouse cursor passing over a face, the face will be highlighted
	 * as a sort of hint
	 */
	int m_highlightedFid;

	/* record recently selected box corner index */
	int m_selectedVid;

	/* whenever a mouse cursor passing over a vertex (corner), a special icon
	 * will hightlight it to indicate the readiness of corner stretching
	 */
	int m_highlightedVid;

	/* record recently selected box edge index - combination of vertex indices */
	edge_idx_t m_selectedEid;

	/* whenever a mouse cursor passing over an edge, a special icon
	 * will hightlight it to indicate the readiness of box resizing by edge
	 * dragging
	 */
	edge_idx_t m_highlightedEid;

	/* z in the window coordinate of the box vertice that is the nearest to
	 * the point of mouse click
	 */
	GLdouble m_nearestZ;

	/* record object coordinate of mouse point when 
	 * any of the mouse buttons is pressed */
	GLdouble m_mx, m_my, m_mz;

	// reference to the geometry of the encompassed object
	vector<GLdouble>* m_pvertices;

	// reference to the flags used to control the visibility of vertices
	vector<GLboolean>* m_pedgeflags;

	// reference to the button store
	vector<button_t>* m_pbuttons;

	// coordinate offsets, possibly needed for judging if vertices of the
	// encompassed object is inside the box
	GLdouble m_dx, m_dy, m_dz;

	// custom output stream sharing with that of the client
	MyCout m_cout;

	// limit on the selection box stretching/moving
	GLboolean m_bmvlimit;

	// hint for front facing faces
	GLboolean m_bhint;

	// ready for moving box along Z axis
	GLboolean m_bMoveAlongZ;

	// ready for equal-proportion scaling by right dragging
	GLboolean m_bEPscaling;

private:

	// update selection box vertices ON THE TWO OPPOSITE CORNERS 
	int _updateMinMax();

	/* 
	 * @brief update box vertices while the two corners change and check 
	 * if the box will be stretched out of the viewport
	 * @param bCheckRange enumeration controlling the checking purview
	 *	- 0 no check at all
	 *	- 1 only check vertices on the selected face
	 *	- 2 check all vertices on the selected selection box
	 * @return 0 symbolizing pass and -1 for failure 
	 */
	int _onMinMaxUpdated(int bCheckRange=0);
	
	// internal initialization for all boundary faces
	int _initFaces();

	// convert box vertices of object coordinates to that of window coordinates
	int _objcoord2wincoord();

	// convert a window coordinate, the point of mouse click, say, to object
	// coordinate
	int _wincoord2objcoord( int winx, int winy, int winz,
		GLdouble *objx, GLdouble *objy, GLdouble *objz);

	// stretching a face according to given magnitude of movement
	int _strechFace(int fidx, GLdouble dx, GLdouble dy, GLdouble dz);

	// locate the buttons on currently hightlighted face
	int _calcBtnLayout(int fidx = -1);
};

#endif // _ISELBOX_H_

/* set ts=4 sts=4 tw=80 sw=4 */

