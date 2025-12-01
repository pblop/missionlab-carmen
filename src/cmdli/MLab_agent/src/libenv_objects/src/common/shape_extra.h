#ifndef shape_EXTRA_H
#define shape_EXTRA_H
/**********************************************************************
 **                                                                  **
 **  shape_extra.h                                                   **
 **                                                                  **
 **  base class for extra shape records                              **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: shape_extra.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: shape_extra.h,v $
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
* Revision 1.3  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.2  2002/11/07 23:31:43  doug
* 2d circles are working
*
* Revision 1.1  2002/11/04 22:34:34  doug
* snapshot
*
**********************************************************************/

#include "mic.h"
#include "shape.h"

/**********************************************************************/
namespace sara
{
class shape_extra
{
public:
   /// Called by the front rec to create the data record.
   static shape_extra *create(class shape *p);

   /// Holds the pointer to the factory function
   static shape_extra *(* factory)(class shape *p);

   // No extra work to do here in creating or deleting
   shape_extra() {};
   virtual ~shape_extra() {};
};
}
/*********************************************************************/
#endif
