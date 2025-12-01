#ifndef robot_vehicle_H
#define robot_vehicle_H
/**********************************************************************
 **                                                                  **
 **  robot_vehicle.h                                                 **
 **                                                                  **
 **  Implement a robot_vehicle-like environmental object.            **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: robot_vehicle.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: robot_vehicle.h,v $
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
* Revision 1.2  2003/03/15 19:46:04  doug
* working on opcon
*
* Revision 1.1  2002/11/15 14:03:17  doug
* renamed robot class to robot_vehicle to remove conflict with the hardware library
*
**********************************************************************/

#include "shape.h"
#include "env_object.h"

/**********************************************************************/
namespace sara
{

class robot_vehicle : public env_object
{
public:
   // robot_vehicles are movable.
   bool is_movable() {return false;}

   // Constructor
   robot_vehicle(shape *theShape);

   // Constructor
   // The strings "label" will be owned and freed by the robot_vehicle object.
   robot_vehicle(shape *theShape, const strings *label);
   robot_vehicle(shape *theShape, const strings *label, const string &color);

   // destructor
   ~robot_vehicle();
};

}
/**********************************************************************/
#endif

