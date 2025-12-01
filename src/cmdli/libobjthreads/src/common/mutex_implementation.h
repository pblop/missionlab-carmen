#ifndef MUTEX_IMPLEMENTATION_H
#define MUTEX_IMPLEMENTATION_H
/**********************************************************************
 **                                                                  **
 **  mutex_implementation.h                                          **
 **                                                                  **
 **  abstract base class for implementations of mutex variables      **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: mutex_implementation.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: mutex_implementation.h,v $
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
// Revision 1.3  1997/11/10 11:48:31  doug
// made the access function pure virtual
//
// Revision 1.2  1997/11/07 17:06:34  doug
// Tests OK
//
// Revision 1.1  1997/11/07 12:44:19  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

namespace sara
{
class mutex_implementation
{
   public:
      static mutex_implementation *create_mutex();

      // No extra work to do here
      virtual ~mutex_implementation() {};

      virtual void lock() = 0;
      virtual void unlock() = 0;

   private:
      friend class posix_mutex;
      friend class win32_mutex;
      friend class djgpp_mutex;

      // Only allow the friends access to the constructor
      mutex_implementation() {};
};

/*********************************************************************/
}
#endif
