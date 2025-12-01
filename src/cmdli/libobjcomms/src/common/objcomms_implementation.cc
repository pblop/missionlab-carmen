/**********************************************************************
 **                                                                  **
 **  objcomms_implementation.cc                                                  **
 **                                                                  **
 **  base for the objcomm implementations                            **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: objcomms_implementation.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: objcomms_implementation.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:17  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:55  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:31:55  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.6  2004/05/11 19:34:34  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.5  2004/03/24 22:15:37  doug
// working on win32 port
//
// Revision 1.4  2003/01/23 22:39:12  doug
// making progress
//
// Revision 1.1  2003/01/22 22:09:54  doug
// snapshot
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include "objcomms_implementation.h"

// Get the correct include file
#if defined(WIN32)
   #include "posix_udp_objcomms.h"
//   #include "win32_objcomms.h"
#elif defined(DJGPP)
   #include "djgpp_objcomms.h"
#elif defined(linux)
   #include "posix_udp_objcomms.h"
#else
   #error Unsupported target architecture in objcomms_implementation.cc
#endif

namespace sara
{

// Create a new comm object
objcomms_implementation *
objcomms_implementation::create(objcomms *root)
{
// Create an implementation object based on the compile target architecture.
#if defined(WIN32)
   return new posix_udp_objcomms(root);
//   return new win32_objcomms(root);

#elif defined(DJGPP)
   return new djgpp_objcomms(root);
   
#elif defined(linux)
   return new posix_udp_objcomms(root);

#else
   #error Unsupported target architecture in objcomms_implementation.cc
#endif

   /*not reached*/
}

/*********************************************************************/
}
