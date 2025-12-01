/**********************************************************************
 **                                                                  **
 **  test the syncronized template                                   **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1997, Mobile Intelligence Corporation                 **
 **  7963 Pine Hollow Drive, SE                                      **
 **  Alto, Michigan  49302-9724                                      **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: test_synchronized.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: test_synchronized.cc,v $
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

#include "mic.h"
#include <stdio.h>
#include <stdlib.h>
#include "mutex.h"
#include "synchronized.h"

namespace sara
{
// Exit codes:
// Exit codes:
//   0   : Successful test
//   <>0 : Test failed
void failed()
{
   cout << "FAILED synchronized test\n";
   exit(1);
}

int 
main(int, char *[])
{
   // test simple creation and deletion
   mutex lock1;
   mutex lock2;

   int i=0;

   cerr << "before the lock" << endl;
   synchronized(lock1)
   {
      cerr << "in lock1" << endl;
      synchronized(lock2)
      {
         cerr << "in lock2" << endl;
      }
      cerr << "out of lock2" << endl;
   }
   cerr << "out of lock1" << endl;

   cerr << "trying to relock" << endl;
   synchronized(lock1)
   {
      cerr << "relocked" << endl;
   }

   cerr << "out of the lock" << endl;

//      cout << "Deleting an unlocked mutex threw an unknown mic_exception\n";
////      failed();

   cout << "SUCESSFUL mutex test\n";
   return 0;
}

}
