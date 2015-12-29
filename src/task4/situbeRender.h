// ----------------------------------------------------------------------------
// situbeRender.h: load geometry in the format of "tgdata" and then wrap each
//					each streamline with rings resembling tubes. 
//
//					Note that the tubes produced are not of any anatomical
//					meanings but simply a geometrical artifacts. So they are not
//					genuine streamtubes
//
// Creation : Feb. 6th 2011
//
// Revision:
//	@Feb. 21th 
//		1.Geometry-based direction color encoding
//		2.add interaction option for using vertex coordinate as vertex normal 
//	@Feb. 24th
//		1. correct the problem of negative colors in the orientation color
//		encoding. By take fabs over the negative ones, this coloring becomes
//		a pure orientation color encoding scheme
//	@Feb. 26th
//		1. add a continuous and anatomically symmetric coloring encoding by
//			scaling the gamut of [-1,1] to [0,2] and then to [0,1]
//		2. add multiple coloring scheme support for coloring comparative
//		analysis
//	@Feb. 27th
//		1. extract coloring scheme stuff into an independent component as a
//			template class CColorMapper in colorscheme.h
//	@Feb. 28th
//		1. add an anatomical coordinate system, together with the spheric color
//		map gadget
//	@Mar. 2nd
//		1. add the DWI image embedding, held by one more gadget : CDcmExplorer
//		2. in order to ease implementation of mitubeRender, this single model
//		render, originally held in a single cpp, is mutated into an ordinary
//		class design and then instantiated in a boosting main-like cpp. Here
//		thus comes this header file
//	@Mar. 4th
//		1.add glutIdleFunc callback to support an IPC mechanism to receive user
//		interaction from other sinlge DTI model rendering process so that to
//		implement a synchronous interaction response, as is expressly serving
//		the MitubeRender
//	@Mar. 5th
//		1.add boolean flag to adapt the CSitubeRender class for building both a
//		single render and serving the CMitubeRender for building multiple-window
//		DTI model render - simply using m_nSiblings==0 as the boolean
//		2. add SIGUSR1 handler to receive the notification from parent that one
//		more sibling process died and thus to cut off the number of alive
//		siblings by 1, this is non-trivial since it will help CSitubeRender keep
//		aware of the latest number of siblings thus make possible the
//		interaction synchronization will still be correct even some of the
//		processes terminate on the way
//	@Mar. 6th
//		1.Fix the bug that when button is bounced up, the IPC mechanism does not
//		count when the event dispatching has finished, causing the event to be
//		kept being dispatched thus raising disorders, by forcing
//		glutPostRedisplay() even in the responser to button event while the
//		state being GLUT_UP, both in GLcppmoth and iselbox classes
//		2.When receiving SIGTERM, this process, mostly in the role of child
//		process, should grasp the last chance to release all resources,i.e to
//		invoke cleanup
//		3.Change special key response: END to terminate current process,
//		PAGE_DOWN to terminate all members of the process group
//		4.To enhance the flexibility in terms of being managed as
//		children/component, add a m_bSuspended switch to allow the manager to
//		postpone the entrance into GL event loop
//	@Mar. 9th
//		.add interaction help prompt gadget, a GL textbox; and an extra command
//		line argument for loading help text from a file; another key mapping is
//		also added accordingly as a switch of help text box presence:
//			F1 - switch help text box presence
//		.add task prompt gadget, a subtyped CGLTextbox; and an extra command
//		line argument for loading the task list file
//	@Mar. 10th
//		.for performing visualization tasks, going to the next task as the response
//		 to Space bar pressing is added
//		.Before each task is confirmed to start, nothing will be drawn on the
//		screen except the task box and all other inputs are blocked as well then
//		except for the Space bar
//	@Mar. 11th
//		.Two types of associative relation among all selection boxes are
//		defined: 
//			AND pattern - conditions must be met for all boxes (for selection
//			mode, a fiber is selected only if it goes through all boxes; for
//			removal mode, a fiber is removed only if it is outside of all
//			boxes);
//			OR pattern = conditions need be met for at least one boxes(for
//			selection mode, a fiber is selected if only one of the boxes covers
//			it; for removal mode, a fiber is culled if only one of the boxes
//			excludes it)
//		and another key shortcut is added accordingly
//			F2 - switching associative pattern among selection boxes
//		.Logic in isTubeInSelbox() is changed to be consistent with isLineInBox,
//		which is actually required to make sure that the selection/removal mode,
//		either in AND or OR pattern, works correctly geometrically.
//	@Mar. 12th
//		1.For the visualization study's purpose, saving/loading partial brain
//		model, one or certain other number of regions, say, is added as an
//		auxiliary functionality; this mainly impact CTgdataLoader::dump though,
//		and the change needed in CSitubeRender is the addition of new member
//		function dumpRegions(), as might be the response to key stroke or part
//		of process upon a task is done
//		--- the m_edgeflags member in parent class CGLIBoxApp is put into use
//		for this purpose, which is in fact meant for implementing vertex
//		selection that is not in use in CSitubeRender where we do not use vertex
//		flag to determine a streamline is in a box or not. So it is safe to make
//		this very reuse of the seemingly deprecated member here in CSitubeRender
// @Mar. 13th
//		1. For task0, change on the basis of study-specific version:
//			--- Keyboard and mouse mapping are all frosted except only
//			following:
//			Space : navigate,only forwards, through tasks
//			Mouse Left button: rotate around X/Y
//			Mouse left button + CTRL: rotate around Z
//
//			--- only use FA color, i.e. the original color offered in the tgdata
//			source
//
//			--- no selection box
//
//			--- show several point by circles and let user select the one of
//			highest FA value
//
//			--- 3D spherical color map changed to 2D rectangular color map
// @Mar. 14th
//			-- exclusively for task 0: select two vertices of the model, draw
//			straight line with letter at the end as indicator; we want the user
//			to compare FAs of the two vertices
//			-- new member _getAnswer() is added to record what would be the
//			user's answer to the question raised in the task
// @Mar. 17th
//			-- The way of getting FA points is changed from loading command line
//			string argument to loading from a file; since We also want the task
//			key is dynamically determined by loading at the runtime, rather than
//			hardcoded in the program.
//			NOTE: there is tragedy here : before moving on improving the FA
//			point searching tool selectFApoints, this afternoon the change has
//			been made and passed the test; Just sloppiness caused this
//			resumption since task6/situbeRender.* covered the code in these two
//			files for task0 including this change!..
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#ifndef  _SITUBERENDER_H_

#ifdef DWI_EMBEDDING
#include "DCMviewer.h"
#endif
#include "colorscheme.h"
#include "GLgadget.h"
#include "GLoader.h"
#include "GLiboxmoth.h"
#include "glrand.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/signal.h>

using std::cout;
using std::cerr;
using std::vector;
using std::string;

// possible user input event
typedef enum _input_event_t {
	IE_KEY_PRESSED = 0x00,
	IE_SPECIAL_KEY,
	IE_MOUSE_CLICKED,
	IE_MOUSE_MOVE,
	IE_MOUSE_PASSIVE_MOVE,
	IE_MOUSE_WHEEL_ROLL,
	IE_NONE = -1
}input_event_t;

class CSelectFAPoints;

class CSitubeRender: public CGLIBoxApp {
protected:
	/*
	 * this is actually delineating a piece of shared memory holding information
	 * on user interaction which is meant for communciating between multiple
	 * processes of DTI model render
	 */
	typedef struct _interaction_info_t {
		// flagging current key or mouse event, an enumeration of input_event_t
		int event; 
		// mouse position while either clicking or moving
		int x,y;
		// mouse button and the state
		int button, state;
		// key pressed, wheel pushed and the wheel rolling direction
		int key, wheel, direction;

		// indicate if new interaction input reached
		bool bUpdated;

		// current number of processes already synchronized
		int nFinished;
	}interaction_info_t;

	static const char* SITR_SHM_NAME;

public:
	friend class CSelectFAPoints;

	CSitubeRender(int argc, char **argv);
	virtual ~CSitubeRender();

	// randomly offering a color for the uniform coloring
	void genColors();

	// decide the longest and shortest streamline segment among all streamtubes
	// in order to determine tube radius automatically
	void findMaxMinSegLen();

	/*
	 * MAKING artificial tubes by wrapping rings around a single streamline
	 * given by the index of the streamline storage, as is a linear structure
	 * created by loading the input geometry of streamlines
	 */
	void buildTubefromLine(unsigned long lineIdx);

	/*
	 * serialize the streamtube geometry generated into a file in the format
	 * of WaveFront obj
	 */
	int serializeTubes(const string& fnobj);

	/* 
	 * read a file in the format of "tgdata", a .data ouput from tubegen into a
	 * series of linear array as vectors, each holding a streamline including
	 * its vertices and colors, then all these vectors are indexed in a headlist
	 * in which elements are pointers to these vectors respectively
	 *
	 * After the streamline geometry loaded, streamtube geometry was calculated
	 * by wrapping around each streamline a slew of rings that is expected to
	 * simulate a shape of tube only in the geometrical sense, not pertinent to
	 * visually encoding tensor attributes like the eigenvectors and eigenvalue
	 * in each voxel in the tensor field
	 */
	int loadGeometry();

	/*
	 * judge if the tube indicated by the index within the streamtube store is
	 * inside all selection boxes
	 */
	bool isTubeInSelbox(const vector<GLfloat>& line);

	// overloadings to those in the parent class for customized actions
	void glInit(void);
	int handleOptions(int optv);

	void keyResponse(unsigned char key, int x, int y);
	void specialResponse(int key, int x, int y);
	void mouseResponse(int button, int state, int x, int y);
	void mouseMotionResponse(int x, int y);
	void mousePassiveMotionResponse(int x, int y);
	void mouseWheelRollResponse(int wheel, int direction, int x, int y);

	int mainstay();
	void draw();
	void onIdle(void);

	// tell the number of sibling processes in order to realize an imperative
	// interaction input synchronization
	void setNumberOfSiblings(int n);

	// update the suspense switch
	void suspend(bool bSuspend = false);

	// query the sync status
	bool isSync() const;

	// help text switch
	bool switchhelp(bool bon);

	// dump the geometry of streamlines associated with streamtubes currently
	// visible( i.e. selected ) into a file of the given name in the format of
	// "tgdata" as exactly the same as that of the file where the streamline
	// geometry is loaded;
	// when the file name is empty string, a fabricated name will be used - the
	// value of -f option plus current time in seconds (this can make unique the
	// naming)
	int dumpRegions(const char* fnRegion="");

private:
	/* when coloring schemes are all not used, this color is the uniform color
	 * for all tubes which is customizable in the run-time*/
	GLfloat m_colors[3];
	/* a trivial switch telling if to use the color originally produced during
	 * fiber tracking for each streamline */
	GLboolean m_bUseOrgColor;
	/* a binary switch too telling if to apply direction/orientation color
	 * encoding */
	GLboolean m_bUseDirectionColor;
	/* used to create a special visual effect, taking vertex coordinate as the
	 * normal for the cooresponding vertex */
	GLboolean m_bVnormal;

	/* the contained sub-object used for source geometry loading and parsing */
	CTgdataLoader m_loader;

	/* used for loading the skeleton geometry */
	CTgdataLoader m_skeletonLoader;
	CTgdataLoader m_orgSkeleton, m_staticSkeleton;

	/* source file holding the most essential streamline geometry */
	string m_strfnsrc;
	/* file to store dumped geometry in the format of OBJ */
	string m_strfnobj;
	/* directory holding DWI images to embed when needed, mostly b0 images
	 */
	string m_strdwidir;
	/* text file containing interaction help prompt */
	string m_strfnhelp;
	/* text file holding a list of tasks for a single session */
	string m_strfntask;

	/* Level of Detail, the granularity of interpolation in the fabrication of
	 * streamtube geometry*/
	GLubyte				m_lod;
	/* number of selection box */
	GLubyte				m_nselbox;

	/* if use variant radius for tubes */
	GLboolean			m_bVradius;
	/* switch between selection and removal mode */
	GLboolean			m_bRemovalbased;
	/* switch between AND and OR associative pattern among boxes */
	GLboolean			m_bOR;

	/* fantastic factor tunning the streamtube generation */
	GLfloat m_fAdd;

	/* tube radius and radius of the bounding box of the streamline model */
	GLfloat m_fRadius, m_fbdRadius;

	/* the maximal and minimal length over all streamline segments */
	GLfloat m_fMaxSegLen, m_fMinSegLen;

	/* stash of streamtube geometry for all streamlines  */
	vector< vector<GLfloat> >		m_alltubevertices;
	vector< vector<GLfloat>	>		m_alltubenormals;
	vector< vector<GLfloat>	>		m_alltubecolors;
	vector< vector<GLfloat>	>		m_encodedcolors;
	vector< vector<GLuint> >		m_alltubefaceIdxs;

	/* get the maximal X,Y and Z coordinate among all vertices */
	GLdouble m_maxCoord[3];	
	/* get the minimal X,Y and Z coordinate among all vertices */
	GLdouble m_minCoord[3];	

	/* color mapper responsible for color encoding */
	CColorMapper<GLfloat> m_colormapper;
	/* currently we might want to compare multiple coloring encoding scheme */
	int m_colorschemeIdx;
	/* a rectangular color map gadget */
	CGLGadget* m_pcmGadget;
	/* anatomical axes gadget */
	CGLGadget* m_paxesGagdet;

#ifdef DWI_EMBEDDING
	/* A DICOM DWI IMAGE explorer */
	CDcmExplorer m_dcmexplorer;
#endif

	/* DWI Image embedding switch */
	bool m_bShowDWIImage;

	/* a help prompt gadget */
	CGLTextbox m_helptext;
	/* help text box presence switch */
	bool m_bShowHelp;

	/* a task prompt gadget */
	CGLTaskbox m_taskbox;

	/* point to the shared memory structure of interaction information */
	interaction_info_t* m_pIntInfo;

	/* switching synchronous mode */
	bool m_bSync;
	/* in order to force interaction input synchronization, we need to know the
	 * number of sibligns
	 */
	int m_nSiblings;

	// to bypass the callback function limitations and OO design
	// : we want to visit non-static members from a static member (seems
	// evil...)
	static CSitubeRender* m_psitInstance;

	/* to suspend entrance into GL event loop or to release it to make enter */
	bool m_bSuspended;

	// specific to task0: (m_v1,m_v1e) and (m_v2,m_v2e) are two straight line
	// indicator of two selected FA points
	_point_t<GLfloat> m_v1, m_v2, m_v1e, m_v2e;

	// record what the user typed as the answer to the task 
	std::string m_strAnswer;

	// the task key
	std::string m_strKey;

	// where to load the indices for selected FA points
	std::string m_strfnFApoints;

	/* another streamline geometry as the fiber bundle skeleton associated with
	 * the primary geometries in m_strfnsrc
	 */
	string m_strfnskeleton;

	/* if the skeleton projection has been initializd */
	bool m_bSkeletonPrjInitialized;
private:

	// draw a canvas to which the skeleton will be projected
	void drawCanvas(bool bGrid = true);

	// project fiber bundle skeletons to the XZ plane
	int projectSkeleton(int x, int y, bool bZ = false);
	// draw skeleton fibers
	void drawSkeleton();

	/*
	 * @brief open (or create for this process launched) the shared memory and
	 * initialize it
	 * @param bInit a boolean telling if to initialize after opening/creating
	 * @return 0 for success negative values, -1, say, for failure
	 */
	int	_initSharedInfo(bool bInit=true);

	/*
	 * @brief release the shared memory reference
	 * @return 0 for success negative values, -1, say, for failure
	 */
	int	_releaseSharedInfo();

	/*
	 * @brief this is the real stuff to be invoked as the response to key
	 * pressing
	 */
	void _realkeyResponse(unsigned char key, int x, int y);

	/*
	 * @brief this is the real stuff to be invoked as the response to special
	 * key pressing
	 */
	void _realSpecialResponse(int key, int x, int y);

	/*
	 * @brief treat the signal of SIGUSR1 and then update the number of alive
	 * siblings
	 */
	static void _on_sibling_exit(int sig);

	/*
	 * @brief treat the signal of SIGTERM and then release all resources that
	 * would not be automatically released when the process is forced to
	 * terminate
	 */
	static void _on_killed(int sig);

	template<typename _T>
	void _findNearest(const _T& x0, const _T& y0, const _T& z0,
								_T& x1, _T& y1, _T& z1,
								unsigned long& idxl,
								unsigned long& idxp);
	int _selectFApoints(GLfloat dist, GLfloat scale, bool bAlongNormal=false);


	bool _getAnswer(unsigned char key);
};

#endif // _SITUBERENDER_H_

/* set ts=4 sts=4 tw=80 sw=4 */

