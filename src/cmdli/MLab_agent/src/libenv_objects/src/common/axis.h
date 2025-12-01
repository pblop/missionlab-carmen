#ifndef axis_H
#define axis_H
/**********************************************************************
 **                                                                  **
 **  axis.h                                                          **
 **                                                                  **
 **  Implement a axis-like environmental object.                     **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: axis.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: axis.h,v $
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
* Revision 1.1  2002/09/20 13:05:19  doug
* Initial revision
*
**********************************************************************/

#include "polygon.h"

/**********************************************************************/
namespace sara
{

class axis : public polygon
{
public:
   // assembly areas are not movable.
   bool is_movable() {return false;}

   // Constructor
   axis();

   // Constructor
   axis(const string &name, const Vectors &verticies, const double width, const string &color);

   // destructor
   ~axis();

   /// build a string of polylines that will draw the axis
   static Vectors build_axis_polygon(const Vectors &verticies, const double width);
};
}

/**********************************************************************/
#endif

