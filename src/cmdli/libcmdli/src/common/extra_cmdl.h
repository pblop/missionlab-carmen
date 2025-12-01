#ifndef EXTRA_cmdl_H
#define EXTRA_cmdl_H
/**********************************************************************
 **                                                                  **
 **  extra_cmdl.h                                                     **
 **                                                                  **
 **  The extra_cmdl object saves state info for the interpreter.      **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: extra_cmdl.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: extra_cmdl.h,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:45  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:14:58  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.4  2004/05/11 19:34:50  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.3  2004/03/08 14:52:35  doug
* cross compiles on visual C++
*
* Revision 1.2  2003/11/11 22:04:13  doug
* getting it to execute
*
* Revision 1.1.1.1  2003/11/07 20:27:33  doug
* initial import
*
**********************************************************************/

#include "cmdl.h"
#include "cmdl_extra.h"

namespace sara
{
/*********************************************************************/
   
class extra_cmdl : public cmdl_extra
{
   public:
      /// Functional constructor
      static cmdl_extra *create(cmdl *p);

      // Constructor
      extra_cmdl(cmdl *p);

      // Destructor
      ~extra_cmdl() {};

   /******************************************************************/
   private:
      cmdl *main;	// Pointer to the main data record
};

/*********************************************************************/
}
#endif
