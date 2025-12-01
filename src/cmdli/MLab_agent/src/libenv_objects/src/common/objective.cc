/**********************************************************************
 **                                                                  **
 **  objective.cc                                                    **
 **                                                                  **
 **  Implement a objective-like environmental object.                **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: objective.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: objective.cc,v $
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
* Revision 1.5  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
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

#include "mic.h"
#include "objective.h"

namespace sara
{
/**********************************************************************/
// Constructor
objective::objective(shape *theShape) :
   env_object(theShape)
{
}

/**********************************************************************/
// Constructor
objective::objective(shape *theShape, const strings *label) :
   env_object(theShape, label)
{
   /* empty */
}

/**********************************************************************/
// Constructor
objective::objective(shape *theShape, const strings *label, const string &new_color) :
   env_object(theShape, label, new_color)
{
   /* empty */
}

/**********************************************************************/
// destructor
objective::~objective()
{
}

/**********************************************************************/
}
