#ifndef MIC_H
#define MIC_H
/**********************************************************************
 **                                                                  **
 **  mic.h                                                           **
 **                                                                  **
 **  system wide defines, constants, etc. for libmic                 **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: mic.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: mic.h,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:55  endo
* MissionLab 7.0
*
* Revision 1.2  2006/07/11 10:09:29  endo
* Compiling error fixed.
*
* Revision 1.1  2006/07/01 00:30:29  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.51  2004/11/07 03:02:35  doug
* updated budmon
*
* Revision 1.50  2004/10/25 22:58:11  doug
* working on mars2020 integrated demo
*
* Revision 1.49  2004/09/22 21:29:20  doug
* sliderbar now displays as a sliderbar
*
* Revision 1.48  2004/09/22 18:05:30  doug
* moved map_to_index.cc to inline, and made it a template
*
* Revision 1.47  2004/05/11 19:34:29  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.46  2004/03/23 19:47:49  doug
* working on win32 build
*
* Revision 1.45  2004/03/08 14:52:13  doug
* cross compiles on visual C++
*
* Revision 1.44  2004/02/20 14:39:30  doug
* cmdl interpreter runs a bit
*
* Revision 1.43  2003/02/26 14:54:16  doug
* controller now runs with objcomms
*
* Revision 1.42  2002/11/04 18:09:49  doug
* added seperate strings class so could enhance constructors
*
* Revision 1.41  2002/10/22 21:05:59  doug
* updated to gcc 3.0
*
* Revision 1.40  2002/09/27 19:14:50  doug
* added nint macro
*
* Revision 1.39  2002/09/03 21:34:57  doug
* added doubles as a datatype
*
* Revision 1.38  2002/06/13 15:19:15  doug
* *** empty log message ***
*
* Revision 1.37  2001/09/21 20:27:17  doug
* Added ubyte, uword, and ulong types
*
* Revision 1.36  1999/08/01 01:23:38  doug
* added strstream include to get string streams.
*
* Revision 1.35  1999/04/01 20:22:40  doug
* added sign macro
*
* Revision 1.34  1998/11/08 20:45:43  doug
* added define for sqr function
*
* Revision 1.33  1998/11/01 03:21:10  doug
* added stdio.h include per WIN32 code
*
* Revision 1.32  1998/06/15 21:05:08  doug
* added algorithm include
*
* Revision 1.31  1998/04/06 23:51:15  doug
* added assert include
*
* Revision 1.30  1998/02/16 11:24:12  doug
* added option for window based or not
*
* Revision 1.29  1998/02/14 11:10:26  doug
* added check_for_response_file
*
* Revision 1.28  1998/02/13 14:27:51  doug
* WIN32 changes
*
* Revision 1.27  1998/02/09 13:23:28  doug
* turn off another MS compiler warning
*
* Revision 1.26  1998/02/08 13:47:45  doug
* pull in iostream here since is different for win32
*
* Revision 1.25  1998/02/06 14:37:39  doug
* removed timeout enum. conflicts with win32
*
* Revision 1.24  1998/02/06 12:51:01  doug
* more WIN32 fixes
*
* Revision 1.23  1998/02/04 14:01:22  doug
* Working on WIN32 code
*
* Revision 1.22  1998/02/04 13:35:04  doug
* fixed win32 ifdef
*
* Revision 1.21  1998/02/02 15:06:48  doug
* added namespace using statement for win32
*
* Revision 1.20  1998/01/28 12:49:48  doug
* added ints and uints data types.
*
* Revision 1.19  1998/01/26 14:16:25  doug
* added the bools datatype as a vector of bool
*
* Revision 1.18  1998/01/26 14:12:41  doug
* added TriState and TriStates as datatypes.
*
* Revision 1.17  1997/12/30 17:58:23  doug
* docified
*
* Revision 1.16  1997/12/30 17:53:17  doug
* *** empty log message ***
*
* Revision 1.15  1997/12/30 17:39:26  doug
* docifying
*
* Revision 1.14  1997/12/15 13:07:43  doug
* added map_to_index
*
* Revision 1.13  1997/12/13 16:41:02  doug
* Added set.h to includes
*
* Revision 1.12  1997/12/12 11:49:27  doug
* added are_equal to compare floats within EPS_ZERO of each other
*
* Revision 1.11  1997/12/12 01:09:47  doug
* *** empty log message ***
*
* Revision 1.10  1997/12/11 10:54:27  doug
* Added PRINT
*
* Revision 1.9  1997/12/10 11:58:58  doug
* *** empty log message ***
*
* Revision 1.8  1997/12/09 15:58:48  doug
* *** empty log message ***
*
* Revision 1.7  1997/11/25 10:44:30  doug
* removed typeinfo include
*
* Revision 1.6  1997/11/24 12:40:57  doug
* added degree to radian macros
*
* Revision 1.5  1997/11/19 13:20:06  doug
* renamed isTypeOf to PointsTo
*
* Revision 1.4  1997/11/19 13:07:34  doug
* fixed typo
*
* Revision 1.3  1997/11/19 12:55:05  doug
* Add  the IsKindOf macro for rtti type checking
*
* Revision 1.2  1997/11/18 19:02:06  doug
* Delete USE macro.  To eliminate warnings of unused parms, just
* don't name the parm (i.e., f(int i, short) won't warn of the
* second parm not being used.
*
* Revision 1.1  1997/11/13 11:45:23  doug
* Initial revision
*
**********************************************************************/

#include <sys/stat.h>
#ifdef USE_RTTI
#include <typeinfo>
#endif
#include <string>
#include <functional>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <iomanip>
#include <string.h>	//Old compilation error (several functions not defined)
#include <stdlib.h>	//Old compilation error (memset not defined)

// Used everywhere.
#ifdef WIN32
//#include <ios.h>
//#include <windows.h>
// Undefine ERROR from wingdi.h in the windows includes.
#undef ERROR	
#endif

// Get the standard I/O streams.
#include <iostream>

// Pickup the string streams.
#include <sstream>

#if !defined(WIN32)
#include <unistd.h>
#endif

#ifndef SKIPUSING
using namespace std;		// lift all standard names to global space.
#endif

//#ifdef	WIN32
// C4786: "identifier was truncated to '255' characters in the debug info"
//#pragma warning(disable:4786)       // disable C4586 warning
// #pragma warning(default:4786)    // use this to reenable, if desired

// C4503: "decorated name length exceeded, name was truncated"
//#pragma warning(disable:4503)       // disable C4503 warning
// #pragma warning(default:4503)    // use this to reenable, if desired
//#endif


#ifdef USE_EXCEPTIONS
#include "mic_exception.h"
#endif

#include "mstrings.h"

namespace sara
{
/**********************************************************************/
/**@name typedefs */
//@{
/// Define the #bools# data type as a vector of bool.
typedef std::vector<bool> bools;

/// Define the #ints# data type as a vector of int.
typedef std::vector<int> ints;

/// Define the #byte# data type
typedef unsigned char  byte;

/// Define the #uchar# data type
typedef unsigned char  uchar;

/// Define the #ubyte# data type: unsigned, 8 bits
typedef unsigned char  ubyte;

/// Define the #ushort# data type
typedef unsigned short ushort;

/// Define the #uword# data type: unsigned, 16 bits
typedef unsigned short uword;

/// Define the #word# data type: signed, 16 bits
typedef short word;

/// Define the #uwords# data type as a vector of double.
typedef std::vector<uword> uwords;

#ifdef WIN32
/// Define the #ulong# data type: unsigned, 32 bits
// dcm: as of gcc 3.3.2 is now declared in types.h on linux
typedef unsigned long  ulong;

/// Define the #uint# data type
// dcm: as of gcc 3.3.2 is now declared in types.h on linux
typedef unsigned int   uint;
#endif

/// Define the #uints# data type as a vector of uint.
typedef std::vector<uint> uints;

/// Define the #doubles# data type as a vector of double.
typedef std::vector<double> doubles;

/// Define the #TriState# data type as an enum with three values.
typedef enum {CLEAR=0, SET=1, UNKNOWN=2} TriState;

/// Define the #TriStates# data type as a vector of TriStates.
typedef std::vector<TriState> TriStates;

/// Prototype for a void function
typedef void (*vFunction)(void);
//@}

/*********************************************************************/
/// What is the path seperator character for this operating system?
#ifdef unix
#define SEPCHAR "/"
#else
#define SEPCHAR "\\"
#endif

//@}

/*********************************************************************/
/// Check if the object pointed to is derived from the specified class 
#define PointsTo(OBJECT_POINTER,TYPE) \
((dynamic_cast<TYPE*>(OBJECT_POINTER) != NULL) ? true : false) 

/**********************************************************************/
/**@name Angle Conversions */
//@{
// Define PI if it isn't already
#ifndef PI
#define     PI          3.14159265358979323846
#endif
        
/// Define macros for converting degrees back and forth to radians.
#define DEGREES_FROM_RADIANS(val) ((val)*180.0/PI)
#define RADIANS_FROM_DEGREES(val) ((val)*PI/180.0) 
#define RADIANS_TO_DEGREES(val) DEGREES_FROM_RADIANS(val)
#define DEGREES_TO_RADIANS(val) RADIANS_FROM_DEGREES(val)

/// bound an angle so that low <= Theta and Theta < hi 
template<class T1, class T2, class T3> T1 normalize_angle(T1 val, T2 low, T3 hi)
{
   ///
   while (val < low)
      val += hi - low;

   ///
   while (val >= hi)
      val -= hi - low;

   ///
   return val;
}

/// Define a macro to return the sign of a number (-1 if negative, +1 else).
#ifndef sign
#define sign(x) ((x)<0 ? -1 : 1)
#endif

/// Define a macro to return the nearest integer of a number.
#ifndef nint
#define nint(x) ((x)<0 ? -1 : 1) * (int)(fabs(x)+0.5)
#endif

//@}
/**********************************************************************/

/**@name Dealing With Small Values */
//@{
/// Small value (epsilon zero) used to compare floats for equality.
#define EPS_ZERO (0.00001)

/// Check if two values are nearly equal. 
template<class T1, class T2> bool are_equal(T1 val, T2 target)
{
   return val >= target - EPS_ZERO && val <= target + EPS_ZERO;
}
//@}

/**@name Dealing With Large Values */
//@{
/// Specify the requested precision when converting to and from floating point numbers
extern uint FloatPrecision;
//@}

/*********************************************************************/
/**@name Manipulating Files */
//@{
/// If the file exists in the specified directory, it returns true and sets fullpath to be the full path name.
bool findfile(const std::string &fname, const std::string &dirname, std::string &fullpath);

/// Tries to find the file and return the full pathname to it.
bool findfile(const std::string &name, std::string &fullpath);

/// find the file by looking through the list of directories
bool findfile(const std::string &name, const strings &dirs, std::string &fullpath);

/// extract the list of colon seperated directories from an environment variable
strings extract_directories(const char *name);

/// Return the date the file was last modified or -1 if not found
time_t  file_date(const char *name);
//@}


/*********************************************************************/
/**@name Generic Output Functions */
//@{
/// Set to true if window-based error handlers should be used.
extern bool use_window_error_handlers;

/// If set, will write to the file handle instead of stderr.
/// default is stderr.
extern FILE *logging_file;

/// If true and logging_file is set, the messages will be printed both
/// to the logging file and to stderr.  default is false.
extern bool logging_to_file_and_stderr;

/// Print the characters passed, without a leader or a carriage return.
void PRINT(const char *fmt, ...);

/// Inform of an expected event
void INFORM(const char *fmt, ...);

/// Warn of an unusual event
void WARN(const char *fmt, ...);

/// Warn of an unusual event and list the system error message
void WARN_with_perror(const char *fmt, ...);

/// Report an error
void ERROR(const char *fmt, ...);

/// Report an error and list the system error message
void ERROR_with_perror(const char *fmt, ...);

/// Print the message and die
void FATAL_ERROR(const char *fmt, ...);

/// Print the message and die
void FATAL_ERROR_with_perror(const char *fmt, ...);

/// Used by FATAL... to drop into the debugger or dump core.
void my_abort();
//@}


/*********************************************************************/
/**@name Index Manipulation */
//@{
/// Bound a variable to a low and high limit.
#ifdef bound
#undef bound
#endif
template<class T1, class T2, class T3> T1 bound(T1 val, T2 low, T3 hi)
{
   return val <= low ? low : (val >= hi ? hi : val);
}

/// Map the input value $(in_low <= val <= in_high)$ to the output range $(out_low <= output <= out_high)$.
int map_to_index(int out_low, int out_high, int in_low, int in_high, int val);

//@}
/*********************************************************************/
/**@name Math Functions */
//@{
/// Return the square of a value
template<class T> T sqr(T x) { return x * x; }
//#ifndef sqr
//#define sqr(x) (x*x)
//#endif

//@}
/*********************************************************************/
/**@name WIN32 specific support */
//@{
/// Transparantly handle response files, by loading and resetting argv, argc.
void check_for_response_file(int *argc, char ***argv);

//@}
/*********************************************************************/
/**@name String helper functions */
//@{
/// compare two strings, case insensitive
int cmp_nocase(const std::string &s, const std::string &s2);

/// Map the input value $(in_low <= val <= in_high)$ to the output range $(out_low <= output <= out_high)$.
template<class T> T map_to_index(T out_low, T out_high, T in_low, T in_high, T val)
{
   // Compute what percentage of the input range val is.
   double ratio = (double)(val - in_low) / (in_high - in_low);

   // Get the corresponding value in the output range.
   return (T)(ratio * (out_high - out_low) + out_low);
}

/// Define a set of strings
typedef std::set<std::string> stringSet;

/// Define a single key/value string pair
typedef std::pair< std::string, std::string > keyValuePair;

/// Define a map of key/value string pairs
typedef std::map< std::string, std::string, std::less<std::string> > keyValueMap;

//@}
/*********************************************************************/
}
#endif
