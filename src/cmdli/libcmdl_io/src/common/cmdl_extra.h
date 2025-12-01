#ifndef cmdl_EXTRA_H
#define cmdl_EXTRA_H
/**********************************************************************
 **                                                                  **
 **  cmdl_extra.h                                                    **
 **                                                                  **
 **  Factory object to create the extra data record                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: cmdl_extra.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

/**********************************************************************
* $Log: cmdl_extra.h,v $
* Revision 1.1.1.1  2008/07/14 16:44:15  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:45  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:19:15  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.2  2004/05/11 19:34:49  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.1  2003/11/07 20:31:00  doug
* adding interpreter
*
**********************************************************************/

namespace sara 
{
/**********************************************************************/
class cmdl_extra
{
   public:
      // Called by the front rec to create the data record.
      static cmdl_extra *create(class cmdl *p);

      // Holds the pointer to the factory function
      static cmdl_extra *(* factory)(class cmdl *);

      // No extra work to do here in creating or deleting
      virtual ~cmdl_extra() {};
      cmdl_extra() {};
};

/*********************************************************************/
}
#endif
