#ifndef gap_EXTRA_H
#define gap_EXTRA_H
/**********************************************************************
 **                                                                  **
 **  gap_extra.h                                                    **
 **                                                                  **
 **  Factory object to create the extra data record for gaps        **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: gap_extra.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: gap_extra.h,v $
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
* Revision 1.1  2002/09/20 13:05:19  doug
* Initial revision
*
**********************************************************************/

namespace sara
{
/**********************************************************************/
class gap_extra
{
   public:
      // Called by the front rec to create the data record.
      static gap_extra *create(class gap *p);

      // Holds the pointer to the factory function
      static gap_extra *(* factory)(class gap *p);

      // No extra work to do here in creating or deleting
      virtual ~gap_extra() {};
      gap_extra() {};

      // Hook for implementions to see changes to object colors.
      virtual void set_color(const string &new_color) {};

      // Hook for implementions to see changes to the disk label
      virtual void changeLabel(const string &newLabel) {};

      // Hook for implementions to see changes to the disk label
      virtual void addLabel(const string &newLabel) {};
};

}
/*********************************************************************/
#endif
