/**********************************************************************
 **                                                                  **
 **  waves_extra.cc                                                   **
 **                                                                  **
 **  Factory object to create the extra data record for wavess        **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: waves_extra.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: waves_extra.cc,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:21:05  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.4  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.3  2003/07/11 20:19:42  doug
* cleanup debug messages
*
* Revision 1.2  2002/10/04 17:48:21  doug
* tweaking
*
* Revision 1.1  2002/09/20 22:34:44  doug
* Initial revision
*
* Revision 1.1  2002/09/20 13:05:19  doug
* Initial revision
*
**********************************************************************/

#include "mic.h"
#include "waves_extra.h"

namespace sara
{
/**********************************************************************/

/// Holds the pointer to the factory function
waves_extra *(* waves_extra::factory)(class waves *) = NULL;

/**********************************************************************/
// Create an extra record
waves_extra *
waves_extra::create(waves *p)
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
