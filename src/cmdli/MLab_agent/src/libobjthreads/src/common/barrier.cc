/**********************************************************************
 **                                                                  **
 **  barrier class                                                   **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1997, Mobile Intelligence Corporation                 **
 **  7963 Pine Hollow Drive, SE                                      **
 **  Alto, Michigan  49302-9724                                      **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: barrier.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: barrier.cc,v $
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
// Revision 1.9  2004/05/11 19:34:33  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.8  2003/07/11 22:07:51  doug
// Convert condition.wait to use a TimeOfDay timeout
//
// Revision 1.7  2002/06/13 15:19:33  doug
// *** empty log message ***
//
// Revision 1.6  1998/02/06 14:53:08  doug
// renamed NoTimeOut to NoWaitTimeOut
//
// Revision 1.5  1997/12/10 11:56:51  doug
// *** empty log message ***
//
// Revision 1.4  1997/12/09 15:59:18  doug
// *** empty log message ***
//
// Revision 1.3  1997/11/18 19:04:42  doug
// giving up on rtti
//
// Revision 1.2  1997/11/13 11:37:15  doug
// works
//
// Revision 1.1  1997/11/12 17:35:52  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include "barrier.h"

namespace sara 
{
/*********************************************************************/
// Create a barrier
barrier::barrier(const int members) :
   num_queued(0),
   num_members(members),
   callback(NULL),
   cond("barrier"),
   guard("barrier")
{
}

/*********************************************************************/
// Wait at the barrier until all members reach it
void
barrier::enter()
{
   // Get access
   guard.lock();

   // Add us to the number queued 
   num_queued ++;

   // Are we the last to enter?
   if( num_queued >= num_members )
   {
      // Invoke a callback routine, if one is established
      if( callback )
         (*callback)();

      // Release the threads
      restart_blocked();
   }
   else
   {
      // Wait on the condition variable
      cond.wait(guard);
   }

   // Release the lock
   guard.unlock();
}


/*********************************************************************/
// Restart the queue'd processes
// NOTE: Assumes you have guard locked or don't care
void 
barrier::restart_blocked()
{
   // Restart the queued threads
   cond.broadcast();

   // Non waiting
   num_queued = 0;
}

/*********************************************************************/
}
