#ifndef OBJTHREADS_H
#define OBJTHREADS_H
/**********************************************************************
 **                                                                  **
 **  objthreads.h                                                    **
 **                                                                  **
 **  base file which users subclass to get object threads            **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: objthreads.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: objthreads.h,v $
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
// Revision 1.12  2004/05/11 19:34:33  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.11  2004/03/25 22:26:30  doug
// added a thread_yield method
//
// Revision 1.10  2003/02/19 22:57:57  doug
// change thread_sleep to use TimeOfDay instead of msecs
//
// Revision 1.9  2003/02/06 16:18:06  doug
// added ability to set threads as real time
//
// Revision 1.8  2002/12/31 20:13:20  doug
// added thread_sleep call
//
// Revision 1.7  1998/02/06 14:51:34  doug
// made wait return a bool
//
// Revision 1.6  1998/01/29 11:16:21  doug
// made exceptions compile time option
//
// Revision 1.5  1997/12/10 11:56:56  doug
// *** empty log message ***
//
// Revision 1.4  1997/12/09 15:59:10  doug
// *** empty log message ***
//
// Revision 1.3  1997/11/11 15:55:52  doug
// Test case Works
//
// Revision 1.2  1997/11/11 00:27:54  doug
// Getting closer
//
// Revision 1.1  1997/11/06 15:32:38  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include "mutex.h"
#include "condition.h"
#include "TimeOfDay.h"

namespace sara
{
// Use the default stack size
const uint USE_DEFAULT_STACK_SIZE = 0;

typedef enum ThreadState {TCREATED, TRUNNING, TCOMPLETED} ThreadState;     
typedef int  ThreadData;

/**********************************************************************/
class objthreads 
{
public:
   /// Create a thread when the object is created
   /// Need to call "schedule_thread" to release it
   objthreads(ThreadData runparm = 0, const char* name = "Unnamed", const uint stack_size = USE_DEFAULT_STACK_SIZE);

   /// Destroy the thread
   virtual ~objthreads();

   /// Start a newly created thread running
   void start_thread();

   /// Attempt to make the thread realtime and set its priority
   /// Requires superuser permissions
   /// The priority ranges from 1 (low) to 99 (high)
   /// Returns true on sucess, false if insufficient privilages
   bool setRealtimePriority(const int priority);

   /// User objects must implement the "run" function.
   /// This is the function which is called by the scheduler
   virtual void run(ThreadData parm) = 0;

   /// Get the name of the thread
   const char* get_name() const;

   /// Wait for a thread to exit or timeout
   /// Returns true on success, false on timeout.
   bool wait_for_completion(uint msecs = NoWaitTimeOut);

   /// Mark that the thread has completed (called by the thread itself)
   void mark_completed();

   /// suspend the thread for the specified delay
   void thread_sleep( const TimeOfDay &delay );

   /// yield the cpu, but leave us runnable
   void thread_yield();

   mutex protect;	   // Used to access state and state_change variables
   condition state_change; // Used to wait until a thread changes state
   ThreadState state;      // The current state of the thread
   ThreadData theparm;     // The parm that the user set.

private:

   class objthreads_implementation *impl;
   char *tname;	// strdup'd and free'd

   /// Disable copy constructors
   objthreads(const objthreads&);
   objthreads& operator= (const objthreads&);
};


/*********************************************************************/
}
#endif
