/**********************************************************************
 **                                                                  **
 **  Command_extra.cc                                                **
 **                                                                  **
 **  Factory object to create the extra data record for commands     **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: Command_extra.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

/**********************************************************************
* $Log: Command_extra.cc,v $
* Revision 1.1.1.1  2008/07/14 16:44:15  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:45  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:19:14  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.2  2004/05/11 19:34:49  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.1  2003/11/07 20:31:00  doug
* adding interpreter
*
**********************************************************************/

#include "mic.h"
#include "Command_extra.h"

namespace sara 
{
/**********************************************************************/

/// Holds the pointer to the factory function
Command_extra *(* Command_extra::factory)(Command *)= NULL;

/**********************************************************************/
// Create an extra record
Command_extra *
Command_extra::create(Command *p)
{
   if( factory )
   {
      return factory(p);
   }
   else
   {
      return NULL;
   }
}

/*********************************************************************/
}
