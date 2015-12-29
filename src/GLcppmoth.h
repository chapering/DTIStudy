// ----------------------------------------------------------------------------
// GLcppmoth.h : a skeleton class for quick building of simple openGL applications
//				to run with the legacy X window in a console mode, in which
//				following features will be modeled
//			1. all the GL and GLUT routines needed for an ordinary, practical
//			   openGL program.
//			2. the windows will be centered by default on the screen whose size
//			   is always automatically retrieved inside this skeleton
//			3. default keyword response 
//				- ESC to exit
//				- '9' Shading model switching
//				- '0' simulating random key pressing
//				- a/A for increasing the red channel
//				- d/D for descreasing the red channel 
//				- w/W for increasing the green channel
//				- x/X for descreasing the green channel
//				- s/S for increasing the blue channel
//				- f/F for descreasing the blue channel
//			      in the color applied in drawing vertices
//			    - j/J for rotating clockwise around X axis
//			    - l/L for rotating anticlockwise around X axis
//			    - i/I for rotating clockwise around Y axis
//			    - ,/< for rotating anticlockwise around Y axis
//			    - h/H for rotating clockwise around Z axis
//			    - k/K for rotating anticlockwise around Z axis
//			    - Up arrow for translating upwards (on Y axis)
//			    - Down arrow for translating downwards (on Y axis)
//			    - Left arrow for translating leftwards (on X axis)
//			    - Right arrow for translating rightwards (on X axis)
//			    - 'z' for translating backwards (on Z axis)
//			    - 'Z' for translating forwards (on Z axis)
//			    - Whitespace for resetting to original positions
//			    - '=', meaning '+', for zooming in with view reset
//			    - '-' for zooming out with view reset
//			    
//			4. default mouse response while clicking
//				- left button for rotating, cooperating with clicking
//				- right button for translating, cooperating with clicking
//				- middle button for lighting switching and positioning
//			5. default mouse response while moving
//				- left button for rotating, cooperating with clicking
//				- right button for translating, cooperating with clicking
//				- middle button (the roller/wheel) for zooming
//
// Creation : Jan. 11th 2011
//
// Revisions:
// @Feb. 4th
//			1.engulf most previous global variables for projection transformation,
//			color random switch and lighting position, etc., as class members
//			2.add data members for viewing transformation control
// @Feb. 7th
//			1.add zoom in and zoom out control by capturing thus handling +/-
//			.....
// @Feb. 15th
//			1.correct the mouse-based rotating handling to make it completely 
//			intuitive by multiplying the rotating axis (-dy,-dx,0), which is
//			perpendicular to the mouse motion vector, by current modelview
//			matrix
//			2.modify the right moust button response for translating, not scaling
//			3.add ALT and CTRL capturing
//			4.SHIFT+Right_Mouse_Button for scaling
//			5.CTRL+Left_Mouse_Button for rotating around Z-axis
//			6.enchance the continous mouse interaction by updating m_mx and m_my
//			even in the mouseMotionResponse callback
// @Feb. 17th
//			1.add mouse interaction for global translation, with the customary
//			trigger Right_Mouse_Button
//			2.add CTRL+Right_Mouse_Button for translating along Z-axis,
//			equivalent to Zooming in or out
// @Feb. 23th
//			1. add the m_dx, m_dy and m_dz (actually moved upwards from, and
//			merged in terms of functionality, the original holding subclass 
//			GLIBoxApp) to record piece of info about
//			the object coordinate system so that, at least, the lighting
//			position control could be refined from the originally quite
//			meaningless version, the _wincoord2objcoord will also be more
//			precise  with this info.
// @Feb. 25th
//			1. add gadget holding and management extension, each of the gadgets
//			in the store is implemented as an instance of the gadget class
//				note that gadget is not widget, it is particularly used in the
//				cases like you want a:
//				.orientation indicator when the rendering is in 3D space
//				.runtime prompt for interaction to be placed as a static textbox
//				.a button for navigation
//				...
// @Mar. 4th
//			1.a bug found : once entering glutMainLoop, it will never return
//			until the program terminates, this is aversive for object oriented
//			design in that the destructor will lose to act, as is a potential
//			chance for memory leakage...! thus an extra virtual member, cleanup,
//			is added. it is called while the quit key ESC is pressed and make
//			resource that is not recycled automatically, heap memory for
//			example, get destroyed before the process ends
//			--- since we have CGLApp::~CGLApp() set as a virtual function, all
//			its derived children classes' destructors will be called
//			automatically in order only if this parent destructor gets invoked.
//			This is simply another merit of C++ !
// @Mar. 8th
//			1.add m_bFirstReshape to avoid always resetting everything when the
//			window is resized since SPACE is already set for this action 
// @Mar. 10th
//			1.key stroke aimming at action for resetting changed from Space bar
//			to Home, which seems to make more sense.
// @April 13th
//			1.embrace the bounding box information from its descendant
//			CGLIBoxApp which is used to ameliorate the precision of the 
//			moving/scaling interactions by mouse
//			2.strip the "reset" side effect of the response to key stroke "+/-"
//			and make it zoom in/out objects "from there" rather than always from
//			the scratch (reset the scene and then start zoom in or out)
//			3.sublime all key mapping responses for moving/rotating as alternatives
//			to corresponding mouse interaction by applying MODELVIEW matrix
//			transformation on the motion vectors
//			-- by this way, those keys are gonna serve the real role of fine
//			adjusting
// @April 18th
//			.add help text box and related interactions (moved from
//			CSitubeRender)
// @April 25th
//			.For intuitive interaction for commom model transformation, key
//			mappings are partially updated to
//			    - i/I for rotating clockwise around X axis
//			    - ,/< for rotating anticlockwise around X axis
//			    - j/J for rotating clockwise around Y axis
//			    - l/L for rotating anticlockwise around Y axis
// @April 26th
//			.for consistency with the VTK convention, right dragging is changed
//			to be mapped to zooming, panning is less often used thus mapped to
//			CTRL+right dragging (VTK uses middle button dragging for this, but
//			we did not use middle button basically)
// @May 3rd 
//			.place constraints on Zoom in/out - disallow zoom out of the frustrum
//			.make a precise (render the object always follow the mouse cursor, or keep the relative 
//				distance between the object and the mouse cursor unchanged) translation even resistant of 
//				zooming
//			
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#ifndef _GLMOTH_H_
#define _GLMOTH_H_

#include "cppmoth.h"
#include "GLgadget.h"
#include <X11/Xlib.h>
#include <GL/glut.h>

using std::string;
using std::vector;
///////////////////////////////////////////////////////////////////////////////
// this is the outermost shell to cover the model application described above
class CGLApp : public CApplication {
public:
	static int getRootWindowSize(int *w, int *h);
	static int getScreenSize(int *w, int *h);

public:
	CGLApp();
	CGLApp(int argc, char** argv);
	virtual ~CGLApp(); 

	/*
	 * @brief using the GLUT to handle with windowing stuff, so here we can set
	 *  parameters for GLUT to display windows in our penchant
	 * @param wTitle a string giving window title
	 * @param x an integer giving the x component in the coordinate for the
	 *  upperleft corner of the window to display 
	 * @param y an integer giving the y component in the coordinate for the
	 *  upperleft corner of the window to display 
	 * @param width an integer giving the width of the window to display
	 * @param height an integer giving the height of the window to display
	 * @param dispMode an integer using combination of GLUT Macroes to designate
	 *  the desirable display mode
	 * @return none
	 */
	void setGlutInfo(const char* wTitle="GL program moth",
						  int x = -1, int y = -1,
						  int width = -1, int height = -1,
						  int dispMode = GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	/*
	 * @brief set parameters for the project transformation
	 * @param fvoy the angle of the field of view in the yz plane
	 * @param near distance from viewpoint to the near clip plane
	 * @param far distance from viewpoint to the far clip plane
	 * @return none
	 */
	void setPrjInfo(GLfloat fvoy, GLfloat near, GLfloat far);

	/*
	 * @brief set parameters for the viewing transformation
	 * @param eyeX,eyeY,eyeZ position of the viewpoint, i.e. location of the
	 * camera
	 * @param centerX,centerY,centerZ position of the center of the view towards
	 * which the camera will be aimming
	 * @param upX,upY,upZ the vector pointing to the up direction
	 * @return none
	 */
	void setViewInfo(GLdouble eyeX = .0, GLdouble eyeY = .0, GLdouble eyeZ =.0,
		GLdouble centerX = .0, GLdouble centerY = .0, GLdouble centerZ = .0,
		GLdouble upX = .0, GLdouble upY = 1.0, GLdouble upZ = .0);

	/*
	 * @brief set the window size be the size of display
	 * @return 0 for sucess and non-zero for error
	 */
	int setFullWindow();

	/*
	 * @brief set the maximal and minimal vertex coordinate among all the
	 *			encompassed vertices
	 * @param minx,miny,minz the coordinates of the "minimal" vertex
	 * @param maxx,maxy,maxz the coordinates of the "maximal" vertex
	 * @return none
	 * @see associateObj
	 */
	void setMinMax(GLdouble minx, GLdouble miny, GLdouble minz,
					GLdouble maxx, GLdouble maxy, GLdouble maxz);

	/*
	 * @brief set the coordinate offset from object coordinate system to the
	 * grand fixed coordinate system. currently this provides simly a way that this
	 * uppermost GLmoth class gets to know about the object coordinate system
	 */
	void setObjOffset(GLdouble dx, GLdouble dy, GLdouble dz);

	/*
	 * @brief update vertex coordinate range once that of the object updated
	 * @return none
	 */
	void updateGadgets();

	/*
	 * @brief set the clearing color
	 * @param red the red channel value
	 * @param greeen the green channel value
	 * @param blue the blue channel value
	 * @param alpha the alpha channel value
	 * @return none
	 */
	void setClearColor(GLclampf red=.0, GLclampf green=.0, 
						GLclampf blue=.0, GLclampf alpha=.0);
	/*
	 * @brief simply apply the color currently stored, which is changable with
	 * the key interactions
	 * @return none
	 */
	void useCurrentColor();

	/*
	 * @brief adopt single or double buffering to flush drawing automatically
	 * @return none
	 */
	void trigger(void);

	// help text switch
	bool switchhelp(bool bon);

protected:
	/*
	 * @brief the callback set for holding all logic to be valid application
	 *        all logic flow other than argument processing should be placed
	 *        herein
	 * @note MUST be overloaded for a non-trivial application
	 * @return an integer just like what you want to the main returns to OS
	 */
	virtual int mainstay();

	/*
	 * @brief on the top of that is provided in parent class, GL version will be
	 *  added
	 * @return none
	 */
	virtual void show_version();

	/* 
	 * @brief need to handle a special option to run the openGL GUI if no other
	 *        application specific option to do so
	 * @param optv an integer giving what the option that is being proceesed is
	 * @return an integer of meaning as follows:
	 *  - < 0 for invalid argument value
	 *  - == 0 for correctly handled, process the next one
	 *  - > 0 for terminating the option handling and exit the program
	 * @see CApplication::handleOptions
	 */
	virtual int handleOptions(int optv);

	/* 
	 * @brief a backbone in the display callback
	 * @note this simply provides a alternative to overload the display
	 *  procedure against display(), which would be more preferred by those
	 *  lazybones whose do not even bother to write the routine calls in the
	 *  display() callback like glClear(...) at the beginning and trigger() in
	 *  the end
	 * @return none
	 */
	virtual void draw();

	/*
	 * @brief draw all gadget in current store
	 * @return none
	 */
	void drawGadgets();

	/*
	 * @brief add a gadget to the end of the store
	 * @param gadget a pointer to a gadget object to add
	 * @return -1 for error, otherwise current size of the gadget store
	 */
	int addGadget(CGLGadget* gadget);

	/*
	 * @brief remove a specific gadget from the store
	 * @param index the No. of the gadget to remove
	 * @return -1 for error, otherwise current size of the gadget store
	 */
	int removeGadget(int index);

	/* following members are simply listed to expect overloading to provide what
	 * a practical openGL program has as components
	 */
	virtual void glInit( void );
	virtual void keyResponse(unsigned char key, int x, int y);
	virtual void specialResponse(int key, int x, int y);
	virtual void mouseResponse(int button, int state, int x, int y);
	virtual void mouseMotionResponse(int x, int y);
	virtual void mousePassiveMotionResponse(int x, int y);
	virtual void mouseWheelRollResponse(int wheel, int direction, int x, int y);
	virtual void onReshape( GLsizei w, GLsizei h );
	virtual void display( void );
	virtual void onIdle( void );

protected:
	// all bunch of GLUT windowing parameters
	/* window title */
	string m_wTitle;
	/* upperleft position of the window to show */
	int m_x, m_y;
	/* initial size of the window */
	int m_width, m_height;
	/* display mode */
	int m_dispMode;
//private:
	/* if animate by stroking keys randomly */
	bool m_bAnimated;
	/* record which mouse button is being pressed, -1 indicates none */
	int m_pressedbtn;
	/* record mouse coordinate when any of the mouse buttons is pressed */
	int m_mx, m_my;
	/* if initially display in full size window */
	bool m_bFullWindow;

protected:
	// projection-specific parameters
	/* the angle of the field of view in the yz plane */
	GLdouble m_fvoy;
	/* distance from the viewpoint to the near clipping plane */
	GLdouble m_near;
	/* distance from the viewpoint to the far clipping plane */
	GLdouble m_far;

	// viewing-specific parameters
	/* Z coordinate of the eye position */
	GLdouble m_eyeX;
	GLdouble m_eyeY;
	GLdouble m_eyeZ;
	GLdouble m_centerX;
	GLdouble m_centerY;
	GLdouble m_centerZ;
	GLdouble m_upX;
	GLdouble m_upY;
	GLdouble m_upZ;

	// coordinate offsets, this is usually regarded as the offset from the
	// centroid in the object coordinate system to that of the grand fixed
	// coordinate system which is (0,0,0) as default
	GLdouble m_dx, m_dy, m_dz;

	/* two opposite vertices to define the bounding box of 
	 * all objects to encompass
	 */
	GLdouble m_minx, m_miny, m_minz;
	GLdouble m_maxx, m_maxy, m_maxz;

	/* light source position */
	GLfloat m_position [4];
	/* primitive color */
	GLfloat m_color[3];

	// stash a group of gadgets
	vector<CGLGadget*> m_gadgets;

	// retrieve X information and fill the windowing parameters all as default
	int _setDefaultWindow();

	// reverse transformation from a window coordinate to an object coordinate
	// i.e. convert window coordinate (winx,winy,winz) to object coordinate
	// (objx, objy, objz)
	int _wincoord2objcoord(int winx, int winy, int winz,
							GLdouble *objx, GLdouble *objy, GLdouble *objz,
							bool notrans = true);

	/* in order to evade the conflict between polymorphism, by which we want the
	 * child classes could simply overload the GLUT routines beblow and make
	 * them effect, and the static prototype of functions as callback for the
	 * GLUT accessor functions, we adopt a roundabout approach, in which
	 * following wrapers are defined as static functions thus act as callbacks,
	 * then the virtual functions implementing the real stuff will be invoked
	 * via a static pointer member in these wrapers
	 *
	 * note that callbacks must not be non-static class members
	 */
	static void _keyResponse(unsigned char key, int x, int y);
	static void _specialResponse(int key, int x, int y);
	static void _mouseResponse(int button, int state, int x, int y);
	static void _mouseMotionResponse(int x, int y);
	static void _mousePassiveMotionResponse(int x, int y);
	static void _mouseWheelRollResponse(int wheel, int direction, int x, int y);
	static void _onReshape( GLsizei w, GLsizei h );
	static void _display( void );
	static void _onIdle( void );

	/*
	 * @brief a room whre to place any processing for resource recycling,since
	 * class destructor might not be able to be exerted, actions meant for being
	 * in the destructor otherwise should be placed herein therefore
	 */
	virtual void cleanup();

protected:
	/* pointer to the derived or this class object */
	// this is a handy way towards polymorphism while giving the subtype classes
	// freedom of unnecessary use of pointer or reference
	static	CGLApp* m_pInstance;
	
	// switch effect of gadgets
	GLboolean m_bGadgetEnabled;

	// flag if it is the 1st time the reshape callback invoked
	bool m_bFirstReshape;

	/* a help prompt gadget */
	CGLTextbox m_helptext;
	/* text file containing interaction help prompt */
	string m_strfnhelp;
	/* help text box presence switch */
	bool m_bShowHelp;

};

#endif // _GLMOTH_H_

/*set ts=4 tw=80 sts=4 sw=4*/

