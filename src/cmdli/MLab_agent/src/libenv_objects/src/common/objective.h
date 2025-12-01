#ifndef objective_H
#define objective_H
/**********************************************************************
 **                                                                  **
 **  objective.h                                                     **
 **                                                                  **
 **  Implement a objective-like environmental object.                **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: objective.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: objective.h,v $
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
* Revision 1.6  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.5  2003/03/25 21:45:14  doug
* moved libccl_code to the sara namespace
*
* Revision 1.4  2002/11/07 23:31:43  doug
* 2d circles are working
*
* Revision 1.3  2002/11/04 22:34:34  doug
* snapshot
*
* Revision 1.2  2002/11/01 22:44:34  doug
* snapshot
*
* Revision 1.1  2002/09/20 13:05:19  doug
* Initial revision
*
**********************************************************************/

#include "shape.h"
#include "env_object.h"
#include "actionable.h"

/**********************************************************************/
namespace sara
{

class objective : public env_object, public actionable
{
public:
   // objectives are movable.
   bool is_movable() {return true;}

   // Constructor
   objective(shape *theShape);

   // Constructor
   objective(shape *theShape, const strings *label);
   objective(shape *theShape, const strings *label, const string &color);

   // destructor
   ~objective();
};

}
/**********************************************************************/
#endif

