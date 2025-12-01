#ifndef POSIX_CONDITION_H
#define POSIX_CONDITION_H
/**********************************************************************
 **                                                                  **
 **  posix_condition.h                                               **
 **                                                                  **
 **  posix implementation of condition variables                     **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: posix_condition.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: posix_condition.h,v $
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
// Revision 1.6  2004/11/06 01:11:55  doug
// snap
//
// Revision 1.5  2004/05/11 19:34:33  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.4  2003/07/11 22:07:51  doug
// Convert condition.wait to use a TimeOfDay timeout
//
// Revision 1.3  1998/02/06 14:47:54  doug
// redid the wait function to return a bool
//
// Revision 1.2  1997/11/10 18:31:14  doug
// Builds!
//
// Revision 1.1  1997/11/07 18:09:56  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include <pthread.h>
#include "condition_implementation.h"

namespace sara
{
class posix_condition : public condition_implementation
{
   public:
      // Create a condition variable
      posix_condition();

      // Delete a condition variable
      ~posix_condition();
   
      // Wait for the condition to be signaled or broadcasted
      // Returns true 
      bool wait(mutex &mtx);

      // Wait for the condition to be signaled, broadcasted, or timeout
      // Returns true on success, false on timeout.
      bool wait(mutex &mtx, const TimeOfDay &delay);

      // Signal one thread waiting on the condition
      void signal();
   
      // Signal all threads waiting on the condition
      void broadcast();
   
   private:
      pthread_cond_t        cnd;
};


/*********************************************************************/
}
#endif
