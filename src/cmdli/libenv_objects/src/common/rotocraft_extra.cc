/**********************************************************************
 **                                                                  **
 **  rotocraft_extra.cc                                                 **
 **                                                                  **
 **  Factory object to create extra data record for rotocraft objects   **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: rotocraft_extra.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: rotocraft_extra.cc,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:21:04  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.2  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.1  2003/11/26 15:24:21  doug
* snapshot
*
**********************************************************************/

#include "mic.h"
#include "rotocraft_extra.h"

namespace sara
{
/**********************************************************************/

/// Holds the pointer to the factory function
rotocraft_extra *(* rotocraft_extra::factory)(class rotocraft *) = NULL;

/**********************************************************************/
// Create an extra record
rotocraft_extra *
rotocraft_extra::create(class rotocraft *p)
{
   if( factory )
   {
      rotocraft_extra *ptr = factory(p);
      return ptr;
   }
   else
   {
      return NULL;
   }
}

/*********************************************************************/
}
