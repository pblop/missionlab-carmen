/**********************************************************************
 **                                                                  **
 **                       CWaypointstack.CPP	     		    	 **
 **                                                                  **
 **                                                                  **
 **  Written by:  Patrick Ulam                                       **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: CWaypointstack.cpp,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#include "stdafx.h"

#include <stdio.h>
#include <math.h>
#include "CWaypointstack.h"

#define WAYPOINT_MAX 10000

WaypointStack::WaypointStack(int interval) {
  _interval = interval;
  _stopped = false;
  _Waypoint_Cnt = 0;
}


void WaypointStack::stop()
{
  _stopped = true;
}

void WaypointStack::start()
{
  _counter = 0;
  _stopped = false;
  _Waypoint_Cnt = 0;
}

void WaypointStack::addPosition(Comm_Vector v)
{
  Comm_Vector oldpos;
  if (!_stopped)
    {
      _counter ++;
      
      if (_old_positions.size() == 0)
        {
          _old_positions.push(v);
        }
      else {

              oldpos = _old_positions.top();
              oldpos.x = oldpos.x - v.x;
              oldpos.y = oldpos.y - v.y;
              if (sqrt(oldpos.x * oldpos.x + oldpos.y * oldpos.y) > 4) {
                _old_positions.push(v);
              }
      }
    }
}

Comm_Vector WaypointStack::peek()
{
  Comm_Vector v;
  v.x = -2;
  v.y = -2;
  v.z = -2;

      if ((!_old_positions.empty()) && (_Waypoint_Cnt < WAYPOINT_MAX))
        {
          v = _old_positions.top();
        }
  else
    {
      v.x = -2;
      v.y = -2;
      v.z = -2;
    }

  return (v);
}
Comm_Vector WaypointStack::next()
{
  Comm_Vector v;

  v.x = -2;
  v.y = -2;
  v.z = -2;

  // if it is the last waypoint, don't pop it just return it
  if ((!_old_positions.empty()) && (_Waypoint_Cnt < WAYPOINT_MAX))
    {
      v = _old_positions.top();
      if (_old_positions.size() > 1) {
	_old_positions.pop();
	_Waypoint_Cnt++;
      }
    }
  else
    {
      v = _old_positions.top();
    }

  return (v);
}

/**********************************************************************
 * $Log: CWaypointstack.cpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2006/06/08 14:47:42  endo
 * CommBehavior from MARS 2020 migrated.
 *
 **********************************************************************/
