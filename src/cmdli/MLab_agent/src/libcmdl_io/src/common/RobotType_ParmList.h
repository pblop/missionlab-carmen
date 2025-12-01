#ifndef RobotType_ParmList_H
#define RobotType_ParmList_H
/**********************************************************************
 **                                                                  **
 **  RobotType_ParmList.h - A parameter list                             **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: RobotType_ParmList.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: RobotType_ParmList.h,v $
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
// Revision 1.2  2003/10/24 21:03:39  doug
// finally compiles
//
// Revision 1.1  2003/10/23 19:14:44  doug
// closer to compiling a minimal set
//
///////////////////////////////////////////////////////////////////////


#include "mic.h"

/***************************************************************************/
namespace sara 
{
class RobotType_ParmList 
{
   public:
      /// Constructor
      RobotType_ParmList();

      /// Destructor
      ~RobotType_ParmList();

      /// Stream out in a form suitable for the loader
      friend ostream & operator << (ostream & out, const RobotType_ParmList &r);

      /// Has the parameter name been defined?
      bool exists(const string &name) const;

      // Define the parameter name 
      // returns true on success, false if failed because already exists
      bool define(const string &name, const string &value);

      // look up the parameter and return its value 
      // returns the empty string if the parameter was not defined
      string get(const string &name) const;

   private:
      // convience type
      typedef map<string, string> RobotType_ParmList_t;

      // The list
      RobotType_ParmList_t parmlist;
};

/***************************************************************************/
}
#endif
