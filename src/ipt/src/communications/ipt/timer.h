#ifndef timer_h
#define timer_h

#include <ipt/resource.h>
#include <ipt/time.h>

class IPTimerCallback;

/* class for an event timer. */    
class IPTimer : public IPResource {
public:
    IPTimer(double, IPTimerCallback*, int);
    ~IPTimer();
    IPTimerCallback* Action() const { return _action; }
    int one_shot() const { return _one_shot; }
    double interval() const { return _period; }
    double time_left() const { return _time_left; }
    void clear();

private:
    IPTimer* _next;         /* next in the list of timers */
    IPTimer* _lock_stepped; /* next timer with same period */
    double _period;             /* period of the timer */
    double _time_left;          /* how much time is left to go */
    IPTimerCallback* _action;        /* action when timer elapses */
    IPTime _last_acted;          /* time timer was last fired */
    int _one_shot;

friend class IPTimerTable;
};

#endif
