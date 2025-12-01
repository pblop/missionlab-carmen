#ifndef circle_EXTRA_H
#define circle_EXTRA_H
/**********************************************************************
 **                                                                  **
 **  circle_extra.h                                                  **
 **                                                                  **
 **  Factory object to create extra data record for circle objects   **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: circle_extra.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: circle_extra.h,v $
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
* Revision 1.6  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.5  2002/11/05 17:05:46  doug
* builds again
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
#include "shape_extra.h"

/**********************************************************************/
namespace sara
{
class circle_extra : public shape_extra
{
   public:
      // Called by the front rec to create the data record.
      static circle_extra *create(class circle *p);

      // Holds the pointer to the factory function
      static circle_extra *(* factory)(class circle *p);

      // No extra work to do here in creating or deleting
      virtual ~circle_extra() {};
      circle_extra() {};
};

}
/*********************************************************************/
#endif
