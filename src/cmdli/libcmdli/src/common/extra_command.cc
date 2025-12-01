/**********************************************************************
 **                                                                  **
 **  extra_command.cc                                                **
 **                                                                  **
 **  Keeps track of information for a specific command               **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: extra_command.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: extra_command.cc,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:45  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:14:58  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.3  2004/05/11 19:34:50  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.2  2004/03/08 14:52:35  doug
* cross compiles on visual C++
*
* Revision 1.1.1.1  2003/11/07 20:27:33  doug
* initial import
*
**********************************************************************/

#include "mic.h"
#include "extra_command.h"

namespace sara
{
/*********************************************************************/
/// A static functional constructor  
Command_extra *
extra_command::create(Command *p)
{ 
   return new extra_command(p);
}

/*********************************************************************/
extra_command::extra_command(Command *p) :
   cache_valid(false),
   main(p)
{
   /* empty */
}

/*********************************************************************/
/// Get action index
uint 
extra_command::getAction() const
{
   if( !cache_valid )
      FATAL_ERROR("Cache invalid in extra_command::getAction");

   return cached_action_index; 
}

/*********************************************************************/
/// Get list of required triggers (vector of indicies)
const uints &
extra_command::getTriggersRequired() const
{
   if( !cache_valid )
      FATAL_ERROR("Cache invalid in extra_command::getTriggersRequired");

   return cached_triggers_required; 
}

/*********************************************************************/
}
