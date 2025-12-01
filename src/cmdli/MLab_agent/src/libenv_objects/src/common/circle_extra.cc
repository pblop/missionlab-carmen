/**********************************************************************
 **                                                                  **
 **  circle_extra.cc                                                 **
 **                                                                  **
 **  Factory object to create extra data record for circle objects   **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: circle_extra.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: circle_extra.cc,v $
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
* Revision 1.5  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.4  2002/11/04 22:34:34  doug
* snapshot
*
* Revision 1.3  2002/09/20 13:05:19  doug
* *** empty log message ***
*
* Revision 1.2  1999/03/10 00:22:28  doug
* moved extra to here from env_object
*
* Revision 1.1  1999/03/09 20:55:36  doug
* Initial revision
*
**********************************************************************/

#include "mic.h"
#include "circle_extra.h"

namespace sara
{
/**********************************************************************/

/// Holds the pointer to the factory function
circle_extra *(* circle_extra::factory)(class circle *) = NULL;

/**********************************************************************/
// Create an extra record
circle_extra *
circle_extra::create(class circle *p)
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
