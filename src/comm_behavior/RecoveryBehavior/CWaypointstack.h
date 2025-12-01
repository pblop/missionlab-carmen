/**********************************************************************
 **                                                                  **
 **                       CWaypointstack.h	     		    		 **
 **                                                                  **
 **                                                                  **
 **  Written by:  Patrick Ulam                                       **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: CWaypointstack.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#ifndef CWAYPOINTSTACK_H
#define CWAYPOINTSTACK_H

#include <iostream>
#include <stack>
#include "comm_data.h"
//#include "data_structs.h"

using namespace std;

class WaypointStack
{

public:

  // Create a waypoint stack.  It will collect points at an interval
  // every *interval* calls to addPosition
  WaypointStack(int interval);

  // Pauses the WaypointStack from recording new points to the stack
  void stop();

  // Starts the WaypointStack recording points
  void start();

  // add this point to the stack
  void addPosition(Comm_Vector v);
  
  // get the last position from the stack
  Comm_Vector next();

  // look at the next point on the stack
  Comm_Vector peek();

  bool IsStopped(void);
  
  bool IsLast(void) { if (_old_positions.size() == 1) return 1; return 0;};

private:

  stack<Comm_Vector> _old_positions;
  bool _stopped;
  int _counter;
  int _interval;
  int _Waypoint_Cnt;

};

#endif

/**********************************************************************
 * $Log: CWaypointstack.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2006/06/08 14:47:43  endo
 * CommBehavior from MARS 2020 migrated.
 *
 **********************************************************************/
