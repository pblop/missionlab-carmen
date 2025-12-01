/**********************************************************************
 **                                                                  **
 **  ATRV_extra.cc                                                   **
 **                                                                  **
 **  Factory object to create extra data record for ATRV objects     **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: ATRV_extra.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: ATRV_extra.cc,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:21:03  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.3  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.2  2004/03/02 14:41:06  doug
* oops, fix the typo
*
* Revision 1.1  2004/03/01 23:37:48  doug
* working on real overlay
*
**********************************************************************/

#include "mic.h"
#include "ATRV_extra.h"

namespace sara
{
/**********************************************************************/

/// Holds the pointer to the factory function
ATRV_extra *(* ATRV_extra::factory)(class ATRV *) = NULL;

/**********************************************************************/
// Create an extra record
ATRV_extra *
ATRV_extra::create(class ATRV *p)
{
   if( factory )
   {
      ATRV_extra *ptr = factory(p);
      return ptr;
   }
   else
   {
      return NULL;
   }
}

/*********************************************************************/
}
