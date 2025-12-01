/**********************************************************************
 **                                                                  **
 **                     console_side_com.c                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: console_side_com.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "ipt/ipt.h"
#include "ipt/message.h"
#include "ipt/connection.h"

#include "gt_sim.h"
#include "gt_simulation.h"
#include "console_side_com.h"
#include "gt_ipt_msg_def.h"
#include "console.h"
#include "gt_console_db.h"
#include "gt_measure.h"
#include "env_change_defs.h"
#include "telop.h"
#include "../telop/status_window.h"
#include "mission_design.h"
#include "terrainmap.h"
#include "mlab_cnp.h"
#include "3d_vis_wid.h"
#include "gt_scale.h"
#include "bitmap_icon_displayer.h"

using std::string;
using std::vector;


float signalLevelAnt = 70;
//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const string CSC_EMPTY_STRING = "";

//------------------------------------------------------------------------------
// Typedefs
//------------------------------------------------------------------------------
typedef struct RobotIPTLink_t {
    int serverID;
    vector<int> clientIDs;
};

//------------------------------------------------------------------------------
// Variables
//------------------------------------------------------------------------------
char ipt_home[ 512 ];
char console_ipt_port[ 200 ];

IPCommunicator* communicator;

extern TerrainMap *Terrain;

const string EMPTY_STRING = "";
const double COMMUNICATOR_SLEEPTIME_SEC = 0.1;

vector<RobotIPTLink_t> robotIPTLinks;

//-----------------------------------------------------------------------
void csc_pickup(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    int robot_id;
    int object_id;

    REC_Pickup* rec = (REC_Pickup*) msg->FormattedData();
    robot_id = rec->robot_id;
    object_id = rec->object_id;

    if (debug)
    {
        fprintf(stderr,"csc_pickup(%d) %d\n", robot_id, object_id);
    }

    sim_pickup(robot_id, object_id);

}

//-----------------------------------------------------------------------
// probes an object to determine if it is safe or not
//-----------------------------------------------------------------------
void csc_probe(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    int robot_id;
    int object_id;

    REC_Probe* rec = (REC_Probe*) msg->FormattedData();
    robot_id = rec->robot_id;
    object_id = rec->object_id;

    if (debug)
    {
        fprintf(stderr,"csc_probe(%d) %d\n", robot_id, object_id);
    }

    sim_probe(robot_id, object_id);
}

//-----------------------------------------------------------------------
// This funcation checks the vehicle type of the object (if it is the
// robot).
//-----------------------------------------------------------------------
void csc_check_vehicle_type(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_CheckVehicleType *rec = NULL;
    REC_VehicleType reply;
    int robotID;
    int objectID;
    int vehicleType;

    rec = (REC_CheckVehicleType *)msg->FormattedData();
    robotID = rec->robotID;
    objectID = rec->objectID;

    vehicleType = sim_check_vehicle_type(robotID, objectID);

    reply.vehicleType = vehicleType;
    communicator->Reply(msg, MSG_VehicleType, &reply);

    if (debug)
    {
        fprintf(
            stderr,
            "csc_check_vehicle_type(%d) object[%d] = %s\n",
            robotID,
            objectID,
            ROBOT_MOBILITY_TYPE_NAMES[vehicleType].c_str());
    }
}

//-----------------------------------------------------------------------
// This function checks to see of the target object is trackable by this
// robot.
//-----------------------------------------------------------------------
void csc_check_target_untrackable(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_CheckTargetUntrackable *rec = NULL;
    REC_TargetUntrackable reply;
    int robotID;
    int objectID;
    bool untrackable;

    rec = (REC_CheckTargetUntrackable *)msg->FormattedData();
    robotID = rec->robotID;
    objectID = rec->objectID;

    untrackable = sim_check_target_trackable(robotID, objectID);

    reply.untrackable = untrackable;
    communicator->Reply(msg, MSG_TargetUntrackable, &reply);

    if (debug)
    {
        fprintf(
            stderr,
            "csc_check_target_untrackable(%d) object[%d] untrackable = %d\n",
            robotID,
            objectID,
            untrackable);
    }
}

//-----------------------------------------------------------------------
// alerts message.
//-----------------------------------------------------------------------
void csc_alert(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_Alert* rec = (REC_Alert*) msg->FormattedData();

    char* alert_message = rec->message;

    if (debug)
    {
        fprintf(stderr,"csc_alert: msg = \n%s\n", alert_message);
    }

    sim_alert(alert_message);
}

/****************************************************************************
* csc_ask_to_proceed: pops up the proceed window.                           *
*                                                                           *
****************************************************************************/
void csc_ask_to_proceed(IPCommunicator* comm, IPMessage* msg, void* hndData)
{

    REC_AskToProceed* rec = (REC_AskToProceed*) msg->FormattedData();

    int ask = rec->ask;
    int robot_id = rec->robot_id;

    if (debug)
    {
        fprintf(stderr,"csc_ask_to_proceed(): ask = %d, robot_id = %d\n", ask, robot_id);
    }

    fprintf(stderr, "\a");
    popup_start_proceed_dialog(TYPE_B);
}

/****************************************************************************
* csc_popup_telop_interface: pops up the telop interface.                   *
*                                                                           *
****************************************************************************/
void csc_popup_telop_interface(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_PopUpTelop* rec = (REC_PopUpTelop*) msg->FormattedData();

    int popup = rec->popup;
    int robot_id = rec->robot_id;

    if (debug)
    {
        fprintf(stderr,"csc_popup_telop_interface(): popup = %d, robot_id = %d\n", popup, robot_id);
    }

#ifdef TELOP
    fprintf(stderr, "\a");
    popup_telop_interface();
#else
    warn_userf("Error: Unable to execute TELEOPERATION command!\nTeleoperation code is disabled.");
#endif

}

/****************************************************************************
* csc_change_object_color: attempt to change the color of an object         *
*                                                                           *
****************************************************************************/
void csc_change_object_color(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_ChangeColor* rec = (REC_ChangeColor*) msg->FormattedData();
    int robot_id = rec->robot_id;
    int object_id = rec->object_id;
    char* new_color = rec->new_color;

    if (debug)
    {
        fprintf(stderr,"csc_change_color(%d) %d %s\n", robot_id, object_id, new_color);
    }

    sim_change_color(robot_id, object_id, new_color);

    // We send an acknowledgment so the robot can syncronize on the change
    REC_Ack ack;
    communicator->Reply(msg, MSG_Ack, &ack);
}

/****************************************************************************
* csc_change_robot_color: attempt to change the color of a robot            *
* *
 ****************************************************************************/

void csc_change_robot_color(IPCommunicator *comm, IPMessage *msg, void *hndData) {

    int robot_id;
    //int object_id;
    char *new_color;

    REC_ChangeRobotColor* rec = (REC_ChangeRobotColor *) msg->FormattedData();
    robot_id = rec->robot_id;
    new_color = rec->new_color;

    sim_change_robot_color(robot_id, new_color);

    // We send an acknowledgment so the robot can syncronize on the change
    REC_Ack ack;
    communicator->Reply(msg, MSG_Ack, &ack);
}

/****************************************************************************
* csc_add_object: add a new object to the world                             *
*                                                                           *
****************************************************************************/
void csc_add_object(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_AddObject* rec = (REC_AddObject*) msg->FormattedData();

    erase_robots();
    gt_add_object(rec->x, rec->y, rec->diameter, rec->color, (OBJ_STYLES)rec->style);
    draw_robots();

}

//-----------------------------------------------------------------------
// csc_add_passage_point: add a passage point on the overlay.
//-----------------------------------------------------------------------
void csc_add_passage_point(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_AddPassagePoint *rec = NULL;
    float x, y;
    char *label = NULL;
    const int DEFAULT_PP_DIAMETER_PIXEL = 15;

    rec = (REC_AddPassagePoint*) msg->FormattedData();;

    x = rec->x;
    y = rec->y;
    label = new char[(rec->labelSize)+1];
    memcpy(label, rec->label, rec->labelSize);
    label[rec->labelSize] = '\0';

    gt_add_passage_point(
        label,
        x,
        y,
        DEFAULT_PP_DIAMETER_PIXEL*meters_per_pixel);

    delete [] label;
    label = NULL;
}

//-----------------------------------------------------------------------
// csc_display_bitmap_icon: display bitmap icon on the overlay.
//-----------------------------------------------------------------------
void csc_display_bitmap_icon(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_DisplayBitmapIcon *rec = NULL;
    string bitmapName, label, enforcedFgColor, enforcedBgColor;
    float x, y;
    char *buf = NULL;
    int robotID;

    rec = (REC_DisplayBitmapIcon *) msg->FormattedData();;

    robotID = rec->robotID;
    x = rec->x;
    y = rec->y;

    buf = new char[(rec->bitmapNameSize)+1];
    memcpy(buf, rec->bitmapName, rec->bitmapNameSize);
    buf[rec->bitmapNameSize] = '\0';
    bitmapName = buf;
    delete [] buf;
    buf = NULL;

    buf = new char[(rec->labelSize)+1];
    memcpy(buf, rec->label, rec->labelSize);
    buf[rec->labelSize] = '\0';
    label = buf;
    delete [] buf;
    buf = NULL;

    buf = new char[(rec->enforcedFgColorSize)+1];
    memcpy(buf, rec->enforcedFgColor, rec->enforcedFgColorSize);
    buf[rec->enforcedFgColorSize] = '\0';
    enforcedFgColor = buf;
    delete [] buf;
    buf = NULL;

    buf = new char[(rec->enforcedBgColorSize)+1];
    memcpy(buf, rec->enforcedBgColor, rec->enforcedBgColorSize);
    buf[rec->enforcedBgColorSize] = '\0';
    enforcedBgColor = buf;
    delete [] buf;
    buf = NULL;

    gBitmapIconDisplayer->displayIcon(
        x,
        y,
        bitmapName,
        label,
        enforcedFgColor,
        enforcedBgColor);
}

/****************************************************************************
* csc_drop_in_basket: attempt to drop an object in a basket                 *
*                                                                           *
****************************************************************************/
void csc_drop_in_basket(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_DropInBasket* rec = (REC_DropInBasket*) msg->FormattedData();
    int robot_id = rec->robot_id;
    int object_id = rec->object_id;
    int basket_id = rec->basket_id;

    if (debug)
    {
        fprintf(stderr,"csc_drop_in_basket(%d) %d -> %d\n", robot_id, object_id, basket_id);
    }

    sim_drop_in_basket(robot_id, object_id, basket_id);
}

//-----------------------------------------------------------------------
void csc_move_robot(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    gt_Point v;

    REC_MoveRobot* rec = (REC_MoveRobot*) msg->FormattedData();
    v.x = rec->x;
    v.y = rec->y;
    v.z = rec->z;
    int id = rec->robot_id;


    if (debug)
    {
        fprintf(stderr,"csc_move_robot(%d) <%f %f %f>\n", id, v.x, v.y, v.z);
    }

    sim_move_robot(id, v);
}

//-----------------------------------------------------------------------
void csc_set_mobility_type(IPCommunicator *comm, IPMessage *msg, void* hndData)
{
    REC_SetMobilityType *rec = NULL;
    int robotID, mobilityType;

    rec = (REC_SetMobilityType *)msg->FormattedData();
    robotID = rec->robotID;
    mobilityType = rec->mobilityType;

    sim_set_mobility_type(robotID, mobilityType);
}

void csc_get_mobility(IPCommunicator *comm, IPMessage *msg, void* hndData) {
  REC_GetMobility *rec = NULL;
  REC_CurMobility rep;
  int robotID;

  rec = (REC_GetMobility *)msg->FormattedData();
  robotID = rec->robotID;
  rep.mobility = sim_get_mobility(robotID);

  comm->Reply(msg, MSG_CurMobility, &rep);

}


//-----------------------------------------------------------------------
void csc_spin_robot(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_SpinRobot* rec = (REC_SpinRobot*) msg->FormattedData();
    float w = rec->w; // angular velocity
    int  id = rec->robot_id;

    if (debug)
    {
        fprintf(stderr,"csc_spin_robot(%d) <%f>\n", id, w);
    }

    sim_spin_robot(id, w);
}

//-----------------------------------------------------------------------
void csc_get_elevation(IPCommunicator* comm, IPMessage* msg, void* hndData) {
  REC_CurElevation rep;

  REC_GetElevation *rec = (REC_GetElevation*) msg->FormattedData();

  rep.elevation = sim_get_elevation(rec->robot_id);

  comm->Reply(msg, MSG_CurElevation, &rep);
}


//-----------------------------------------------------------------------
// This function checks to see if the task for bidding is available. The
// robot ID will be used to check if the robot is in the exclusion list
// or not.
//-----------------------------------------------------------------------
void csc_cnp_got_a_task(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_CurTask rep;
    REC_GetTask *rec = NULL;
    MlabCNP_RequestMess_t mess;
    int robotID, errorNum = 0;
    bool hadError = false;
    const string DEFAULT_TASK_DESC = "None";
    const string DEFAULT_RESTRICTION = "None";
    const int DEFAULT_TASK_ID = -1;
    const int DEFAULT_ITERATION = -1;
    const bool DEBUG_CSC_CNP_GOT_A_TASK = false;

    // Convert the data.
    rec = (REC_GetTask *)msg->FormattedData();

    // Check the robot ID
    robotID = rec->robot_id;

    // Fill default values
    mess.TaskID = DEFAULT_TASK_ID;
    mess.Iteration = DEFAULT_ITERATION;
    mess.TaskDesc = strdup(DEFAULT_TASK_DESC.c_str());
    mess.Restriction = strdup(DEFAULT_RESTRICTION.c_str());

    // Check it.
    if (gMlabCNP != NULL)
    {
        gMlabCNP->checkForTask(robotID, &mess);
    }
    else
    {
        hadError = true;
        errorNum = 1;
    }

    // Copy the message
    rep.TaskID = mess.TaskID;
    rep.Iteration = mess.Iteration;
    rep.TaskDesc = strdup(mess.TaskDesc.c_str());
    rep.Restriction = strdup(mess.Restriction.c_str());

    // Reply the message
    comm->Reply(msg, MSG_CurTask, &rep);

    if (hadError)
    {
        fprintf(stderr, "Error(%d) in csc_cnp_got_a_task().\n", errorNum);
    }

    if (debug || DEBUG_CSC_CNP_GOT_A_TASK)
    {
      fprintf(stderr, "csc_cnp_got_a_task():\n");
      fprintf(stderr, " Task: [ID: %d] - %s:\n", rep.TaskID, rep.Restriction);
    }

    // Clean up
    free(rep.TaskDesc);
    free(rep.Restriction);
}

//-----------------------------------------------------------------------
// This function sends the task constraints
//-----------------------------------------------------------------------
void csc_cnp_send_task_constraints(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_CNP_TaskConstraintsData rec;
    string infoNameData = EMPTY_STRING, nameData = EMPTY_STRING, valueData = EMPTY_STRING;
    vector<CNP_TaskConstraint> cnpTaskConstraints;
    int i;
    int errorNum = 0;
    bool hadError = false;

    if (gMlabCNP != NULL)
    {
        cnpTaskConstraints = gMlabCNP->getCurrentCNPTaskConstraints();
    }
    else
    {
        hadError = true;
        errorNum = 1;
    }

    // Get the number of the constraints.
    rec.numConstraints = cnpTaskConstraints.size();

    // Allocate the data sizes
    rec.taskInfoIDs = new int[rec.numConstraints];
    rec.taskInfoNameSizes = new int[rec.numConstraints];
    rec.nameSizes = new int[rec.numConstraints];
    rec.types = new int[rec.numConstraints];
    rec.valueSizes = new int[rec.numConstraints];
    rec.taskInfoTotalNameSize = 0;
    rec.totalNameSize = 0;
    rec.totalValueSize = 0;

    for (i = 0; i < (rec.numConstraints); i++)
    {
        // Get the info
        rec.taskInfoIDs[i] = cnpTaskConstraints[i].taskInfo.id;
        rec.taskInfoNameSizes[i] = cnpTaskConstraints[i].taskInfo.name.size();
        rec.taskInfoTotalNameSize += rec.taskInfoNameSizes[i];
        infoNameData += cnpTaskConstraints[i].taskInfo.name;

        // Get the name size and its content.
        rec.nameSizes[i] = cnpTaskConstraints[i].strConstraintName.size();
        rec.totalNameSize += rec.nameSizes[i];
        nameData += cnpTaskConstraints[i].strConstraintName;

        // Get the type.
        rec.types[i] = cnpTaskConstraints[i].constraintType;

        // Get the value size and its conent.
        rec.valueSizes[i] = cnpTaskConstraints[i].strConstraintValue.size();
        rec.totalValueSize += rec.valueSizes[i];
        valueData += cnpTaskConstraints[i].strConstraintValue;
    }

    // Copy the info
    rec.taskInfoNameData = new char[(rec.taskInfoTotalNameSize)+1];
    memcpy(rec.taskInfoNameData, infoNameData.c_str(), rec.taskInfoTotalNameSize);
    rec.taskInfoNameData[rec.taskInfoTotalNameSize] = '\0';

    // Copy the constraint name data.
    rec.nameData = new char[(rec.totalNameSize)+1];
    memcpy(rec.nameData, nameData.c_str(), rec.totalNameSize);
    rec.nameData[rec.totalNameSize] = '\0';

    // Copy the value data.
    rec.valueData = new char[(rec.totalValueSize)+1];
    memcpy(rec.valueData, valueData.c_str(), rec.totalValueSize);
    rec.valueData[rec.totalValueSize] = '\0';

    // Send to the task.
    comm->Reply(msg, MSG_CNP_TaskConstraintsData, &rec);

    // Clean up the data.
    delete [] rec.taskInfoIDs;
    delete [] rec.taskInfoNameSizes;
    delete [] rec.taskInfoNameData;
    delete [] rec.nameSizes;
    delete [] rec.nameData;
    delete [] rec.types;
    delete [] rec.valueSizes;
    delete [] rec.valueData;

    if (hadError)
    {
        fprintf(stderr, "Error(%d) in csc_cnp_got_a_task().\n", errorNum);
    }
}

//-----------------------------------------------------------------------
// This function cancels the task.
//-----------------------------------------------------------------------
void csc_cnp_cancel_task(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_CancelTask *ct;
    CNP_Cancel canc;
    char mess[1024];

    ct = (REC_CancelTask *)msg->FormattedData();
    canc.TaskID = ct->TaskID;
    canc.Iteration = ct->Iteration;
    canc.RobotID = ct->RobotID;
    canc.Consensual = ct->Consensual;


    if (gMlabCNP != NULL)
    {
        if (ct->IsReneged)
        {
            canc.Info = ct->Info;

            sprintf(
                mess,
                "Robot(%d) renegs contract %d. Reason: %s\n",
                canc.RobotID,
                canc.TaskID,
                (canc.Info == NULL)? "Unknown" : canc.Info);

            sim_alert(mess);
            gMlabCNP->cancelTask(canc);
        }
        else
        {
            gMlabCNP->completeTask(ct->TaskID);
            sprintf(
                mess,
                "Robot(%d) completed task\n",
                canc.RobotID);
            sim_alert(mess);
        }

    }
}

//-----------------------------------------------------------------------
void csc_cnp_is_water(IPCommunicator* comm, IPMessage* msg, void* hndData) {
  REC_CurWater cw;
  REC_GetWater *gw;

  gw = (REC_GetWater *)msg->FormattedData();
  if (Terrain->Is_Water(gw->tx, gw->ty)) {
	cw.Water = 1;
  }
  else {
	cw.Water = 0;
  }
  comm->Reply(msg, MSG_CurWater, &cw);
}

//-----------------------------------------------------------------------
// This function bids the task
//-----------------------------------------------------------------------
void csc_cnp_task_bid(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_BidTask *bt;
    CNP_Offer offer;

    bt = (REC_BidTask *)msg->FormattedData();

    if (gMlabCNP == NULL)
    {
        fprintf(stderr, "Error in csc_cnp_task_bid().gMlabCNP is NULL.\n");
        return;
    }

    offer.TaskID = bt->TaskID;
    offer.Iteration = bt->Iteration;
    offer.RobotID = bt->RobotID;
    offer.TaskAccept = bt->TaskAccept;
    offer.Bid = bt->Bid;

    gMlabCNP->bidTask(offer);
}

//-----------------------------------------------------------------------
// This function gets an award
//-----------------------------------------------------------------------
void csc_cnp_get_award(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_GetAward *ga;
    REC_CurAward ca;
    CNP_Award currentAward;

    ga = (REC_GetAward *)msg->FormattedData();

    // Marshall the input
    currentAward.RobotID = ga->RobotID;
    currentAward.TaskID = ga->TaskID;
    currentAward.Iteration = ga->Iteration;

    // Find the record
    if (gMlabCNP != NULL)
    {
        // Robot ID should be filled by this function.
        gMlabCNP->getAward(&currentAward);
    }
    else
    {
        fprintf(stderr, "Error in csc_cnp_get_award().\n");
    }

    // Copy the data.
    ca.RobotID = currentAward.RobotID;
    ca.TaskID = currentAward.TaskID;
    ca.Iteration = currentAward.Iteration;

    // Send the reply.
    comm->Reply(msg, MSG_CurAward, &ca);
}

//-----------------------------------------------------------------------
// This function sends the initial robot constraints to the robot executable.
//-----------------------------------------------------------------------
void csc_cnp_send_robot_constraints(
    IPCommunicator* comm,
    IPMessage* msg,
    void* hndData)
{
    REC_CNP_RobotConstraintsRequest *req;
    REC_CNP_RobotConstraintsData rec;
    vector<CNP_RobotConstraint> cnpRobotConstraints;
    string nameData = EMPTY_STRING, valueData = EMPTY_STRING;
    int i;

    // Get the robot ID.
    req = (REC_CNP_RobotConstraintsRequest *)msg->FormattedData();
    rec.robotID = req->robotID;

    // Get the initial CNP robot constraints from the record.
    if (gMlabCNP != NULL)
    {
        cnpRobotConstraints = gMlabCNP->getInitCNPRobotConstraints(rec.robotID);
    }
    else
    {
        fprintf(stderr, "Warning: csc_cnp_send_robot_constraints(). gMlabCNP not enabled.\n");
    }

    // Get the number of the constraints.
    rec.numConstraints = cnpRobotConstraints.size();

    // Allocate the data sizes
    rec.nameSizes = new int[rec.numConstraints];
    rec.types = new int[rec.numConstraints];
    rec.valueSizes = new int[rec.numConstraints];
    rec.totalNameSize = 0;
    rec.totalValueSize = 0;

    for (i = 0; i < (rec.numConstraints); i++)
    {
        // Get the name size and its content.
        rec.nameSizes[i] = cnpRobotConstraints[i].strConstraintName.size();
        rec.totalNameSize += rec.nameSizes[i];
        nameData += cnpRobotConstraints[i].strConstraintName;

        // Get the type.
        rec.types[i] = cnpRobotConstraints[i].constraintType;

        // Get the value size and its conent.
        rec.valueSizes[i] = cnpRobotConstraints[i].strConstraintValue.size();
        rec.totalValueSize += rec.valueSizes[i];
        valueData += cnpRobotConstraints[i].strConstraintValue;
    }

    // Copy the name data.
    rec.nameData = new char[(rec.totalNameSize)+1];
    memcpy(rec.nameData, nameData.c_str(), rec.totalNameSize);
    rec.nameData[rec.totalNameSize] = '\0';

    // Copy the value data.
    rec.valueData = new char[(rec.totalValueSize)+1];
    memcpy(rec.valueData, valueData.c_str(), rec.totalValueSize);
    rec.valueData[rec.totalValueSize] = '\0';

    // Send to the robot.
    communicator->Reply(msg, MSG_CNP_RobotConstraintsData, &rec);

    // Clean up the data.
    delete [] rec.nameSizes;
    delete [] rec.types;
    delete [] rec.valueSizes;
    delete [] rec.nameData;
    delete [] rec.valueData;
}


//-----------------------------------------------------------------------
// This function saves the current task constraints.
//-----------------------------------------------------------------------
void csc_cnp_save_current_task_constraints(
    IPCommunicator* comm,
    IPMessage* msg,
    void* hndData)
{
    REC_CNP_TaskConstraintsData *data;
    CNP_TaskConstraint cnpTaskConstraint;
    list<CNP_TaskConstraint> cnpTaskConstraints;
    list<CNP_TaskConstraint>::const_iterator iterTC;
    char *buf = NULL;
    char *taskInfoNameData = NULL, *nameData = NULL, *valueData = NULL;
    char *taskInfoNameDataInitPtr = NULL, *nameDataInitPtr = NULL, *valueDataInitPtr = NULL;
    int i, numConstraints;
    int taskInfoNameSize, nameSize, valueSize;
    int taskInfoTotalNameSize, totalNameSize, totalValueSize;
    const CNP_TaskConstraint EMPTY_CNP_TASK_CONSTRAINT = {{0, string()}, string(), CNP_STRING, string()};
    const bool DEBUG_SAVE_CURRENT_TASK_CONSTRAINTS = false;

    // Get the formatted data.
    data = (REC_CNP_TaskConstraintsData *)msg->FormattedData();

    // Get some key numbers.
    numConstraints = data->numConstraints;
    taskInfoTotalNameSize = data->taskInfoTotalNameSize;
    totalNameSize = data->totalNameSize;
    totalValueSize = data->totalValueSize;

    // Allocate the size
    taskInfoNameData = new char[taskInfoTotalNameSize];
    nameData = new char[totalNameSize];
    valueData = new char[totalValueSize];

    // Copy the data
    memcpy(taskInfoNameData, data->taskInfoNameData, taskInfoTotalNameSize);
    memcpy(nameData, data->nameData, totalNameSize);
    memcpy(valueData, data->valueData, totalValueSize);

    // Remember the pointers
    taskInfoNameDataInitPtr = taskInfoNameData;
    nameDataInitPtr = nameData;
    valueDataInitPtr = valueData;

    // Copy the contents of the constraints
    for (i = 0; i < numConstraints; i++)
    {
        // Clear the previous values.
        cnpTaskConstraint = EMPTY_CNP_TASK_CONSTRAINT;

        // Copy the TaskInfo ID.
        cnpTaskConstraint.taskInfo.id = data->taskInfoIDs[i];

        // Copy the TaskInfo name.
        taskInfoNameSize = data->taskInfoNameSizes[i];
        buf = new char[taskInfoNameSize+1];
        memcpy(buf, taskInfoNameData, taskInfoNameSize);
        buf[taskInfoNameSize] = '\0';
        cnpTaskConstraint.taskInfo.name = buf;
        delete [] buf;
        taskInfoNameData += taskInfoNameSize;

        // Copy the constraint name.
        nameSize = data->nameSizes[i];
        buf = new char[nameSize+1];
        memcpy(buf, nameData, nameSize);
        buf[nameSize] = '\0';
        cnpTaskConstraint.strConstraintName = buf;
        delete [] buf;
        nameData += nameSize;

        // Copy the constraint type
        cnpTaskConstraint.constraintType = (CNP_ConstraintType)(data->types[i]);

        // Copy the constraint value
        valueSize = data->valueSizes[i];
        buf = new char[valueSize+1];
        memcpy(buf, valueData, valueSize);
        buf[valueSize] = '\0';
        cnpTaskConstraint.strConstraintValue = buf;
        delete [] buf;
        valueData += valueSize;

        // Put the constraint in the list
        cnpTaskConstraints.push_back(cnpTaskConstraint);
    }

    // Save the list in gMlabCNP.
    if (gMlabCNP != NULL)
    {
        gMlabCNP->saveCurrentCNPTaskConstraints(cnpTaskConstraints);
    }
    else
    {
        fprintf(
            stderr,
            "Error: csc_cnp_save_current_task_constraints(). gMlabCNP not enabled.\n");
    }

    delete [] taskInfoNameDataInitPtr;
    delete [] nameDataInitPtr;
    delete [] valueDataInitPtr;
    //delete msg;

    if (debug || DEBUG_SAVE_CURRENT_TASK_CONSTRAINTS)
    {
        fprintf(stderr, "csc_cnp_save_current_task_constraints:\n");

        i = 0;

        for (iterTC = cnpTaskConstraints.begin();
             iterTC != cnpTaskConstraints.end();
             iterTC++)
        {

            fprintf(
                stderr,
                "[%d] {%d, %s, %s, %d, %s}\n",
                i++,
                iterTC->taskInfo.id,
                iterTC->taskInfo.name.c_str(),
                iterTC->strConstraintName.c_str(),
                iterTC->constraintType,
                iterTC->strConstraintValue.c_str());
        }
    }
}

/****************************************************************************
* csc_robot_location - move the robot TO the indicated global position      *
*                                                                           *
****************************************************************************/
void csc_robot_location(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    gt_Point v;

    REC_RobotLocation* rec = (REC_RobotLocation*) msg->FormattedData();
    double theta = rec->heading;
    int id = rec->robot_id;
    v.x = rec->x;
    v.y = rec->y;

    if (debug)
    {
        fprintf(stderr,"csc_robot_location(%d) <%f %f>\n", id, v.x, v.y);
    }

    sim_robot_location(id, v, theta);
}

/****************************************************************************
* csc_battery_info - return battery information                             *
* trw zk                                                                    *
****************************************************************************/
void csc_battery_info(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    int level;
    int id;
    char level_string[5];

    REC_BatteryInfo* rec = (REC_BatteryInfo*) msg->FormattedData();
    level = rec->level;
    id = rec->id;

    sprintf(level_string, "%d", level);

    gt_update(id, "battery_level", level_string);
}

/****************************************************************************
* csc_gps_position - return gps postion and update GPS display window       *
* trw lilia                                                                 *
****************************************************************************/
void csc_gps_position(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    double latitude;
    double longitude;
    double direction;
    double pingtime;
    int pingloss;
    char cLatitude [15];
    char cLongitude [15];
    char cDirection [15];
    char cPingtime [15];
    char cPingloss [15];

    REC_GpsPosition* rec = (REC_GpsPosition*) msg->FormattedData();
    int id = rec->robot_id;
    latitude = rec->latitude;
    longitude = rec->longitude;
    direction = rec->direction;
    pingtime = rec->pingtime;
    pingloss = rec->pingloss;

    if (debug)
    {
        fprintf(stderr,"csc_gps_position(%d) <%f %f>\n", id, latitude, longitude);
    }

    if (latitude == 0)
    {
        sprintf(cLatitude, "N/A");
    }
    else
    {
        sprintf(cLatitude, "%.4f", latitude);
    }
    if (longitude == 0)
    {
        sprintf(cLongitude, "N/A");
    }
    else
    {
        sprintf(cLongitude, "%.4f", longitude);
    }

    sprintf(cDirection, "%.5f", direction);
    sprintf(cPingtime, "%.3f", pingtime);
    sprintf(cPingloss, "%d", pingloss);


    gt_update (id, "latitude", cLatitude);
    gt_update (id, "longitude", cLongitude);
    gt_update (id, "direction", cDirection);
    gt_update (id, "pingtime", cPingtime);
    gt_update (id, "pingloss", cPingloss);
    UpdateGPS (id);
}


/****************************************************************************
* csc_get_xy location of robot                                              *
*                                                                           *
****************************************************************************/
void csc_get_position(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    // Declare this as a structure. That way, the memory will be
    // returned to the stack when function exits.
    REC_RobotLocation reply;

    // Declare this as a pointer and IPT will take care of memory deallocation
    REC_GetPosition* request = (REC_GetPosition*) msg->FormattedData();


    robot_position in = sim_get_position(request->robot_id);
    reply.x = in.v.x;
    reply.y = in.v.y;
    reply.z = in.v.z;
    reply.heading = in.heading;

    if (debug)
    {
        fprintf(stderr,"csc_get_position(%d): robot loc=(%f %f %f) heading=%f\n",
                request->robot_id, reply.x, reply.y, reply.z, reply.heading);
    }

    communicator->Reply(msg, MSG_RobotLocation, &reply);

}

/****************************************************************************
* csc_get_map_position                                                      *
*                                                                           *
* get the position of a named map overlay feature                           *
****************************************************************************/
void csc_get_map_position(IPCommunicator* comm, IPMessage* msg, void* hndData)
{

    REC_InquireMapLocation* in = (REC_InquireMapLocation*)msg->FormattedData();
    REC_MapLocation out;

    gt_Measure* m = gt_get_db_measure(in->name);

    if (m)
    {
        gt_Point p;
        gt_center_of_measure(m, &p);

        out.pos.x = p.x;
        out.pos.y = p.y;
        out.valid = true;
    }
    else
    {
        out.valid = false;
    }

    communicator->Reply(msg, MSG_MapLocation, &out);

}

/************************************************************************/

void csc_detect_obstacles(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_ObstacleList reply;

    REC_DetectObstacles* rec = (REC_DetectObstacles*) msg->FormattedData();

    Obs_List rsp = sim_detect_obstacles(rec->robot_id, rec->max_dist);

    reply.num_obs = rsp.num_obs;
    reply.obs = new gt_Point[rsp.num_obs];
    reply.radius = new float[rsp.num_obs];

    for (int i = 0; i < reply.num_obs; i++)
    {
        reply.obs[i].x = rsp.obs[i].x;
        reply.obs[i].y = rsp.obs[i].y;
	reply.obs[i].z = rsp.obs[i].z;
        reply.radius[i] = rsp.r[i];
    }
    communicator->Reply(msg, MSG_ObstacleList, &reply);
    delete [] reply.obs;
    delete [] reply.radius;
}

/************************************************************************/

void csc_detect_room(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_DetectRoom* rec = (REC_DetectRoom*) msg->FormattedData();

    bool rsp = sim_detect_in_room(rec->robot_id, rec->robot_loc);
    communicator->Reply(msg, MSG_DetectRoomReply, &rsp);
}

/************************************************************************/

void csc_detect_doorway(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_DetectDoorway* rec = (REC_DetectDoorway*) msg->FormattedData();

    DoorwayList rsp = sim_detect_doorway(rec->robot_id, rec->sensor_dir,
                                          rec->sensor_fov, rec->sensor_range);
    communicator->Reply(msg, MSG_DetectDoorwayReply, &rsp);
    if (rsp.num_doors)
    {
        free(rsp.doors);
    }
}

/************************************************************************/

void csc_mark_doorway(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_MarkDoorway* rec = (REC_MarkDoorway*) msg->FormattedData();

    sim_markunmark_nearest_doorway(rec->robot_id, rec->mark_status);
}


/************************************************************************/

void csc_detect_hallway(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_DetectHallway* rec = (REC_DetectHallway*) msg->FormattedData();

    HallwayList rsp = sim_detect_hallway(rec->robot_id, rec->robot_loc);
    communicator->Reply(msg, MSG_DetectHallwayReply, &rsp);
    if (rsp.num_hways)
    {
        free(rsp.hways);
    }
}

/************************************************************************/

void csc_detect_objects(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_DetectObjects* rec = (REC_DetectObjects*) msg->FormattedData();
    REC_ObjectList reply;

    ObjectList rsp = sim_detect_objects(rec->robot_id, rec->max_dist);

    reply.num_objects = rsp.count;
    reply.object = (REC_ObjectReading*) calloc(rsp.count, sizeof(REC_ObjectReading));

    for (int i = 0; i < rsp.count; i++)
    {
        reply.object[i].object_color = rsp.objects[i].objcolor;
        reply.object[i].id = rsp.objects[i].id;
        reply.object[i].object_shape = rsp.objects[i].objshape;
        reply.object[i].x1 = rsp.objects[i].x1;
        reply.object[i].y1 = rsp.objects[i].y1;
        reply.object[i].r = rsp.objects[i].r;
        reply.object[i].x2 = rsp.objects[i].x2;
        reply.object[i].y2 = rsp.objects[i].y2;
        reply.object[i].ismovable = rsp.objects[i].ismovable;

    }

    communicator->Reply(msg, MSG_ObjectList, &reply);

    free(reply.object);

    if (debug)
    {
        fprintf(stderr,"console: csc_detect_objects(%d): max_dist=%.1f Meters\n",
                rec->robot_id, rec->max_dist);

        for(int i = 0; i < rsp.count; i++)
        {
            fprintf(stderr,"  %2d: <%.1f %.1f> %.1f\n",
                    rsp.objects[i].id, rsp.objects[i].x1, rsp.objects[i].y1,
                    rsp.objects[i].r);
        }

        fprintf(stderr,"\n");
    }

}

/****************************************************************************/

void csc_robot_death(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_RobotDeath* rec = (REC_RobotDeath*) msg->FormattedData();

    if (debug)
    {
        fprintf(stderr,"Robot %d has died: %s\n",rec->robot_id, rec->message);
    }

    // delete the record and graphic
    delete_robot_rec(rec->robot_id);
}


//-----------------------------------------------------------------------
// This callback function is called to terminate the entire mission by
// halting the mlab process.
//-----------------------------------------------------------------------
void csc_terminate_mission(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_TerminateMission *rec;

    rec = (REC_TerminateMission *)msg->FormattedData();

    if (debug)
    {
        fprintf(
            stderr,
            "csc_terminate_mission: called by Robot(%d)\n",
            rec->robotID);
    }

    // Add quit_mlab() as a work process. This will prevent IPT to hang.
    call_quit_mlab();
}

/****************************************************************************
*                                                                           *
* csc_detect_robots returns location of the other visible robots            *
*                                                                           *
****************************************************************************/

void csc_detect_robots(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    int i;

    REC_DetectRobots* rec = (REC_DetectRobots*) msg->FormattedData();

    RobotReadings rtn = sim_detect_robots(rec->robot_id, rec->max_dist);

    REC_CurRobots reply;
    reply.num_robots = rtn.num_readings;
    if (reply.num_robots)
    {
        reply.robots = (gt_Id_and_v*) calloc(reply.num_robots, sizeof(gt_Id_and_v));
        for (i = 0; i < reply.num_robots; i++)
        {
            reply.robots[ i ].id  = rtn.readings[ i ].id;
            reply.robots[ i ].v.x = rtn.readings[ i ].v.x;
            reply.robots[ i ].v.y = rtn.readings[ i ].v.y;
            reply.robots[ i ].v.z = rtn.readings[ i ].v.z;
        }
    }
    else
    {
        reply.robots = NULL;
    }

    communicator->Reply(msg, MSG_CurRobots, &reply);

    if (debug)
    {
        fprintf(stderr, "DETECT_ROBOTS(%d)\n", rec->robot_id);
        for (i = 0; i < rtn.num_readings; i++)
        {
            fprintf(stderr, "   %d: (%.1f %.1f %.1f)\n",
                     rtn.readings[ i ].id,
                     rtn.readings[ i ].v.x,
                     rtn.readings[ i ].v.y,
                     rtn.readings[ i ].v.z);
        }
    }
}


/****************************************************************************/

void csc_robot_put_state(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_PutState* rec = (REC_PutState*) msg->FormattedData();

    gt_update(rec->robot_id, rec->key, rec->value);

    if (debug)
    {
        fprintf(stderr,"ROBOT_PUT_STATE(%d): '%s'='%s'\n",
                 rec->robot_id, rec->key, rec->value);
    }
}

/****************************************************************************/

void csc_robot_birth(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    IPConnection* com_link;

    REC_RobotBirth* rec = (REC_RobotBirth*) msg->FormattedData();

    if (debug)
    {
        fprintf(stderr, "csc_robot_birth: robot_id=%d <%.1f %.1f> %s %s\n",
                 rec->robot_id, rec->start_x, rec->start_y, rec->port_name,
                 rec->com_version);
    }

    if (strcmp(rec->com_version, ComVersion) != 0)
    {
        fprintf(stderr,"robot com version %s does not match console com version %s\n",
                 rec->com_version, ComVersion);

        // Kill it, it is not up to our high standards.
        if ((com_link = sim_get_com_link(rec->robot_id)) != NULL)
        {
            communicator->SendMessage(com_link, MSG_RobotSuicide, &(rec->robot_id));
        }

        // delete the record and graphic
        delete_robot_rec(rec->robot_id);

        console_error = ROBOT_VERSION_MISMATCH;
    }
    else
    {
        gt_Point p;
        p.x = rec->start_x;
        p.y = rec->start_y;
        p.z = rec->start_z;

        // connect to the robot module using ipt
        com_link = communicator->Connect(rec->port_name, IPT_REQUIRED);

        sim_robot_birth(rec->robot_id, p, rec->start_heading, com_link);

	// Now that the robot is running and ready
	// Send the initialization values for lat/lon to X/Y mapping

	char cLat[16], cLon[16], cMapW[16], cMapH[16], cMapScale[16];

	sprintf(cLat, "%.6f", g_ref_latitude );
	sprintf(cLon, "%.6f", g_ref_longitude);
	sprintf(cMapH, "%.6f", map_width);
	sprintf(cMapW, "%.6f", map_height);
	sprintf(cMapScale, "%.6f", map_scale);

	gt_update(rec->robot_id, "ref_latitude", cLat );
	gt_update(rec->robot_id, "ref_longitude", cLon);
	gt_update(rec->robot_id, "map_width", cMapW);
	gt_update(rec->robot_id, "map_height", cMapH);
	gt_update(rec->robot_id, "map_scale", cMapScale);

    }
}

/****************************************************************************/

void csc_notify_config(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_NotifyConfig* rec = (REC_NotifyConfig*) msg->FormattedData();

    if (debug)
    {
        fprintf(stderr,"csc_notify_config: robot_id=%d %d %d\n",
                 rec->robot_id, rec->robot_type, rec->run_type);
    }

    // Link up robots among themselves.
    link_robots(comm);

    sim_notify_config(rec->robot_id, rec->robot_type, rec->run_type);
}

/****************************************************************************/

void csc_report_sensors(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_SensorReadings* rec = (REC_SensorReadings*) msg->FormattedData();

    if (debug)
    {
        fprintf(stderr,"csc_report_sensors(%d): %d readings\n",
                rec->robot_id, rec->num_readings);
    }

    sim_report_sensors(rec->robot_id, rec->num_readings, (SensorReading*) rec->reading);
    status_report_sensors(rec->robot_id, rec->num_readings, (SensorReading*) rec->reading);  // trw zk
}

/****************************************************************************
* csc_reset_world:                                                          *
*                                                                           *
****************************************************************************/
void csc_reset_world(IPCommunicator *comm, IPMessage *msg, void *hndData)
{
    REC_ResetWorld* rec = (REC_ResetWorld*) msg->FormattedData();
    int  lnp = rec->load_new_map;
    ResetWorld(rec->new_overlay, lnp);
}

/****************************************************************************/

void csc_report_laser(IPCommunicator* comm, IPMessage* msg, void* hndData)
{

    REC_LaserReadings* rec = (REC_LaserReadings*) msg->FormattedData();

    if (debug)
    {
        fprintf(stderr,"csc_report_laser(%d)\n", rec->robot_id);
    }

    sim_report_laser(rec->robot_id, rec->num_readings, rec->readings, rec->angles);
}

/****************************************************************************/

void csc_update_lm_params(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_UpdateLMParams* rec = (REC_UpdateLMParams*) msg->FormattedData();

    if (debug)
    {
        fprintf(stderr, "csc_update_lm_params: id = %d\n", rec->iRobotID);
    }

    UpdateLMParamsFor(rec->iRobotID);
}

/****************************************************************************
*                                                                           *
* csc_reported_state                                                        *
*  * Once mlab received the information, containing the what state          *
*    the robot is currently in, from robot_side_com.c through iptserver,    *
*    it will call report_user(msg) to popup the scrollable text window    *
*    for report_current_state.                                              *
*                                                                           *
****************************************************************************/

void csc_reported_state(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_ReportedState* rec = (REC_ReportedState*) msg->FormattedData();

    if (debug)
    {
        fprintf(stderr,"csc_reported_state: msg = \n%s\n", rec->message);
    }

    report_user(rec->message);
}

//-----------------------------------------------------------------------
// This callback function is called when the robot creates a new logfile.
//-----------------------------------------------------------------------

void csc_reported_logfile(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_ReportedLogfile *rec;
    string logfileName;

    rec = (REC_ReportedLogfile*) msg->FormattedData();
    logfileName = rec->logfileName;

    if (debug)
    {
        fprintf(
            stderr,
            "csc_reported_logfile: logfileName = %s\n",
            logfileName.c_str());
    }

    if (gUseMlabMissionDesign)
    {
        gMMD->saveLogfile(logfileName);
    }
}

/****************************************************************************
* csc_ping_console							    *
*  * This function gets called when the robot pings mlab. It simply         *
*    returns the receive message to the robot.                              *
*                                                                           *
************************************************************************/
void csc_ping_console(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_PingSend* rec = (REC_PingSend*) msg->FormattedData();
    REC_PingBack rsp;

    rsp.check = rec->check;

    communicator->Reply(msg, MSG_PingBack, &rsp);
}

/****************************************************************************
* csc_step_taken                                                            *
*                                                                           *
****************************************************************************/

void csc_step_taken(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    static int iRobotID = -1;

    REC_StepTaken* rec = (REC_StepTaken*) msg->FormattedData();

    if (debug)
    {
        fprintf(stderr, "csc_step_taken: robot id = %d, step = %d)\n", rec->iRobotID, rec->iStep);
    }

    // multiple robots may be sending the step information, but we only want to count
    // each step once, so we'll remember the ID of the first robot that the message
    // comes from and then only pay attention to messages from that robot
    if (iRobotID == -1)
    {
        iRobotID = rec->iRobotID;
    }

    if (iRobotID == rec->iRobotID)
    {
        ChangeEnv(rec->iStep);
    }
}


/****************************************************************************
* csc_csb_get_robot_pos                                                            *
*                                                                           *
****************************************************************************/

void csc_csb_get_robot_pos(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
	REC_GetRobotsPos *rec = (REC_GetRobotsPos *)msg->FormattedData();
	REC_RobotsPosReply reply;

	RobotReadings rtn = sim_get_robot_pos(rec->robot_id, rec->max_dist);
	reply.numRobots = rtn.num_readings;

	if (reply.numRobots)
	{
		reply.robots = (robot_pos_id_t *) calloc(reply.numRobots, sizeof(robot_pos_id_t));
	    for (int i = 0; i < reply.numRobots; i++)
	    {
	    	reply.robots[ i ].id  = rtn.readings[ i ].id;
	        reply.robots[ i ].x = rtn.readings[ i ].v.x;
	    	reply.robots[ i ].y = rtn.readings[ i ].v.y;
	    	reply.robots[ i ].z = rtn.readings[ i ].v.z;
	    	reply.robots[ i ].heading = rtn.readings[ i ].heading;
	    	//printf("Robot %d:  X: %g  Y: %g  Z: %g  Heading: %g\n", reply.robots[ i ].id,
	    	//		reply.robots[ i ].x, reply.robots[ i ].y, reply.robots[ i ].z,
	    	//		reply.robots[ i ].heading);
	    }
	}
	else
		reply.robots = NULL;

	comm->Reply(msg, MSG_RobotsPosReply, &reply);
	if (reply.numRobots)
		free(reply.robots);
}

void csc_csb_get_signal_strength(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
	REC_SignalStrength reply;
	REC_GetSignalStrength *rec = (REC_GetSignalStrength *)msg->FormattedData();
	if(rec->filter==0)
	{
		//Emisor en la posición (5,5) Señal decreciente con la distancia
		robot_position position = sim_get_position(rec->robotId);
		reply.value = 100-(2*sqrt((position.v.x-5)*(position.v.x-5) + (position.v.y-5)*(position.v.y-5)));
		comm->Reply(msg, MSG_SignalStrength, &reply);
	}
	else
	{
		int i, j;
		float signalLevel;
		char line[200];
		FILE *f;

		//Caso real, devuelvo la señal entre el teleoperador y el router que va en el robot
	   	system("iwconfig 2> /dev/null | grep \"Link Quality\" > salida.txt");
		f = fopen("salida.txt", "r");
		if (f != NULL)
		{
			if (fgets(line, 200, f) != NULL)
			{
				for (i = 0; line[i] != '='; i++);
				for (j = i; line[j] != '/'; j++);
				line[j] = '\0';
				signalLevel = atof(&line[i + 1]);
			}
			else
			{
				signalLevel = 0;
			}

			fclose(f);
			unlink("salida.txt");
		}
		else
		{
			signalLevel = 0;
		}

    	printf("Señal real: %g Filtro: %g\n", signalLevel, rec->filter);

    	if(signalLevel==0)
    		signalLevel=signalLevelAnt;

    	signalLevel = rec->filter*signalLevelAnt+(1-rec->filter)*signalLevel;

    	reply.value = signalLevel;
		signalLevelAnt = signalLevel;
		comm->Reply(msg, MSG_SignalStrength, &reply);
	}
}


/****************************************************************************/

// These commands are called by the console

/****************************************************************************/

void send_robot_suicide_msg(int robot_id)
{
    if (debug)
    {
        fprintf(stderr,"send_robots_suicide_msg\n");
    }

    // OK, probably safe to kill it
    if (debug)
    {
        fprintf(stderr, "send_suicide_msg to robot %d\n", robot_id);
    }

    IPConnection* conn = sim_get_com_link(robot_id);

    if (conn != NULL)
    {
        REC_RobotSuicide msg = robot_id;
        communicator->SendMessage(conn, MSG_RobotSuicide, &msg);
    }

    // delete the record and graphic
    delete_robot_rec(robot_id);
}

/****************************************************************************/

void send_robots_suicide_msg()
{
    int robot_id;
    double SUICIDE_INTERVAL_SEC = 0.1;

    do
    {
        // OK, probably safe to kill one
        if ((robot_id = find_first_robot_rec()) >= 0)
        {
            send_robot_suicide_msg(robot_id);
        }

        // Be very careful to not send messages to any robots dying while we
        // are in this routine
        communicator->Sleep(SUICIDE_INTERVAL_SEC);

    } while (robot_id >= 0);

    if (debug)
    {
        fprintf(stderr,"robots halting\n");
    }
}


/****************************************************************************/

void send_robot_pause_state(int robot_id, int val)
{
    IPConnection* conn = sim_get_com_link(robot_id);
    if (conn != NULL)
    {
        REC_RobotPause msg = val;
        communicator->SendMessage(conn, MSG_RobotPause, &msg);
    }

    if (debug)
    {
        fprintf(stderr,"send_robot_pause_state: robot(%d) is %s\n",
                 robot_id, val ? "paused":"running");
    }
}

/****************************************************************************/

void send_robots_pause_state(int val)
{
    int robot_id = find_first_robot_rec();

    while (robot_id >= 0)
    {
        send_robot_pause_state(robot_id, val);
        robot_id = find_next_robot_rec();
    }

    if (debug)
    {
        fprintf(stderr,"robots %s\n", val ? "paused":"running");
    }
}

/****************************************************************************/
void send_robot_feedback_state(int robot_id, int val)
{
    IPConnection *conn = NULL;
    REC_RobotFeedback msg;

    conn = sim_get_com_link(robot_id);

    if (conn != NULL)
    {
        msg = val;
        communicator->SendMessage(conn, MSG_RobotFeedback, &msg);
    }
}

/****************************************************************************/
void send_robots_feedback_state(int val)
{
    int robot_id = find_first_robot_rec();

    while (robot_id >= 0)
    {
        send_robot_feedback_state(robot_id, val);
        robot_id = find_next_robot_rec();
    }
}

/****************************************************************************/

void send_robot_step_one_cycle(int robot_id, int val)
{
    IPConnection* conn = sim_get_com_link(robot_id);
    if (conn != NULL)
    {
        REC_StepOneCycle msg = val;
        communicator->SendMessage(conn, MSG_StepOneCycle, &msg);
    }
}

/****************************************************************************/

void send_robots_step_one_cycle(int val)
{
    int robot_id = find_first_robot_rec();

    while (robot_id >= 0)
    {
        send_robot_step_one_cycle(robot_id, val);
        robot_id = find_next_robot_rec();
    }
}

/****************************************************************************/

void send_robot_debug_state(int robot_id, int val)
{
    IPConnection* conn = sim_get_com_link(robot_id);
    if (conn != NULL)
    {
        REC_RobotDebug msg = val;
        communicator->SendMessage(conn, MSG_RobotDebug, &msg);
    }

    if (debug)
    {
        fprintf(stderr,"robot(%d): debug is %s\n", robot_id, val ? "on":"off");
    }
}

/****************************************************************************/

void send_robots_debug_state(int val)
{
    int robot_id = find_first_robot_rec();

    while (robot_id >= 0)
    {
        send_robot_debug_state(robot_id, val);
        robot_id = find_next_robot_rec();
    }

    if (debug)
    {
        fprintf(stderr,"robots: debug is %s\n", val ? "on":"off");
    }
}

/****************************************************************************
*                                                                           *
* send_robot(s)_report_state                                                *
*  * This functions will be called when "Report Current State" feature      *
*     was enabled. They will send the "Turn-On" signal to robot-executable  *
*     (hardware_drivers/robot_side_com.c) via iptserver.                    *
*                                                                           *
****************************************************************************/

void send_robot_report_state(int robot_id, int val)
{
    IPConnection* conn = sim_get_com_link(robot_id);
    if (conn != NULL)
    {
        communicator->SendMessage(conn, MSG_ReportState, &val);
    }

    if (debug)
    {
        fprintf(stderr,"robot(%d): report current state is %s\n", robot_id, val ? "on":"off");
    }
}

/****************************************************************************/

void send_robots_report_state(int val)
{
    int robot_id = find_first_robot_rec();

    while (robot_id >= 0)
    {
        send_robot_report_state(robot_id, val);
        robot_id = find_next_robot_rec();
    }

    if (debug)
    {
        fprintf(stderr,"robots: report current state is %s\n", val ? "on":"off");
    }
}

/****************************************************************************
*                                                                           *
* send_robots_proceed_mission                                               *
*  * This function broadcast to the robots to proceed the mission using     *
*    sim_put_state() function.                                              *
*                                                                           *
****************************************************************************/
void send_robots_proceed_mission()
{
    sim_put_state(BROADCAST_ROBOTID, "CONSOLE->ROBOT PROCEED MISSION", "1");
}


/****************************************************************************
*                                                                           *
* send_robots_db_value                                                      *
*  * This function broadcast to the robots to change the specified db       *
*    value using                                                            *
*    sim_put_state() function.                                              *
*                                                                           *
****************************************************************************/
void send_robots_db_value(char * key, char *value)
{
    sim_put_state(BROADCAST_ROBOTID, key, value);
}

/****************************************************************************/

void send_drawing_vector_field_msg(int iID)
{
    IPConnection* conn = sim_get_com_link(iID);
    if (conn != NULL)
    {
        REC_DrawingVectorField rec = 0;
        communicator->SendMessage(conn, MSG_DrawingVectorField, &rec);
    }
}

/****************************************************************************/

void send_done_drawing_vector_field_msg(int iID)
{
    IPConnection* conn = sim_get_com_link(iID);
    if (conn != NULL)
    {
        REC_DoneDrawingVectorField rec = 0;
        communicator->SendMessage(conn, MSG_DoneDrawingVectorField, &rec);
    }
}

/****************************************************************************/

void send_robot_report_sensor_readings(int iID)
{
    IPConnection* conn = sim_get_com_link(iID);
    if (conn != NULL)
    {
        REC_ReportSensorReadings rec = 0;
        communicator->SendMessage(conn, MSG_ReportSensorReadings, &rec);
    }
}

/****************************************************************************/

void new_command(int robot_id, gt_Command* cmd)
{
    int i = 0;

    REC_NewCommand rec;

    // set all the path points
    rec.num_path_points = cmd->num_path_points;
    if (rec.num_path_points != 0)
    {
        rec.path = (gt_Point*) calloc(rec.num_path_points, sizeof(gt_Point));
        for(i = 0; i < rec.num_path_points; i++)
        {
            rec.path[ i ].x = cmd->path[ i ].x;
            rec.path[ i ].y = cmd->path[ i ].y;
        }
    }
    else
    {
        rec.path = NULL;
    }

    // set all units
    rec.unit_size = cmd->unit_size;
    if (rec.unit_size != 0)
    {
        rec.unit = (int*) calloc(rec.unit_size, sizeof(int));
        for(i = 0; i < rec.unit_size; i++)
        {
            rec.unit[ i ] = cmd->unit[ i ];
        }
    }
    else
    {
        rec.unit = NULL;
    }

    // set the phase lines
    rec.num_pl_points = cmd->num_pl_points;
    if (rec.num_pl_points != 0)
    {
        rec.phase_line = (gt_Point*) calloc(rec.num_pl_points, sizeof(gt_Point));
        for(i=0; i<rec.num_path_points; i++)
        {
            rec.phase_line[ i ].x = cmd->phase_line[ i ].x;
            rec.phase_line[ i ].y = cmd->phase_line[ i ].y;
        }
    }
    else
    {
        rec.phase_line = NULL;
    }

    // set everything else
    rec.goal.x           = cmd->goal.x;
    rec.goal.y           = cmd->goal.y;
    rec.behavior         = cmd->behavior;
    rec.formation        = cmd->formation;
    rec.technique        = cmd->technique;
    rec.speed            = cmd->speed;
    rec.width            = cmd->width;
    rec.phase_line_given = cmd->phase_line_given;
    rec.time             = cmd->time;

    if (cmd->phase_line_ack_msg != NULL)
    	rec.phase_line_ack_msg = strdup(cmd->phase_line_ack_msg);
    else
    	rec.phase_line_ack_msg = strdup("");;

    rec.wait_at_phase_line = cmd->wait_at_phase_line;
    rec.completion_given = cmd->completion_given;

    if (cmd->completion_msg != NULL)
    	rec.completion_msg = strdup(cmd->completion_msg);
    else
    	rec.completion_msg = strdup("");;

    rec.freeze_on_completion = cmd->freeze_on_completion;
    rec.until_timeout_given = cmd->until_timeout_given;
    rec.robot_id = cmd->robot_id;
    rec.seq_num = cmd->seq_num;


    IPConnection* conn;
    if (robot_id == -1)
    {
        int id = find_first_robot_rec();

        while (id >= 0)
        {
            conn = sim_get_com_link(id);
            if (conn != NULL)
            {
                communicator->SendMessage(conn, MSG_NewCommand, cmd);
            }

            id = find_next_robot_rec();
        }
    }
    else
    {
        conn = sim_get_com_link(robot_id);
        if (conn != NULL)
            communicator->SendMessage(conn, MSG_NewCommand, cmd);
    }

    if (rec.path != NULL)
    {
        free(rec.path);
    }

    if (rec.unit != NULL)
    {
        free(rec.unit);
    }

    if (rec.phase_line != NULL)
    {
        free(rec.phase_line);
    }

    if (rec.phase_line_ack_msg != NULL)
    {
        free(rec.phase_line_ack_msg);
    }

    if (rec.completion_msg != NULL)
    {
        free(rec.completion_msg);
    }

}

/****************************************************************************/

void sim_put_state(int robot_id, char* key, char* value)
{
    REC_NewState rec;
    IPConnection* conn;

    rec.key = key;
    rec.value = value;

    if (robot_id == BROADCAST_ROBOTID)
    {
        // send a copy to all the robots
        robot_id = find_first_robot_rec();

        while (robot_id >= 0)
        {
            conn = sim_get_com_link(robot_id);
            if (conn != NULL)
            {
                communicator->SendMessage(conn, MSG_NewState, &rec);

                if (debug)
                {
                    fprintf(stderr,"sim_put_state(%d, '%s' = '%s'\n",robot_id, key, value);
                }
            }
            robot_id = find_next_robot_rec();
        }
    }
    else
    {
        // send a copy to the desired robot
        conn = sim_get_com_link(robot_id);
        if (conn != NULL)
        {
            communicator->SendMessage(conn, MSG_NewState, &rec);
        }

        if (debug)
        {
            fprintf(stderr,"sim_put_state(%d, '%s' = '%s'\n",robot_id, key, value);
        }
    }
}

/****************************************************************************
*                                                                           *
* link_robots(IPCommunicator* comm)                                         *
*  * This functions will tell the robots to connect each other by establi-  *
*    shing a server-client relationship. Each robot is, therefore, a server *
*    and client of all other robots.                                        *
*                                                                           *
****************************************************************************/

void link_robots(IPCommunicator* comm)
{
    IPConnection* conn_client = NULL;
    IPConnection* conn_server = NULL;
    RobotIPTLink_t robotIPTLink;
    int robot_id = find_first_robot_rec();
    int biggest_robot_id = robot_id;
    int server_id, client_id, robotIPTLinkIndex = 0;
    int i, j;
    bool linkedAlready = false;

    // Looking for a robot_id which is largest.
    while (robot_id > 0)
    {
        robot_id = find_next_robot_rec();
        if (robot_id > biggest_robot_id)
        {
            biggest_robot_id = robot_id;
        }
    }

    // Connect robots for all combinations.
    for (server_id = 0; server_id <= biggest_robot_id; server_id++)
    {
        robotIPTLinkIndex = -1;

        for (i = 0; i < (int)(robotIPTLinks.size()); i++)
        {
            if (robotIPTLinks[i].serverID == server_id)
            {
                robotIPTLinkIndex = i;
                break;
            }
        }

        if (robotIPTLinkIndex < 0)
        {
            robotIPTLinkIndex = robotIPTLinks.size();
            robotIPTLink.serverID = server_id;
            robotIPTLink.clientIDs.clear();
            robotIPTLinks.push_back(robotIPTLink);
        }

        //for (int client_id = 1; client_id<=biggest_robot_id; client_id++)
        for (client_id = 0; client_id<=biggest_robot_id; client_id++)
        {
            linkedAlready = false;

            for (j = 0; j < (int)(robotIPTLinks[robotIPTLinkIndex].clientIDs.size()); j++)
            {
                if (robotIPTLinks[robotIPTLinkIndex].clientIDs[j] == client_id)
                {
                    linkedAlready = true;
                    break;
                }
            }

            if ((!linkedAlready) && (server_id != client_id))
            {
                conn_client = sim_get_com_link(client_id);
                conn_server = sim_get_com_link(server_id);

                if ((conn_server != NULL) && (conn_client != NULL))
                {
                    REC_Declare_iptClient rec;

                    rec.server_id = server_id;
                    rec.server_name = strdup(conn_server->Name());

                    if (rec.server_name != NULL)
                    {
                        if (debug)
                        {
                            fprintf(stderr, "link_robots: server(%d, %s), client(%d)\n",
                                    rec.server_id, rec.server_name, client_id);
                        }

                        comm->SendMessage(conn_client, MSG_Declare_iptClient, &rec);

                        robotIPTLinks[robotIPTLinkIndex].clientIDs.push_back(client_id);
                    }
                }
            }
        }
    }
}

/****************************************************************************
****************************************************************************/

static int com_open = false;
void gt_init_communication(char* suggested_ipt_home)
{
    char hostname[ 100 ];
    int i;

    // if the user didn't specify a host machine, look in the environment
    if (suggested_ipt_home == NULL)
    {
        suggested_ipt_home = getenv("IPTHOST");
    }
    if (suggested_ipt_home == NULL)
    {
        suggested_ipt_home = getenv("TCXHOST");
    }

    // if none specified, the current machine will be the default
    if (suggested_ipt_home == NULL)
    {
        suggested_ipt_home = getenv("HOST");
    }

    // if HOST wasn't set either, use the hostname as the last resort
    if (suggested_ipt_home == NULL)
    {
        if (gethostname(hostname, 100) != 0)
        {
            fprintf(stderr, "Aborting: Unable to determine host for IPT server!\n");
            exit(1);
        }
        suggested_ipt_home = hostname;
    }

    // save it for the robots
    strcpy(ipt_home,suggested_ipt_home);

    // create a unique name for ourselves
    sprintf(console_ipt_port,"%s_%d",DefaultConsoleName, getpid());

    // setup ipt connection
    if (debug)
    {
        fprintf(stderr, "iptCommunicatorInstance(%s,%s)\n", console_ipt_port, ipt_home);
    }
    communicator = IPCommunicator::Instance(console_ipt_port, ipt_home);

    // register message formatters
    communicator->RegisterNamedFormatters(formatArray);

    // register message formats
    communicator->RegisterMessages(messageArray);

    // register handlers
    IPMsgHandlerSpec hndArray[] = {
        {MSG_MoveRobot,          csc_move_robot,            IPT_HNDL_ALL, NULL},
        {MSG_SpinRobot,          csc_spin_robot,            IPT_HNDL_ALL, NULL},
        {MSG_GetPosition,        csc_get_position,          IPT_HNDL_ALL, NULL},
        {MSG_RobotLocation,      csc_robot_location,        IPT_HNDL_ALL, NULL},
        {MSG_GpsPosition,        csc_gps_position,          IPT_HNDL_ALL, NULL},
        {MSG_BatteryInfo,        csc_battery_info,          IPT_HNDL_ALL, NULL},
        {MSG_DetectObstacles,    csc_detect_obstacles,      IPT_HNDL_ALL, NULL},
        {MSG_RobotDeath,         csc_robot_death,           IPT_HNDL_ALL, NULL},
        {MSG_TerminateMission,   csc_terminate_mission,     IPT_HNDL_ALL, NULL},
        {MSG_DetectRobots,       csc_detect_robots,         IPT_HNDL_ALL, NULL},
        {MSG_PutState,           csc_robot_put_state,       IPT_HNDL_ALL, NULL},
        {MSG_ReportedState,      csc_reported_state,        IPT_HNDL_ALL, NULL},
        {MSG_ReportedLogfile,    csc_reported_logfile,      IPT_HNDL_ALL, NULL},
        {MSG_RobotBirth,         csc_robot_birth,           IPT_HNDL_ALL, NULL},
        {MSG_NotifyConfig,       csc_notify_config,         IPT_HNDL_ALL, NULL},
        {MSG_Pickup,             csc_pickup,                IPT_HNDL_ALL, NULL},
        {MSG_Probe,              csc_probe,                 IPT_HNDL_ALL, NULL},
        {MSG_CheckVehicleType,   csc_check_vehicle_type,    IPT_HNDL_ALL, NULL},
        {MSG_CheckTargetUntrackable, csc_check_target_untrackable, IPT_HNDL_ALL, NULL},
        {MSG_Alert,              csc_alert,                 IPT_HNDL_ALL, NULL},
        {MSG_PopUpTelop,         csc_popup_telop_interface, IPT_HNDL_ALL, NULL},
        {MSG_AskToProceed,       csc_ask_to_proceed,        IPT_HNDL_ALL, NULL},
        {MSG_DropInBasket,       csc_drop_in_basket,        IPT_HNDL_ALL, NULL},
        {MSG_DetectObjects,      csc_detect_objects,        IPT_HNDL_ALL, NULL},
        {MSG_ChangeColor,        csc_change_object_color,   IPT_HNDL_ALL, NULL},
        {MSG_ChangeRobotColor,   csc_change_robot_color,    IPT_HNDL_ALL, NULL},
        {MSG_InquireMapLocation, csc_get_map_position,      IPT_HNDL_ALL, NULL},
        {MSG_AddObject,          csc_add_object,            IPT_HNDL_ALL, NULL},
        {MSG_AddPassagePoint,    csc_add_passage_point,     IPT_HNDL_ALL, NULL},
        {MSG_DisplayBitmapIcon,  csc_display_bitmap_icon,   IPT_HNDL_ALL, NULL},
        {MSG_DetectDoorway,      csc_detect_doorway,        IPT_HNDL_ALL, NULL},
        {MSG_DetectRoom,         csc_detect_room,           IPT_HNDL_ALL, NULL},
        {MSG_DetectHallway,      csc_detect_hallway,        IPT_HNDL_ALL, NULL},
        {MSG_MarkDoorway,        csc_mark_doorway,          IPT_HNDL_ALL, NULL},
        {MSG_SensorReadings,     csc_report_sensors,        IPT_HNDL_ALL, NULL},
        {MSG_LaserReadings,      csc_report_laser,          IPT_HNDL_ALL, NULL},
        {MSG_PingSend,           csc_ping_console,          IPT_HNDL_ALL, NULL},
        {MSG_ResetWorld,         csc_reset_world,           IPT_HNDL_ALL, NULL},
        {MSG_UpdateLMParams,     csc_update_lm_params,      IPT_HNDL_ALL, NULL},
        {MSG_StepTaken,          csc_step_taken,            IPT_HNDL_ALL, NULL},
        {MSG_SetMobilityType,    csc_set_mobility_type,     IPT_HNDL_ALL, NULL},
        {MSG_GetMobility,        csc_get_mobility,          IPT_HNDL_ALL, NULL},
        {MSG_GetElevation,	     csc_get_elevation,         IPT_HNDL_ALL, NULL},
        {MSG_GetTask,            csc_cnp_got_a_task,	    IPT_HNDL_ALL, NULL},
        {MSG_GetTaskConstraints, csc_cnp_send_task_constraints, IPT_HNDL_ALL, NULL},
        {MSG_BidTask,            csc_cnp_task_bid,          IPT_HNDL_ALL, NULL},
        {MSG_GetAward,           csc_cnp_get_award,         IPT_HNDL_ALL, NULL},
        {MSG_GetWater,		     csc_cnp_is_water,          IPT_HNDL_ALL, NULL},
        {MSG_CancelTask,         csc_cnp_cancel_task,       IPT_HNDL_ALL, NULL},
        {MSG_CNP_RobotConstraintsRequest, csc_cnp_send_robot_constraints, IPT_HNDL_ALL, NULL},
        {MSG_CNP_TaskConstraintsData, csc_cnp_save_current_task_constraints, IPT_HNDL_ALL, NULL},
        {MSG_GetRobotsPos, 		 csc_csb_get_robot_pos,		IPT_HNDL_ALL, NULL},
        {MSG_GetSignalStrength,  csc_csb_get_signal_strength,		IPT_HNDL_ALL, NULL}
    };

    for(i = 0; i < (int)(sizeof(hndArray)/sizeof(IPMsgHandlerSpec)); i++)
    {
        IPMessageType* msg_type = communicator->LookupMessage(hndArray[i].msg_name);

        communicator->RegisterHandler(
            msg_type,
            hndArray[i].callback,
            hndArray[i].data,
            hndArray[i].context);
    }

    com_open = true;
}


/****************************************************************************/

void clear_communications(void)
{
}


/****************************************************************************/

void gt_close_communication(void)
{
}


/****************************************************************************/

int gt_simulation_loop(void)
{
    // Sleep for some second to ease the mlab CPU.
    communicator->Idle(COMMUNICATOR_SLEEPTIME_SEC);

    // Return false, so that this work process will be kept looping.
    return false;
}

/**********************************************************************
 * $Log: console_side_com.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.8  2007/08/30 18:33:39  endo
 * BitmapIconDisplayer class added.
 *
 * Revision 1.7  2007/08/10 04:41:05  pulam
 * Added scale fix and military unit drawing fix.
 *
 * Revision 1.6  2007/03/12 00:06:33  nadeem
 * Fixed code so that the mission area is expressed in meters internally for simulations.
 *
 * Revision 1.5  2007/03/06 05:06:52  nadeem
 * Modified code to use Mercator projection to map between lat/lon and X/Y positions.
 *
 * Revision 1.4  2007/02/08 19:40:43  pulam
 * Added handling of zones (no-fly, weather)
 * Fixed up some of the icarus objectives to add turn limiting
 *
 * Revision 1.3  2006/10/24 22:03:47  endo
 * Last minute change for October demo.
 *
 * Revision 1.2  2006/09/20 18:34:48  nadeem
 * Added the code to convert latitude/longitude to X/Y and vice-versa. A lot of files had to be touched to make sure that the initialisation values can flow across from mlab to the robot executables as well as to ensure that the functions were available for use on both sides of the code.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.23  2006/07/11 06:39:47  endo
 * Cut-Off Feedback functionality merged from MARS 2020.
 *
 * Revision 1.22  2006/07/10 17:46:29  endo
 * Clean-up for MissionLab 7.0 release.
 *
 * Revision 1.21  2006/06/08 22:49:18  endo
 * CommBehavior from MARS 2020 migrated.
 *
 * Revision 1.20  2006/05/15 02:07:35  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.19  2006/05/15 01:50:11  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.18  2006/05/06 04:25:14  endo
 * Bugs of TrackTask fixed.
 *
 * Revision 1.17  2006/05/02 04:19:59  endo
 * TrackTask improved for the experiment.
 *
 * Revision 1.15  2006/03/05 23:27:20  pulam
 * Addition of SL-UAV code and CNP code for calculating bids for intercept/inspect tasks
 *
 * Revision 1.14  2006/02/19 17:58:00  endo
 * Experiment related modifications
 *
 * Revision 1.13  2006/01/30 02:43:17  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.12  2006/01/19 01:45:41  pulam
 * Cleaned up CNP code
 *
 * Revision 1.11  2006/01/12 20:32:44  pulam
 * cnp cleanup
 *
 * Revision 1.10  2006/01/10 06:10:31  endo
 * AO-FNC Type-I check-in.
 *
 * Revision 1.9  2005/07/31 03:40:42  endo
 * Robot ID specified by CNP can be now recognized by mlab during execution.
 *
 * Revision 1.8  2005/07/27 20:38:09  endo
 * 3D visualization improved.
 *
 * Revision 1.7  2005/07/23 05:58:20  endo
 * 3D visualization improved.
 *
 * Revision 1.6  2005/06/07 23:34:32  alanwags
 * CNP reivsion for premission specification
 *
 * Revision 1.5  2005/04/08 01:56:03  pulam
 * addition of cnp behaviors
 *
 * Revision 1.4  2005/03/27 01:19:37  pulam
 * addition of uxv behaviors
 *
 * Revision 1.3  2005/03/23 07:36:55  pulam
 * *** empty log message ***
 *
 * Revision 1.2  2005/02/07 23:12:34  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 23:00:12  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.74  2003/04/06 12:44:19  endo
 * gcc 3.1.1
 *
 * Revision 1.73  2003/04/02 22:00:52  zkira
 * Battery and GPS for telop GUI
 *
 * Revision 1.72  2002/10/31 21:36:37  ebeowulf
 * Added the code to reset the simulation environment.
 *
 * Revision 1.71  2002/10/31 20:05:29  ebeowulf
 * Added the ability to change the color of a robot.
 *
 * Revision 1.70  2002/07/18 17:13:29  endo
 * libtelop.a now compiles with g++.
 *
 * Revision 1.69  2002/07/02 20:39:03  blee
 * made changes so mlab could draw vector fields
 *
 * Revision 1.68  2002/04/04 18:45:00  blee
 * Added stuff to support changing the environment dynamically.
 *
 * Revision 1.67  2000/12/12 22:41:19  blee
 * Altered call to sim_report_laser() in csc_report_laser()
 *
 * Revision 1.66  2000/11/10 23:35:26  blee
 * added interface for learning momentum stuff and the
 * ability to log sensor data
 *
 * Revision 1.65  2000/08/19 00:58:23  endo
 * csc_ping_console added.
 *
 * Revision 1.64  2000/08/15 20:53:17  endo
 * csc_ask_to_procced, send_robots_proceed_mission added.
 *
 * Revision 1.63  2000/07/02 00:54:45  conrad
 * rechecked in aferter goahead
 *
 * Revision 1.61  2000/06/27 22:24:15  conrad
 * added sensor report
 *
 * Revision 1.60  2000/06/13 16:50:10  endo
 * link_robots() added to improve the communication capability of
 * robots. The robot can now broadcast information to other
 * robots by establishing server-clients relationships.
 *
 * Revision 1.59  2000/05/19 21:40:39  endo
 * csc_popup_telop_interface added.
 *
 * Revision 1.58  2000/04/25 09:57:35  jdiaz
 * sim_birth
 *
 * Revision 1.57  2000/03/30 01:43:42  endo
 * csc_spin_robot added.
 *
 * Revision 1.56  2000/02/28 23:35:35  jdiaz
 * added mark doorway message handler
 *
 * Revision 1.55  2000/02/18 01:49:56  endo
 * Added MSG_Alert, csc_alert.
 *
 * Revision 1.54  2000/02/07 05:11:07  jdiaz
 * doorways walls hallways rooms
 *
 * Revision 1.53  1999/12/18 09:36:54  endo
 * Data logging and Report current state capabilities
 * added.
 *
 * Revision 1.52  1999/12/17 00:46:24  endo
 * *** empty log message ***
 *
 * Revision 1.51  1999/12/16 22:47:58  mjcramer
 * RH6 Port'
 *
 * Revision 1.48  1999/05/26 22:13:35  jdiaz
 * *** empty log message ***
 *
 * Revision 1.47  1999/05/26 22:09:20  jdiaz
 * added callback for GetCompassHeading message
 *
 * Revision 1.46  1999/05/17 17:01:23  jdiaz
 * added support for displaying the current compass heading.
 *
 * Revision 1.45  1997/02/12 05:42:50  zchen
 * *** empty log message ***
 *
 * Revision 1.44  1996/08/22  15:09:48  doug
 * added linux compatibility code
 *
 * Revision 1.43  1996/03/20 00:30:45  doug
 * *** empty log message ***
 *
 * Revision 1.42  1996/03/09  01:08:43  doug
 * *** empty log message ***
 *
 * Revision 1.41  1996/03/04  22:51:58  doug
 * *** empty log message ***
 *
 * Revision 1.40  1996/03/01  00:47:09  doug
 * *** empty log message ***
 *
 * Revision 1.39  1996/02/28  03:54:51  doug
 * *** empty log message ***
 *
 * Revision 1.38  1996/02/22  00:56:22  doug
 * y
 *
 * Revision 1.37  1995/11/07  14:31:57  doug
 * added baskets and command to drop oranges into them
 *
 * Revision 1.36  1995/10/16  21:33:52  doug
 * *** empty log message ***
 *
 * Revision 1.35  1995/10/12  20:13:34  doug
 * Added pickup function
 *
 * Revision 1.34  1995/10/11  20:35:47  doug
 * *** empty log message ***
 *
 * Revision 1.33  1995/10/03  21:25:06  doug
 * add support for get_objects call
 *
 * Revision 1.32  1995/06/16  15:01:03  jmc
 * Minor rearrangement to eliminate compiler complaints (with -Wall).
 *
 * Revision 1.31  1995/05/08  19:38:22  jmc
 * Changed some externally visible references to TCX to IPT.
 *
 * Revision 1.30  1995/05/08  19:33:17  jmc
 * Added checking for IPTHOST before TCXHOST.
 *
 * Revision 1.29  1995/04/26  22:59:52  doug
 * need to get rid of colons in tcx module name
 *
 * Revision 1.28  1995/04/26  22:02:40  doug
 * Converted from tcx to ipt.  Keep your fingers crossed!
 *
 * Revision 1.27  1995/04/20  15:55:33  jmc
 * Corrected spelling.
 *
 * Revision 1.26  1995/04/19  21:52:28  jmc
 * Fixed a few insignificant problems that were pointed out by the
 * latest g++ compiler in -Wall mode.
 *
 * Revision 1.25  1995/04/10  16:15:14  jmc
 * Tweaked include files.
 *
 * Revision 1.24  1995/04/03  18:08:12  jmc
 * Added copyright notice.
 *
 * Revision 1.23  1995/02/12  02:14:48  doug
 * porting to linux
 *
 * Revision 1.22  1995/01/10  19:32:58  doug
 * add notify_config msg
 *
 * Revision 1.21  1994/11/29  17:02:32  doug
 * support reporting obstacles
 *
 * Revision 1.20  1994/11/21  17:54:41  doug
 * *** empty log message ***
 *
 * Revision 1.19  1994/11/16  22:15:34  doug
 * fixing handling of robots that have mismatching library versions.
 *
 * Revision 1.18  1994/11/16  20:52:48  doug
 * combined get_xy and get_heading messages into get_location
 *
 * Revision 1.17  1994/11/11  21:49:50  jmc
 * Changed some printfs to print only when debugging.
 *
 * Revision 1.16  1994/11/09  22:57:26  doug
 * Add delay in robot_suicide to handle ones dying during the command
 *
 * Revision 1.15  1994/11/04  03:34:32  doug
 * removed old style detect_obstacles
 *
 * Revision 1.14  1994/11/03  19:34:13  doug
 * Added single step cycle command
 *
 * Revision 1.13  1994/11/02  21:52:28  doug
 * Fixed avoid obstacle
 * added draw_halo option
 *
 * Revision 1.12  1994/11/01  23:40:47  doug
 * Name tcx socket with pid
 * handle robot error messages from bad version
 *
 * Revision 1.11  1994/10/19  21:54:09  doug
 * *** empty log message ***
 *
 * Revision 1.10  1994/10/18  20:55:29  doug
 * fixing prototypes
 *
 * Revision 1.9  1994/10/13  21:48:25  doug
 * Working on exit conditions for the robots
 *
 * Revision 1.8  1994/10/11  20:12:50  doug
 * added tcxclose call
 *
 * Revision 1.7  1994/10/11  17:10:48  doug
 * added seq field to gt_command record
 *
 * Revision 1.6  1994/10/07  21:10:27  jmc
 * Added a last ditch check for the hostname using gethostname in
 * gt_init_communication in case TCXHOST and HOST aren't set.
 *
 * Revision 1.5  1994/10/06  20:29:26  doug
 * fixing execute
 *
 * Revision 1.4  1994/10/05  18:50:32  doug
 * fix keypress stuff
 *
 * Revision 1.3  1994/10/04  22:20:50  doug
 * Getting execute to work
 *
 * Revision 1.2  1994/10/03  21:13:00  doug
 * Switched to tcx
 *
 * Revision 1.1  1994/09/28  20:44:41  doug
 * Initial revision
 *
 **********************************************************************/
