#ifndef env_object_EXTRA_H
#define env_object_EXTRA_H
/**********************************************************************
 **                                                                  **
 **  env_object_extra.h                                              **
 **                                                                  **
 **  Factory object to create extra data records for env_object      **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: env_object_extra.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: env_object_extra.h,v $
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
* Revision 1.1  2002/11/01 21:49:41  doug
* working on it
*
**********************************************************************/

/**********************************************************************/
namespace sara
{
class env_object_extra
{
   public:
      // Called by the front rec to create the data record.
      static env_object_extra *create(class env_object *p);

      // Holds the pointer to the factory function
      static env_object_extra *(* factory)(class env_object *p);

      // No extra work to do here in creating or deleting
      virtual ~env_object_extra() {};
      env_object_extra() {};
};

}
/*********************************************************************/
#endif
