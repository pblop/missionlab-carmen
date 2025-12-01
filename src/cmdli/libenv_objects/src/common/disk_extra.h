#ifndef disk_EXTRA_H
#define disk_EXTRA_H
/**********************************************************************
 **                                                                  **
 **  disk_extra.h                                                    **
 **                                                                  **
 **  Factory object to create the extra data record for disk objects **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: disk_extra.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: disk_extra.h,v $
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
* Revision 1.2  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.1  2002/09/20 22:34:44  doug
* Initial revision
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

/**********************************************************************/
namespace sara
{
class disk_extra
{
   public:
      // Called by the front rec to create the data record.
      static disk_extra *create(class disk *p);

      // Holds the pointer to the factory function
      static disk_extra *(* factory)(class disk *p);

      // No extra work to do here in creating or deleting
      virtual ~disk_extra() {};
      disk_extra() {};

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
