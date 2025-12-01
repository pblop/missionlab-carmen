/**********************************************************************
 **                                                                  **
 **  RobotType_Info.cc - A parameter list                                **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: RobotType_Info.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: RobotType_Info.cc,v $
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
#include "RobotType_ParmList.h"
#include "RobotType_Info.h"

namespace sara 
{
/***************************************************************************/
// Constructor
RobotType_Info::RobotType_Info() :
   parms(NULL)
{
}

/***************************************************************************/
// Destructor
RobotType_Info::~RobotType_Info()
{
   if( parms )
   {
      delete parms;
      parms = NULL;
   }
}

/***************************************************************************/
// Stream out in a form suitable for the loader
ostream & operator << (ostream & out, const RobotType_Info &r)
{
   out << "New-Robot " << r.name << " ";
   if( !r.executable.empty() )
   {
      out << "\"" << r.executable << "\" ";
   }
   if( !r.host.empty() )
   {
      out << r.host << " ";
   }
   if( !r.color.empty() )
   {
      out << r.color << " ";
   }

   if( r.parms )
   {
      out << " " << *r.parms;
   }

   return out;
}

//**************************************************************************
}
