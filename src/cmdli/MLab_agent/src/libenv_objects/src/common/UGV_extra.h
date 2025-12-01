#ifndef UGV_EXTRA_H
#define UGV_EXTRA_H
/**********************************************************************
 **                                                                  **
 **  UGV_extra.h                                                     **
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

/* $Id: UGV_extra.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: UGV_extra.h,v $
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
#include "shape_extra.h"

namespace sara
{
/**********************************************************************/
class UGV_extra : public shape_extra
{
   public:
      // Called by the front rec to create the data record.
      static UGV_extra *create(class UGV *p);

      // Holds the pointer to the factory function
      static UGV_extra *(* factory)(class UGV *p);

      // No extra work to do here in creating or deleting
      virtual ~UGV_extra() {};
      UGV_extra() {};
};

}
/*********************************************************************/
#endif
