/**********************************************************************
 **                                                                  **
 **  mic.cc                                                          **
 **                                                                  **
 **  system wide defines, constants, etc. for libmic                 **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: mic.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: mic.cc,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:55  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:30:29  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.3  2004/10/25 22:58:11  doug
* working on mars2020 integrated demo
*
* Revision 1.2  2004/05/11 19:34:29  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.1  2002/05/31 21:42:35  doug
* Initial revision
*
**********************************************************************/

#include "mic.h"

using namespace std;
namespace sara
{
// ********************************************************************
/// needs to be big to handle LAT/LONG values where 12 decimal points are significant
uint FloatPrecision = 24;

// ********************************************************************
/// compare two strings, case insensitive
int cmp_nocase(const string &s, const string &s2)
{
   string::const_iterator p = s.begin();
   string::const_iterator p2 = s2.begin();

   while( p!=s.end() && p2!=s2.end() )
   {
      if( toupper(*p) != toupper(*p2) )
         return (toupper(*p) < toupper(*p2)) ? -1 : 1;
      ++p;
      ++p2;
   }

   return s2.size() - s.size();
};

/*********************************************************************/
}
