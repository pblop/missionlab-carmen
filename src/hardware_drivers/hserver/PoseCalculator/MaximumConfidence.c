/**********************************************************************
 **                                                                  **
 **                      MaximumConfidence.c                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: MaximumConfidence.c,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include "MaximumConfidence.h"
#include "PoseCalculatorTypes.h"
#include "PoseCalculator.h"

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------
extern double getCurrentEpochTime(void);
extern void printfTextWindow(const char* arg, ...);

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constructor for MaximumConfidence
//-----------------------------------------------------------------------
MaximumConfidence::MaximumConfidence(
    PoseCalcSensoryDataBusInterface *sensoryDataBus) :
    PoseCalcSensorFuser(sensoryDataBus, POSECALC_FUSER_TYPE_MAXMUM_CONFIDENCE)
{
    initialize();
}

//-----------------------------------------------------------------------
// Destructor for MaximumConfidence
//-----------------------------------------------------------------------
MaximumConfidence::~MaximumConfidence(void)
{
}

//-----------------------------------------------------------------------
// This function returns the output of the fused sensor information.
//-----------------------------------------------------------------------
PoseCalcFusedPose_t MaximumConfidence::getOutput(void)
{
    PoseCalcFusedPose_t fusedPose;
    vector<PoseCalcLocationInfo_t> locData;
    vector<PoseCalcRotationInfo_t> rotData;
    double maxConfidenceLevel;
    int i;

    // Get the latest location and rotation data from the sensory bus.
    locData = sensoryDataBus_->getLatestLocationData(FUSER_TYPE_);
    rotData = sensoryDataBus_->getLatestRotationData(FUSER_TYPE_);

    // Initialize data.
    memset((PoseCalcFusedPose_t *)&fusedPose, 0x0, sizeof(PoseCalcFusedPose_t));
    fusedPose.fuserType = FUSER_TYPE_;
    fusedPose.confidence.locConfidence.value = PoseCalc::MIN_CONFIDENCE_LEVEL;
    fusedPose.confidence.rotConfidence.value = PoseCalc::MIN_CONFIDENCE_LEVEL;
    fusedPose.extra.validPose = false;
    fusedPose.extra.time = getCurrentEpochTime();

    // Get the location with the highest confidence.
    maxConfidenceLevel = PoseCalc::MIN_CONFIDENCE_LEVEL;

    for (i = 0; i < (int)(locData.size()); i++)
    {
        if ((locData[i].confidence.value) > maxConfidenceLevel)
        {
            fusedPose.pose.loc = locData[i].loc;
            fusedPose.confidence.locConfidence = locData[i].confidence;
            fusedPose.variance.locVariance = locData[i].variance;
            fusedPose.extra.locSensorType = locData[i].sensorType;
            fusedPose.extra.validLoc = true;

            maxConfidenceLevel = locData[i].confidence.value;
        }
    }
    
    // Get the rotation with the highest confidence.
    maxConfidenceLevel = PoseCalc::MIN_CONFIDENCE_LEVEL;

    for (i = 0; i < (int)(rotData.size()); i++)
    {
        if ((rotData[i].confidence.value) > maxConfidenceLevel)
        {
            fusedPose.pose.rot = rotData[i].rot;
            fusedPose.confidence.rotConfidence = rotData[i].confidence;
            fusedPose.variance.rotVariance = rotData[i].variance;
            fusedPose.extra.rotSensorType = rotData[i].sensorType;
            fusedPose.extra.validRot = true;
            
            maxConfidenceLevel = rotData[i].confidence.value;
        }
    }

    // Mark the validity.
    if ((fusedPose.extra.validLoc) && (fusedPose.extra.validRot))
    {
        fusedPose.extra.validPose = true;
    }

    // Set the time stamp for the pose.
    fusedPose.pose.extra.time = 
        (fusedPose.pose.loc.extra.time >= fusedPose.pose.rot.extra.time)?
        fusedPose.pose.loc.extra.time : fusedPose.pose.rot.extra.time;

    return fusedPose;
}

//-----------------------------------------------------------------------
// This function initialzes the fuser.
//-----------------------------------------------------------------------
PoseCalcFusedPose_t MaximumConfidence::initialize(void)
{
    PoseCalcFusedPose_t fusedPose;

    fusedPose = getOutput();

    return fusedPose;
}

/**********************************************************************
# $Log: MaximumConfidence.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:53  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1  2004/09/10 19:59:15  endo
# New PoseCalc integrated.
#
#**********************************************************************/
