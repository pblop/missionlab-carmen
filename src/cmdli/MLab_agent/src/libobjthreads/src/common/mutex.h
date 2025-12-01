#ifndef MUTEX_H
#define MUTEX_H
/**********************************************************************
 **                                                                  **
 **  mutex.h                                                         **
 **                                                                  **
 **  A mutex object class                                            **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: mutex.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: mutex.h,v $
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
// Revision 1.8  1998/02/06 14:39:41  doug
// win32 fixes
//
// Revision 1.7  1998/02/06 12:52:21  doug
// remove exception include.
//
// Revision 1.6  1998/01/29 11:10:11  doug
// made exceptions compile option
//
// Revision 1.5  1997/11/11 00:28:08  doug
// Fixed msg
//
// Revision 1.4  1997/11/10 18:22:46  doug
// compiles again
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

namespace sara
{
class mutex 
{
   public:
      // Create a mutex variable (is initially unlocked)
      mutex(const char* name = "");

      // Delete a mutex variable
      ~mutex();
    
      // lock a mutex (blocks until operation completes)
      void lock();
    
      // unlock a mutex
      void unlock();

      // return the name
      const char *mutex_name() const;

   private:
      friend class posix_condition;	// Needs to get at impl
      friend class win32_condition;	// Needs to get at impl

      // Pointer to the implementation of the mutex variable
      class mutex_implementation *impl;

      // Name of the mutex variable
      char *mname;

      // Disable these copy constructors
      mutex(const mutex&);
      mutex& operator= (const mutex&);
};


/*********************************************************************/
}
#endif
