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

/* $Id: test1_mutex.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: test1_mutex.cc,v $
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
main(int, char *[])
{
   // test simple creation and deletion
   mutex *a = NULL;
#ifdef USE_EXCEPTIONS
   try
#endif
   {
      a = new mutex("A");
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

   // Check if the name was assigned
   if( strcmp(a->mutex_name(), "A") != 0 )
   {
      cout << "Mutex name handling is broken: A->" << a->mutex_name() << '\n';
      failed();
   }
   
   // a should now be unlocked.  Try unlocking it again to see what happens
#ifdef USE_EXCEPTIONS
   try
#endif
   {
//      a->unlock();

// Hmm, current linux implementation doesn't report an error 
//      cout << "Unlocking an unlocked mutex unexpectedly succeeded!\n";
//      failed();
   }
#ifdef USE_EXCEPTIONS
   catch (const already &)
   {
      // OK, we were expecting that error
   }
   catch (const mic_exception &e)
   {
      cout << "Unlocking an unlocked mutex threw an mic_exception: " << e << '\n';
      failed();
   }
   catch (...)
   {
      cout << "Unlocking an unlocked mutex threw an unknown mic_exception\n";
      failed();
   }
#endif

//******TD: Need a timeout wrapper around this, since lock may block 

   // Now try locking the mutex
#ifdef USE_EXCEPTIONS
   try
#endif
   {
      a->lock();
   }
#ifdef USE_EXCEPTIONS
   catch (const mic_exception &e)
   {
      cout << "Locking a mutex threw an mic_exception: " << e << '\n';
      failed();
   }
   catch (...)
   {
      cout << "Locking a mutex threw an unknown mic_exception\n";
      failed();
   }
#endif

#if 0
//******TD: Need a timeout wrapper around this, since will deadlock

   // Now try locking a locked mutex to make sure get blocked
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
#endif

   // Now unlock the mutex
#ifdef USE_EXCEPTIONS
   try
#endif
   {
      a->unlock();
   }
#ifdef USE_EXCEPTIONS
   catch (const mic_exception &e)
   {
      cout << "Unlocking a locked mutex threw an mic_exception: " << e << '\n';
      failed();
   }
   catch (...)
   {
      cout << "Unlocking a locked mutex threw an unknown mic_exception\n";
      failed();
   }
#endif

   // Now delete the mutex
#ifdef USE_EXCEPTIONS
   try
#endif
   {
      delete a;
   }
#ifdef USE_EXCEPTIONS
   catch (const mic_exception &e)
   {
      cout << "Deleting an unlocked mutex threw an mic_exception: " << e << '\n';
      failed();
   }
   catch (...)
   {
      cout << "Deleting an unlocked mutex threw an unknown mic_exception\n";
      failed();
   }
#endif

   cout << "SUCESSFUL mutex test\n";
   return 0;
}

}
