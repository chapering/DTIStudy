// ----------------------------------------------------------------------------
// mitubeRenderWindow : a multiple-window itubeRender capsule, loading multiple
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
//	@Mar. 4th
//		.sublimate the servant class CMitubeRender for the previous holistic
//		multipleitr into an independent component for the purpose of
//		extensibility
//		.add glutIdelFunc callback overloading to set a place for the IPC with
//		the ultimate aim of the synchronous user interaction dispatching
//	@Mar. 5th
//		.establish the child process management by process grouping; when one of
//		the children in the group exits, parent will get to know exactly which
//		one sends the SIGCHLD and then update the number of currently alive
//		children, and finally broadcast the update to the group
//	@Mar. 6th
//		.Found that self-cooked multiple process modality, workable though, does
//		not satisfy the smooth interaction, thus overhaul the modality to multiple
//		window with the legacy GLUT multiple window mechanism
//	@Mar. 7th
//		.Finalize the multiple view phase - adding a switch of synchronization
//		control on the interaction input ( this control, already provided in the
//		CSitubeRender though, did not help much the GLUT-based multiple
//		view/window control
//		.GLUT-based multiple viewport implementations were designed separately
//		into two independent classes, CMitubeRenderView for the multiple view
//		implementation and CMitubeRenderWindow for the multiple window one
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#if !defined(_MITUBERENDER_WINDOW_H_)
#define _MITUBERENDER_WINDOW_H_

#include "situbeRender.h"

#include <unistd.h>
#include <sys/signal.h>
#include <sys/wait.h>

using std::vector;
using std::string;

class CMitubeRenderWindow: public CSitubeRender {
public:
	CMitubeRenderWindow(int argc, char **argv);
	virtual ~CMitubeRenderWindow();

	// overloadings to those in the parent class for customized actions
	int handleOptions(int optv);

	/*
	 * by overloading following callback hooks, it is aimint at a centralized
	 * control of interaction input, including almost all inputs like mouse and
	 * keyboard interaction - it firstly intercepts all these inputs and then
	 * distributes to windows that is deemed necessary to receive them while
	 * holding sway over the synchronization of interaction across multiple
	 * windows or views
	 */
	void keyResponse(unsigned char key, int x, int y);
	void specialResponse(int key, int x, int y);
	void mouseResponse(int button, int state, int x, int y);
	void mouseMotionResponse(int x, int y);
	void mousePassiveMotionResponse(int x, int y);
	void mouseWheelRollResponse(int wheel, int direction, int x, int y);

	void onReshape( GLsizei w, GLsizei h );
	void display( void );

	int mainstay();

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

	// a pointer list kept for recording the argument list each of the renders
	// to be fed with
	vector<char**>		m_allargvs;

	// this should do the trick : 
	// to bypass the callback function limitations and OO design
	static CMitubeRenderWindow* m_pmitInstance;

	// switch of synchronization: when it is off, the interaction input will
	// only be relayed to current window, i.e. the one that is being focused
	// NOTE : SINCE this CMitubeRenderWindow class acts as a container of a set of
	// CSitubeRender instances while being a subtyped CSitubeRender itself, it
	// is avoiding being confused to have a separate, with a same name though,
	// synchronization control rather than to inherit from the parent class, 
	// CSitubeRender
	bool m_bSync;

private:

	// calculate a default, evenlys spaced, window layout
	int _createAllWindows();

	// extract from the original command line to form command line for each
	// render instance
	int _assign_argvs(int renderIdx);

	// just create instances of single itubeRender for each of the DTI models
	int _createAllRenders();

	// do everything each render needs to do before entering into the event loop 
	int _armRenders();
};

#endif // _MITUBERENDER_WINDOW_H_

/* set ts=4 sts=4 tw=80 sw=4 */

