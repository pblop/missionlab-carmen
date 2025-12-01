#ifndef DEFS_H
#define DEFS_H
/**********************************************************************
 **                                                                  **
 **                            defs.h                                **
 **                                                                  **
 **  Definition include file for the RC parser.                      **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: defs.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include <stdio.h>
#include "event_list.hpp"

void el_SyntaxError(const char *Message);
void semantic_error(const char *fmt, ...);

extern FILE *el_in;
extern int el_debug;
extern int yylineno;

int el_lex(void);
int el_parse(void);

extern bool had_error;
extern double last_time;

//*********************************************************************
// The event data structures

class events {
    int             event_class_;
    char           *name;
    char           *message_;
public:
    bool		   canceled;
    double          start;
    double          end;
    event_list<events *>  sub_events;

    char *event_name() {

        return name;
    }

    char *message() {

        return message_;
    }

    int event_class() {

        return event_class_;
    }

    events *check_for_su_match(int event_class, char *event_name) {

        void   *cur;
        events *p;
        if( (cur = sub_events.first(&p)) )
        {
            do
            {
                if( p->event_class_ == event_class &&
                    strcmp(p->name, event_name) == 0 )
                {
                    return p;
                }   
            } while( (cur = sub_events.next(&p,cur)) );
        }
        return NULL;
    }

    events(double estart, int eclass, char *ename) {

        event_class_ = eclass;
        name = strdup(ename);
        start = estart;
        end = estart;
        message_ = NULL;
        canceled = false;
    }

    events(double estart, int eclass, char *ename, char *msg) {

        event_class_ = eclass;
        name = strdup(ename);
        start = estart;
        end = estart;
        message_ = strdup(msg);
        canceled = false;
    }

    void add_subevent(events *sub) {

        sub_events.append(sub);
    }

    bool is_start_for(const char *evnt) const {

        return( strcmp(evnt, name) == 0 );
    }

    double duration() const {

        return end - start;
    }

    void cancel(double eend) {

        end = eend;
        canceled = true;
    }

    void finish(double eend) {

        end = eend;
    }

    void finish(double eend, event_list<events *>  subs) {

        end = eend;
        sub_events = subs;
    }

    void cancel(double eend, event_list<events *>  subs) {

        end = eend;
        sub_events = subs;
        canceled = true;
    }

    /*
    virtual bool is_start_for(const char *evnt) const {

        return( strcmp(evnt, name) == 0 );
    }

    virtual double duration() const {

        return end - start;
    }

    virtual void cancel(double eend) {

        end = eend;
        canceled = true;
    }

    virtual void finish(double eend) {

        end = eend;
    }

    virtual void finish(double eend, event_list<events *>  subs) {

        end = eend;
        sub_events = subs;
    }

    virtual void cancel(double eend, event_list<events *>  subs) {

        end = eend;
        sub_events = subs;
        canceled = true;
    }
    */
};

// Stack of events that haven't closed yet
extern event_list<events *> open_stack;

// List of events in the time line.
extern event_list<events *> time_line;

/**********************************************************************
 * $Log: defs.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/05/12 22:51:56  endo
 * g++-3.4 upgrade.
 *
 * Revision 1.1.1.1  2005/02/06 22:59:27  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.4  2003/04/06 11:39:04  endo
 * gcc 3.1.1
 *
 * Revision 1.3  1996/09/03  16:06:13  doug
 * fixed for linux
 *
 * Revision 1.2  1996/04/13  21:54:42  doug
 * *** empty log message ***
 *
 * Revision 1.1  1996/03/05  22:24:39  doug
 * Initial revision
 *
 **********************************************************************/
#endif
