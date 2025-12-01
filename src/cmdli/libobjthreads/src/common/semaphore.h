#ifndef SEMAPHORE_H
#define SEMAPHORE_H

/**********************************************************************
 **                                                                  **
 **  A semaphore class for thread synchronization                    **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1997, Mobile Intelligence Corporation                 **
 **  7963 Pine Hollow Drive, SE                                      **
 **  Alto, Michigan  49302-9724                                      **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: semaphore.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: semaphore.h,v $
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
// Revision 1.4  1998/02/13 16:57:18  doug
// WIN32 changes
// ,.
//
// Revision 1.3  1997/12/10 11:57:07  doug
// *** empty log message ***
//
// Revision 1.2  1997/11/13 11:43:52  doug
// Rewrote to allow multiple threads access, if desired
//
// Revision 1.1  1997/11/13 11:38:06  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include <stdio.h>

#include "mutex.h"
#include "condition.h"

namespace sara
{
class semaphore
{
public:

   // Create a semaphore
   semaphore(uint _max = 1) :
      max_allowed(_max),
      current(0),
      cond("semaphore"),
      guard("semaphore")
   {};

   // Delete the semaphore
   ~semaphore() {};

   // Wait on the semaphore until available
   void p();

   // Release a semaphore previously p'd
   void v();

private:
   // The maximum number of processes allowed in the critical section
   uint max_allowed;

   // The number of processes in the critical section currently
   uint current;

   // Processes queue at the barrier by waiting on this condition
   condition cond;

   // Protect the member data
   mutex guard;
};

}
#endif
