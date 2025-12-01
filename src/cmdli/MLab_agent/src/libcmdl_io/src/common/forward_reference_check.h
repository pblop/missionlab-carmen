#ifndef forward_reference_check_H
#define forward_reference_check_H
/**********************************************************************
 **                                                                  **
 **  forward_reference_check.h                                       **
 **                                                                  **
 **  Keeps track of information for a specific transition            **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: forward_reference_check.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

/**********************************************************************
* $Log: forward_reference_check.h,v $
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
* Revision 1.1  2004/10/22 21:40:29  doug
* added goto and command exception support
*
**********************************************************************/

#include "mic.h"

namespace sara 
{
/**********************************************************************/

class forward_reference_check
{
   public:
      /// Constructor
      forward_reference_check(uint lineNumber,
      				 const string &errorMessage, 
				 class Block *theBlock,
		                 const string &theLabel);

      /// Destructor
      ~forward_reference_check() {};

      /// check it
      void checkIt() const;

   /*********************************************************************/
   private:
      uint line;
      string msg;
      class Block *block;
      string label;
};

/*********************************************************************/
}
#endif
