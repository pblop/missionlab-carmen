/***********************************************************************

	sensor_blackboard.h

	Description:
		Define the sensor blackboard structure

	Modification History:
		Date		Author		Remarks
		10 NOV 99	Doug MacKenzie  Original


        Copyright 1999, Georgia Tech Research Corporation 
        Atlanta, Georgia  30332-0415
        ALL RIGHTS RESERVED, See file COPYRIGHT for details. 

*************************************************************************/

/* $Id: sensor_blackboard.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include <vector>
#include "gt_std_types.h"

using std::vector;

typedef struct 
{
   // The robot's X location in world coordinates.
   double x;

   // The robot's Y location in world coordinates.
   double y;

   // The robot's theta heading in world coordinates.
   double theta;

   // Holds the latest obstacle readings.  
   // NOTE: Uses a C++ STL vector (variable size array) - NOT a AuRA Vector !!!!
   vector<obs_reading> sensed_objects;

} T_sensor_blackboard;

// Holds the latest copy of the sensor readings.
extern T_sensor_blackboard sensor_blackboard;

/**********************************************************************
 * $Log: sensor_blackboard.h,v $
 * Revision 1.1.1.1  2006/07/12 13:37:56  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 22:59:39  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.13  2003/04/06 13:20:20  endo
 * gcc 3.1.1
 *
 * Revision 1.12  2000/01/19 18:13:39  endo
 * *** empty log message ***
 *
 * Revision 1.11  2000/01/19 18:12:14  endo
 * Code checked in for Doug. He added blackboard variable
 * feature.
 *
 *
 **********************************************************************/
