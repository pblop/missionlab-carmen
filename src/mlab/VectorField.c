/**********************************************************************
 **                                                                  **
 **                            VectorField.c                         **
 **                                                                  **
 **  Written by:  Brian Lee                                          **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: VectorField.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $*/

#include "VectorField.h"
#include "draw.h"
#include "gt_sim.h"
#include "gt_simulation.h"
#include "console_side_com.h"

ClVectorField::ClVectorField(void)
{
    SetRobotID(-1);
    SetEdgeDistance(5.0);
    SetResolution(1.0);
    SetStopDrawing(false);
}

ClVectorField::~ClVectorField(void)
{
}

int ClVectorField::GetRobotID(void)
{
    return m_iRobotID;
}

void ClVectorField::SetRobotID(int iRobotID)
{
    m_iRobotID = iRobotID;
}

double ClVectorField::GetEdgeDistance(void)
{
    return m_dEdgeDistance;
}

void ClVectorField::SetEdgeDistance(double dEdgeDistance)
{
    m_dEdgeDistance = dEdgeDistance;
}

double ClVectorField::GetResolution(void)
{
    return m_dResolution;
}

void ClVectorField::SetResolution(double dResolution)
{
    m_dResolution = dResolution;
}

void ClVectorField::SetMissionArea(double dWidth, double dHeight)
{
    m_dMissionAreaWidth  = dWidth;
    m_dMissionAreaHeight = dHeight;
}

void ClVectorField::SetStopDrawing(bool bStopDrawing)
{
    m_bStopDrawing = bStopDrawing;
}

void ClVectorField::DrawVectorField(void)
{
    int oldRunType;
    EraseVectors(); // get rid of any old vectors

    // set the robot to SIMULATION mode
    SetRunType(GetRobotID(), SIMULATION, oldRunType);

    // tell the robot it's drawing the vector field
    SetDrawingVectorField(GetRobotID(), true);

    // if we're using a physical robot, get its readings
    if (oldRunType == REAL)
    {
        ResetSensorsReceived(GetRobotID());
        send_robot_report_sensor_readings(GetRobotID());
        WaitForSensors(GetRobotID());
    }

    send_drawing_vector_field_msg(GetRobotID());

    // iterate through all positions
    Vector suRobotLoc;
    GetRobotLocation(GetRobotID(), suRobotLoc);
    Vector suCurrentLocation;
    for (suCurrentLocation.x =  suRobotLoc.x - GetEdgeDistance(); 
          suCurrentLocation.x <= suRobotLoc.x + GetEdgeDistance(); 
          suCurrentLocation.x += GetResolution())
    {
        for (suCurrentLocation.y =  suRobotLoc.y - GetEdgeDistance(); 
              suCurrentLocation.y <= suRobotLoc.y + GetEdgeDistance(); 
              suCurrentLocation.y += GetResolution())
        {
            // make sure we're within bounds of the mission area
            if ((suCurrentLocation.x >= 0) && 
                 (suCurrentLocation.y >= 0) && 
                 (suCurrentLocation.x <= m_dMissionAreaWidth) && 
                 (suCurrentLocation.y <= m_dMissionAreaHeight))
            {
                // tell the robot to move a step
                SetRobotLocation(GetRobotID(), suCurrentLocation);
                ResetStepTaken(GetRobotID());
                send_robot_step_one_cycle(GetRobotID(), 1);

                // wait for the response
                WaitForRobotStep(GetRobotID());

                PumpEventQueue();
            }

            // see if we should quit drawing
            if (m_bStopDrawing)
            {
                break;
            }
        }

        // see if we should quit drawing
        if (m_bStopDrawing)
        {
            break;
        }
    }

    send_done_drawing_vector_field_msg(GetRobotID());

    // get rid of the stored obstacles
    if (oldRunType == REAL)
    {
        RestoreReadingsFromBackup();
    }

    // restore the running mode of the robot
    SetDrawingVectorField(GetRobotID(), false);
    SetRobotLocation(GetRobotID(), suRobotLoc);
    SetRunType(GetRobotID(), oldRunType);
}

void ClVectorField::DrawVectorField(int iRobotID, double dEdgeDistance, double dResolution)
{
    SetRobotID(iRobotID);
    SetEdgeDistance(dEdgeDistance);
    SetResolution(dResolution);
    DrawVectorField();
}

void ClVectorField::DrawVector(const Vector& suFrom, const Vector& suDisplacement)
{
    SuMovement suMovement;
    suMovement.suFrom = suFrom;
    suMovement.suDisplacement = suDisplacement;
    m_objVectorList.push_back(suMovement);

    ::DrawVector(suFrom, suDisplacement, true);
}

void ClVectorField::EraseVectors(void)
{
    int iSize = m_objVectorList.size();
    while (iSize > 0)
    {
        ::DrawVector(m_objVectorList.front().suFrom, m_objVectorList.front().suDisplacement, true);
        m_objVectorList.pop_front();
        iSize--;
    }
}

/**********************************************************************
 * $Log: VectorField.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.3  2006/05/15 02:07:35  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.2  2006/05/02 04:19:59  endo
 * TrackTask improved for the experiment.
 *
 * Revision 1.1.1.1  2005/02/06 23:00:12  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.1  2002/07/02 20:47:28  blee
 * Initial revision
 *
 *
 **********************************************************************/
