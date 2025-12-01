/**********************************************************************
 **                                                                  **
 **  RobotType_ParmList.cc - A parameter list                            **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: RobotType_ParmList.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: RobotType_ParmList.cc,v $
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
#include "RobotType_ParmList.h"

namespace sara 
{
/***************************************************************************/
// Constructor
RobotType_ParmList::RobotType_ParmList()
{
}

/***************************************************************************/
// Destructor
RobotType_ParmList::~RobotType_ParmList()
{
}

// ***************************************************************************
// Has the parameter name been defined?
bool 
RobotType_ParmList::exists(const string &name) const
{
   RobotType_ParmList_t::const_iterator it = parmlist.find( name );
   return it != parmlist.end();
}

// ***************************************************************************
// Define the parameter name 
// returns true on success, false if failed because already exists
bool 
RobotType_ParmList::define(const string &name, const string &value)
{
   if( exists( name ) )
   {
      return false;
   }

   parmlist[name] = value;
   return true;
}

// ***************************************************************************
// look up the parameter and return its value 
// returns the empty string if the parameter was not defined
string
RobotType_ParmList::get(const string &name) const
{
   RobotType_ParmList_t::const_iterator it = parmlist.find( name );
   return it->second;
}


/***************************************************************************/
// Stream out in a form suitable for the loader
ostream & operator << (ostream & out, const RobotType_ParmList &r)
{
   if( !r.parmlist.empty() )
   {
      out << " (" << endl;
      RobotType_ParmList::RobotType_ParmList_t::const_iterator it;
      bool needComma = false;
      for(it = r.parmlist.begin(); it != r.parmlist.end(); ++it)
      {
         // dump it
         if( needComma )
            out << "," << endl;
         needComma = true;
   
         out << "\t" << it->first << " = " << it->second;
      }
      out << " )" << endl;
   }

   return out;
}

//**************************************************************************
}
