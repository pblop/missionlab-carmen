/**********************************************************************
 **                                                                  **
 **                          EventLogging.h                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 2001 Georgia Tech Research Corporation          **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: EventLogging.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */


#ifndef EVENTLOGGING_H
#define EVENTLOGGING_H

#include <string>

using std::string;

class EventLogging {

protected:
    FILE *logfile_;
    char *logfileName_;
    long startSecs_;
    long startMSecs_;

    void printTime_(void);
    void logKeyPress_(char key, string widgetName);
    void logButtonPress_(int buttonNum, string widgetName);
 
    static void cbKBInputHandler_(
	Widget w,
	XtPointer clientData,
	XEvent *event,
	Boolean *continueToDispatch);

public:

    EventLogging(void);
    ~EventLogging(void);
    void initialize(const char *dir);
    void pause();
    void resume(bool readStartTimeFromLog);
    void close(void);
    void start(const char *fmt, ...);
    void startModify(const char *fmt, ...);
    void endModify(const char *fmt, ...);
    void cancel(const char *name);
    void end(const char *fmt, ...);
    void log(const char *fmt, ...);
    void status(const char *fmt, ...);
    bool eventLoggingIsON(void);
    void getTime(int &sec, int &msec);
    char *getLogfileName(void);
    void setLogfileName(const char *logfileName);
    void logKBInput(Widget w);
};

inline void EventLogging::setLogfileName(const char *logfileName) {logfileName_ = strdup(logfileName);}

extern EventLogging *gEventLogging;


#endif


/**********************************************************************
 * $Log: EventLogging.h,v $
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
 * Revision 1.8  2003/04/06 13:48:59  endo
 * *** empty log message ***
 *
 * Revision 1.7  2003/04/06 11:39:04  endo
 * gcc 3.1.1
 *
 * Revision 1.6  2002/01/13 01:52:11  endo
 * The event logging functions were organized with the object orient style.
 *
 * Revision 1.5  2000/02/13 20:45:35  sapan
 * Declaration of GetTime function
 *
 * Revision 1.4  1996/03/12 17:46:32  doug
 * *** empty log message ***
 *
 * Revision 1.3  1996/03/08  00:47:29  doug
 * *** empty log message ***
 *
 * Revision 1.2  1996/03/06  20:43:59  doug
 * fixed resume
 *
 * Revision 1.1  1996/03/05  22:24:39  doug
 * Initial revision
 *
 * Revision 1.1  1996/02/29  23:40:01  doug
 * Initial revision
 *
 **********************************************************************/
