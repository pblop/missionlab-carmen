/**********************************************************************
 **                                                                  **
 **                          jbox_display.c                          **
 **                                                                  **
 **                                                                  **
 **  Written by: Yoichiro Endo                                       **
 **                                                                  **
 **  Copyright 2004 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: jbox_display.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <memory.h>
#include <malloc.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <vector>
#include <algorithm>

#include "gt_sim.h"
#include "gt_scale.h"
#include "shape.hpp"
#include "robot.hpp"
#include "list.hpp"
#include "console.h"
#include "jbox_mlab.h"
#include "jbox_display.h"

using std::vector;

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------
extern GTList<shape*> object_list;    // Defined in simulation_server.c

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const int JBoxDisplay::UPDATE_TIMEOUT_MSEC_ = 100;
const double JBoxDisplay::MIN_ROBOT_LENGTH_ = 0.5;
const double JBoxDisplay::DEFAULT_ROBOT_LENGTH_ = 0.5;

//-----------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------
JBoxDisplay::JBoxDisplay(XtAppContext app, bool overlayLoaded)
{
  app_ = app;

  if (!overlayLoaded)
  {
      Pick_new_map();
  }

  loadAllowedRobotJBoxList_();

  XtAppAddTimeOut(
     app_,
     UPDATE_TIMEOUT_MSEC_,
     (XtTimerCallbackProc)displayLoop,
     NULL);
}

//-----------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------
JBoxDisplay::~JBoxDisplay(void)
{
}

//-----------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------
void JBoxDisplay::loadAllowedRobotJBoxList_(void)
{
    char *str; 
    void *p = NULL;

    // Get the list of allowed robot JBoxes.
    rc_chain *allowedRobotJBoxList = (rc_chain *)rc_table.get("AllowedRobotJBoxIDs");

    if (allowedRobotJBoxList)
    {
        p = allowedRobotJBoxList->first(&str); 

        while (p)
        {
            allowedRobotJBoxIDs_.push_back(atoi(str));
            p = allowedRobotJBoxList->next(&str, p);
        }
    }
}

//-----------------------------------------------------------------------
// This function updates the robot on the screen.
//-----------------------------------------------------------------------
void JBoxDisplay::displayLoop(void)
{
  gJBoxDisplay->displayJBoxRobot_();

  XtAppAddTimeOut(
     gJBoxDisplay->app_,
     JBoxDisplay::UPDATE_TIMEOUT_MSEC_,
     (XtTimerCallbackProc) (gJBoxDisplay->displayLoop),
     NULL);
}

//-----------------------------------------------------------------------
// This function updates the robot on the screen.
//-----------------------------------------------------------------------
void JBoxDisplay::displayJBoxRobot_(void)
{
    JBOX_data jboxData;
    JBoxDataList_t jboxDataList;
    vector<int>::iterator listIterator;
    gt_Point pos;
    char buf[1024];
    double latitude, longitude, gpsHeading, heading;
    double timeStamp;
    static double lastTimeStamp = 0;
    int i, index, robotID, jboxID;

    if (gJBoxMlab == NULL)
    {
        return;
    }

    jboxDataList = gJBoxMlab->getJBoxDataList(&timeStamp);

    if (timeStamp == lastTimeStamp)
    {
        // Data not updated.
        return;
    }
    else
    {
        lastTimeStamp = timeStamp;
    }


    for (i = 0; i < (int)(jboxDataList.size()); i++)
    {
        jboxData = jboxDataList[i];
        jboxID = jboxData.ID;

        listIterator = find(
            allowedRobotJBoxIDs_.begin(),
            allowedRobotJBoxIDs_.end(),
            jboxID);

        if (listIterator == allowedRobotJBoxIDs_.end())
        {
            // This JBox is not a robot. Don't display.
            continue;
        }
        
        // Get the data.
        gpsHeading = jboxData.gps_reading[JBoxMlab::GPS_HEADING];
        latitude = jboxData.gps_reading[JBoxMlab::GPS_LATITUDE];
        longitude = jboxData.gps_reading[JBoxMlab::GPS_LONGITUDE];
        
        // Convert.
        pos.x = gJBoxMlab->LongToX(longitude);
        pos.y = gJBoxMlab->LatToY(latitude);
        heading = gJBoxMlab->gpsHeadToMlabHead(gpsHeading);

        listIterator = find(
            quickJBoxIDList_.begin(),
            quickJBoxIDList_.end(),
            jboxID);

        if (listIterator == quickJBoxIDList_.end())
        {
            // It's a new JBox robot. Start it as a new.
            startJBoxRobot_(jboxID, pos, heading, "green");
        }
        else
        {
            index = listIterator - quickJBoxIDList_.begin();
            robotID = quickRobotIDList_[index];
            sim_robot_location(robotID, pos, heading);
        }


        sprintf(
            buf,
            "JBox ID %d <Lat: %.3f, Lon: %.3f, Heading: %.1f>\n           <X: %.1f, Y: %.1f, T: %.1f>\n",
            jboxID,
            latitude,
            longitude,
            gpsHeading,
            pos.x,
            pos.y,
            heading);


        if (debug)
        {
            report_user(buf);
            fprintf(stderr, buf);
        }
    }
}

//-----------------------------------------------------------------------
// This function starts robot and saves it in the data base.
//-----------------------------------------------------------------------
void JBoxDisplay::startJBoxRobot_(
    int jboxID,
    gt_Point pos,
    double heading,
    char *color)
{
    T_robot_state* cur;
    RobotData_t robotData;
    vector<int> allowedRobotJBoxIDs;
    int robotID;
    
    // We don't have a particular ID yet.
    cur = add_new_robot(-1);

    if (cur == NULL)
    {
        fprintf(
            stderr,
            "Warning: JBoxDisplay::startJBoxRobot_(): Failed to start a robot (JBox ID: %d).\n",
            jboxID);
        return;
    }
   
    // We now have a robot ID.
    robotID = cur->robot_id;

    // Set it as alive.
    cur->alive = true;

    // Create a new object and add it to the list
    cur->object_ptr = new robot(pos.x, pos.y, 0.2, color);
    object_list.append(cur->object_ptr);

    // Save the robot data.
    robotData.posX = pos.x;
    robotData.posY = pos.y;
    robotData.heading = heading;
    robotData.robotID = robotID;
    robotData.jboxID = jboxID;
    saveRobotData_(robotData);

    // Set the robot size.
    if (robot_length_meters < MIN_ROBOT_LENGTH_)
    {
        set_robot_length(DEFAULT_ROBOT_LENGTH_);
    }

    sim_robot_location(
        robotID,
        pos,
        heading);
}

//-----------------------------------------------------------------------
// This function saves the robot data.
//-----------------------------------------------------------------------
void JBoxDisplay::saveRobotData_(RobotData_t data)
{
    robotDataList_.push_back(data);
    quickRobotIDList_.push_back(data.robotID);
    quickJBoxIDList_.push_back(data.jboxID);
}

/**********************************************************************
 * $Log: jbox_display.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/07/12 06:16:42  endo
 * Clean-up for MissionLab 7.0 release.
 *
 * Revision 1.1  2006/07/11 17:15:31  endo
 * JBoxMlab merged from MARS 2020.
 *
 * Revision 1.2  2004/11/18 00:43:41  endo
 * JBox report supressed.
 *
 * Revision 1.1  2004/04/21 04:59:27  endo
 * JBoxMlab improved.
 *
 * Revision 1.1.1.1  2004/02/27 22:30:57  endo
 * Initial MARS 2020 revision
 *
 * Revision 1.1  2003/06/19 20:35:46  endo
 * Initial revision
 *
 **********************************************************************/
