#ifndef CONDITION_IMPLEMENTATION_H
#define CONDITION_IMPLEMENTATION_H
/**********************************************************************
 **                                                                  **
 **  condition_implementation.h                                      **
 **                                                                  **
 **  abstract base class for implementations of condition variables  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: condition_implementation.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: condition_implementation.h,v $
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

#include "condition.h"
#include "mutex.h"

namespace sara
{
class condition_implementation
{
   public:
      static condition_implementation *create_condition();

      // No extra work to do here
      virtual ~condition_implementation() {};

      virtual bool wait(mutex &mtx, const TimeOfDay &delay) = 0; 
      virtual bool wait(mutex &m) = 0;
      virtual void signal() = 0;
      virtual void broadcast() = 0;

   private:
      friend class posix_condition;
      friend class win32_condition;
      friend class djgpp_condition;

      // Only allow friends access to the constructor
      condition_implementation() {};
};

/*********************************************************************/
}
#endif
