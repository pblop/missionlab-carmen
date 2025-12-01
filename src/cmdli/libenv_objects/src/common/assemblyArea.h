#ifndef assemblyArea_H
#define assemblyArea_H
/**********************************************************************
 **                                                                  **
 **  assemblyArea.h                                                     **
 **                                                                  **
 **  Implement a assemblyArea-like environmental object.                **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: assemblyArea.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: assemblyArea.h,v $
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
* Revision 1.1  2002/11/13 17:40:44  doug
* assemblyArea added
*
**********************************************************************/

#include "shape.h"
#include "env_object.h"

/**********************************************************************/
namespace sara
{

class assemblyArea : public env_object
{
public:
   // assemblyAreas are movable.
   bool is_movable() {return false;}

   // Constructor
   assemblyArea(shape *theShape);

   // Constructor
   assemblyArea(shape *theShape, const strings *label);
   assemblyArea(shape *theShape, const strings *label, const string &color);

   // destructor
   ~assemblyArea();
};
}

/**********************************************************************/
#endif

