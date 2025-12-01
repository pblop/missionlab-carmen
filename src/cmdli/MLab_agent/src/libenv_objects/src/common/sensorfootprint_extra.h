#ifndef sensorfootprint_EXTRA_H
#define sensorfootprint_EXTRA_H
/**********************************************************************
 **                                                                  **
 **  sensorfootprint_extra.h                                                  **
 **                                                                  **
 **  Factory object to create extra data record for sensorfootprint objects   **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: sensorfootprint_extra.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: sensorfootprint_extra.h,v $
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
* Revision 1.2  2002/11/14 19:44:03  doug
* sensorfootprint works
*
* Revision 1.1  2002/11/14 15:25:09  doug
* snapshot
*
**********************************************************************/

#include "mic.h"
#include "shape_extra.h"

/**********************************************************************/
namespace sara
{
class sensorfootprint_extra : public shape_extra
{
   public:
      // Called by the front rec to create the data record.
      static sensorfootprint_extra *create(class sensorfootprint *p);

      // Holds the pointer to the factory function
      static sensorfootprint_extra *(* factory)(class sensorfootprint *p);

      // No extra work to do here in creating or deleting
      virtual ~sensorfootprint_extra() {};
      sensorfootprint_extra() {};
};
}
/*********************************************************************/
#endif
