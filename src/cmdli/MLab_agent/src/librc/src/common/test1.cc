/**********************************************************************
 **                                                                  **
 **  test the resources object                                       **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1997, Mobile Intelligence Corporation                 **
 **  7963 Pine Hollow Drive, SE                                      **
 **  Alto, Michigan  49302-9724                                      **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: test1.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: test1.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:17  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:55  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:34:51  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.6  2004/05/11 19:34:31  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.5  2002/10/10 20:22:10  doug
// Added save capabilities to the resource library
//
// Revision 1.4  1998/02/09 13:28:57  doug
// Added win32 changes
//
// Revision 1.3  1997/12/10 12:14:51  doug
// *** empty log message ***
//
// Revision 1.2  1997/12/08 14:58:47  doug
// *** empty log message ***
//
// Revision 1.1  1997/12/01 15:38:07  doug
// Initial revision
//
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include <stdio.h>
#include <stdlib.h>
#include "resources.h"

namespace sara
{
// Exit codes:
//   0   : Successful test
//   <>0 : Test failed

void failed()
{
   cout << "FAILED resources test\n";
   exit(1);
}

int 
main(int, char *[])
{
   // Try loading a good file
   resources res;
   if( res.load("test1.res") )
      failed();

//   cout << "The loaded file:\n" << res;

   cout << "SUCESSFUL resources test\n";

   if( res.save("output.res", true) )

   return 0;
}

}
