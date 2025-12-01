#ifndef Robot_Info_H
#define Robot_Info_H
/**********************************************************************
 **                                                                  **
 **  Robot_Info.h                                                    **
 **                                                                  **
 **  Keep track of all the information involved with a single robot  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: Robot_Info.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: Robot_Info.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:19:15  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.3  2004/05/11 19:34:49  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.2  2003/10/23 19:31:41  doug
// snapshot
//
// Revision 1.1  2003/10/23 19:14:44  doug
// closer to compiling a minimal set
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"

/***************************************************************************/
namespace sara 
{
class Robot_Info 
{
   public:
      /// Constructor
      Robot_Info();

      /// Destructor
      ~Robot_Info();

      // Stream out in a form suitable for the loader
      string print() const;

      // The info.
      string name;
      string executable;
      string host;
      string color;
      class Robot_ParmList *parms;	
};

/***************************************************************************/
}
#endif
