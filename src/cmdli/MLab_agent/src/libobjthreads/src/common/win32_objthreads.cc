/**********************************************************************
 **                                                                  **
 **  win32_objthreads.cc                                             **
 **                                                                  **
 **  win32 implementation of the objthreads class                    **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: win32_objthreads.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

/**********************************************************************
* $Log: win32_objthreads.cc,v $
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
* Revision 1.6  2004/05/11 19:34:33  doug
* massive changes to integrate with USC and GaTech
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
* finished coding.
*
* Revision 1.1  1998/01/29 14:09:07  doug
* Initial revision
*
*
**********************************************************************/

#include "mic.h"
//#include "Windows.h"
#include "win32_objthreads.h"
namespace sara
{

// *********************************************************************
win32_objthreads::win32_objthreads(objthreads *base, const uint stack_size)
{
   // Remember our base record
   ourbase = base;

   // Create the thread suspended using the "internal_start_thread" wrapper.
   // It will block until set runnable.
   // NOTE: Manually passing the this pointer, since internal_start_thread 
   //       is a memberfnc
   thread_handle = CreateThread(
            NULL, // default security
            stack_size,
            (LPTHREAD_START_ROUTINE)internal_start_thread,
            (LPVOID)this,
            CREATE_SUSPENDED,  
            &thread_id);

   // If didn't get the thread, give up.
   if( thread_handle == 0 )
   {
#ifdef USE_EXCEPTIONS
      throw no_resources("win32_objthreads::win32_objthreads Max threads already created");
#else
      FATAL_ERROR("win32_objthreads::win32_objthreads Max threads already created");
#endif
   }
}

// *********************************************************************
win32_objthreads::~win32_objthreads()
{
   /// Free the thread data.
   CloseHandle(thread_handle);

   // So we don't get into trouble later
   ourbase = NULL;

   // Hmm, should make sure the thread has completed and if not, kill it.
}

// *********************************************************************
void 
win32_objthreads::internal_start_thread(win32_objthreads *th)
{
   // Now run
   if( th->ourbase )
      th->ourbase->run(th->ourbase->theparm);

   if( th->ourbase )
      th->ourbase->mark_completed();
}
    
// *********************************************************************
void
win32_objthreads::schedule_thread()
{
   if( ResumeThread(thread_handle) == (DWORD)-1 )
   {
#ifdef USE_EXCEPTIONS
      throw invalid_operation("win32_objthreads::schedule_thread ResumeThread failed");
#else
      FATAL_ERROR("win32_objthreads::schedule_thread ResumeThread failed");
#endif
   }
}

// *********************************************************************
/// Called by the thread to exit with a return code.
void
win32_objthreads::exit(int code)
{
    ExitThread(code);
}

// *********************************************************************
/// suspend the thread for the specified delay
void 
win32_objthreads::thread_sleep( const TimeOfDay &delay ) 
{
   Sleep( delay.msecs() );
}

// *********************************************************************
/// yield the cpu, but leave us runnable
void 
win32_objthreads::thread_yield()
{
   // a sleep call for 0 seconds seems to be a reliable yield.
   Sleep(0);
}

// *********************************************************************
/// Attempt to make the thread realtime and set its priority
/// Requires superuser permissions
/// The priority ranges from 1 (low) to 99 (high)
/// Returns true on sucess, false if insufficient privilages
bool 
win32_objthreads::setRealtimePriority(const int priority)
{
   WARN("win32_objthreads::setRealtimePriority - Not Implemented!");
   return false;
}

// *********************************************************************
}
