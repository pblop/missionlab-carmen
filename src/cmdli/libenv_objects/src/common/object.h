#ifndef object_H
#define object_H
/**********************************************************************
 **                                                                  **
 **  object.h                                                        **
 **                                                                  **
 **  Implement a object-like environmental object.                   **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: object.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: object.h,v $
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
* Revision 1.3  2002/11/07 23:31:43  doug
* 2d circles are working
*
* Revision 1.2  2002/11/06 19:12:01  doug
* compiles!
*
* Revision 1.1  2002/09/21 14:10:04  doug
* Initial revision
**********************************************************************/

#include "shape.h"
#include "env_object.h"

/**********************************************************************/
namespace sara
{

class object : public env_object
{
public:
   /// check if we can be moved
   bool is_movable() {return ismovable;}

   /// check if we can hold other objects
   bool is_container() {return iscontainer;}

   // Constructors
   object(shape *theShape);
   object(shape *theShape, const strings *label);
   object(shape *theShape, const strings *label, const string &color);

   // destructor
   ~object();

private:
   /// is this object movable?
   bool ismovable;

   /// can this object hold other objects?
   bool iscontainer;
};
}

/**********************************************************************/
#endif

