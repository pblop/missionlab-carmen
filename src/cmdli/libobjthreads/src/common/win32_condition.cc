/**********************************************************************
 **                                                                  **
 **  win32_condition.cc                                              **
 **                                                                  **
 **  win32 implementatin of condition variables for objthreads       **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: win32_condition.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

/**********************************************************************
* $Log: win32_condition.cc,v $
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
* Revision 1.5  2004/05/11 19:34:33  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.4  2004/03/24 17:54:07  doug
* builds under win32
*
* Revision 1.3  1998/02/06 19:54:38  doug
* renamed NoTimeOut to NoWaitTimeOut
*
* Revision 1.2  1998/02/06 14:47:54  doug
* redid the wait function to return a bool
*
* Revision 1.1  1998/02/05 21:21:59  doug
* Initial revision
*
*
**********************************************************************/

#include "mic.h"
#include "mutex.h"
#include "mutex_implementation.h"
#include "win32_mutex.h"
#include "win32_condition.h"

namespace sara
{
/**********************************************************************/
// Create a condition variable
win32_condition::win32_condition()
{
   event = CreateEvent(NULL,   // default security
		       FALSE,  // auto-reset
		       FALSE,  // non sigaled originally
	               NULL);  // no name.
   waiters = 0;

   if( event == FALSE )
   {
#ifdef USE_EXCEPTIONS
      throw no_resources("win32_condition::win32_condition Unable to create event variable");
#else
      FATAL_ERROR("win32_condition::win32_condition Unable to create event variable");
#endif
   }
}

/**********************************************************************/
// Delete a condition variable
win32_condition::~win32_condition()
{
   CloseHandle(event);
   if( waiters > 0 )
      WARN("Destroying busy condition variable");
};

// *********************************************************************
// Wait for the condition to be signaled or broadcasted
// Returns true 
bool
win32_condition::wait(mutex &mtx)
{
#ifdef USE_RTTI
   win32_mutex *pmutex = dynamic_cast<win32_mutex *>(mtx.impl);
   if (ptmutex == NULL )
   {
#ifdef USE_EXCEPTIONS
      throw invalid_object("win32_condition::wait failed because the mutex variable passed as parameter 1 is invalid");
#else
      FATAL_ERROR("win32_condition::wait failed because the mutex variable passed as parameter 1 is invalid");
#endif
   }
#else
   win32_mutex *pmutex = (win32_mutex *)mtx.impl;
#endif

    // Mark we are waiting.
    waiters++;

    // Wait for the signal.
    DWORD wret;

#ifdef WINNT
    wret = SignalObjectAndWait(pmutex, event, INFINITE, FALSE);
#else
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/// DCM:: This is a race condition, but I don't know what to do about it.
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
    pmutex->unlock();
    wret = WaitForSingleObject(event, INFINITE);
#endif

    pmutex->lock();

    // We are done waiting.
    waiters--;

    if (wret == WAIT_FAILED)
    {
#ifdef USE_EXCEPTIONS
       throw invalid_object("win32_condition::wait pthread_cond_wait failed");
#else
       FATAL_ERROR("win32_condition::wait pthread_cond_wait failed");
#endif
    } 

    // Timeout
    if (wret == WAIT_TIMEOUT)
       return  false;

   return true;
}



// *********************************************************************
// Wait for the condition to be signaled, broadcasted, or timeout
// Returns true on success, false on timeout.
bool
win32_condition::wait(mutex &mtx, const TimeOfDay &delay)
{
#ifdef USE_RTTI
   win32_mutex *pmutex = dynamic_cast<win32_mutex *>(mtx.impl);
   if (ptmutex == NULL )
   {
#ifdef USE_EXCEPTIONS
      throw invalid_object("win32_condition::wait failed because the mutex variable passed as parameter 1 is invalid");
#else
      FATAL_ERROR("win32_condition::wait failed because the mutex variable passed as parameter 1 is invalid");
#endif
   }
#else
   win32_mutex *pmutex = (win32_mutex *)mtx.impl;
#endif

    // Mark we are waiting.
    waiters++;

    // Wait for the signal.
    DWORD wret;

#ifdef WINNT
    wret = SignalObjectAndWait(pmutex, event, delay.msecs(), FALSE);
#else
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/// DCM:: This is a race condition, but I don't know what to do about it.
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
    pmutex->unlock();
    wret = WaitForSingleObject(event, delay.msecs());
#endif

    pmutex->lock();

    // We are done waiting.
    waiters--;

    if (wret == WAIT_FAILED)
    {
#ifdef USE_EXCEPTIONS
       throw invalid_object("win32_condition::wait pthread_cond_wait failed");
#else
       FATAL_ERROR("win32_condition::wait pthread_cond_wait failed");
#endif
    } 

    // Timeout
    if (wret == WAIT_TIMEOUT)
       return  false;

   return true;
}

//--------------------------------------------------------
// Signal one thread waiting on the condition
void
win32_condition::signal()
{
   if( !PulseEvent(event) )
   {
#ifdef USE_EXCEPTIONS
     throw invalid_object("win32_condition::signal PulseEvent failed");
#else
     FATAL_ERROR("win32_condition::signal PulseEvent failed");
#endif
   }
}

//--------------------------------------------------------

void
win32_condition::broadcast()
{
   // Send it n times, where n is the number waiting.
   int count = waiters;
   while(count--)
   {
      if( !PulseEvent(event) )
      {
#ifdef USE_EXCEPTIONS
        throw invalid_object("win32_condition::signal PulseEvent failed");
#else
        FATAL_ERROR("win32_condition::signal PulseEvent failed");
#endif
      }
   }
}

/*********************************************************************/
}
