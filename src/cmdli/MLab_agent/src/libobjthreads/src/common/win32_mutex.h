#ifndef WIN32_MUTEX_H
#define WIN32_MUTEX_H
/**********************************************************************
 **                                                                  **
 **  win32_mutex.h                                                   **
 **                                                                  **
 **  win32 implementation of mutex variables                         **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: win32_mutex.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

/**********************************************************************
* $Log: win32_mutex.h,v $
* Revision 1.1.1.1  2008/07/14 16:44:17  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:55  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:33:51  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.4  2004/05/11 19:34:33  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.3  2004/03/26 16:57:18  doug
* switched to using make.target
*
* Revision 1.2  1998/02/06 14:39:41  doug
* win32 fixes
*
* Revision 1.1  1998/02/05 21:02:37  doug
* Initial revision
*
*
**********************************************************************/

#include "mutex_implementation.h"
#include <windows.h>

namespace sara
{
// Each mutex has the number of threads waiting and an Event.
// Waiting threads wait on the Event.
// Signal and broadcast PulseEvent the Event.
// The number of waiting threads is a hint used in broadcast() to know
// how many times to Pulse the Event

class win32_mutex : public mutex_implementation
{
   public:
      // Create a win32 mutex
      win32_mutex();

      // Delete a win32 mutex
      ~win32_mutex();
   
      // lock a mutex
      void lock();

      // unlock a mutex
      void unlock();
   
   private:
      friend class win32_condition;	// Needs to get at pt_mutex

      HANDLE          the_mutex;	// The win32 mutex
};


/*********************************************************************/
}
#endif
