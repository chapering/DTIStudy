// ----------------------------------------------------------------------------
// cppmoth.cpp : a coarse, even a bit of tacky, skeleton of GNU Linux C++ console 
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
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#include "cppmoth.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
/* all the global data structures to be used throughout this namespace*/
#ifdef VERSION
static const char* g_version = VERSION;
#else
static const char* g_version = "unknown version";
#endif

#ifdef __DATE__
static const char* g_date = __DATE__;
#else
static const char* g_date = "unknown date";
#endif

#ifdef __TIME__
static const char* g_time = __TIME__;
#else
static const char* g_time = "unknown time";
#endif

///////////////////////////////////////////////////////////////////////////////
// this class was designed to put an overall control upon the debugging dumping
// through standard output stream
template <typename _T>
_T& myendl(_T& myos);

MyCout::MyCout(bool soff, ostream& os) : 
	m_switchoff(soff), 
	m_btime(false),
	m_bnewline(true),
	m_os(os)
{
}

MyCout::MyCout(const MyCout& other) :
	m_switchoff(other.m_switchoff),
	m_btime(other.m_btime),
	m_bnewline(other.m_bnewline),
	m_os(other.m_os)
{
}
	
void MyCout::switchoff(bool soff) 
{ 
	m_switchoff = soff;
}

bool MyCout::isswitchon() const
{
	return !m_switchoff;
}

void MyCout::switchtime(bool bon)
{
	m_btime = bon;
}

// template specification for operator << when entry type is of "const char*"
MyCout& MyCout::operator << (const char* otherthing)
{
	if (m_switchoff) { 
		return *this; 
	}

	if ( strlen(otherthing) < 1 ) {
		return *this;
	}

	const char *pos = strchr(otherthing, '\n');
	if ( m_btime && NULL != pos ) {
		m_bnewline = true;

		m_os << otherthing;
		time_t calender = time(NULL);
		struct tm *ptm = localtime(&calender);

		/*
	    m_os << " @ " << asctime( gmtime(&calender) ) 
		*/
		m_os << "["
			<< setfill('0') << setw(2) << ptm->tm_hour << ":" 
			<< setfill('0') << setw(2) << ptm->tm_min << ":"
			<< setfill('0') << setw(2) << ptm->tm_sec << " " 
			<< setfill('0') << setw(2) << (1+ptm->tm_mon) << "/" 
			<< setfill('0') << setw(2) << ptm->tm_mday << "/" 
			<< setfill('0') << setw(4) << (1900+ptm->tm_year)
			<< "] ";
		m_bnewline = false;
		return *this;
	}
	m_os << otherthing;
	return *this;
}

MyCout& MyCout::operator = (const MyCout& other)
{
	m_switchoff = other.m_switchoff;
	m_bnewline = other.m_bnewline;
	m_btime = other.m_btime;
	return *this;
}

///////////////////////////////////////////////////////////////////////////////
// this the outermost shell to cover the model application described above
CApplication::CApplication() : 
	m_argc(0), 
	m_argv(NULL),
	m_optstring(NULL),
	m_plongopts(NULL),
	m_nlongopts(0),
	m_verinfo("skeleton-noarg")
{
}

CApplication::CApplication(int argc, char** argv) : 
	m_argc(argc), 
	m_argv(argv),
	m_optstring(NULL),
	m_plongopts(NULL),
	m_nlongopts(0),
	m_verinfo("skeleton")
{
	// add default options
	addOption('h', false, "help", "show this usage");
	addOption('v', false, "version", "show version information");
	addOption('V', false, "verbose", "switch on the debug mode so that to inspect"
			" what the program will be doing in detail");

	/*
	m_argv = new char*[argc];
	for (int i = 0; i<argc; ++i) {
		m_argv[i] = new char[ strlen(argv[i]) + 1];
		strncpy( m_argv[i], argv[i], strlen(argv[i]) );
	}
	m_argc = argc;
	*/
}

CApplication::~CApplication() 
{
	delete [] m_optstring;
	delete [] m_plongopts;
	m_optinfo.clear();
}

int CApplication::setOptions(const char* opstring, 
							 const struct option* longopts)
{
	if (0 >= m_argc || 1 >= strlen(opstring)) {
		return -1;
	}

	int sz_longopts = 0;

	try {
		m_optstring = new char [strlen(opstring) + 1];
		if ( longopts && 1 <= (sz_longopts = ARRAY_SIZE(longopts)) )  { 
			m_plongopts = new option[sz_longopts]; 
		}
	}
	catch(std::bad_alloc& e) {
		cerr << "Fail: allocate for opstring or longopts structure array, size"
			"= " << sz_longopts << endl;
		return -1;
	}

	strncpy(m_optstring, opstring, strlen(opstring));
	m_optstring[ strlen(opstring) ] = '\0';

	if ( 1 <= sz_longopts ) {
		// since the whole structure was used as a constant data, simply 
		// perform a shallow copy here therefore
		memcpy(m_plongopts, longopts, sizeof (struct option) * sz_longopts);
	}

	return m_nlongopts = sz_longopts;	
}

int CApplication::addOption(char optname, bool hasarg, 
			  const char* longname, const char* message)
{
	m_optinfo [ optname ] = optitem_t(longname, hasarg, message);

	return 0;
}

void CApplication::setVerinfo(const char* verinfo)
{
	m_verinfo = verinfo;
}

int CApplication::run(bool noopt) 
{
	int ret = 1;

	if ( ! noopt && m_argv && 1 <= m_argc ) {
		_mergeOptions();
		if ( m_plongopts ) {
			ret = _handleLongopts();
		}
		else {
			ret = _handleShortopts();
		}
	}

	/* this following reset operation, before I know about it, has been baffling
	 * me for more than 2 hours; why the getopt implementation is bit of
	 * repugnant so much in that it uses global variables here and there...
	 */
	// NOW, yes, in order to guard against multiple times of usage of getopt or
	// getopt_long, and others of this sort, to name the few, we MUST RESET it
	optind = 1;

	if ( ! ret ) {
		return ret;
	}

	return mainstay();
}

int CApplication::mainstay() 
{
	m_cout << "This is in the top level nutshell.\n";
	return 1;
}

void CApplication::show_helpmsg(bool detailed)
{
	fprintf(stdout, "usage:  %s [option]...\n\n", m_argv[0]);

	if (!detailed) {
		fprintf(stdout, "type -h or --help for more details on usage.\n\n");
		return;
	}

	fprintf(stdout, "mandatory arguments to long options are mandatory for "
					"short options too.\n\n");

	optinfo_t::iterator it = m_optinfo.begin();
	int nLen = 0, nTotalLen = 0, nPos = 0, nsLen = 0;

	for (; it != m_optinfo.end(); ++it ) {
		nPos = 0;
		nLen = fprintf(stdout, "\t-%c", it->first);
		if ( it->second.optname.length() >= 2) {
			nLen += fprintf(stdout, ",\t--%-20s", it->second.optname.c_str());
		}

		nTotalLen = it->second.message.length(); 
		nsLen = nTotalLen;
		// rare case really
		if ( nLen >= MAX_FMT_LINELEN ) {
			nLen = MAX_FMT_LINELEN/2;
			fprintf(stdout, "\n");
		}
		else {
			nLen = MAX_FMT_LINELEN - nLen;
			if ( nsLen > nLen ) {
				// never cut out a single word
				while ( ' ' != it->second.message[nPos + nLen] &&
						'\r' != it->second.message[nPos + nLen] &&
						'\n' != it->second.message[nPos + nLen] &&
						nLen > 0) {
					nLen --;
				}
				if ( nLen < 0 ) {
					nLen = nsLen;
				}
			}
			else {
				nLen = nsLen;
			}
			fprintf(stdout, "%s\n", it->second.message.substr(nPos, nLen).c_str());
			nTotalLen -= nLen;
			nPos += nLen;
		}

		if ( 0 >= nTotalLen ) {
			continue;
		}

		// now nLen is the length of each following line segment for this single
		// option
		while (nTotalLen > 0) {
			fprintf(stdout, "\t   ");
			// padding towards left-alignment
			if ( it->second.optname.length() >= 2) {
				string tmp(it->second.optname);
				tmp.replace(0, it->second.optname.length(), " ");
				fprintf(stdout, " \t  %-20s", tmp.c_str());
			}
			
			int startPos = min(nPos, nsLen - 1);
			int segLen = nLen;
			if ( nsLen - startPos > segLen) {
				// never cut out a single word
				while ( ' ' != it->second.message[startPos + segLen] &&
						'\r' != it->second.message[startPos + segLen] &&
						'\n' != it->second.message[startPos + segLen] && 
						segLen > 0) {
					segLen --;
				}
				if ( segLen <= 0 ) {
					segLen = nsLen - startPos;
				}
			}
			else {
				segLen = nsLen - startPos;
			}

			fprintf(stdout, "%s\n", 
					it->second.message.substr( startPos+1, segLen ).c_str());
			nPos += segLen;
			nTotalLen -= segLen;
		}
	}

	fprintf(stdout, "\n");
}

void CApplication::show_version()
{
	fprintf(stdout, "%s %s\n", m_verinfo.c_str(), g_version);
	fprintf(stdout, "compiled at: %s %s\n", g_date, g_time);
	fprintf(stdout, "compiler: gcc %s\n", __VERSION__);
}

int CApplication::handleOptions( int optv ) 
{
	return 1;
}

bool CApplication::_handleLongopts()
{
	// every option is optional!
	int longidx = 0;
	int opt = 0;
	bool anyops = false;
	while ( (opt = getopt_long(m_argc, m_argv, 
							   m_optstring, m_plongopts, 
							   &longidx)) != -1) {
		anyops = true;
		switch(opt) {
			case 'h':
				show_helpmsg();
				return false;
			case 'v':
				show_version();
				return false;
			case 'V':
				m_cout.switchoff( false );
				break;
			default:
				{
					int ret = handleOptions( opt );
					if ( ret < 0 ) {
						goto invalid_argument;
					}
					else if ( ret > 0 ) {
						return false;
					}
				}
				break;
		}
	}

	if (!anyops) {
		show_helpmsg(false);
		return false;
	}

	return true;
invalid_argument:
	cerr << "invalid argument!" << endl;
	return false;
}

bool CApplication::_handleShortopts()
{
	// every option is optional!
	int opt = 0;
	bool anyops = false;
	while ( (opt = getopt(m_argc, m_argv, m_optstring)) != -1 ) {
		anyops = true;
		switch(opt) {
			case 'h':
				show_helpmsg();
				return false;
			case 'v':
				show_version();
				return false;
			case 'V':
				m_cout.switchoff( false );
				break;
			default:
				{
					int ret = handleOptions( opt );
					if ( ret < 0 ) {
						goto invalid_argument;
					}
					else if ( ret > 0 ) {
						return false;
					}
				}
				break;
		}
	}

	if (!anyops) {
		show_helpmsg(false);
		return false;
	}

	return true;
invalid_argument:
	cerr << "invalid argument!" << endl;
	return false;
}

bool CApplication::_mergeOptions()
{
	struct option* ptmp = NULL;
	int sz_longopts = 0;

	// merge optstring if necessary
	if ( 1 > m_optinfo.size() ) {
		return true;
	}
	ostringstream ostropt;
	if ( m_optstring ) {
		ostropt << m_optstring;
		delete m_optstring;
	}

	optinfo_t::const_iterator it = m_optinfo.begin();
	while ( it != m_optinfo.end() ) {
		ostropt << it->first;
		if ( it->second.hasarg ) {
			ostropt << ":";
		}

		// only count the long option with valid long option name
		if ( it->second.optname.length() >= 2 ) {
			sz_longopts ++;
		}

		it ++;
	}

	ostropt << ends;
	try {
		//m_optstring = new char [strlen(ostropt.str().c_str()) + 1];
		m_optstring = new char [ostropt.str().length() + 1];
		// only if at least one valid long option name ever added,
		// the long option handling will be enabled
		if ( 1 <= sz_longopts )  { 
			sz_longopts = static_cast<int>( m_optinfo.size() );
			// extra slot for the tailing one "0, 0, 0,0"
			ptmp = new option[sz_longopts + m_nlongopts + 1]; 
		}
	}
	catch(std::bad_alloc& e) {
		cerr << "Fail: allocate for opstring or longopts structure array." << endl;
		return false;
	}

	//strncpy(m_optstring, ostropt.str().c_str(), strlen(ostropt.str().c_str()));
	strncpy(m_optstring, ostropt.str().c_str(), ostropt.str().length());
	m_optstring[ ostropt.str().length() ] = '\0';

	// merge long option items all into the underlying structure if necessary
	if ( 1 <= sz_longopts )  { 
		sz_longopts += m_nlongopts;
		if ( m_plongopts ) {
			memcpy(ptmp, m_plongopts, m_nlongopts * sizeof (struct option) ); 
			delete [] m_plongopts;
		}
		m_plongopts = ptmp;

		it = m_optinfo.begin();

		// the original structure must has already contain a tailing tuple "0,0,0,0"
		// so we just simply overwrite it at this end position and start merging
		// options in the map from there
		int idx = m_nlongopts >= 1? m_nlongopts - 1 : 0;
		while ( it != m_optinfo.end() ) {
			m_plongopts [ idx ].name = it->second.optname.c_str();
			m_plongopts [ idx ].has_arg = it->second.hasarg?1:0;
			m_plongopts [ idx ].flag = NULL;
			m_plongopts [ idx ].val = it->first;
			idx ++;
			it ++;
		}

		// extra slot for the tailing one "0, 0, 0,0"
		m_plongopts [ idx ].name = 0;
		m_plongopts [ idx ].has_arg = 0;
		m_plongopts [ idx ].flag = 0;
		m_plongopts [ idx ].val = 0;

		m_nlongopts = sz_longopts;
	}

	return true;
}

/*set ts=4 tw=80 sts=4 sw=4*/

