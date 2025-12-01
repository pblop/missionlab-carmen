/**********************************************************************
 **                                                                  **
 **  wall.cc                                                         **
 **                                                                  **
 **  Implement a wall-like environmental object.                     **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: wall.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: wall.cc,v $
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
* Revision 1.4  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.3  2004/03/01 23:37:49  doug
* working on real overlay
*
* Revision 1.2  2003/11/26 15:21:47  doug
* back from GaTech
*
* Revision 1.1  2002/09/23 21:42:14  doug
* Initial revision
*
**********************************************************************/

#include "mic.h"
#include "wall.h"

namespace sara
{
// *************************************************************
// Constructor
wall::wall(shape *theShape) :
   env_object(theShape)
{ /* empty */ }

// *************************************************************
// Constructor
wall::wall(shape *theShape, const strings *label) :
   env_object(theShape, label)
{ /* empty */ }

// *************************************************************
// Constructor
wall::wall(shape *theShape, const strings *label, const string &new_color) :
   env_object(theShape, label, new_color)
{ /* empty */ }


// *************************************************************
// destructor
wall::~wall()
{
}

// *************************************************************
}
