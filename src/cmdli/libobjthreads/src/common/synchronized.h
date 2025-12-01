#ifndef synchronized_H
#define synchronized_H
/**********************************************************************
 **                                                                  **
 **  synchronized.h                                                  **
 **                                                                  **
 **  A monitor implementation patterned after Java's "synchronized"  **
 **                                                                  **
 **  Based on "Concurrent Access Control & C++"                      **
 **  Haifeng Li and KLeshu Zhang                                     **
 **  C/C++ Users Journal, January 2004, pg. 32-37.                   **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: synchronized.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: synchronized.h,v $
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
// Revision 1.3  2004/05/11 19:34:33  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.2  2004/03/10 16:35:05  doug
// seems to work, but gives a variable not used warning
//
// Revision 1.1  2004/03/10 15:49:50  doug
// not yet working
//
///////////////////////////////////////////////////////////////////////

namespace sara
{
// ********************************************************************
// This is tricky.
// The const forces the reference to the lock, and thus the lock, to live
// until the reference goes out of scope

/// Provide synchronized blocks of code
#define synchronized(lock) \
   if (const lock_block &lock ## __LINE__ = make_lock(lock))

/// implement the internal lock_block class
class lock_block
{
   public:
      operator bool() const {return true;}
};

template < typename Lock_T >
class auto_lock : public lock_block
{
   public:
      /// constructor gets the lock
      auto_lock(Lock_T &lock):
         _lock(&lock)
         {
            _lock->lock();
         }

      /// destructor frees the lock
      ~auto_lock()
         {
            _lock->unlock();
         }

   private:
      /// the mutex
      Lock_T * _lock;
};

/// implement the internal make_lock template
template < typename Lock_T >
inline auto_lock<Lock_T>
make_lock(Lock_T &lock)
{
   return auto_lock<Lock_T>(lock);
};

// ********************************************************************
}
#endif
