/**********************************************************************
 **                                                                  **
 **                         robothandler.cpp                         **
 **                                                                  **
 ** Written by: Patrick Daniel Ulam                                  **
 **                                                                  **
 ** Copyright 2005 Georgia Tech Research Corporation                 **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **                                                                  **
 **********************************************************************/

/* $Id: robothandler.cpp,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#include "3d_vis_wid.h"
#include "robothandler.h"

const string RobotHandler::EMPTY_STRING_ = "";
const string RobotHandler::STRING_18WHEEL_3DS_ = "18WHEEL.3DS";
const string RobotHandler::STRING_AH64_3DS_ = "AH64.3DS";
const string RobotHandler::STRING_USS_LA_3DS_ = "USS_LA.3DS";
const string RobotHandler::STRING_BOAT_3DS_ = "BOAT.3DS";
const string RobotHandler::STRING_HUMMER_3DS_ = "HUMMER.3DS";
const string RobotHandler::STRING_A10_3DS_ = "a10.3DS";
const string RobotHandler::INVALID_TERRAIN_ROBOT_COLOR1_ = "orange";
const string RobotHandler::INVALID_TERRAIN_ROBOT_COLOR2_ = "gray";
const double RobotHandler::DEFAULT_ROBOT_LENGTH_ = 5.0;

RobotHandler::RobotHandler(string modelPath) :
    modelPath_(modelPath)
{
    int i;

    for (i = 0; i < NUM_MODEL_TYPES; i++)
    {
        robotModels_[i] = NULL;
    }

    memset(modelLength_, 0x0, sizeof(double)*NUM_MODEL_TYPES);
}

// delete all the robots currently tracked
RobotHandler::~RobotHandler(void)
{
    RobotRecord *temprec;
    Model_3ds *robotModel;

    vector<RobotRecord *>::iterator rrit = RobotList.begin();


    while (rrit != RobotList.end()) {
      temprec = *rrit;
      delete temprec->CurrentRobot;
      delete temprec->Model;
      delete temprec;

      rrit++;
    }

    while ((int)(createdModels_.size()) > 0)
      {
        robotModel = createdModels_.back();
        createdModels_.pop_back();
      }
}

Model_3ds *RobotHandler::assignRobotModel_(
    int robotType,
    double robotLength,
    bool water)
{
    string modelFileName = EMPTY_STRING_;
    int modelType = MODEL_HUMMER; // Default
    bool createNewModel = false;
    float s;

    switch (robotType) {

    case UGV:
        modelType = MODEL_HUMMER;
        break;

    case UXV:
        if (water)
        {
            modelType = MODEL_BOAT;
        }
        else
        {
            modelType = MODEL_HUMMER;
        }
        break;

    case UUV:
        modelType = MODEL_USS_LA;
        break;

    case USV:
        modelType = MODEL_BOAT;
        break;

    case UAV:
    case SLUAVL:
    case SLUAVU:
        modelType = MODEL_UAV;
        break;
    }

    if ((modelLength_[modelType] != robotLength) ||
        (robotModels_[modelType] == NULL))
    {
        createNewModel = true;
    }

    modelLength_[modelType] = robotLength;
    s = robotLength;

    if (createNewModel)
    {
        if (modelPath_ != EMPTY_STRING_)
        {
            modelFileName = modelPath_;
            modelFileName += "/";
        }

        switch (modelType) {

        case MODEL_18WHEEL:
            modelFileName += STRING_18WHEEL_3DS_;
            robotModels_[MODEL_18WHEEL] = new Model_3ds(
                (char *)(modelFileName.c_str()),
                0.012*s,
                0.006*s,
                0.012*s,
                -90.0f,
                -364.0f,
                1078.0f,
                false,
                Z_AXIS);
            break;

        case MODEL_AH64:
            modelFileName += STRING_AH64_3DS_;
            robotModels_[MODEL_AH64] = new Model_3ds(
                (char *)(modelFileName.c_str()),
                0.24*s,
                0.24*s,
                0.24*s,
                -90.0f,
                0.0f,
                0.0f,
                true,
                Z_AXIS);
            break;

        case MODEL_USS_LA:
            modelFileName += STRING_USS_LA_3DS_;
            robotModels_[MODEL_USS_LA] = new Model_3ds(
                (char *)(modelFileName.c_str()),
                0.03*s,
                0.01*s,
                0.02*s,
                -90.0f,
                0.0f,
                -90.0f,
                true,
                Z_AXIS);
            break;

        case MODEL_BOAT:
            modelFileName += STRING_BOAT_3DS_;
            robotModels_[MODEL_BOAT] = new Model_3ds(
                (char *)(modelFileName.c_str()),
                0.02*s,
                0.02*s,
                0.02*s,
                -90.0f,
                0.0f,
                -90.0f,
                false,
                Z_AXIS);
            break;


            // PU SLUAV
        case MODEL_UAV:
            modelFileName += STRING_A10_3DS_;
            robotModels_[MODEL_UAV] = new Model_3ds(
                (char *)(modelFileName.c_str()),
                0.13*s,
                0.13*s,
                0.13*s,
                0.0f,
                0.0f,
                90.0f,
                false,
                X_AXIS);
            break;

        case MODEL_HUMMER:
        default:
            modelFileName += STRING_HUMMER_3DS_;
            robotModels_[MODEL_HUMMER] = new Model_3ds(
                (char *)(modelFileName.c_str()),
                0.99*s,
                0.99*s,
                0.99*s,
                -90.0f,
                0.0f,
                0.0f,
                false,
                Z_AXIS);
            break;
        }


        createdModels_.push_back(robotModels_[modelType]);
    }

    return robotModels_[modelType];
}


void RobotHandler::Update_Uncertainty(int id, int x, int y, int dx, int dy) {
  RobotRecord *temprec;

  vector<RobotRecord *>::iterator rrit = RobotList.begin();


  while (rrit != RobotList.end()) {
    temprec = *rrit;

    // if we find a match
    if (temprec->RobotNum == id) {
      temprec->CurrentRobot->Update_Uncertainty(x, y, dx, dy);
    }
    rrit++;
  }
}


// add or update a robot record
void RobotHandler::Place_Robot(
    int robotnum,
    double x,
    double y,
    double z,
    double heading,
    string color,
    int rtype,
    double robotlength,
    bool water,
    bool uncert,
    bool vis)
{
    RobotRecord *temprec;
    bool updatedone = false;
    bool invalidTerrain = false;


    vector<RobotRecord *>::iterator rrit = RobotList.begin();

    // go through existing records to see if this robot has already been added
    while (rrit != RobotList.end()) {

      temprec = *rrit;

      // if we find a match
      if (temprec->RobotNum == robotnum)
	{

	  // update the position
	  temprec->CurrentRobot->Update_Position(x, y, z, heading);

	  // Check for invalid terrain
	  switch (rtype) {

	  case UGV:
	    if (water)
	      {
		invalidTerrain = true;
	      }
	    break;

	  case UUV:
	  case USV:
	    if (!water)
	      {
		invalidTerrain = true;
	      }
	    break;
	  }

	  if (invalidTerrain)
	    {
	      temprec->CurrentRobot->Toggle_Colors(
						   INVALID_TERRAIN_ROBOT_COLOR1_,
						   INVALID_TERRAIN_ROBOT_COLOR2_);
	    }
	  else if ((temprec->CurrentRobot->Get_Color()) != color)
	    {
	      temprec->CurrentRobot->Set_Color(color);
	    }

	  temprec->Model = assignRobotModel_(rtype, robotlength, water);
	  temprec->Type = rtype;
	  temprec->Visable = vis;
	  temprec->Uncertainty = uncert;
	  temprec->CurrentRobot->Update_Uncertainty(temprec->Uncertainty);

	  updatedone = true;
	}
      rrit++;
    }


    // if no matches were found we add a new record
    if (updatedone != true)
      {
        temprec = new RobotRecord;
        temprec->CurrentRobot = new Robot(robotnum, x, y, z, heading, color, rtype);
        temprec->RobotNum = robotnum;
        temprec->Type = rtype;
        temprec->Model = assignRobotModel_(rtype, robotlength, water);

        RobotList.push_back(temprec);
      }
}

void RobotHandler::Draw(void)
{

  vector<RobotRecord *>::iterator rrit = RobotList.begin();
    RobotRecord *temprec;

    while (rrit != RobotList.end()) {

      temprec = *rrit;
      if (temprec != NULL) {
	if (temprec->Type != SLUAVU) {
	  if (temprec->Visable) {
	    temprec->CurrentRobot->Draw_Robot(0.0f, temprec->Model);
	  }
	}
      }
      rrit++;
    }
}

/**********************************************************************
 * $Log: robothandler.cpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.14  2006/07/10 17:47:13  endo
 * Clean-up for MissionLab 7.0 release.
 *
 * Revision 1.13  2006/06/29 21:49:22  pulam
 * New 3ds model loader, normal calculation and other fixes
 *
 * Revision 1.12  2006/05/15 01:04:51  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.11  2006/05/02 04:12:36  endo
 * TrackTask improved for the experiment.
 *
 * Revision 1.10  2005/11/08 17:10:20  endo
 * Memory leak problem solved.
 *
 * Revision 1.9  2005/08/01 19:12:12  endo
 * *** empty log message ***
 *
 * Revision 1.8  2005/07/30 03:02:08  endo
 * 3DS moved to terrain/models.
 *
 * Revision 1.7  2005/07/30 02:18:07  endo
 * 3D visualization improved.
 *
 * Revision 1.6  2005/07/28 04:55:35  endo
 * 3D visualization improved more.
 *
 * Revision 1.5  2005/07/27 20:39:45  endo
 * 3D visualization improved.
 *
 * Revision 1.4  2005/07/23 06:02:06  endo
 * 3D visualization improved.
 *
 **********************************************************************/
