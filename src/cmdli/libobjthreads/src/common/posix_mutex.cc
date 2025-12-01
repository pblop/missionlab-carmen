/**********************************************************************
 **                                                                  **
 **  posix_mutex.cc                                                  **
 **                                                                  **
 **  posix implementatin of mutex variables for objthreads           **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: posix_mutex.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: posix_mutex.cc,v $
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
// Revision 1.11  2003/04/07 12:44:41  doug
// moving to RedHat 9
//
// Revision 1.10  2000/01/12 21:34:32  doug
// /* DCM 1-12-00: the function call is no longer defined in RH 6.0
//
// Revision 1.9  1998/01/29 11:10:11  doug
// made exceptions compile option
//
// Revision 1.8  1997/12/10 11:56:32  doug
// *** empty log message ***
//
// Revision 1.7  1997/12/09 15:58:57  doug
// *** empty log message ***
//
// Revision 1.6  1997/11/19 12:57:34  doug
// RTTI is back in
//
// Revision 1.5  1997/11/18 19:06:21  doug
// giving up on rtti
//
// Revision 1.4  1997/11/10 18:43:27  doug
// adding typeinfo include
//
// Revision 1.3  1997/11/10 18:22:46  doug
// compiles again
//
// Revision 1.2  1997/11/07 17:06:34  doug
// Tests OK
//
// Revision 1.1  1997/11/07 12:44:19  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include "mic_exception.h"
#include "posix_mutex.h"

namespace sara 
{

/*********************************************************************/
posix_mutex::posix_mutex()
{
   if( pthread_mutex_init(&pt_mutex, NULL) )
   {
#ifdef USE_EXCEPTIONS
      throw no_resources("Unable to create pthread mutex variable");
#else
      FATAL_ERROR("posix_mutex::posix_mutex - Unable to create pthread mutex variable");
#endif
   }
}

/*********************************************************************/
posix_mutex::~posix_mutex()
{
   pthread_mutexattr_destroy(&attr);
   if( pthread_mutex_destroy(&pt_mutex) )
      WARN("Destroying busy mutex variable");
}


//--------------------------------------------------------

void
posix_mutex::lock()
{
   if( pthread_mutex_lock(&pt_mutex) )
   {
#ifdef USE_EXCEPTIONS
      throw invalid_object("posix_mutex::lock failed because the pthread mutex variable is invalid");
#else
      FATAL_ERROR("posix_mutex::lock failed because the pthread mutex variable is invalid");
#endif
   }
}

//--------------------------------------------------------

void
posix_mutex::unlock()
{
   if( pthread_mutex_unlock(&pt_mutex) )
   {
#ifdef USE_EXCEPTIONS
      throw invalid_object("posix_mutex::unlock failed because the pthread mutex variable is invalid");
#else
      FATAL_ERROR("posix_mutex::unlock failed because the pthread mutex variable is invalid");
#endif
   }
}

/*********************************************************************/
}
