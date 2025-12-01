/**********************************************************************
 **                                                                  **
 **  mutex_implementation.cc                                         **
 **                                                                  **
 **  base for the objthread implementations                          **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: mutex_implementation.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: mutex_implementation.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:17  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:55  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:33:51  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.7  2004/05/11 19:34:33  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.6  1997/12/10 11:56:32  doug
// *** empty log message ***
//
// Revision 1.5  1997/12/09 15:58:57  doug
// *** empty log message ***
//
// Revision 1.4  1997/11/19 12:57:34  doug
// RTTI is back in
//
// Revision 1.3  1997/11/18 19:06:21  doug
// giving up on rtti
//
// Revision 1.2  1997/11/10 18:43:27  doug
// adding typeinfo include
//
// Revision 1.1  1997/11/07 12:44:19  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include "mutex_implementation.h"

// Get the correct include file
#if defined(WIN32)
   #include "win32_mutex.h"
#elif defined(DJGPP)
   #include "djgpp_mutex.h"
#elif defined(linux)
   #include "posix_mutex.h"
#else
   #error Unsupported target architecture in mutex_implementation.cc
#endif

namespace sara 
{
// Create a mutex variable
mutex_implementation *
mutex_implementation::create_mutex()
{
// Create an implementation object based on the compile target architecture.
#if defined(WIN32)
   return new win32_mutex();

#elif defined(DJGPP)
   return new djgpp_mutex();
   
#elif defined(linux)
   return new posix_mutex();

#else
   #error Unsupported target architecture in mutex_implementation.cc
#endif

   /*not reached*/
}

/*********************************************************************/
}
