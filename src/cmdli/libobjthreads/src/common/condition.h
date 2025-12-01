#ifndef CONDITION_H
#define CONDITION_H
/**********************************************************************
 **                                                                  **
 **  condition.h                                                     **
 **                                                                  **
 **  fnd desc                                                        **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: condition.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: condition.h,v $
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
// Revision 1.8  2002/12/31 20:13:20  doug
// added thread_sleep call
//
// Revision 1.7  1998/02/06 14:47:54  doug
// redid the wait function to return a bool
//
// Revision 1.6  1998/02/06 12:52:49  doug
// remove mic.h include.
//
// Revision 1.5  1998/01/29 11:09:59  doug
// made exceptions compile option
//
// Revision 1.4  1997/12/10 11:56:43  doug
// *** empty log message ***
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

#include "mutex.h"
#include "TimeOfDay.h"

namespace sara
{
/// Define an infinite wait timeout.
const uint NoWaitTimeOut = 0;

class condition 
{
   public:
      /// Create a condition variable
      condition(const char* name = "");

      /// Delete a condition variable
      ~condition();
    
      /// Wait for the condition to be signaled or broadcasted
      /// Returns true
      bool wait(mutex &mtx); 

      /// Wait for the condition to be signaled or broadcasted
      /// Returns true on success, false on timeout.
      bool wait(mutex &mtx, const TimeOfDay &delay); 

      /// Signal one thread waiting on the condition
      void signal(); 

      /// Signal all threads waiting on the condition
      void broadcast(); 

      /// return the name
      const char *condition_name() const;

   private:
      /// Pointer to the implementation of the condition variable
      class condition_implementation *impl;

      /// Name of the condition variable
      char *cname;

      /// Disable these copy constructors
      condition(const condition&);
      condition& operator= (const condition&);
};




/*********************************************************************/
}
#endif
