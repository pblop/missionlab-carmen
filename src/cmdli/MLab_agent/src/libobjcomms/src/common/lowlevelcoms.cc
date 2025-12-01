/**********************************************************************
 **                                                                  **
 **  lowlevelcoms.cc                                                 **
 **                                                                  **
 **  base for the objcomm lowlevelcoms implementations               **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: lowlevelcoms.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: lowlevelcoms.cc,v $
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
// Revision 1.1  2004/03/24 22:15:37  doug
// working on win32 port
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include "lowlevelcoms.h"

// Get the correct include file
#if defined(WIN32)
   #include "win32_lowlevelcoms.h"
#elif defined(linux)
   #include "linux_lowlevelcoms.h"
#else
   #error Unsupported target architecture in lowlevelcoms.cc
#endif

namespace sara
{

// Create a new comm object
lowlevelcoms *
lowlevelcoms::create(objcomms *root)
{
// Create an implementation object based on the compile target architecture.
#if defined(WIN32)
   return new win32_lowlevelcoms(root);

#elif defined(linux)
   return new linux_lowlevelcoms(root);

#else
   #error Unsupported target architecture in lowlevelcoms.cc
#endif

   /*not reached*/
}

/*********************************************************************/
}
