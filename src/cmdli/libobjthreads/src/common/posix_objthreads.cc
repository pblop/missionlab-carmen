/**********************************************************************
 **                                                                  **
 **  posix_objthreads.cc                                             **
 **                                                                  **
 **  posix implementation of the objthreads class                    **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: posix_objthreads.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: posix_objthreads.cc,v $
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
// Revision 1.19  2004/05/11 19:34:33  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.18  2004/04/14 18:06:29  doug
// missed a header
//
// Revision 1.17  2004/03/25 22:26:30  doug
// added a thread_yield method
//
// Revision 1.16  2003/02/28 22:36:11  doug
// renamed TimeOfDay.secs to TimeOfDay.whole_secs
//
// Revision 1.15  2003/02/28 13:56:28  doug
// updated to use new TimeOfDay call
//
// Revision 1.14  2003/02/26 14:56:29  doug
// controller now runs with objcomms
//
// Revision 1.13  2003/02/19 22:57:57  doug
// change thread_sleep to use TimeOfDay instead of msecs
//
// Revision 1.12  2003/02/06 16:18:06  doug
// added ability to set threads as real time
//
// Revision 1.11  2002/12/31 20:13:20  doug
// added thread_sleep call
//
// Revision 1.10  2001/08/15 22:33:04  doug
// had to convert start_thread to static to get to work with new ISO C++
//
// Revision 1.9  1998/02/04 22:17:43  doug
// *** empty log message ***
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
// Revision 1.1  1997/11/06 18:25:10  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include <errno.h>
#include "mic.h"
#include "posix_objthreads.h"

namespace sara 
{
// *********************************************************************
// Note: Don't appear to have anyway to modify the stack size in pthreads
posix_objthreads::posix_objthreads(objthreads *base, const uint)
{
   assert(this);

   // Remember our base record
   ourbase = base;

   // Create the mutex and condition used to wait until we become runnable
   can_run = false;

   mtx = new pthread_mutex_t;
   pthread_mutex_init(mtx, NULL);

   cond  = new pthread_cond_t;
   pthread_cond_init(cond, NULL);

   // Build attribute structure so can create the thread with system wide contention
   pthread_attr_t attr;
   pthread_attr_init(&attr); /* initialize attr with default attributes */
   pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);  
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);  

   // Create the thread using the "start_thread" wrapper.
   // It will run down to the mutex lock and wait to become runnable
   // NOTE: Manually passing "this" pointer, since start_thread is a member fnc
   if( pthread_create(&thread_id, &attr, reinterpret_cast<void*(*)(void*)>(&start_thread), (void *)this) )
   {
#ifdef USE_EXCEPTIONS
      throw no_resources("posix_objthreads::posix_objthreads Max threads already created");
#else
      FATAL_ERROR("posix_objthreads::posix_objthreads Max threads already created");
#endif
   }
}

// *********************************************************************
posix_objthreads::~posix_objthreads()
{
   // So we don't get into trouble later
   ourbase = NULL;

   // Hmm, should make sure the thread has completed and if not, kill it.
}

// *********************************************************************
void *
posix_objthreads::start_thread(posix_objthreads *rec)
{
   // Wait until they make us runnable
   pthread_mutex_lock(rec->mtx);
   while (!rec->can_run)
      pthread_cond_wait(rec->cond, rec->mtx);
   pthread_mutex_unlock(rec->mtx);

   // Since these are not used again delete them to free some resources
   pthread_mutex_destroy(rec->mtx);
   delete rec->mtx;
   rec->mtx = NULL;
   pthread_cond_destroy(rec->cond);
   delete rec->cond;
   rec->cond = NULL;

   // Now run
   if( rec->ourbase )
      rec->ourbase->run(rec->ourbase->theparm);

   if( rec->ourbase )
      rec->ourbase->mark_completed();

   // never happens
	return 0;
}
    
// *********************************************************************
void
posix_objthreads::schedule_thread()
{
   if( mtx == NULL || cond == NULL )
   {
#ifdef USE_EXCEPTIONS
      throw invalid_operation("posix_objthreads::schedule_thread schedule_thread called on invalid object");
#else
      FATAL_ERROR("posix_objthreads::schedule_thread schedule_thread called on invalid object");
#endif
   }

    pthread_mutex_lock(mtx);
    can_run = true;
    pthread_cond_signal(cond);
    pthread_mutex_unlock(mtx);   
}

// *********************************************************************
void
posix_objthreads::exit(int code)
{
    pthread_exit(&code);
}

// *********************************************************************
/// suspend the thread for the specified delay
void 
posix_objthreads::thread_sleep( const TimeOfDay &delay ) 
{
   timespec rec;
   rec.tv_sec = delay.whole_secs();
   rec.tv_nsec = delay.fract_nsecs();
   int rtn;
   do
   { 
      rtn = nanosleep(&rec, &rec);
   } while( rtn == -1 && errno == EINTR );
}

// *********************************************************************
/// yield the cpu, but leave us runnable
void 
posix_objthreads::thread_yield()
{
   // a nanosleep call for 0 seconds seems to be a reliable yield.
   timespec rec;
   rec.tv_sec = 0;
   rec.tv_nsec = 0;
   nanosleep(&rec, &rec);
}

// *********************************************************************
/// Attempt to make the thread realtime and set its priority
/// Requires superuser permissions
/// The priority ranges from 1 (low) to 99 (high)
/// Returns true on sucess, false if insufficient privilages
bool 
posix_objthreads::setRealtimePriority(const int priority)
{
   // Build attribute structure so can create the thread with system wide contention
   sched_param parm;
   parm.sched_priority = bound(priority, 1, 99);
   int rtn = pthread_setschedparam(thread_id, SCHED_FIFO, &parm);  
   if( rtn == EPERM )
   {
      return false;
   }
   else if( rtn != 0 )
   {
      char msg[2048];
      sprintf(msg, "posix_objthreads::setRealtimePriority - unable to set the desired priority. Error=%s", rtn==EINVAL ? "EINVAL" : rtn==ESRCH ? "ESRCH" : rtn==EFAULT ? "EFAULT" : "Unknown");
#ifdef USE_EXCEPTIONS
      throw invalid_operation(msg);
#else
      FATAL_ERROR(msg);
#endif
   }

   return true;
}

// *********************************************************************
}
