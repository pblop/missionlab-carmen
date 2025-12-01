/**********************************************************************
 **                                                                  **
 **  error_handlers.cc                                               **
 **                                                                  **
 **  report errors to the user                                       **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: error_handlers.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: error_handlers.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:16  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:55  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:30:29  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.2  2006/06/29 21:15:45  endo
// Compiler warning fix.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.20  2004/11/12 22:26:05  doug
// turn off tee by default
//
// Revision 1.19  2004/11/12 21:47:24  doug
// fixed error handlers so can't overflow buffer
//
// Revision 1.18  2004/11/07 03:02:35  doug
// updated budmon
//
// Revision 1.17  2004/05/11 19:34:29  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.16  2004/03/23 19:47:49  doug
// working on win32 build
//
// Revision 1.15  2004/03/08 14:52:13  doug
// cross compiles on visual C++
//
// Revision 1.14  2002/10/23 20:22:07  doug
// fix deprecated code
//
// Revision 1.13  2002/10/23 20:09:32  doug
// fix deprecated code
//
// Revision 1.12  2002/10/23 20:00:53  doug
// remove deprecated warnings
//
// Revision 1.11  1998/11/01 03:30:01  doug
// Added logmsg call for WIN32 not using windows to match
// the voice_io stuff.
//
// Revision 1.10  1998/06/18 20:08:26  doug
// *** empty log message ***
//
// Revision 1.9  1998/02/16 11:24:12  doug
// added option for window based or not
//
// Revision 1.8  1998/02/13 17:02:09  doug
// fixed error handler sprintf's to append
//
// Revision 1.7  1998/02/13 14:27:51  doug
// WIN32 changes
//
// Revision 1.6  1997/12/11 11:00:15  doug
// flush the output stream in PRINT so shows up immediatly
//
// Revision 1.5  1997/12/11 10:54:27  doug
// Added PRINT
//
// Revision 1.4  1997/12/10 11:58:58  doug
// *** empty log message ***
//
// Revision 1.3  1997/12/09 15:58:48  doug
// *** empty log message ***
//
// Revision 1.2  1997/11/25 10:44:30  doug
// Added ERROR and ERROR_with_perror
//
// Revision 1.1  1997/11/13 11:45:23  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include "mic.h"

#ifdef WIN32
#define vsnprintf _vsnprintf
#define snprintf _snprintf
#endif

namespace sara
{
bool use_window_error_handlers = false;
FILE *logging_file = NULL;
static const int BUFSIZE = 4096;
bool logging_to_file_and_stderr = false;

/**********************************************************************/
/// Print the buffer using the appropriate mechanism.
static void
PRINT_BUF(const char *buf)
{
   if( use_window_error_handlers )
   {
//#ifdef WIN32
//      MessageBox(NULL,buf,NULL,MB_OK|MB_ICONERROR);
//#else 
      fprintf(stderr,buf);
//#endif
   }
   else
   {
      if( logging_file )
      {
         fprintf(logging_file, buf);
         fflush(logging_file);

	 if( logging_to_file_and_stderr )
            fprintf(stderr,buf);
      }
      else
      {
         fprintf(stderr,buf);
      }
   }
}

/**********************************************************************/
// Print the characters passed, without a leader or a carriage return.
void
PRINT(const char *fmt, ...)
{
   char buf[BUFSIZE];

   va_list args;
   va_start(args, fmt);
   vsnprintf(buf, BUFSIZE-16, fmt, args);
   va_end(args);

   PRINT_BUF(buf);
}

/**********************************************************************/
// Warn of an unusual event
void
WARN(const char *fmt, ...)
{
   char buf[BUFSIZE];

   va_list args;
   va_start(args, fmt);
   sprintf(buf,"WARNING: ");
   vsnprintf(&buf[strlen(buf)], BUFSIZE-16, fmt, args);
   va_end(args);

   PRINT_BUF(buf);
   PRINT_BUF("\n");
}

/**********************************************************************/
// Report an error
void
ERROR(const char *fmt, ...)
{
   char buf[BUFSIZE];

   va_list args;
   va_start(args, fmt);
   sprintf(buf,"ERROR: ");
   vsnprintf(&buf[strlen(buf)], BUFSIZE-16, fmt, args);
   va_end(args);

   PRINT_BUF(buf);
   PRINT_BUF("\n");
}

/**********************************************************************/
// Inform of an event
void
INFORM(const char *fmt, ...)
{
   char buf[BUFSIZE];

   va_list args;
   va_start(args, fmt);
   sprintf(buf,"Note: ");
   vsnprintf(&buf[strlen(buf)], BUFSIZE-16, fmt, args);
   va_end(args);

   PRINT_BUF(buf);
   PRINT_BUF("\n");
}

/**********************************************************************/
// Warn of an unusual event and list the system error message
void
WARN_with_perror(const char *fmt, ...)
{
   char buf[BUFSIZE];

   va_list args;
   va_start(args, fmt);
   sprintf(buf,"WARNING: ");
   vsnprintf(&buf[strlen(buf)], BUFSIZE-16, fmt, args);
   va_end(args);
   PRINT_BUF(buf);
   PRINT_BUF("\n");

   snprintf(buf, BUFSIZE-16, "System Error Message: %s\n", strerror(errno));
   PRINT_BUF(buf);
}

/**********************************************************************/
// Report an error and list the system error message
void
ERROR_with_perror(const char *fmt, ...)
{
   char buf[BUFSIZE];

   va_list args;
   va_start(args, fmt);
   sprintf(buf,"WARNING: ");
   vsnprintf(&buf[strlen(buf)], BUFSIZE-16, fmt, args);
   va_end(args);
   PRINT_BUF(buf);
   PRINT_BUF("\n");

   snprintf(buf, BUFSIZE-16, "System Error Message: %s\n", strerror(errno));
   PRINT_BUF(buf);
}

/**********************************************************************/
// Print the message and die
void
FATAL_ERROR(const char *fmt, ...)
{
   char buf[BUFSIZE];

   va_list args;
   va_start(args, fmt);
   sprintf(buf,"FATAL ERROR: ");
   vsnprintf(&buf[strlen(buf)], BUFSIZE-16, fmt, args);
   va_end(args);

   PRINT_BUF(buf);
   PRINT_BUF("\n");

   my_abort();
}


/**********************************************************************/
// Print the message and die
void
FATAL_ERROR_with_perror(const char *fmt, ...)
{
   char buf[BUFSIZE];

   va_list args;
   va_start(args, fmt);
   sprintf(buf,"FATAL ERROR: ");
   vsnprintf(&buf[strlen(buf)], BUFSIZE-16, fmt, args);
   va_end(args);
   PRINT_BUF(buf);
   PRINT_BUF("\n");

   snprintf(buf, BUFSIZE-16, "System Error Message: %s\n", strerror(errno));
   PRINT_BUF(buf);

   my_abort();
}


/**********************************************************************/
// Try to drop into the debugger
void
my_abort()
{
    //*((char *)NULL);
    abort();
    exit(1);
}

}
