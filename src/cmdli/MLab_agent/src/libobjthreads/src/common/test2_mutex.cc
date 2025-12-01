/**********************************************************************
 **                                                                  **
 **  test the mutex object                                           **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1997, Mobile Intelligence Corporation                 **
 **  7963 Pine Hollow Drive, SE                                      **
 **  Alto, Michigan  49302-9724                                      **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: test2_mutex.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: test2_mutex.cc,v $
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
// Revision 1.8  1998/02/08 13:54:28  doug
// get win32 version to test
//
// Revision 1.7  1998/02/06 20:08:21  doug
// make execptions optional
//
// Revision 1.6  1997/12/10 11:56:32  doug
// *** empty log message ***
//
// Revision 1.5  1997/12/09 15:58:57  doug
// *** empty log message ***
//
// Revision 1.4  1997/11/19 13:07:45  doug
// rtti is back in
//
// Revision 1.3  1997/11/18 19:06:21  doug
// giving up on rtti
//
// Revision 1.2  1997/11/10 18:43:27  doug
// adding typeinfo include
//
// Revision 1.1  1997/11/10 11:35:32  doug
// Initial revision
//
// Revision 1.1  1997/11/07 17:07:11  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include <stdio.h>
#include <stdlib.h>
#include "mutex.h"

namespace sara 
{
// Exit codes:
//   0   : Successful test
//   <>0 : Test failed

void failed()
{
   cout << "FAILED mutex test\n";
   exit(1);
}

int 
main(int , char *[])
{
   // Create a mutex
   mutex *a = NULL;


   // Create and lock the mutex
#ifdef USE_EXCEPTIONS
   try
#endif
   {
      a = new mutex("A");
      a->lock();

   }
#ifdef USE_EXCEPTIONS
   catch (const mic_exception &e)
   {
      cout << "Simple mutex creation threw an mic_exception: " << e << '\n';
      failed();
   }
   catch (...)
   {
      cout << "Simple mutex creation threw an unknown mic_exception\n";
      failed();
   }
#endif
   if( a == NULL )
   {
      cout << "Simple mutex creation returned a NULL pointer\n";
      failed();
   }

//******TD: Need a timeout wrapper around this, so can run in automatic tests

   // Now try locking a locked mutex to make sure get blocked
   cerr << "Now going to deadlock trying to lock a locked mutex\n";
   cerr << "You will need to manually kill this process, if successful\n";
#ifdef USE_EXCEPTIONS
   try
#endif
   {
      a->lock();
   }
#ifdef USE_EXCEPTIONS
   catch (const mic_exception &e)
   {
      cout << "Locking a locked mutex threw an mic_exception: " << e << '\n';
      failed();
   }
   catch (...)
   {
      cout << "Locking a locked mutex threw an unknown mic_exception\n";
      failed();
   }
#endif

   cout << "ERROR: Locking a locked mutex did not deadlock\n";
   failed();

   return 1;
}

}
