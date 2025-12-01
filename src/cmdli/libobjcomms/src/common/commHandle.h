#ifndef commHandle_H
#define commHandle_H
/**********************************************************************
 **                                                                  **
 **  commHandle.h                                                    **
 **                                                                  **
 **  base file which users subclass to get object comms              **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: commHandle.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: commHandle.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:17  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:55  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:31:54  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.2  2004/05/11 19:34:34  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.1  2003/01/20 22:03:05  doug
// snapshot
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include "objcomms.h"

namespace sara
{
/**********************************************************************/
class commHandle 
{
public:
   /// Create a communications object handle
   commHandle();

   /// Free a communications object handle
   ~commHandle();
};

/*********************************************************************/
}
#endif
