#ifndef loiterArea_H
#define loiterArea_H
/**********************************************************************
 **                                                                  **
 **  loiterArea.h                                                    **
 **                                                                  **
 **  Implement a loiterArea-like environmental object.               **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: loiterArea.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: loiterArea.h,v $
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
* Revision 1.3  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.2  2003/03/25 21:45:13  doug
* moved libccl_code to the sara namespace
*
* Revision 1.1  2002/11/20 22:17:28  doug
* can assign loiter areas in 2d
*
**********************************************************************/

#include "shape.h"
#include "env_object.h"
#include "actionable.h"

/**********************************************************************/
namespace sara
{

class loiterArea : public env_object, public actionable
{
public:
   /// loiterAreas are not movable.
   bool is_movable() {return false;}

   /// Constructor
   loiterArea(shape *theShape);

   /// Constructors
   loiterArea(shape *theShape, const strings *label);
   loiterArea(shape *theShape, const strings *label, const string &color);

   /// destructor
   ~loiterArea();

   /// Number of loitering vehicles allowed
   uint numAllowed;
};

}
/**********************************************************************/
#endif

