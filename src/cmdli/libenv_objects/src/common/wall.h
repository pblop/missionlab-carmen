#ifndef wall_H
#define wall_H
/**********************************************************************
 **                                                                  **
 **  wall.h                                                          **
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

/* $Id: wall.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: wall.h,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:21:05  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.3  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.2  2004/03/01 23:37:49  doug
* working on real overlay
*
* Revision 1.1  2002/09/23 21:42:14  doug
* Initial revision
*
**********************************************************************/

#include "shape.h"
#include "env_object.h"

/**********************************************************************/
namespace sara
{

class wall : public env_object
{
public:
   // walls are movable.
   bool is_movable() {return false;}

   // Constructor
   wall(shape *theShape);

   // Constructor
   wall(shape *theShape, const strings *label);
   wall(shape *theShape, const strings *label, const string &color);

   // destructor
   ~wall();
};
}
/**********************************************************************/
#endif

