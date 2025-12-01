#ifndef dynamicLoadable_H
#define dynamicLoadable_H
/**********************************************************************
 **                                                                  **
 **  dynamicLoadable.h                                               **
 **                                                                  **
 **  base file for classes which can be dynamically loaded           **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: dynamicLoadable.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: dynamicLoadable.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:16  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:55  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:30:29  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.3  2004/05/11 19:34:29  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.2  2003/08/27 22:37:24  doug
// refactored dynamicLoader and dynamicLoadable
//
// Revision 1.1  2003/08/27 20:42:09  doug
// create a generic dynamic loader base class
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"

namespace sara
{
/**********************************************************************/
// Base class for all runtime data objects
class dynamicLoadable 
{
public:
   // Destructor
   virtual ~dynamicLoadable();

   // Constructor
   dynamicLoadable();
};

/*********************************************************************/
}
#endif
