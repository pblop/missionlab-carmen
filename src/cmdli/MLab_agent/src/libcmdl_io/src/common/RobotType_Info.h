#ifndef RobotType_Info_H
#define RobotType_Info_H
/**********************************************************************
 **                                                                  **
 **  RobotType_Info.h                                                    **
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

/* $Id: RobotType_Info.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: RobotType_Info.h,v $
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
// Revision 1.2  2004/05/11 19:34:49  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.1  2003/11/03 16:45:16  doug
// snapshot
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"

/***************************************************************************/
namespace sara 
{
class RobotType_Info 
{
   public:
      /// Constructor
      RobotType_Info();

      /// Destructor
      ~RobotType_Info();

      const string &getName() const {return name;}

      // Stream out in a form suitable for the loader
      friend ostream & operator << (ostream & out, const RobotType_Info &r);

      // The info.
      string name;
      string executable;
      string host;
      string color;
      class RobotType_ParmList *parms;	
};

/***************************************************************************/
}
#endif
