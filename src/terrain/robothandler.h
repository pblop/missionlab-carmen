/**********************************************************************
 **                                                                  **
 **                         robothandler.h                           **
 **                                                                  **
 ** Written by: Patrick Daniel Ulam                                  **
 **                                                                  **
 ** Copyright 2005 Georgia Tech Research Corporation                 **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **                                                                  **
 **********************************************************************/

/* $Id: robothandler.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#ifndef ROBOTHANDLER_H
#define ROBOTHANDLER_H

#include <string>
#include <vector>

#include "gt_robot_type.h"
#include "model_3ds.h"

using std::string;
using std::vector;


typedef struct {
  Robot *CurrentRobot;
  int RobotNum;
  int Type;
  Model_3ds *Model;
  bool Uncertainty;
  bool Visable;
} RobotRecord;

class RobotHandler {

    /* Obsolete: See src/include/gt_robot_type.h
    // This should match with RobotMobilityType_t in gt_simulation.h
    typedef enum RobotType_t {
        UGV,
        URV,
        UUV,
        USV,
        UAV,
        SLUAVU,
        SLUAVL,
        NUM_ROBOT_MOBILITY_TYPES // Keep this at the end of the list.
    };
    */

    typedef enum ModelType_t {
        MODEL_18WHEEL,
        MODEL_AH64,
        MODEL_USS_LA,
        MODEL_BOAT,
        MODEL_HUMMER,
	MODEL_UAV,
        NUM_MODEL_TYPES // Keep this at the end of the list.
    };

private:
    vector<RobotRecord *> RobotList;
    Model_3ds *robotModels_[NUM_MODEL_TYPES];
    string modelPath_;
    vector <Model_3ds *> createdModels_;
    double modelLength_[NUM_MODEL_TYPES];

    static const string EMPTY_STRING_;
    static const string STRING_18WHEEL_3DS_;
    static const string STRING_AH64_3DS_;
    static const string STRING_USS_LA_3DS_;
    static const string STRING_BOAT_3DS_;
    static const string STRING_HUMMER_3DS_;
    static const string STRING_A10_3DS_;
    static const string INVALID_TERRAIN_ROBOT_COLOR1_;
    static const string INVALID_TERRAIN_ROBOT_COLOR2_;
    static const double DEFAULT_ROBOT_LENGTH_;

    Model_3ds *assignRobotModel_(int robotType, double robotLength, bool water);
    void Load_Models(void);

public:
    RobotHandler(string modelPath);
    ~RobotHandler(void);
    void Place_Robot(
        int robotnum, 
        double x,
        double y,
        double z,
        double theta,
        string color,
        int rtype,
        double robotlength,
        bool water,
	bool uncert,
	bool vis);
    void Draw(void);
    void Update_Uncertainty(int id, int x, int y, int dx, int dy);
    
};




#endif

/**********************************************************************
 * $Log: robothandler.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.10  2006/06/29 21:49:22  pulam
 * New 3ds model loader, normal calculation and other fixes
 *
 * Revision 1.9  2006/05/02 04:12:36  endo
 * TrackTask improved for the experiment.
 *
 * Revision 1.8  2006/03/05 23:27:20  pulam
 * Addition of SL-UAV code and CNP code for calculating bids for intercept/inspect tasks
 *
 * Revision 1.7  2005/08/01 19:12:13  endo
 * *** empty log message ***
 *
 * Revision 1.6  2005/07/30 03:02:08  endo
 * 3DS moved to terrain/models.
 *
 * Revision 1.5  2005/07/30 02:18:08  endo
 * 3D visualization improved.
 *
 * Revision 1.4  2005/07/27 20:39:45  endo
 * 3D visualization improved.
 *
 **********************************************************************/
