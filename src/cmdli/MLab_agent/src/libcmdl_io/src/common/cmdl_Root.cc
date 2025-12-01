/**********************************************************************
 **                                                                  **
 **  cmdl_Root.cc - Keep track of stuff that doesn't change          **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: cmdl_Root.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: cmdl_Root.cc,v $
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
// Revision 1.7  2004/05/11 19:34:49  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.6  2004/02/20 14:39:35  doug
// cmdl interpreter runs a bit
//
// Revision 1.5  2003/11/10 22:48:30  doug
// working on cmdli
//
// Revision 1.4  2003/11/07 22:54:51  doug
// working on cmdl_io
//
// Revision 1.3  2003/11/07 20:29:27  doug
// added cmdli interpreter library
//
// Revision 1.2  2003/11/03 16:43:31  doug
// snapshot
//
// Revision 1.1  2003/10/24 21:03:39  doug
// finally compiles
//
///////////////////////////////////////////////////////////////////////


#include "mic.h"
#include "RobotType_ParmList.h"
#include "cmdl_Root.h"
#include "RobotType_Info.h"
#include "environment.h"
#include "Unit.h"

namespace sara 
{
/***************************************************************************/
// Constructor
cmdl_Root::cmdl_Root()
{
}

/***************************************************************************/
// Destructor
cmdl_Root::~cmdl_Root()
{
}

// ***************************************************************************
// Has the robot name been defined?
bool 
cmdl_Root::robotTypeExists(const string &name) const
{
   Robots_T::const_iterator it = robots.find( name );
   return it != robots.end();
}

// ***************************************************************************
// Define the robot 
// takes over ownership of robot_rec
// returns true on success, false if failed because already exists
bool 
cmdl_Root::defineRobotType(RobotType_Info *robot_rec)
{
   if( robotTypeExists( robot_rec->name ) )
   {
      return false;
   }

   robots[robot_rec->name] = robot_rec;
   return true;
}

// ***************************************************************************
// look up the robot record and return a pointer to it.
// returns a pointer to the robot record (the user must not delete it)
//         returns NULL if the robot is not defined
RobotType_Info *
cmdl_Root::getRobotType(const string &name) const
{
   Robots_T::const_iterator it = robots.find( name );
   return it->second;
}

// ***************************************************************************
// Has the unit name been defined?
bool 
cmdl_Root::unitExists(const string &name) const
{
   Units_T::const_iterator it = units.find( name );
   return it != units.end();
}

// ***************************************************************************
// Define the unit 
// takes over ownership of rec
// returns true on success, false if failed because already exists
bool 
cmdl_Root::defineUnit(Unit *rec)
{
// what do we do with an anonymous unit???????
   if( rec->isNamed() )
   {
      if( unitExists( rec->getName() ) )
      {
         // duplicate definition
         return false;
      }

      // add this one
      units[rec->getName()] = rec;
   }

   // good
   return true;
}

// ***************************************************************************
// look up the unit record and return a pointer to it.
// returns a pointer to the unit record (the user must not delete it)
//         returns NULL if the unit is not defined
Unit *
cmdl_Root::getUnit(const string &name) const
{
   Units_T::const_iterator it = units.find( name );
   return it->second;
}

// ***************************************************************************
/// Get a reference to the named object or return NULL if not found
/// The user must not delete the returned pointer
env_object *
cmdl_Root::getObject(const string &name) const
{
   return theEnvironment->getObject(name);
}

/***************************************************************************/
// Stream out in a form suitable for the loader
string 
cmdl_Root::print() const
{
   stringstream out;

/*
   cmdl_Root::cmdl_Root_t::const_iterator it;
   bool needComma = false;
   for(it = Info.begin(); it != Info.end(); ++it)
   {
      // dump it
      if( needComma )
         out << ", ";
      needComma = true;

      out << (*it)->first << " = " << (*it)->second;
   }
*/
   return out.str();
}

// **************************************************************************
/// Get the list of robots mentioned in this mission
stringSet 
cmdl_Root::getRobotNames() const
{
   stringSet names;

   Units_T::const_iterator it;
   for(it=units.begin(); it!=units.end(); ++it)
   {
      // get the list of robots used by each sub
      stringSet subs = it->second->getRobotNames();

      // add any new ones into our set
      stringSet::iterator sit;
      for(sit=subs.begin(); sit!=subs.end(); ++sit)
      {
         if( names.find( *sit ) == names.end() )
         {
            names.insert( *sit );
         }
      }
   }

   return names;
}

// **************************************************************************
/// set a default syncronization timeout
void 
cmdl_Root::setDefaultSyncTimeout(const TimeOfDay &timeout)
{
   defaultSyncTimeoutSpecified = true;
   defaultSyncTimeout = timeout;
}

// **************************************************************************
}
