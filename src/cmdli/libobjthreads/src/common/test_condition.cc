/**********************************************************************
 **                                                                  **
 **  test the condition variables                                    **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1997, Mobile Intelligence Corporation                 **
 **  7963 Pine Hollow Drive, SE                                      **
 **  Alto, Michigan  49302-9724                                      **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: test_condition.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: test_condition.cc,v $
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
// Revision 1.7  2004/05/11 19:34:33  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.6  1998/02/08 13:52:06  doug
// got win32 version to test
//
// Revision 1.5  1998/02/06 20:12:14  doug
// made exceptions optional
//
// Revision 1.4  1997/12/10 11:56:43  doug
// *** empty log message ***
//
// Revision 1.3  1997/11/19 13:08:05  doug
// rtti is back in
//
// Revision 1.2  1997/11/10 18:46:54  doug
// Test works
//
// Revision 1.1  1997/11/10 18:31:40  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include <stdio.h>
#include <stdlib.h>
#include "mutex.h"
#include "condition.h"

namespace sara 
{
// Exit codes:
//   0   : Successful test
//   <>0 : Test failed

void failed()
{
   cout << "FAILED condition test\n";
   exit(1);
}

int 
main(int, char *[])
{
   // test simple creation and deletion
   condition *a = NULL;
#ifdef USE_EXCEPTIONS
   try
#endif
   {
      a = new condition("C");
   }
#ifdef USE_EXCEPTIONS
   catch (const mic_exception &e)
   {
      cout << "Simple condition creation threw an mic_exception: " << e << '\n';
      failed();
   }
   catch (...)
   {
      cout << "Simple condition creation threw an unknown mic_exception\n";
      failed();
   }
#endif
   if( a == NULL )
   {
      cout << "Simple condition creation returned a NULL pointer\n";
      failed();
   }



   // Check if the name was assigned
   if( strcmp(a->condition_name(), "C") != 0 )
   {
      cout << "condition name handling is broken: C->" << a->condition_name() << '\n';
      failed();
   }


   
   // It should now be empty.  Try signaling it to see what happens
#ifdef USE_EXCEPTIONS
   try
#endif
   {
      a->signal();
   }
#ifdef USE_EXCEPTIONS
   catch (const mic_exception &e)
   {
      cout << "Signaling a condition threw an mic_exception: " << e << '\n';
      failed();
   }
   catch (...)
   {
      cout << "Signaling a condition threw an unknown mic_exception\n";
      failed();
   }
#endif


   // It should now be empty.  Try signaling it to see what happens
#ifdef USE_EXCEPTIONS
   try
#endif
   {
      a->broadcast();
   }
#ifdef USE_EXCEPTIONS
   catch (const mic_exception &e)
   {
      cout << "broadcast a condition threw an mic_exception: " << e << '\n';
      failed();
   }
   catch (...)
   {
      cout << "broadcast a condition threw an unknown mic_exception\n";
      failed();
   }
#endif


   mutex b;
   b.lock();

   // wait on the condition var to check if the timeout works
#ifdef USE_EXCEPTIONS
   try
#endif
   {
      // Should return after 1 second
      a->wait(b,1000);
   }
#ifdef USE_EXCEPTIONS
   catch (const mic_exception &e)
   {
      cout << "timeout on condition waiting threw an mic_exception: " << e << '\n';
      failed();
   }
   catch (...)
   {
      cout << "timeout on condition waiting threw an unknown mic_exception\n";
      failed();
   }
#endif

   // Unlock b
   b.unlock();


   // Now delete the condition
#ifdef USE_EXCEPTIONS
   try
#endif
   {
      delete a;
   }
#ifdef USE_EXCEPTIONS
   catch (const mic_exception &e)
   {
      cout << "Deleting an unlocked condition threw an mic_exception: " << e << '\n';
      failed();
   }
   catch (...)
   {
      cout << "Deleting an unlocked condition threw an unknown mic_exception\n";
      failed();
   }
#endif

   cout << "SUCESSFUL condition test\n";
   return 0;
}

}
