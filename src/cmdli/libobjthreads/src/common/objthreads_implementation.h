#ifndef OBJTHREADS_IMPLEMENTATION_H
#define OBJTHREADS_IMPLEMENTATION_H
/**********************************************************************
 **                                                                  **
 **  objthreads_implementation.h                                     **
 **                                                                  **
 **  abstract base class for objthreads implementations              **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: objthreads_implementation.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: objthreads_implementation.h,v $
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
// Revision 1.8  2004/05/11 19:34:33  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.7  2004/03/25 22:26:30  doug
// added a thread_yield method
//
// Revision 1.6  2003/02/19 22:57:57  doug
// change thread_sleep to use TimeOfDay instead of msecs
//
// Revision 1.5  2003/02/06 16:18:06  doug
// added ability to set threads as real time
//
// Revision 1.4  2002/12/31 20:13:20  doug
// added thread_sleep call
//
// Revision 1.3  1997/11/11 15:55:52  doug
// Test case Works
//
// Revision 1.2  1997/11/11 00:27:54  doug
// Getting closer
//
// Revision 1.1  1997/11/06 17:03:22  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include "objthreads.h"

namespace sara
{
class objthreads_implementation 
{
   public:
    // Create the thread (it is blocked until "start_thread" is called)
    static objthreads_implementation *create_thread(class objthreads *root, 
		const uint stack_size);


    // No extra work to do here
    virtual ~objthreads_implementation() {};

    // Start the thread running
    virtual void schedule_thread() = 0;

    /// suspend the thread for the specified delay
    virtual void thread_sleep( const TimeOfDay &delay ) = 0;

   /// Attempt to make the thread realtime and set its priority
   /// Requires superuser permissions
   /// The priority ranges from 1 (low) to 99 (high)
   /// Returns true on sucess, false if insufficient privilages
   virtual bool setRealtimePriority(const int priority) = 0;

   /// yield the cpu, but leave us runnable
   virtual void thread_yield() = 0;

   private:
    friend class posix_objthreads;
    friend class win32_objthreads;
    friend class djgpp_objthreads;
		   
    // Only allow the friends access to the constructor
    objthreads_implementation() {};
};

/*********************************************************************/
}
#endif
