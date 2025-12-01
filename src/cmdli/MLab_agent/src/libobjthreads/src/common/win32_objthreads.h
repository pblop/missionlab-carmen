#ifndef WIN32_OBJTHREADS_H
#define WIN32_OBJTHREADS_H
/**********************************************************************
 **                                                                  **
 **  win32_objthreads.h                                              **
 **                                                                  **
 **  implement the objthreads for win32 systems                      **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: win32_objthreads.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

/**********************************************************************
* $Log: win32_objthreads.h,v $
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
* Revision 1.7  2004/05/11 19:34:33  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.6  2004/03/26 16:57:18  doug
* switched to using make.target
*
* Revision 1.5  2004/03/25 22:26:30  doug
* added a thread_yield method
*
* Revision 1.4  2004/03/24 17:54:07  doug
* builds under win32
*
* Revision 1.3  1998/02/08 14:03:41  doug
* got win32 to test
*
* Revision 1.2  1998/02/06 20:01:50  doug
* include windows.h
*
* Revision 1.1  1998/01/29 14:09:07  doug
* Initial revision
*
**********************************************************************/

#include <windows.h>
#include "mutex_implementation.h"
#include "objthreads.h"
#include "objthreads_implementation.h"

namespace sara
{
class win32_objthreads : public objthreads_implementation
{
    HANDLE               thread_handle;
    DWORD                thread_id;

  public:
    // Create the thread (it is blocked until "start_thread" is called)
    win32_objthreads(objthreads *base, const uint stack_size);

    // Start a newly created thread running
    void schedule_thread();

    // Function called by a thread to exit
    void exit(int code);

    // Stop and delete a thread
    ~win32_objthreads();

    /// suspend the thread for the specified delay
    void thread_sleep( const TimeOfDay &delay );

    /// yield the cpu, but leave us runnable
    void thread_yield();

    /// Attempt to make the thread realtime and set its priority
    /// Requires superuser permissions
    /// The priority ranges from 1 (low) to 99 (high)
    /// Returns true on sucess, false if insufficient privilages
    bool setRealtimePriority(const int priority);

  private:
    // Internal function to start the thread running
    static void internal_start_thread(win32_objthreads *th);

    // Pointer to our base record.  Used to access the Run function
    objthreads *ourbase;
};

/*********************************************************************/
}
#endif
