#ifndef BARRIER_H
#define BARRIER_H

/**********************************************************************
 **                                                                  **
 **  barrier class for thread synchronization                        **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1997, Mobile Intelligence Corporation                 **
 **  7963 Pine Hollow Drive, SE                                      **
 **  Alto, Michigan  49302-9724                                      **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: barrier.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: barrier.h,v $
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
// Revision 1.4  2004/05/11 19:34:33  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.3  2002/06/13 15:19:33  doug
// *** empty log message ***
//
// Revision 1.2  1997/11/13 11:37:15  doug
// works
//
// Revision 1.1  1997/11/12 17:35:52  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include <stdio.h>

#include "mutex.h"
#include "condition.h"

namespace sara
{
class barrier
{
public:

   // Create a barrier
   barrier(const int numMembers = 0);

   // Delete the barrier
   ~barrier()
   {
      restart_blocked();
   }

   // Wait at the barrier until all members reach it
   void enter();

   // Set a function to call when the barrier is released
   void set_barrier_callback(vFunction f)
   {
      guard.lock();
      callback = f;
      guard.unlock();
   }

   void add_member(const int num = 1)    
   { 
      guard.lock();
      num_members += num; 
      guard.unlock();
   }
   void delete_member()
   { 
      guard.lock();
      num_members--; 
      guard.unlock();
   }

private:
   int    num_queued;
   int    num_members;
   vFunction callback;

   // Processes queue at the barrier by waiting on this condition
   condition cond;

   // Protect the member data
   mutex     guard;

   // Restart the queue'd processes
   void restart_blocked();
};

}
#endif
