/**********************************************************************
 **                                                                  **
 **  UGV_extra.cc                                                    **
 **                                                                  **
 **  Factory object to create extra data record for UGV objects      **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: UGV_extra.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: UGV_extra.cc,v $
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
* Revision 1.1  2004/08/18 22:52:04  doug
* got ugv demo filmed
*
**********************************************************************/

#include "mic.h"
#include "UGV_extra.h"

namespace sara
{
/**********************************************************************/

/// Holds the pointer to the factory function
UGV_extra *(* UGV_extra::factory)(class UGV *) = NULL;

/**********************************************************************/
// Create an extra record
UGV_extra *
UGV_extra::create(class UGV *p)
{
   if( factory )
   {
      UGV_extra *ptr = factory(p);
      return ptr;
   }
   else
   {
      return NULL;
   }
}

/*********************************************************************/
}
