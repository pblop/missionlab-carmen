/**********************************************************************
 **                                                                  **
 **  semaphore class                                                 **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1997, Mobile Intelligence Corporation                 **
 **  7963 Pine Hollow Drive, SE                                      **
 **  Alto, Michigan  49302-9724                                      **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: semaphore.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: semaphore.cc,v $
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
// Revision 1.7  2003/07/11 22:07:51  doug
// Convert condition.wait to use a TimeOfDay timeout
//
// Revision 1.6  1998/02/13 16:57:18  doug
// WIN32 changes
// ,.
//
// Revision 1.5  1998/02/06 14:53:54  doug
// renamed NoTimeOut NoWaitTimeOut
//
// Revision 1.4  1997/12/09 15:59:26  doug
// *** empty log message ***
//
// Revision 1.3  1997/11/18 19:07:36  doug
// giving up on rtti
//
// Revision 1.2  1997/11/13 11:43:52  doug
// Rewrote to allow multiple threads access, if desired
//
// Revision 1.1  1997/11/13 11:38:06  doug
// Initial revision
//
// Revision 1.1  1997/03/04 09:33:14  doug
// Initial revision
//
//
///////////////////////////////////////////////////////////////////////

#include <typeinfo>
#include "semaphore.h"

namespace sara 
{
/*********************************************************************/
// Wait on the semaphore until available
void
semaphore::p()
{
   // Get access
   guard.lock();

   // Wait for it to become available
   while( current >= max_allowed )
   {
      // Wait on the condition variable
      cond.wait(guard);
   }

   // We are entering the critical section
   current++;

   // Release the lock
   guard.unlock();
}


/*********************************************************************/
// Release a semaphore previously p'd
void 
semaphore::v()
{
   // Get access
   guard.lock();

   // We are leaving the critical section
   current--;

   // Wake up anybody waiting
   cond.signal();

   // Release the lock
   guard.unlock();
}


/*********************************************************************/
}
