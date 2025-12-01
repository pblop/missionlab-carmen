/**********************************************************************
 **                                                                  **
 **                          EventLogging.cc                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 2001 Georgia Tech Research Corporation          **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: EventLogging.cc,v 1.1.1.1 2008/07/14 16:44:13 endo Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <string>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <ctime>

#include "EventLogging.h"

//-----------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------
EventLogging::EventLogging(void)
{
    logfile_ = NULL;
}

//-----------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------
EventLogging::~EventLogging(void)
{
}

//-----------------------------------------------------------------------
// This function returns the current time stamp.
//-----------------------------------------------------------------------
void EventLogging::getTime(int &sec, int &msec)
{
    // Get current time.
    struct timeval tb;

    gettimeofday(&tb, 0);

    // Subtract start time
    int dsec = (int)tb.tv_sec  - startSecs_;
    int dmsec = (int)(tb.tv_usec / 1000) - startMSecs_;

    // Need to borrow.
    if( dmsec < 0 )
    {
	dmsec += 1000;
	dsec --;
    }

    sec = dsec;
    msec = dmsec;
}

//-----------------------------------------------------------------------
// This function logs the current time stamp.
//-----------------------------------------------------------------------
void EventLogging::printTime_(void)
{
    if(logfile_ == NULL) return;

    // Get current time.
    struct timeval tb;
    gettimeofday(&tb,0);

    // Subtract start time
    int dsec = (int)tb.tv_sec - startSecs_;
    int dmsec = (int)(tb.tv_usec / 1000) - startMSecs_;

    // Need to borrow.
    if(dmsec < 0)
    {
	dmsec += 1000;
	dsec --;
    }

    //fprintf(logfile_," %ld.%3.3ld: ", dsec, dmsec);
    fprintf(logfile_," %d.%3.3d: ", dsec, dmsec);
    fflush(logfile_);
}

//-----------------------------------------------------------------------
// This function initializes the logfile and start counting the time.
//-----------------------------------------------------------------------
void EventLogging::initialize(const char *dir)
{
    time_t timep;
    struct timeval tb;
    int i, pid;
    char buf[2048];
    char *currentTime;

    i = 1;
    pid = getpid();

    while(true)
    {
	sprintf(buf,"%s/EventLog.%d",dir,i);
	FILE *tmp = fopen(buf,"r");
	if(tmp == NULL)
	    break;

	fclose(tmp);
	i++;
    }
    logfileName_ = strdup(buf);

    logfile_ = fopen(logfileName_,"w+");

    if(logfile_ == NULL)
    {
	fprintf(stderr,"Unable to create event log file %s.  Logging disabled\n", buf); 
	return;
    }
    fprintf(stderr,"Events are being logged to %s\n", buf);

    time(&timep);
    currentTime = ctime(&timep);
    gettimeofday(&tb,0);
    startSecs_ = tb.tv_sec;
    startMSecs_ = tb.tv_usec / 1000;

    fprintf(logfile_," 0.000: start Session %d on %s", pid, currentTime);
    fprintf(logfile_," 0.000: StartTime \"%ld.%3.3ld\"\n", startSecs_, startMSecs_);
    fflush(logfile_);
}

//-----------------------------------------------------------------------
// This function pauses the event logging.
//-----------------------------------------------------------------------
void EventLogging::pause(void)
{
   if(logfile_ == NULL) return;

   fclose(logfile_);
   logfile_ = NULL;
}

//-----------------------------------------------------------------------
// This function resumes the event logging.
//-----------------------------------------------------------------------
void EventLogging::resume(bool readStartTimeFromLog)
{
    if((logfile_ != NULL) || (logfileName_ == NULL)) return;

    logfile_ = fopen(logfileName_,"r+");

    if(logfile_ == NULL)
    {
	char msg[2048];
	sprintf(msg,"Unable to reopen event log file %s.  Logging disabled\n\n", logfileName_); 
	perror(msg);
	return;
    }

    // read the start time from the log file if desired.
    if(readStartTimeFromLog)
    {
	// Skip to the first double quote which surrounds the start time.
	unsigned long ss;
	unsigned int  sm;
	int cnt = fscanf(logfile_,"%*[^\"]\"%lu.%u",&ss, &sm);

	if(cnt != 2)
	{
	    fprintf(stderr,"Unable to read start time from log file (got %d items).  Logging disabled\n\n",cnt);
	    fclose(logfile_);
	    logfile_ = NULL;
	}
	startSecs_ = ss;
	startMSecs_ = sm;
    }

    // Seek to end of file
    fseek(logfile_, 0, 2);
}

//-----------------------------------------------------------------------
// This function finishes the event logging.
//-----------------------------------------------------------------------
void EventLogging::close(void)
{
    if(logfile_ == NULL) return;

    printTime_();
    fprintf(logfile_,"end Session\n");
    fflush(logfile_);

    fclose(logfile_);
    logfile_ = NULL;
}

//-----------------------------------------------------------------------
// This function logs the begining of an event which has some duration.
// For example, if you want to measure how long a mlab session was
// running, log the starting time with this start() function, and
// ending time with end() below.
//-----------------------------------------------------------------------
void EventLogging::start(const char *fmt, ...)
{
   va_list args;

   if(logfile_ == NULL) return;

   printTime_();
   fprintf(logfile_,"start ");
   fflush(logfile_);

   va_start(args, fmt);
   vfprintf(logfile_, fmt, args);
   va_end(args);

   fprintf(logfile_,"\n");
   fflush(logfile_);
}

//-----------------------------------------------------------------------
// This function logs the ending of an event which has some duration.
// See start() above.
/*-----------------------------------------------------------------------*/
void EventLogging::end(const char *fmt, ...)
{
    va_list args;

    if(logfile_ == NULL) return;

    printTime_();
    fprintf(logfile_,"end ");
    fflush(logfile_);

    va_start(args, fmt);
    vfprintf(logfile_, fmt, args);
    va_end(args);

    fprintf(logfile_,"\n");
    fflush(logfile_);
}

//-----------------------------------------------------------------------
// This function is similar to start() above. It is used to measure
// an event which is related to modification.
//-----------------------------------------------------------------------
void EventLogging::startModify(const char *fmt, ...)
{
   va_list args;

   if(logfile_ == NULL) return;

   printTime_();
   fprintf(logfile_,"StartModify ");
   fflush(logfile_);

   va_start(args, fmt);
   vfprintf(logfile_, fmt, args);
   va_end(args);

   fprintf(logfile_,"\n");
   fflush(logfile_);
}

//-----------------------------------------------------------------------
// This function is similar to end() above. It is used to measure
// an event which is related to modification.
//-----------------------------------------------------------------------
void EventLogging::endModify(const char *fmt, ...)
{
    va_list args;

    if(logfile_ == NULL) return;

    printTime_();
    fprintf(logfile_,"EndModify ");
    fflush(logfile_);

    va_start(args, fmt);
    vfprintf(logfile_, fmt, args);
    va_end(args);

    fprintf(logfile_,"\n");
    fflush(logfile_);
}

//-----------------------------------------------------------------------
// This function logs an event which was generated because the user
// canceled some act.
//-----------------------------------------------------------------------
void EventLogging::cancel(const char *name)
{
    if(logfile_ == NULL) return;

    printTime_();
    fprintf(logfile_,"cancel %s\n", name);
    fflush(logfile_);
}

//-----------------------------------------------------------------------
// This function logs an event.
//-----------------------------------------------------------------------
void EventLogging::log(const char *fmt, ...)
{
   va_list args;

   if(logfile_ == NULL) return;

   printTime_();
   fprintf(logfile_,"event ");
   fflush(logfile_);

   va_start(args, fmt);
   vfprintf(logfile_, fmt, args);
   va_end(args);

   fprintf(logfile_,"\n");
   fflush(logfile_);
}

//-----------------------------------------------------------------------
// This function logs an status.
//-----------------------------------------------------------------------
void EventLogging::status(const char *fmt, ...)
{
   va_list args;

   if(logfile_ == NULL) return;

   printTime_();
   fprintf(logfile_,"status ");
   fflush(logfile_);

   va_start(args, fmt);
   vfprintf(logfile_, fmt, args);
   va_end(args);

   fprintf(logfile_,"\n");
   fflush(logfile_);
}

//-----------------------------------------------------------------------
// This function checks whether the event logging is enabled or not.
//-----------------------------------------------------------------------
bool EventLogging::eventLoggingIsON(void)
{
    return (logfile_ != NULL);
}

//-----------------------------------------------------------------------
// This function installs an X's event handler to a widget, so that
// KeyPress and ButtonPress time can be recorded. It does not work
// if the widget is a slider bar.
//-----------------------------------------------------------------------
void EventLogging::logKBInput(Widget w)
{
    if(logfile_ == NULL) return;

    XtAddRawEventHandler(w, KeyPressMask|ButtonPressMask, true, cbKBInputHandler_, this);
}

//-----------------------------------------------------------------------
// This callback function gets called when there was an KeyPress or
// ButtonPress event if the logKBInput() above installed the event
// handler on this widget.
//-----------------------------------------------------------------------
void EventLogging::cbKBInputHandler_(
    Widget w,
    XtPointer clientData,
    XEvent *event,
    Boolean *continueToDispatch)
{
    EventLogging *eventLoggingInstance = NULL;
    KeySym ks;
    XComposeStatus cs;
    string widgetName;
    char buf[16];
    int buttonNum;

    widgetName = XtName(w);

    eventLoggingInstance = (EventLogging *)clientData;

    switch (event->xany.type) {

    case KeyPress:
	XLookupString(&event->xkey, buf, 16, &ks, &cs);
	eventLoggingInstance->logKeyPress_(buf[0], widgetName);
	break;

    case ButtonPress:
	buttonNum = event->xbutton.button;
	eventLoggingInstance->logButtonPress_(buttonNum, widgetName);
	break;
    }
}

//-----------------------------------------------------------------------
// This function logs KeyPress event with a time stamp.
//-----------------------------------------------------------------------
void EventLogging::logKeyPress_(char key, string widgetName)
{
    if(logfile_ == NULL) return;

    printTime_();
    fprintf(logfile_, "KeyPress: key = %c, widget = %s\n", key, widgetName.c_str());
    fflush(logfile_);
}

//-----------------------------------------------------------------------
// This function logs ButtonPress event with a time stamp.
//-----------------------------------------------------------------------
void EventLogging::logButtonPress_(int buttonNum, string widgetName)
{
    if(logfile_ == NULL) return;

    printTime_();
    fprintf(logfile_, "ButtonPress: button = %d, widget = %s\n", buttonNum, widgetName.c_str());
    fflush(logfile_);
}

//-----------------------------------------------------------------------
// This function returns the name of the logfile.
//-----------------------------------------------------------------------
char *EventLogging::getLogfileName(void)
{
    if (logfileName_ == NULL) return NULL;

    return strdup(logfileName_);
}

///////////////////////////////////////////////////////////////////////
// $Log: EventLogging.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:13  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:53  endo
// MissionLab 7.0
//
// Revision 1.2  2006/05/12 22:51:56  endo
// g++-3.4 upgrade.
//
// Revision 1.1.1.1  2005/02/06 22:59:26  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.7  2002/01/13 01:52:11  endo
// The event logging functions were organized with the object orient style.
//
// Revision 1.6  2000/02/13 20:43:08  sapan
// Added function to get current time
//
// Revision 1.5  1996/03/12 17:46:32  doug
// *** empty log message ***
//
// Revision 1.4  1996/03/08  00:47:29  doug
// *** empty log message ***
//
// Revision 1.3  1996/03/06  20:43:59  doug
// fixed resume
//
// Revision 1.2  1996/03/05  22:56:33  doug
// *** empty log message ***
//
// Revision 1.1  1996/03/05  22:24:18  doug
// Initial revision
//
// Revision 1.1  1996/02/29  23:40:01  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
