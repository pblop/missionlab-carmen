/**********************************************************************
 **                                                                  **
 **  mutex.cc                                                        **
 **                                                                  **
 **  mutex variables                                                 **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: mutex.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: mutex.cc,v $
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
// Revision 1.11  1998/01/29 11:10:11  doug
// made exceptions compile option
//
// Revision 1.10  1997/12/10 11:56:32  doug
// *** empty log message ***
//
// Revision 1.9  1997/12/09 15:58:57  doug
// *** empty log message ***
//
// Revision 1.8  1997/11/19 12:57:34  doug
// RTTI is back in
//
// Revision 1.7  1997/11/18 19:06:21  doug
// giving up on rtti
//
// Revision 1.6  1997/11/13 11:46:07  doug
// *** empty log message ***
//
// Revision 1.5  1997/11/11 00:28:08  doug
// Fixed msg
//
// Revision 1.4  1997/11/10 18:43:27  doug
// adding typeinfo include
//
// Revision 1.3  1997/11/07 18:11:08  doug
// Cleaned up mutex.h includes
//
// Revision 1.2  1997/11/07 17:06:34  doug
// Tests OK
//
// Revision 1.1  1997/11/07 12:44:19  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "mutex.h"
#include "mutex_implementation.h"

namespace sara 
{
/*********************************************************************/

// Create a mutex variable
mutex::mutex(const char* name)
{
   mname = strdup(name);
   impl = mutex_implementation::create_mutex();
}

/*********************************************************************/

// Destroy the mutex variable
mutex::~mutex()
{
   if( mname )
   {
      free(mname);
      mname = NULL;
   }

   if( impl )
   {
      delete impl;
      impl = NULL;
   }
}

/*********************************************************************/

// Lock a mutex variable
void
mutex::lock()
{
   if( impl == NULL )
   {
#ifdef USE_EXCEPTIONS
      throw invalid_operation("mutex::lock called on invalid object");
#else
      FATAL_ERROR("mutex::lock called on invalid object");
#endif
   }

   impl->lock();
}

/*********************************************************************/

// Unlock a mutex variable
void
mutex::unlock()
{
   if( impl == NULL )
   {
#ifdef USE_EXCEPTIONS
      throw invalid_operation("mutex::unlock called on invalid object");
#else
      FATAL_ERROR("mutex::unlock called on invalid object");
#endif
   }

   impl->unlock();
}

/*********************************************************************/
// return the name
const char *
mutex::mutex_name() const
{
   if( mname == NULL )
   {
#ifdef USE_EXCEPTIONS
      throw invalid_object("mutex::mutex_name The mutex name is empty!");
#else
      FATAL_ERROR("mutex::mutex_name The mutex name is empty!");
#endif
   }

   return mname;
};

/*********************************************************************/
}
