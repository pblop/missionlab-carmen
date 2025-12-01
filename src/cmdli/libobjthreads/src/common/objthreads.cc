/**********************************************************************
 **                                                                  **
 **  objthreads.cc                                                   **
 **                                                                  **
 **  base which users subclass to get object threads                 **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: objthreads.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: objthreads.cc,v $
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
// Revision 1.15  2004/05/11 19:34:33  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.14  2004/03/25 22:26:30  doug
// added a thread_yield method
//
// Revision 1.13  2003/07/11 22:07:51  doug
// Convert condition.wait to use a TimeOfDay timeout
//
// Revision 1.12  2003/02/19 22:57:57  doug
// change thread_sleep to use TimeOfDay instead of msecs
//
// Revision 1.11  2003/02/06 16:18:06  doug
// added ability to set threads as real time
//
// Revision 1.10  2002/12/31 20:13:20  doug
// added thread_sleep call
//
// Revision 1.9  1998/02/06 14:51:34  doug
// made wait return a bool
//
// Revision 1.8  1998/01/29 11:16:21  doug
// made exceptions compile time option
//
// Revision 1.7  1997/12/10 11:56:56  doug
// *** empty log message ***
//
// Revision 1.6  1997/12/09 15:59:10  doug
// *** empty log message ***
//
// Revision 1.5  1997/11/19 12:59:48  doug
// rtti is back in
//
// Revision 1.4  1997/11/18 19:07:07  doug
// giving up on rtti
//
// Revision 1.3  1997/11/11 15:55:52  doug
// Test case Works
//
// Revision 1.2  1997/11/11 00:27:54  doug
// Getting closer
//
// Revision 1.1  1997/11/06 18:24:26  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include <stdlib.h>
#include <string.h>
#include "objthreads.h"
#include "objthreads_implementation.h"

namespace sara 
{
// *********************************************************************
/// Create a thread when the object is created
objthreads::objthreads(int runparm, 
		const char* name, 
		   const uint stack_size)
{
   theparm = runparm;
   state = TCREATED;
   tname = strdup(name);
   impl = objthreads_implementation::create_thread(this, stack_size);
}

// *********************************************************************
/// Destroy the thread 
objthreads::~objthreads()
{
   if( tname )
   {
      free(tname);
      tname = NULL;
   }
   
   if( impl )
   {
      delete impl; 
      impl = NULL;
   }
}

// *********************************************************************
/// Start the thread running
void
objthreads::start_thread()
{
   protect.lock();
   bool err = state != TCREATED;
   protect.unlock();

   if( err )
   {
#ifdef USE_EXCEPTIONS
      throw already("objthreads::start_thread start_thread called on thread which has already been started");
#else
      FATAL_ERROR("objthreads::start_thread start_thread called on thread which has already been started");
#endif
   }

   if( impl == NULL )
   {
#ifdef USE_EXCEPTIONS
      throw invalid_operation("objthreads::start_thread start_thread called on invalid thread object");
#else
      FATAL_ERROR("objthreads::start_thread start_thread called on invalid thread object");
#endif
   }

   protect.lock();
   state = TRUNNING;
   state_change.broadcast();
   protect.unlock();

   impl->schedule_thread();
}

// *********************************************************************
/// Mark that the thread has completed (called by the thread itself)
void  
objthreads::mark_completed()
{  
   protect.lock();
   state = TCOMPLETED;
   state_change.broadcast();
   protect.unlock();
}  

// *********************************************************************
/// Attempt to make the thread realtime and set its priority
/// Requires superuser permissions
/// The priority ranges from 1 (low) to 99 (high)
/// Returns true on sucess, false if insufficient privilages
bool 
objthreads::setRealtimePriority(const int priority)
{
   return impl->setRealtimePriority(priority);
}
 
// *********************************************************************
/// Wait for the thread to exit or the timeout to occur
bool
objthreads::wait_for_completion(uint msecs)
{
   bool rtn = true;
   TimeOfDay delay((double)msecs / 1000.0);

   // Wait until the completed flag gets set or we timeout
   protect.lock();
   while (state != TCOMPLETED)
   {
      if( !state_change.wait(protect, delay) )
      {
	 rtn = false;
	 break;
      }
   }
   protect.unlock();

   return rtn;
}  

// *********************************************************************
/// Get the name of the thread
const char* 
objthreads::get_name() const
{
   if( tname == NULL )
   {
#ifdef USE_EXCEPTIONS
      throw invalid_object("objthreads::get_name The thread name is empty!");
#else
      FATAL_ERROR("objthreads::get_name The thread name is empty!");
#endif
   }

   return tname;
}

// *********************************************************************
/// suspend the thread for the specified delay
void 
objthreads::thread_sleep( const TimeOfDay &delay ) 
{
   if( impl == NULL )
   {
#ifdef USE_EXCEPTIONS
      throw invalid_operation("objthreads::start_thread start_thread called on invalid thread object");
#else
      FATAL_ERROR("objthreads::start_thread start_thread called on invalid thread object");
#endif
   }

   impl->thread_sleep( delay );

}

// *********************************************************************
/// yield the cpu, but leave us runnable
void 
objthreads::thread_yield() 
{
   if( impl == NULL )
   {
#ifdef USE_EXCEPTIONS
      throw invalid_operation("objthreads::thread_yield called on invalid thread object");
#else
      FATAL_ERROR("objthreads::thread_yield called on invalid thread object");
#endif
   }

   impl->thread_yield();

}

// *********************************************************************
}
