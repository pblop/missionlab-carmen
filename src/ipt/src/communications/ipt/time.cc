///////////////////////////////////////////////////////////////////////////////
//
//                                 time.cc
//
// This file implements the class IPTime, which provides an interface to the
// Unix timing calls
//
// Classes defined for export:
//    IPTime
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>

#include "./libc.h"

#include <ipt/time.h>

#ifdef VXWORKS
#include <timers.h>
#else
#include <sys/types.h>
#include <sys/time.h>

#endif

/* create an empty time instance */
IPTime::IPTime()
{
    _sec = _msec = 0;
}

/* static creation function to get the current time and return it */
IPTime IPTime::Current()
{
#ifdef VXWORKS
    struct timespec tp;

    clock_gettime(CLOCK_REALTIME, &tp);

    return IPTime(((double) tp.tv_sec) + ((double) tp.tv_nsec)/1000000000.0);
#else    
    struct timeval tp;
    struct timezone tz;
    gettimeofday(&tp, &tz);
    
    return IPTime(int(tp.tv_sec), int(tp.tv_usec/1000));
#endif
}

/* Time creation function that takes time in seconds */
IPTime::IPTime(double time)
{
    _sec = int(time);
    _msec = int((time-_sec)*1000.0);
}

/* Time creation function that takes time in integer seconds and integer
   milliseconds */
IPTime::IPTime(int sec, int msec)
{
    _sec = sec;
    _msec = msec;
}

/* Operator to add two times.  Modifies its own data and returns a reference
   to it */
IPTime& IPTime::operator+=(const IPTime& t)
{
    _sec += t._sec;
    _msec += t._msec;
    while (_msec >= 1000) {
        _sec += 1;
        _msec -= 1000;
    }

    return *this;
}

/* Operator to subtract two times.  Modifies its own data and returns a
   reference to it */
IPTime& IPTime::operator-=(const IPTime& t)
{
    _sec -= t._sec;
    _msec -= t._msec;
    while (_msec < 0) {
        _sec -= 1;
        _msec += 1000;
    }

    return *this;
}

/* returns the time held by the instance in seconds */
double IPTime::Value() const
{
    return double(_sec) + double(_msec)/1000.0;
}

/* returns the time held by the instance in integer seconds and milliseconds */
void IPTime::Value(int& sec, int& msec) const
{
    sec = _sec;
    msec = _msec;
}

/* friendly operator to add two times */
IPTime operator+(const IPTime& t1, const IPTime& t2)
{
    int sec = t1._sec + t2._sec;
    int msec = t1._msec + t2._msec;
    while (msec >= 1000) {
        sec += 1;
        msec -= 1000;
    }

    return IPTime(sec, msec);
}

/* friendly operator to subtract two times */    
IPTime operator-(const IPTime& t1, const IPTime& t2)
{
    int sec = t1._sec - t2._sec;
    int msec = t1._msec - t2._msec;
    while (msec < 0) {
        sec -= 1;
        msec += 1000;
    }

    return IPTime(sec, msec);
}
