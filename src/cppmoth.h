// ----------------------------------------------------------------------------
// cppmoth.h : a coarse, even a bit of tacky, skeleton of GNU Linux C++ console 
//				program, which is meant for providing a inheritable class
//				hierarchy for a prototyped application that includes:
//				1. version display option covering compilation date by
//					-v|--version
//				2. help information option by
//					-h|--help
//				3. verbose output used in debug mode by
//					-V|--verbose
//				4. command line arguments handling, depending on the
//					functionalities of the application in question
//
// Creation : Jan. 6th 2011
//
// Revision:
//	@Feb. 17th
//		make MyCout a subclass of ostream so that the instance of this class
//		will be able to be passed inwards or outwards with a value compatible
//		with commonly used instances like std::cout or that of 
//		std::ostringstream
//	@Mar. 3rd
//		once option handling is finished using getopt.h stuff, reset the
//		scanning position by optind = 1, this is CRUCIALLY important while
//		multiple time of call to getopt or getopt_long is needed.
//	@Mar. 7th
//		.setVerinfo is open to be "public", as sounds more reasonable and
//		extensibly friendly
//	@Mar. 8th
//		.make MyCout support timestamp heading each log line
//	@Mar. 16th
//		.FIX BUGS: while adding little bit more options by addOption (10) in
//		descendant instance, casualty of core-dumped occurred! finally the bug
//		found with the assitance of Valgrind : cppmoth.cpp:447 +
//		cppmoth.cpp:462. the adjacent annotated sentences are the culprits.
//		Not dive into the problem details yet, but here is an impulsive
//		conclusion:
//			string::length is not necessarily equal to strlen(string::c_str()).
// @April. 12th
//		.simplfies output for "-h | --help" esp. for the "-h" and "-v" options
//		.ameliorate the line segmenting to make sure complete words in each line
//		of the output of "-h | --help"
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#ifndef _CPPMOTH_H_
#define _CPPMOTH_H_

#include <unistd.h>
#include <stdint.h>
#include <getopt.h>
#include <sys/time.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <iostream>
#include <iomanip>
#include <vector>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <utility>
#include <string>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <set>

#define ARRAY_SIZE(a)		( NULL==a?0 : (int)(sizeof a / sizeof a[0]) )
#define MAX_FMT_LINELEN		( 80 )
#define ABS(a)				( a >= 1e-6 ? (a) : -(a) )

using std::cout;
using std::ostream;
using std::string;
using std::map;

///////////////////////////////////////////////////////////////////////////////
// this class was designed to put an overall control upon the debugging dumping
// through standard output stream
class CApplication;

class MyCout : public ostream {
public:
	MyCout(bool soff=true, ostream& os=cout);
	MyCout(const MyCout& other);
	
	void switchoff(bool soff=true);
	bool isswitchon() const;

	void switchtime(bool bon);
	
	template <typename _T>
	MyCout& operator << (_T otherthing);

	virtual MyCout& operator << (const char* otherthing);
	virtual MyCout& operator = (const MyCout& other);

	friend class CApplication;

private:
	bool m_switchoff;
	bool m_btime; /* switch of timestamp */
	bool m_bnewline; /* if a newline started */
	ostream & m_os;
};

template <typename _T>
MyCout& MyCout::operator << (_T otherthing) 
{
	if (m_switchoff) { 
		return *this; 
	}

	m_os << otherthing;
	return *this;
}

// the endline manipulator, if left unoverloaded, will make the operator 
// << refuse to work with "std::endl"
template <typename _T>
_T& myendl(_T& myos) 
{
	if (myos.m_switchoff) return myos;
	myos << "\n";
	return myos;
}

///////////////////////////////////////////////////////////////////////////////
// this the outermost shell to cover the model application described above
class CApplication {
protected:
	typedef struct _optitem_t {
		/* if need argument value */
		bool hasarg;
		/* long option name */
		string optname;
		/* descriptive help prompt */
		string message;

		_optitem_t( const char* _longname = "",
					bool _hasarg = true,
					const char* _message = "" ) {
			optname = _longname;
			hasarg = _hasarg;
			message = _message;
		}

		_optitem_t& operator = (const _optitem_t& other) {
			hasarg = other.hasarg;
			optname = other.optname;
			message = other.message;
			return *this;
		}
	}optitem_t;

	/* a map from short option name to helpitem_t */
	typedef map< char,  optitem_t > optinfo_t;

public:
	CApplication();
	CApplication(int argc, char** argv);
	virtual ~CApplication(); 

	/*
	 * @brief the entry for all commandline stuff setting, arguments would be
	 * transmitted to getopt or getopt_long as intact. This is the alternative
	 * to addOption and the mingled use of both is NOT recommended
	 * @param opstring a constant string giving the option description
	 * @param longopts a pointer to the option structure given detailed option
	 *  setting
	 * @return an integer evaluated as 0 if nothing out of exception, non-zero
	 *  value bringing back error code to the caller otherwise
	 * @see addOption
	 */
	int setOptions(const char* opstring, const struct option* longopts = NULL);

	/*
	 * @brief add an option item into the option info store, this is an
	 * alternavte to setOptions and the mingled use of both is NOT recommended 
	 * @param optname a character giving an option name, such as "h" or "v"
	 * @param hasarg a boolean giving if this option needs an argument value
	 * @param longname a string giving the long option name
	 * @param message a string describing this option in detail
	 * @return an integer evaluated as 0 if nothing out of exception, non-zero
	 *  value bringing back error code to the caller otherwise
	 * @see setOptions
	 */
	int addOption(char optname, bool hasarg = true, 
				  const char* longname = "", const char* message = "");

	/*
	 * @brief explictly designated a string as a remarkable version info
	 * @param verinfo a string telling what the desired version info is
	 * @return none
	 */
	void setVerinfo(const char* verinfo);

	/*
	 * @brief the skeleton for the main function 
	 * @param nohv a boolean showing that the any option handling should be
	 *  proscribed
	 * @return an integer whose valued will simply be forwarded to main as its
	 *  return to the OS
	 */
	virtual int run(bool noopt = false);

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
	 * @brief show brief or full help message, formatted in a static
	 *  pattern, in which the long option names always take up to 20 characters
	 *  in width.
	 * @param detailed a boolean giving if show the help message in detail
	 * @return bone
	 */
	virtual void show_helpmsg(bool detailed = true);

	/*
	 * @brief show the version information, including the user defined 
	 *  application version by a Macro in the Makefile - "VERSION", and the
	 *  compilation date, which is the legacy Macro of gcc - __DATE__.
	 * @return none
	 */
	virtual void show_version();

	/*
	 * @brief handle other options than the default ones, i.e. "hvV"
	 * @param optv an integer giving the option currently processed
	 * @return an integer of meaning as follows:
	 *  - < 0 for invalid argument value
	 *  - == 0 for correctly handled, process the next one
	 *  - > 0 for terminating the option handling and exit the program
	 */
	virtual int handleOptions(int optv);

protected:
	/* vertatim mirror of the entry arguments main() received from OS */
	int m_argc;
	char** m_argv;
	/* a homemade ostream object controlling verbose output */
	MyCout m_cout;

private:
	/* option string and option structure, if long option style used */
	char* m_optstring;
	struct option* m_plongopts;
	/* number of actual long options */
	int m_nlongopts;

	/* as an alternative to set options, store all pertinent info ia the map */
	optinfo_t m_optinfo;

	/* version info of the specific application derived from this class
	 * @note ordinarily we transfer the version through Makefile by a Macro like
	 * "VERSION", and we want an automatic definition of this macro according to
	 * the program that is being compiled by a Makefile directive like:
	 * %.o : %.cpp 
	 *		$(CC) -c $< -o $@ $(INC) $(FLAGS) -DVERSION=\""$(@:.o=) 1.0.0.0\""
	 *
	 * as such, when the compiler just compile the derived application, i.e.
	 * converting the source code into object, however, it can not see the
	 * the specific value of the macro VERSION passed to it, and when we simply
	 * call the show_version function inherited from this root class, the value
	 * VERSION will always be the one that was seen by the compiler at the time
	 * this root class was compiled thus always got a value as "cppmoth".
	 *
	 * Since this did not derive a desirable even meaningful version
	 * information, we set a separate variable to take into what the specific
	 * derived application's version is so as to hold a version text in desire.
	 *
	 * if this variable will not be set in the derived application, it will
	 * simply be missed in the output of "${prog} -v|--version"
	 */
	string m_verinfo;

private:
	/* internal use only, for dispatch processing short and long option 
	 * mode
	 */
	bool _handleLongopts();
	bool _handleShortopts();
	bool _mergeOptions();

};

#endif // _CPPMOTH_H_

/*set ts=4 tw=80 sts=4 sw=4*/

