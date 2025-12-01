///////////////////////////////////////////////////////////////////////////////
//
//                                 timerable.cc
//
// Implements a timer table.  This is a table of callbacks to be
// called at fixed intervals.  It does not do the waiting, but only keeps
// track of how much time to wait for for the next timer.  This file also
// implements a C++ interface to the Unix time functions
//
// The timer table is implemented as a list of timer buckets, with timers
// of the same intervals going in the same buckets.  The list is ordered in
// increasing order of time left to go before the timers in that bucket are
// fired.  The timer table maintains this order as the timer buckets are
// fired.
//
// Classes defined for export:
//    EventTimer, TimerList, TimerTable
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#ifdef VXWORKS
#include <timers.h>
#else
#include <sys/types.h>
#include <time.h>

#ifdef IRIX5
struct timeb {
        time_t  time;           /* the seconds portion of the current time */
        unsigned short millitm; /* the milliseconds portion of current time*/
        short   timezone;       /* the local timezone in minutes west of GMT */
        short   dstflag;        /* TRUE if Daylight Savings Time in effect */
};
#else
#include <sys/timeb.h>
#endif
extern "C" int ftime(struct timeb*);
#endif

#include <ipt/callbacks.h>
#include <ipt/timertable.h>
#include <ipt/timer.h>

/* Create a timer for the timer table */
IPTimer::IPTimer(double period, IPTimerCallback* cb, int one_shot)
{
    _one_shot = one_shot;
    _time_left = 0.0;
    _next = _lock_stepped = 0;
    _period = period;
    _action = cb;
    IPResource::ref(_action);
    _last_acted = IPTime();
}

IPTimer::~IPTimer()
{
    IPResource::unref(_action);
}

void IPTimer::clear()
{
    IPTimer* current = _lock_stepped;
    IPTimer* dead;
    while (current) {
        dead = current;
        current = current->_lock_stepped;
        delete dead;
    }
    if (_next)
        _next->clear();

    delete this;   
}

/* Create an empty timer table */
IPTimerTable::IPTimerTable()
{
    _timers = 0;
    _last_access = IPTime();
    _executing = NULL;
}

/* delete a timer table */
IPTimerTable::~IPTimerTable()
{
    if (_timers)
        _timers->clear();
}

/* Add a timer to the timer table */
void IPTimerTable::Add(IPTimer* timer)
{
    double period = timer->_period;
    if (!timer->one_shot()) {
        for (IPTimer* current = _timers; current; current = current->_next) 
            if (current->_period == period) {
                timer->_lock_stepped = current->_lock_stepped;
                current->_lock_stepped = timer;
                return;
            }
    }

    timer->_time_left = period;
    timer->_last_acted = IPTime();
    insert(timer);
}

/* Find out how much time till the next timer. -1.0 means no timers will ever
   fire, 0.0 means that timers are ready to fire now */
double IPTimerTable::TimeLeft()
{
    if (!_timers)
        return -1.0;
    
    if (!_last_access.Set()) {
        _last_access = IPTime::Current();
        return 0.0;
    }

    IPTime now = IPTime::Current();
    double elapsed = (now - _last_access).Value();
    _last_access = now;

    for (IPTimer* current = _timers; current; current = current->_next) 
        if (current->_time_left) {
            current->_time_left -= elapsed;
            if (current->_time_left < 0.0)
                current->_time_left = 0.0;
        }

    return _timers->_time_left;
}

/* Insert a timer "timer" into the timer table at the appropriate spot */
void IPTimerTable::insert(IPTimer* timer)
{
    IPTimer* prev = 0;
    for (IPTimer* current = _timers; current; current = current->_next) {
        if (timer->_time_left <= current->_time_left) {
            if (prev)
                prev->_next = timer;
            else
                _timers = timer;
            timer->_next = current;
            return;
        }
        prev = current;
    }

    if (prev)
        prev->_next = timer;
    else
        _timers = timer;
    timer->_next = 0;

    return;
}

/* Make "t" the list of timers that have been fired, and reorder the 
   timer table list to reflect that these timers now have to wait for their
   intervals to fire again */
void IPTimerTable::FireTimers()
{
    if (_executing || TimeLeft() != 0.0)
        return;
    
    IPTimer* current = _timers;
    while (current) {
        if (current->_time_left > 0.0)
            return;
        else {
            IPTime now = IPTime::Current();
            if (current->_last_acted.Set()) {
                current->_time_left = 2.0*current->_period - 
                    (now - current->_last_acted).Value();
                if (current->_time_left < 0.0)
                    current->_time_left = 0.0;
                if (current->_time_left > current->_period)
                    current->_time_left = current->_period;
            } else 
                current->_time_left = current->_period;
            _last_access = now;
            current->_last_acted = now;
            _timers = current->_next;
            IPTimer* locked;
            IPTimer* prev = NULL;
            IPTimer* next;
            for (locked = current; locked; ) {
                if (locked->Action()) {
                    _executing = locked;
                    locked->Action()->Execute(locked);
                    if (!_executing || locked->one_shot()) {
                        if (!prev) {
                            next = locked->_lock_stepped;
                            if (next) {
                                next->_time_left = locked->_time_left;
                                next->_next = locked->_next;
                            }
                            if (current == _timers) {
                                _timers = next;
                                if (!_timers)
                                    _timers = current->_next;
                            }
                            current = next;
                        } else {
                            prev->_lock_stepped = locked->_lock_stepped;
                        }

                        next = locked->_lock_stepped;
                        IPResource::unref(locked);
                        locked = next;
                    } else {
                        prev = locked;
                        locked = locked->_lock_stepped;
                    }
                    _executing = NULL;
                }
            }

            if (current) 
                insert(current);
            current = _timers;
        }
    }
}

/* Remove a timer from the timer table.  Make sure that we do the proper
   administrative things if there are other timers in the interval bucket */
void IPTimerTable::Remove(IPTimer* timer)
{
    if (timer == _executing) {
        _executing = NULL;
        return;
    }
    
    IPTimer* current = _timers;
    IPTimer* prev = 0;
    for (;current;current = current->_next) {
        if (timer == current) {
            if (timer->_lock_stepped) {
                IPTimer* next = timer->_lock_stepped;
                next->_time_left = timer->_time_left;
                next->_next = timer->_next;
                if (prev)
                    prev->_next = next;
                else
                    _timers = next;
                timer->_lock_stepped = 0;
                return;
            } else {
                if (prev)
                    prev->_next = current->_next;
                else
                    _timers = current->_next;
                return;
            }
        }

        IPTimer* locked = current->_lock_stepped;
        IPTimer* prev_locked = current;
        for (;locked; locked=locked->_lock_stepped) {
            if (timer == locked) {
                prev_locked->_lock_stepped = locked->_lock_stepped;
                timer->_next = 0;
                timer->_lock_stepped = 0;
                return;
            }
            prev_locked = locked;
        }
                
        prev = current;
    }
}

