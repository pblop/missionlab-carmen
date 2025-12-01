#ifndef F16_EXTRA_H
#define F16_EXTRA_H
/**********************************************************************
 **                                                                  **
 **  F16_extra.h                                                  **
 **                                                                  **
 **  Factory object to create extra data record for F16 objects   **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: F16_extra.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: F16_extra.h,v $
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
* Revision 1.2  2002/11/14 19:44:03  doug
* F16 works
*
* Revision 1.1  2002/11/14 15:25:09  doug
* snapshot
*
**********************************************************************/

#include "mic.h"
#include "shape_extra.h"

namespace sara
{
/**********************************************************************/
class F16_extra : public shape_extra
{
   public:
      // Called by the front rec to create the data record.
      static F16_extra *create(class F16 *p);

      // Holds the pointer to the factory function
      static F16_extra *(* factory)(class F16 *p);

      // No extra work to do here in creating or deleting
      virtual ~F16_extra() {};
      F16_extra() {};
};

}
/*********************************************************************/
#endif
