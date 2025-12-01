#ifndef POSIX_OBJTHREADS_H
#define POSIX_OBJTHREADS_H
/**********************************************************************
 **                                                                  **
 **  posix_objthreads.h                                              **
 **                                                                  **
 **  implement the objthreads for posix systems using pthreads       **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: posix_objthreads.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: posix_objthreads.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:17  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:55  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:33:51  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.11  2004/05/11 19:34:33  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.10  2004/03/25 22:26:30  doug
// added a thread_yield method
//
// Revision 1.9  2003/02/19 22:57:57  doug
// change thread_sleep to use TimeOfDay instead of msecs
//
// Revision 1.8  2003/02/06 16:18:06  doug
// added ability to set threads as real time
//
// Revision 1.7  2002/12/31 20:13:20  doug
// added thread_sleep call
//
// Revision 1.6  2002/10/03 19:24:17  doug
// moved all objthreads to a common directory
//
// Revision 1.5  2001/08/15 22:33:04  doug
// had to convert start_thread to static to get to work with new ISO C++
//
// Revision 1.4  1997/12/10 11:56:56  doug
// *** empty log message ***
//
// Revision 1.3  1997/11/11 15:55:52  doug
// Test case Works
//
// Revision 1.2  1997/11/11 00:27:54  doug
// Getting closer
//
// Revision 1.1  1997/11/06 18:25:10  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include <pthread.h>
#include "mutex_implementation.h"
#include "objthreads.h"
#include "objthreads_implementation.h"

namespace sara
{
class posix_objthreads : public objthreads_implementation
{
    pthread_t            thread_id;
    pthread_mutex_t     *mtx;
    pthread_cond_t      *cond;
    volatile bool        can_run;

  public:
    // Create the thread (it is blocked until "start_thread" is called)
    posix_objthreads(objthreads *base, const uint stack_size);

    // Start a newly created thread running
    void schedule_thread();

    // Function called by a thread to exit
    void exit(int code);

    // Stop and delete a thread
    ~posix_objthreads();

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
    static void *start_thread(posix_objthreads *THIS);

    // Pointer to our base record.  Used to access the Run function
    objthreads *ourbase;
};

/*********************************************************************/
}
#endif
