#ifndef Unit_H
#define Unit_H
/**********************************************************************
 **                                                                  **
 **  Unit.h                                                    **
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

/* $Id: Unit.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: Unit.h,v $
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
// Revision 1.4  2004/05/11 19:34:49  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.3  2004/02/20 14:39:35  doug
// cmdl interpreter runs a bit
//
// Revision 1.2  2004/01/15 00:24:09  doug
// working on cmdl
//
// Revision 1.1  2003/11/03 16:45:16  doug
// snapshot
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include "RobotType_Info.h"

/***************************************************************************/
namespace sara 
{
class Unit 
{
   public:
      /// Constructor
      Unit();

      /// Destructor
      ~Unit();

      /// adds the sub-unit into this unit.
      bool addMember(Unit *subunit);

      /// is the robot a member of this unit?
      bool isMember(const string &robotName) const;

      /// adds the robot instance to the unit.
      bool setPrimitive(RobotType_Info *instance);

      /// is the unit primitive?
      bool isPrimitive() const {return primitive != NULL;}

      /// set the name
      void setName(const string &name);

      /// is the unit named?
      bool isNamed() const {return hasName;}

      /// Get the unit name (if isNamed is true) or the empty string (isNamed is false)
      const string &getName() const {return name;}

      /// Get the list of robots mentioned in this mission
      stringSet getRobotNames() const;

      /// Stream out in a form suitable for the loader
      friend ostream & operator << (ostream & out, const Unit &r);

   private:
      /// Is the name set?
      bool hasName;

      // The name of this unit.
      string name;

      // If this is not NULL, then the primitive RobotType is the only member of this unit.
      RobotType_Info *primitive;

      // The members of this unit, if it is not primitive
      typedef vector< Unit * > memberList_T;	
      memberList_T members;
};

/***************************************************************************/
}
#endif
