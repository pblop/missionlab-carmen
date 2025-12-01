///////////////////////////////////////////////////////////////////////////////
//
//                                 timertable.h
//
// Declares the timer table class, which is a table of interval/action
// pairs which keeps track of when to evaluate the next timer action.
//
// Classes declared for export:
//    Time, EventTimer, TimerList, TimerTable
//
// Dec 20, 1993 - Jay Gowdy
//     Documented version of SAUSAGES Link Manager v1.0 completed
//    
///////////////////////////////////////////////////////////////////////////////

#ifndef timertable_h
#define timertable_h

#include <ipt/time.h>

class IPTimer;

/* Table of interval/action pairs called timers.  Timers with the same
   interval are "lock stepped" for efficiency */
class IPTimerTable {
public:
    IPTimerTable();
    ~IPTimerTable();

    void Add(IPTimer*);
    double TimeLeft();

    void FireTimers();
    void Remove(IPTimer*);

private:
    void insert(IPTimer*);

private:
    IPTimer* _timers;  // list of timers
    IPTimer* _executing;  // current executing timer 
    IPTime _last_access;    // time of last access to the timer table
};

#endif
