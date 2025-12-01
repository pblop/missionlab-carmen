/**********************************************************************
 **                                                                  **
 **  strings.cc                                                      **
 **                                                                  **
 **  define a vector of strings                                      **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 2002.  All Rights Reserved.                           **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: strings.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: strings.cc,v $
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
* Revision 1.3  2004/05/11 19:34:29  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.2  2003/07/11 20:19:38  doug
* cleanup debug messages
*
* Revision 1.1  2002/11/04 18:09:49  doug
* added seperate strings class so could enhance constructors
*
**********************************************************************/

#include "mic.h"
#include "mstrings.h"

using namespace std;
namespace sara
{
// *******************************************************************
/// Constructor
strings::strings() 
{
}

// *******************************************************************
/// Constructor
strings::strings(const string &str) 
{
   data.push_back(str);
}

// *******************************************************************
/// Constructor
strings::strings(const vector<string> &strs) :
   data(strs)
{
}

// ********************************************************************
// Stream out contents of the record for debugging.
ostream &operator << (ostream & out, const strings &obj)
{
   strings::strings_T::const_iterator it;
   for(it=obj.data.begin(); it!=obj.data.end(); ++it)
      out << *it << endl;

   return out;
}
                                                                                       
// ***************************************************************************
}
