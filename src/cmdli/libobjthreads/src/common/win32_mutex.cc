/**********************************************************************
 **                                                                  **
 **  win32_mutex.cc                                                  **
 **                                                                  **
 **  win32 implementatin of mutex variables for objthreads           **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: win32_mutex.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

/**********************************************************************
* $Log: win32_mutex.cc,v $
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
* Revision 1.3  1998/02/08 13:54:28  doug
* get win32 version to test
*
* Revision 1.2  1998/02/06 14:39:41  doug
* win32 fixes
*
* Revision 1.1  1998/02/05 21:02:37  doug
* Initial revision
*
*
**********************************************************************/

#include "mic.h"
#include "win32_mutex.h"
namespace sara
{

/*********************************************************************/
win32_mutex::win32_mutex()
{
   the_mutex = CreateMutex(NULL,   // default security
                           FALSE,  // not owned
                           NULL);  // No name

   if( the_mutex == 0 )
   {
#ifdef USE_EXCEPTIONS
      throw no_resources("Unable to create win32 mutex variable");
#else
      FATAL_ERROR("win32_mutex::win32_mutex - Unable to create win32 mutex variable");
#endif
   }
}

/*********************************************************************/
win32_mutex::~win32_mutex()
{
   CloseHandle(the_mutex);
}


//--------------------------------------------------------
/// Currently succeeds if the mutex is already locked by the caller????
void
win32_mutex::lock()
{
    DWORD ret = WaitForSingleObject(the_mutex, INFINITE);
    if (ret == WAIT_FAILED) 
    {
#ifdef USE_EXCEPTIONS
      throw invalid_object("win32_mutex::lock failed waiting for mutex");
#else
      FATAL_ERROR("win32_mutex::lock failed waiting for mutex");
#endif
    } 
}

//--------------------------------------------------------
/// Hmm, currently is an error to unlock and unlocked mutex.
///      I wonder if that is correct?
void
win32_mutex::unlock()
{
   if (ReleaseMutex(the_mutex) == 0) 
   {
#ifdef USE_EXCEPTIONS
      throw invalid_object("win32_mutex::unlock failed releasing the mutex");
#else
      FATAL_ERROR("win32_mutex::unlock failed releasing the mutex");
#endif
   }
}

/*********************************************************************/
}
