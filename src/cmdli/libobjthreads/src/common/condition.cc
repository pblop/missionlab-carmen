/**********************************************************************
 **                                                                  **
 **  condition.cc                                                    **
 **                                                                  **
 **  condition variables                                             **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: condition.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: condition.cc,v $
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
// Revision 1.10  2004/05/11 19:34:33  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.9  2003/07/11 22:07:51  doug
// Convert condition.wait to use a TimeOfDay timeout
//
// Revision 1.8  1998/02/06 14:47:54  doug
// redid the wait function to return a bool
//
// Revision 1.7  1998/01/29 11:09:59  doug
// made exceptions compile option
//
// Revision 1.6  1997/12/10 11:56:43  doug
// *** empty log message ***
//
// Revision 1.5  1997/12/09 15:59:03  doug
// *** empty log message ***
//
// Revision 1.4  1997/11/18 19:05:25  doug
// giving up on rtti
//
// Revision 1.3  1997/11/10 18:46:54  doug
// Test works
//
// Revision 1.2  1997/11/10 18:31:14  doug
// Builds!
//
// Revision 1.1  1997/11/07 18:09:56  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include <stdlib.h>
#include <string.h>
#include "mutex.h"
#include "condition.h"
#include "condition_implementation.h"

namespace sara 
{
/*********************************************************************/

// Create a condition variable
condition::condition(const char* name)
{
   cname = strdup(name);
   impl = condition_implementation::create_condition();
}

/*********************************************************************/

// Destroy the condition variable
condition::~condition()
{
   if( cname )
   {
      free(cname);
      cname = NULL;
   }

   if( impl )
   {
      delete impl;
      impl = NULL;
   }
}

/*********************************************************************/
// Wait for the condition to be signaled or broadcasted
bool 
condition::wait(mutex &mtx)
{
   return impl->wait(mtx);
};

/*********************************************************************/
// Wait for the condition to be signaled or broadcasted
bool 
condition::wait(mutex &mtx, const TimeOfDay &delay)
{
   return impl->wait(mtx,delay);
};

/*********************************************************************/
// Signal one thread waiting on the condition
void 
condition::signal()
{
   impl->signal();
};

/*********************************************************************/
// Signal all threads waiting on the condition
void
condition::broadcast()
{
   impl->broadcast();
};

/*********************************************************************/
// return the name
const char *
condition::condition_name() const
{
   if( cname == NULL )
   {
#ifdef USE_EXCEPTIONS
      throw invalid_object("condition::condition_name The condition name is empty!");
#else
      FATAL_ERROR("condition::condition_name The condition name is empty!");
#endif
   }

   return cname;
};

}
