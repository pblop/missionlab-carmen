/**********************************************************************
 **                                                                  **
 **  TimeOfDay.cc                                                    **
 **                                                                  **
 **  3D TimeOfDay class                                              **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: TimeOfDay.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: TimeOfDay.cc,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:55  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:30:29  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.11  2004/05/11 19:34:29  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.10  2004/03/24 15:11:52  doug
* builds under win32
*
* Revision 1.9  2004/03/08 14:52:13  doug
* cross compiles on visual C++
*
* Revision 1.8  2003/11/26 15:21:34  doug
* back from GaTech
*
* Revision 1.7  2003/03/15 03:11:52  doug
* fixed a bug in nsecs
*
* Revision 1.6  2003/03/10 16:31:34  doug
* FittedCurve is closer
*
* Revision 1.5  2003/02/27 22:43:00  doug
* seems to work
*
* Revision 1.4  2003/02/26 14:54:16  doug
* controller now runs with objcomms
*
* Revision 1.3  2003/02/19 23:28:18  doug
* added multiplication
*
* Revision 1.2  2003/02/19 15:19:35  doug
* Extended TimeOfDay capabilities
*
* Revision 1.1  2003/02/18 21:47:56  doug
* Added TimeOfDay class
*
**********************************************************************/

#ifdef WIN32
#include <sys/timeb.h>
//#include <time.h>
#else
#include <sys/time.h>
#endif
#include "mic.h"
#include "TimeOfDay.h"

namespace sara
{
// ********************************************************************
/// Load a TimeOfDay from a string
/// legal forms:
///    24-hour time formated as "hh:mm:ss"
bool
TimeOfDay::load(const string & strtime)
{
   istringstream ist( strtime.c_str() ); 

   /// load 24-hour time formated as "hh:mm:ss"
   int h,m,s;
   char colon = '.';

   // hours
   ist >> h >> colon;
   if( colon != ':' )
      return false;
   if( h < 0 || h > 23 )
      return false;

   // minutes
   ist >> m >> colon;
   if( colon != ':' )
      return false;
   if( m < 0 || m > 59 )
      return false;

   // seconds
   ist >> s;
   if( s < 0 || s > 59 )
      return false;

   // good one.
   seconds = (double)(h * 3600 + m * 60 + s);
   return true;
}

// ********************************************************************
/// Return an object with the current time
TimeOfDay 
TimeOfDay::now()
{
   TimeOfDay rtn;
   
#ifdef WIN32
   __timeb64 tstruct;
   _ftime64( &tstruct );
   rtn.seconds = (double)tstruct.time + (double)tstruct.millitm/1000;
#else
   struct timeval tv;
   if( gettimeofday(&tv, NULL) )
   {
      ERROR_with_perror("TimeOfDay::now - Unable to gettimeofday");
   }
   else
   {
      rtn.seconds = (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
   }
#endif

   return rtn;
}

// *********************************************************************
/// Return just the absolute value of the fractional seconds as nanoseconds
long 
TimeOfDay::fract_nsecs() const 
{
   double junk;
   return abs((int)(modf(seconds, &junk) * 1000.0 * 1000.0 * 1000.0 + 0.5));
}

// *********************************************************************
/// Support addition by a TimeOfDay (form: V1 + V2)
TimeOfDay 
operator+(const TimeOfDay &v1, const TimeOfDay &v2)
{
   TimeOfDay rtn( v1 );
   rtn += v2;
   return rtn;
}

	 
/*********************************************************************/
/// Support subtraction by a TimeOfDay (form: V1 - V2)
TimeOfDay 
operator-(const TimeOfDay &v1, const TimeOfDay &v2)
{
   TimeOfDay rtn( v1 );
   rtn -= v2;
   return rtn;
}

/*********************************************************************/
/// Support addition by a TimeOfDay (form: V1 += V2)
TimeOfDay &
TimeOfDay::operator+=(const TimeOfDay &v)
{
   seconds += v.seconds;
   return *this;
}

/*********************************************************************/
/// Support subtraction by a TimeOfDay (form: V1 -= V2)
TimeOfDay &
TimeOfDay::operator-=(const TimeOfDay &v)
{
   seconds -= v.seconds;
   return *this;
}

/*********************************************************************/
/// Support multiplication by a double (form: T1 *= d)
TimeOfDay &
TimeOfDay::operator*=(const double &d)
{
   seconds *= d;
   return *this;
}

/*********************************************************************/
/// Support division by a double (form: T1 /= d)
TimeOfDay &
TimeOfDay::operator/=(const double &d)
{
   seconds /= d;
   return *this;
}

/*********************************************************************/
/// Support multiplication by a double (form: T1 * d)
TimeOfDay 
operator*(const TimeOfDay &T1, const double d)
{
   TimeOfDay rtn( T1 );
   rtn *= d;
   return rtn;
}
	 
/*********************************************************************/
/// Support division by a double (form: T1 / d)
TimeOfDay 
operator/(const TimeOfDay &T1, const double d)
{
   TimeOfDay rtn( T1 );
   rtn /= d;
   return rtn;
}

// *********************************************************************
// Stream out the value
ostream & operator << (ostream & out, const TimeOfDay &v)
{
   char buf[256];
   sprintf(buf,"%.6f", v.seconds); 
   out << buf;
   return out;
}

// *********************************************************************
/// Support comparison of a TimeOfDay (form: T1 > T2)
bool 
operator>(const TimeOfDay &T1, const TimeOfDay &T2)
{
   return T1.seconds > T2.seconds;
}

// *********************************************************************
/// Support comparison of a TimeOfDay (form: T1 >= T2)
bool 
operator>=(const TimeOfDay &T1, const TimeOfDay &T2)
{
   return T1.seconds >= T2.seconds;
}

// *********************************************************************
/// Support comparison of a TimeOfDay (form: T1 < T2)
bool 
operator<(const TimeOfDay &T1, const TimeOfDay &T2)
{
   return T1.seconds < T2.seconds;
}

// *********************************************************************
/// Support comparison of a TimeOfDay (form: T1 <= T2)
bool 
operator<=(const TimeOfDay &T1, const TimeOfDay &T2)
{
   return T1.seconds <= T2.seconds;
}

// ********************************************************************
/// Write just the time as a string in 24-hour time formated as "hh:mm:ss"
string
TimeOfDay::timeAsString() const
{
   ostringstream str;

#ifdef WIN32
//   __timeb64 tstruct;
//   _ftime64( &tstruct );
//   rtn.seconds = (double)tstruct.time + (double)tstruct.millitm/1000;
#else
   time_t secs = (time_t)(seconds + 0.5);
   struct tm *theTime = localtime( &secs );
   str << theTime->tm_hour << ":" << theTime->tm_min << ":" << theTime->tm_sec;   
#endif

   return str.str();
}

// *********************************************************************
}
