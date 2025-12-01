#ifndef TimeOfDay_H
#define TimeOfDay_H
/**********************************************************************
 **                                                                  **
 **  TimeOfDay.h                                                     **
 **                                                                  **
 **  Make dealing with times easier                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: TimeOfDay.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: TimeOfDay.h,v $
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
* Revision 1.14  2004/05/11 19:34:29  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.13  2004/03/08 14:52:13  doug
* cross compiles on visual C++
*
* Revision 1.12  2003/11/26 15:21:34  doug
* back from GaTech
*
* Revision 1.11  2003/03/15 03:11:52  doug
* fixed a bug in nsecs
*
* Revision 1.10  2003/03/10 16:31:34  doug
* FittedCurve is closer
*
* Revision 1.9  2003/02/28 23:06:03  doug
* add a secs function returning a double
*
* Revision 1.8  2003/02/28 22:31:30  doug
* change secs function to whole_secs
*
* Revision 1.7  2003/02/28 13:46:55  doug
* refined accessors to get nsecs
*
* Revision 1.6  2003/02/27 22:43:00  doug
* seems to work
*
* Revision 1.5  2003/02/26 14:54:16  doug
* controller now runs with objcomms
*
* Revision 1.4  2003/02/19 23:28:18  doug
* added multiplication
*
* Revision 1.3  2003/02/19 22:20:31  doug
* Add conversions from and to double and get rid of the int conversions
*
* Revision 1.2  2003/02/19 15:19:35  doug
* Extended TimeOfDay capabilities
*
* Revision 1.1  2003/02/18 21:47:56  doug
* Added TimeOfDay class
*
**********************************************************************/

#include <math.h>
#include "mic.h"

namespace sara
{

class TimeOfDay
{
public:
   /**@name Public Member Functions */
   //@{
   /// Create a default 0.0 TimeOfDay
   TimeOfDay() :
      seconds(0.0)
   { /* empty */ };

   /// Create a TimeOfDay with the specified time (in seconds)
   explicit TimeOfDay(double time) :
      seconds( time )
   {};

   /// Load a TimeOfDay from a string
   /// legal forms:
   ///    24-hour time formated as "hh:mm:ss"
   /// returns true on success, false on invalid data
   bool load(const string & time);

   /// Create from another TimeOfDay
   TimeOfDay(const TimeOfDay &v) :
      seconds(v.seconds)
   {};

   /// Destroy the TimeOfDay
   ~TimeOfDay()
   {};

   /// Return an object with the current time
   static TimeOfDay now();

   /// Return the number of seconds since the UNIX epoch as a double
   double secs() const {return seconds;}

   /// Return just the number of seconds since the UNIX epoch
   long whole_secs() const {return (long)(seconds + 0.5);}

   /// Return the time in milliseconds since the UNIX epoch
   long msecs() const {return (long)(seconds * 1000.0 + 0.5);}

   /// Return just the absolute value of the fractional seconds as nanoseconds
   long fract_nsecs() const;

   /// Is the time greater than zero?
   bool isTimeLeft() const {return seconds > 0;}

   /// Support addition by a TimeOfDay (form: T1 * T2)
   friend TimeOfDay operator+(const TimeOfDay &v1, const TimeOfDay &v2);

   /// Support subtraction by a TimeOfDay (form: T1 - T2)
   friend TimeOfDay operator-(const TimeOfDay &v1, const TimeOfDay &v2);

   /// Support addition by a TimeOfDay (form: T1 += T2)
   TimeOfDay &operator+=(const TimeOfDay &v);

   /// Support subtraction by a TimeOfDay (form: T1 -= T2)
   TimeOfDay &operator-=(const TimeOfDay &v);

   /// Support multiplication by a double (form: T1 *= d)
   TimeOfDay &operator*=(const double &d);

   /// Support division by a double (form: T1 /= d)
   TimeOfDay &operator/=(const double &d);

   /// Support multiplication by a double (form: T1 * d)
   friend TimeOfDay operator*(const TimeOfDay &T1, const double d);

   /// Support division by a double (form: V1 / d)
   friend TimeOfDay operator/(const TimeOfDay &T1, const double d);

   /// Support comparison of a TimeOfDay (form: T1 > T2)
   friend bool operator>(const TimeOfDay &v1, const TimeOfDay &v2);

   /// Support comparison of a TimeOfDay (form: T1 >= T2)
   friend bool operator>=(const TimeOfDay &v1, const TimeOfDay &v2);

   /// Support comparison of a TimeOfDay (form: T1 < T2)
   friend bool operator<(const TimeOfDay &v1, const TimeOfDay &v2);

   /// Support comparison of a TimeOfDay (form: T1 <= T2)
   friend bool operator<=(const TimeOfDay &v1, const TimeOfDay &v2);

   /// Write just the time as a string in 24-hour time formated as "hh:mm:ss"
   string timeAsString() const;

   // Stream out the value
   friend ostream & operator << (ostream & out, const TimeOfDay &v);
   //@}

private:
   /**@name Private Data */
   //@{
   /// The internal representation is seconds as a double.
   double seconds;
   //@}
};

}
#endif

