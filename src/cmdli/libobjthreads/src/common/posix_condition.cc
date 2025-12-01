/**********************************************************************
 **                                                                  **
 **  posix_condition.cc                                              **
 **                                                                  **
 **  posix implementatin of condition variables for objthreads       **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: posix_condition.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: posix_condition.cc,v $
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
// Revision 1.12  2004/11/06 01:11:55  doug
// snap
//
// Revision 1.11  2004/05/11 19:34:33  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.10  2003/07/11 22:07:51  doug
// Convert condition.wait to use a TimeOfDay timeout
//
// Revision 1.9  2002/10/03 19:24:17  doug
// moved all objthreads to a common directory
//
// Revision 1.8  2000/01/12 21:35:58  doug
// /* DCM 1-12-00: the function call is no longer defined in RH 6.0
//
// Revision 1.7  1998/02/06 14:47:54  doug
// redid the wait function to return a bool
//
// Revision 1.6  1998/01/29 11:09:59  doug
// made exceptions compile option
//
// Revision 1.5  1997/12/10 11:56:43  doug
// *** empty log message ***
//
// Revision 1.4  1997/12/09 15:59:03  doug
// *** empty log message ***
//
// Revision 1.3  1997/11/19 12:58:29  doug
// rtti is back in
//
// Revision 1.2  1997/11/10 18:31:14  doug
// Builds!
//
// Revision 1.1  1997/11/07 18:09:56  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "mic_exception.h"
#include "mutex.h"
#include "mutex_implementation.h"
#include "posix_mutex.h"
#include "posix_condition.h"

namespace sara 
{
/**********************************************************************/
// Create a condition variable
posix_condition::posix_condition()
{
   if( pthread_cond_init(&cnd, NULL) )
   {
#ifdef USE_EXCEPTIONS
      throw no_resources("posix_condition::posix_condition Unable to create pthread condition variable");
#else
      FATAL_ERROR("posix_condition::posix_condition Unable to create pthread condition variable");
#endif
   }

}

/**********************************************************************/
// Delete a condition variable
posix_condition::~posix_condition()
{
   if( pthread_cond_destroy(&cnd) )
      WARN("Destroying busy condition variable");
};

// *********************************************************************
// Wait for the condition to be signaled, broadcasted, or timeout
// Returns true on success, false on timeout.
bool
posix_condition::wait(mutex &mtx, const TimeOfDay &delay)
{
   int rtn;
 
#ifdef USE_RTTI
   posix_mutex *ptmutex = dynamic_cast<posix_mutex *>(mtx.impl);
   if (ptmutex == NULL )
   {
#ifdef USE_EXCEPTIONS
      throw invalid_object("posix_condition::wait failed because the mutex variable passed as parameter 1 is invalid");
#else
      FATAL_ERROR("posix_condition::wait failed because the mutex variable passed as parameter 1 is invalid");
#endif
   }
#else
   posix_mutex *ptmutex = (posix_mutex *)mtx.impl;
#endif

   // The timed wait requires an absolute time specification
   TimeOfDay endTime = TimeOfDay::now() + delay;

   // Now add in the amount of time to sleep.
   timespec abstime;
   abstime.tv_sec = endTime.whole_secs();
   abstime.tv_nsec = endTime.fract_nsecs();
   rtn = pthread_cond_timedwait(&cnd, &ptmutex->pt_mutex, &abstime);
   if (rtn == EINTR) 
   {
      // Wait was interupted by a signal
      // Let's try returning timeout for now.
      return false;
   }
   if (rtn == ETIMEDOUT) 
   {
      return false;
   }
   else if( rtn )
   {
#ifdef USE_EXCEPTIONS
      throw invalid_object("posix_condition::wait pthread_cond_timedwait failed");
#else
      FATAL_ERROR("posix_condition::wait pthread_cond_timedwait failed");
#endif
   }

   return true;
}

// *********************************************************************
// Wait for the condition to be signaled or broadcasted
// Returns true 
bool
posix_condition::wait(mutex &mtx)
{
#ifdef USE_RTTI
   posix_mutex *ptmutex = dynamic_cast<posix_mutex *>(mtx.impl);
   if (ptmutex == NULL )
   {
#ifdef USE_EXCEPTIONS
      throw invalid_object("posix_condition::wait failed because the mutex variable passed as parameter 1 is invalid");
#else
      FATAL_ERROR("posix_condition::wait failed because the mutex variable passed as parameter 1 is invalid");
#endif
   }
#else
   posix_mutex *ptmutex = (posix_mutex *)mtx.impl;
#endif

   if( pthread_cond_wait(&cnd, &ptmutex->pt_mutex) )
   {
#ifdef USE_EXCEPTIONS
      throw invalid_object("posix_condition::wait pthread_cond_wait failed");
#else
      FATAL_ERROR("posix_condition::wait pthread_cond_wait failed");
#endif
   } 

   return true;
}


//--------------------------------------------------------
// Signal one thread waiting on the condition
void
posix_condition::signal()
{
   if( pthread_cond_signal(&cnd) )
   {
#ifdef USE_EXCEPTIONS
     throw invalid_object("posix_condition::signal pthread_cond_signal failed");
#else
     FATAL_ERROR("posix_condition::signal pthread_cond_signal failed");
#endif
   }
}

//--------------------------------------------------------

void
posix_condition::broadcast()
{
   if( pthread_cond_broadcast(&cnd) )
   {
#ifdef USE_EXCEPTIONS
      throw invalid_object("posix_condition::broadcast pthread_cond_broadcast failed");
#else
     FATAL_ERROR("posix_condition::broadcast pthread_cond_broadcast failed");
#endif
   }
}

/*********************************************************************/
}
