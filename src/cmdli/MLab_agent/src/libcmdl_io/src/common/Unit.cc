/**********************************************************************
 **                                                                  **
 **  Unit.cc - A parameter list                                **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: Unit.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: Unit.cc,v $
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
// Revision 1.5  2004/05/11 19:34:49  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.4  2004/03/08 14:52:34  doug
// cross compiles on visual C++
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
// Revision 1.2  2003/10/23 19:31:41  doug
// snapshot
//
// Revision 1.1  2003/10/23 19:14:44  doug
// closer to compiling a minimal set
//
///////////////////////////////////////////////////////////////////////


#include "mic.h"
#include "Unit.h"

namespace sara 
{
/***************************************************************************/
// Constructor
Unit::Unit() :
   hasName(false),
   primitive(NULL)
{
}

/***************************************************************************/
// Destructor
Unit::~Unit()
{
}

// ***************************************************************************
/// set the name
void 
Unit::setName(const string &newName)
{
   hasName = true;
   name = newName;
}

// ***************************************************************************
/// adds the sub-unit into this unit.
bool 
Unit::addMember(Unit *subunit) 
{ 
   if( primitive )
   {
      // oops, already defined as a primitive, can't be both.
      return false;
   }

   members.push_back(subunit);
   return true;
}

// ***************************************************************************
/// is the robot a member of this unit?
bool 
Unit::isMember(const string &robotName) const
{
   // If we are a primitive, then it is easy.
   if( primitive )
   {
      return cmp_nocase(robotName, primitive->getName()) == 0;
   }

   // otherwise, check our list of members
   Unit::memberList_T::const_iterator it;
   for(it = members.begin(); it != members.end(); ++it)
   {
      if( (*it)->isNamed() )
      {
         if( cmp_nocase(robotName, (*it)->getName()) == 0 )
         {
            return true;
         }
      }
      else
      {
         if( (*it)->isMember(robotName) )
         {
            return true;
         }
//            out << " " << **it;
      }
   }

   return false;
}

// ***************************************************************************
/// adds the robot instance to the unit.
bool 
Unit::setPrimitive(RobotType_Info *instance) 
{ 
   if( !members.empty() )
   {
      // oops, already defined as a unit, can't be both.
      return false;
   }

   primitive = instance;
   return true;
}

// ***************************************************************************
/// Get the list of robots mentioned in this mission
stringSet
Unit::getRobotNames() const
{
   stringSet names;

   if( primitive )
   {
      if( !hasName )
      {
         ERROR("Unit::getRobotNames - Unit does not have a name!");
      }
      else
      {
         names.insert( name );
      }
   }
   else
   {
      memberList_T::const_iterator it;
      for(it=members.begin(); it!=members.end(); ++it)
      {
         // get the list of robots used by each sub
         stringSet subs = (*it)->getRobotNames();

         // add any new ones into our set
         stringSet::const_iterator sit;
         for(sit=subs.begin(); sit!=subs.end(); ++sit)
         {
            if( names.find( *sit ) == names.end() )
            {
               names.insert( *sit );
            }
         }
      }
   }

   return names;
}

// ***************************************************************************
// Stream out in a form suitable for the loader
ostream & operator << (ostream & out, const Unit &r)
{
   out << "Unit ";
   if( r.isNamed() )
   {
      out << "<" << r.getName() << "> (";
   }

   if( r.primitive )
   {
      out << r.primitive->getName();
   }
   else
   {
      bool needSpace = false;
      Unit::memberList_T::const_iterator it;
      for(it = r.members.begin(); it != r.members.end(); ++it)
      {
         if( needSpace )
         {
            out << " ";
         }
         else
         {
            needSpace = true;
         }

         if( (*it)->isNamed() )
         {
            out << (*it)->getName();
         }
         else
         {
            out << **it;
         }
      }
   }

   if( r.isNamed() )
   {
      out << ")";
   }

   return out;
}

//**************************************************************************
}
