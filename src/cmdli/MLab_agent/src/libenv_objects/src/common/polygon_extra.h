#ifndef polygon_EXTRA_H
#define polygon_EXTRA_H
/**********************************************************************
 **                                                                  **
 **  polygon_extra.h                                                 **
 **                                                                  **
 **  Factory object to create the extra data record for polygons     **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: polygon_extra.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: polygon_extra.h,v $
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
* Revision 1.2  2002/11/13 17:15:50  doug
* objectives work!
*
* Revision 1.1  2002/09/20 13:05:19  doug
* Initial revision
*
**********************************************************************/

#include "mic.h"
#include "shape_extra.h"

namespace sara
{
/**********************************************************************/
class polygon_extra : public shape_extra
{
   public:
      // Called by the front rec to create the data record.
      static polygon_extra *create(class polygon *p);

      // Holds the pointer to the factory function
      static polygon_extra *(* factory)(class polygon *p);

      // No extra work to do here in creating or deleting
      virtual ~polygon_extra() {};
      polygon_extra() {};

};

}
/*********************************************************************/
#endif
