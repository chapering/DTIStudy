// ----------------------------------------------------------------------------
// GLiboxmoth.h: an extension to CGLApp that supports multiple box selection on
//			the basis of the simplest collision detection 
//
// Creation : Feb. 12th 2011
//
// Revisions:
// @Feb. 17th
//			1.changing from Middle button to right button of mouse for selection
//			box moving
// @Feb. 21th
//			1.correct the switchable output ostream m_cout passing into selection
//			box instances and thus make the global switch effectual
//			2.add selection box enable/disable control, using F10 for switching
//			3.add selection culling function control, using F9 for switching
// @Feb. 23th
//			1. remove the _wincoord2objcoord member function and use the version
//			in the parent class uniformly so that to make the class hierarchy
//			more sleek
//			2. since valves have been set for the selection box to
//			enable/disable the stretching limit and/or hint feature, there in
//			this class, two hot keys:
//				F8 for switching stretching limit,
//				F7 for switching hint feature
//			are added.
//			3. add hotkey 'b' for random changing of selection box frame colors
//	@Mar. 6th
//			1.in order to ease the implementation of GLUT-multiple-window based
//			CMitubeRender, display() and onReshape() are changed from being
//			"protected" to be "public" in the access control
//	@Mar. 10th
//			1.Extending interaction: 'a' for adding a new selection box by
//			cloning the most recently selected box; 'e' for erasing recently 
//			selected box; if no box ever selected, use recently highlighted 
//			one, if no either, act upon the last one in the box store
//	@Mar. 11th
//			1.removeBox() will delete even the only one box in the store
//			2.duplicateBox() will addBox() even the store is empty then
//			3.keyboard mapping for runtime selection box adding / removing
//			changed to:
//				F3 for adding a box (at most 10 boxes are permitted for now
//				F4 for removing a box (can delete until the box store depletes
//			4.Disable all box-related operations when m_bIboxEnabled is unset
//	@Mar. 12th
//			1.either in duplicateBox or removeBox, the existing box to refer as
//			the source from which to duplicate or target to remove should be
//			determined firstly by whether it is highlighted then if it is
//			selected since highlighting a box is more frequent to happen because
//			the user needs not to click but simply hover upon a face of the box
//  @April 13th 
//			1.bounding box information and related manipulators are moved
//			upwards to the parent class CGLApp since the general operation of
//			object moving and scaling all need the object encompassed to be more
//			precise. 
//			2.gadget management is not suitable to lie in this layer and also
//			moved to CGLApp therefore, leading to the disappearance of the
//			member function updateGadgets
//  @April 18th
//			1.to follow the principle that every optional feature is
//			controllable, an extra switch for toggling box-attaching buttons is
//			added as the boolean member m_bBtnEnabled; and, make it effect by
//			another key mapping - 't'
//			2.add button action handling
//			3. getSelectedBox removed for being deprecated
//			4. getSelectedBtn added because of we have only one instance of a
//			button group, it does not belong to any single box
//			5. integrate two working mode and two associative patterns of the
//			multiple-box modality
// @April 25th
//			1.support box buttons with binary status; the button text / icon
//			image reflect current status
//			2.for scalability, consistently use button index in the store as
//			button value feeding the function glutAddMenuEntry
// @April 26th
//			1.solve the conflict: when no box exists, the right click is
//			exclusively mapped to invoke the menu, in this condition there is no
//			means for ordinary panning operation for the geometry model
//			The recipe: in the PassiveMotion callback, check if the mouse cursor
//			falls within a narrow rectangular region around the center of
//			current window, which is regarded as the only condition enabling the
//			right click menu; otherwise the right button is still mapped to its
//			common function for panning the whole scene.
// @May 9th 2011
//			1.when a box is selected and then moved or resized, the centric
//			point inside the box will be projected to the ground as a specially
//			colored shpere as a "beacon" for exploring, especially in the depth
//			dimension.
// @May 17th 2011
//			1.pick out the code for handling the beacon projection and drawing into a
//			standalone function to make a more maintainable code as a whole
// @May 18th 2011
//			1.the long probing / pesky problem, the beacon projection welters,
//			is hacked by a makeshift. (using line as the guidance still rise
//			challenges )
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#ifndef _GLIBOX_MOTH_H_
#define _GLIBOX_MOTH_H_

#include "GLcppmoth.h"
#include "iselbox.h"

using std::vector;

class CGLIBoxApp: public CGLApp {
public:
	CGLIBoxApp();
	CGLIBoxApp(int argc, char **argv);
	virtual ~CGLIBoxApp();

protected:
	/*
	 * @brief associate vertices of object with the instance of CGLIBoxApp
	 *  - this function MUST be called before adding selection box 
	 * @param pvertices pointer to the client vertex store
	 * @param bcalminmax boolean indicating if to calculate the boundary
	 * @return 0 for success and -1 otherwise
	 * @see addBox,setMinMax
	 */
	int associateObj(vector<GLdouble>* pvertices, bool bcalminmax=false,
						GLdouble dx=0, GLdouble dy=0, GLdouble dz=0);

	/*
	 * @brief push the geometry of all selection boxes into the vertex pipeline
	 * - all selection box should be rendered with independent color and
	 *   lighting of the encompassed object
	 * @return none
	 */
	void drawBoxes();

	/*
	 * @brief add a prepared CIselbox object to the seletion box store
	 * - this way, all properties of a selection box can be fully customized
	 *   before being added
	 * @param selbox a CIselbox object that is already established in memory
	 * @return 0 for success and -1 otherwise
	 * @see associateObj,removeBox
	 */
	int addBox(const CIselbox& selbox);

	/*
	 * @brief request an addition of a selectio box into the box store
	 * - this way, all properties of a selection box will be set automatically
	 * @return 0 for success and -1 otherwise
	 * @see associateObj
	 */
	int addBox();

	/*
	 * @brief remove a specific selection box from the box store
	 * @param idx an integer giving the index of selection box to be removed in
	 *		the selection box store; -1, as the default, means to remove the
	 *		most recently selected or, if no ever selected, highlighted one
	 * @return 0 for success and -1 otherwise
	 * @see addBox
	 */
	int removeBox(int idx=-1);

	/*
	 * @brief add a new box by cloning most recently selected or, if no ever
	 * selected, highlighted box
	 * @param dx,dy,dz displacement along X,Y,Z axis after cloning
	 *  (0==dx && 0==dy && 0 == dz) indicates using the largest dimension of the
	 *  source box as the displacement
	 * @return 0 for success and -1 otherwise
	 */
	int duplicateBox(GLdouble dx=0, GLdouble dy=0, GLdouble dz=0);

	/*
	 * @brief retrieve one of the box buttons currently hit by the mouse cursor
	 * @param x,y coordinates of the mouse point when it hit one of the box buttons
	 * @return >=0 for the index of the selected button, -1 for none selected
	 */
	int getSelectedBtn(GLdouble x, GLdouble y);

	/*
	 * @brief judge if (x,y,z) is selected, i.e. it is inside at least one of
	 * the selection boxes
	 * @param x,y,z coordinates of the point to judge
	 * @return true for yes and false otherwise
	 */
	bool isPointInBox(GLdouble x, GLdouble y, GLdouble z);

	/*
	 * @brief judge if the given line is selected by all boxes
	 *  i.e. for each of the selection boxes, the line must have at least one
	 *  vertex is inside it. This is basic idea of multiple-box selection of
	 *  lines
	 * @param pline a pointer to a vertex array
	 * @param size size of the vertex array, not the number of vertices
	 * @param unit how many elements a vertex takes in the array
	 * @param offset the hopping step of two adjacent points in the vertex
	 *	array pointed by pline
	 * @return true for yes and false otherwise
	 */
	template <typename _T>
	bool isLineInBox(const _T* pline, GLsizei size, 
					GLsizei unit = 3, GLsizei offset = 0,
					_T dx = 0,  _T dy = 0, _T dz = 0) {
		if (!m_bIboxCulling) {
			return true;
		}

		// every box should contain at least one point of the line
		if ( !pline || unit < 0 || size/unit <= 0 ) {
			return false;
		}

		size_t szBoxes = m_boxes.size();
		for (size_t idx = 0; idx < szBoxes; idx++) {
			bool bCurBoxCover = false;
			for (GLsizei j = 0; j < size/unit; j++) {
				if (m_boxes[idx].isInside(pline[j*unit+offset+0] + dx, 
							pline[j*unit+offset+1] + dy, 
							pline[j*unit+offset+2] + dz)) {
					bCurBoxCover = true;
					break;
				}
			}

			// only if one box does not cover it, the line will be judged as unselected
			if ( !bCurBoxCover ) {
				return false;
			}
		}

		return true;
	}

	/*
	 * @brief add a button for in-situ box-based interaction
	 * @param btntext a string giving the button text
	 * @param fnIconImage a pointer to string giving icon image file
	 * @param btntext2 a string giving the button text for secondary status
	 * @param fnIconImage2 a pointer to string giving icon image file for
	 *	secondary status
	 * @return none
	 */
	void addButton(const string& btntext, const char* fnIconImage = "",
					const string& btntext2 = "", const char* fnIconImage2 = "");

protected:
	/* following members are simply listed to expect overloading to provide what
	 * a practical openGL program has as components
	 */
	void glInit( void );
	void keyResponse(unsigned char key, int x, int y);
	void specialResponse(int key, int x, int y);
	void mouseResponse(int button, int state, int x, int y);
	void mouseMotionResponse(int x, int y);
	void mousePassiveMotionResponse(int x, int y);

public:
	int mainstay();
	void onReshape( GLsizei w, GLsizei h );
	void display ( void );

protected:
	// flags are used to control the visibility of vertices
	// all boxes should be bound with the same edge flag array, otherwise it
	// will become a multiple independent single-box selection rather than a
	// multiple-box selection.
	vector<GLboolean>	m_edgeflags;

	/* record currently selected box */
	int m_curselboxidx;
	/* record currently highlighte box */
	int m_curhlboxidx;

	// reference to the geometry of the encompassed object
	// all boxes should be bound with the same vertice array, otherwise it
	// will become a multiple independent single-box selection rather than a
	// multiple-box selection.
	vector<GLdouble>* m_pvertices;

	// switch selection boxes application
	GLboolean m_bIboxEnabled;

	// switch selection boxes culling function
	// Note: when this switch is off, the selection boxes have no any effect on
	// the encompassed object, i.e. will act as dummy boxes
	GLboolean m_bIboxCulling;

	// the selection box store
	vector<CIselbox>	m_boxes;
	// we use uniform color for all selection boxes by default
	GLfloat m_boxcolors[3];
	/* switch between selection and removal mode */
	GLboolean			m_bRemovalbased;
	/* switch between AND and OR associative pattern among boxes */
	GLboolean			m_bOR;

	// the button store
	vector<button_t>	m_buttons;
	// switch box-attached button feature application
	GLboolean			m_bBtnEnabled;
	/* record currently selected button */
	int m_curselbtnidx;
	/* record currently highlighte button */
	int m_curhlbtnidx;

	// for a right click menu
	int m_btnMenu;

	/* switch of devoted mouse input to boxes */
	GLboolean m_bMouseOnlyForBox;
private:
	// calculate and set a default selection box layout on the screen, in which
	// all boxes are evenly distributed and horizontally and vertically aligned
	int _updateDefaultLayout();

	// randomly produce colors for the selection box
	void _genBoxColors();

	static void _handleBtnEvents(int event);
	void _createRCMenu();
protected:
	// respond to box button interaction
	virtual void handleBtnEvents(int event);
	void drawBeacon();
};

#endif // _GLIBOX_MOTH_H_

/* set ts=4 sts=4 tw=80 sw=4 */

