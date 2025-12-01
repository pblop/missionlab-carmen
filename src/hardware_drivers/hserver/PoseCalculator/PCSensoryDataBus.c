/**********************************************************************
 **                                                                  **
 **                         PCSensoryDataBus.c                       **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: PCSensoryDataBus.c,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <unistd.h>

#include "PCSensoryDataBus.h"
#include "PCUtility.h"

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
const double PoseCalcSensoryDataBus::WAIT_DATA_SEC_ = 1.0;
const int PoseCalcSensoryDataBus::SENSORY_BUFFER_SIZE_ = 5;
const int PoseCalcSensoryDataBus::WAIT_DATA_USLEEP_ = 1000;
const int PoseCalcSensoryDataBus::WAIT_DATA_WARN_SEC_ = 3;

//-----------------------------------------------------------------------
// Mutex
//-----------------------------------------------------------------------
pthread_mutex_t PoseCalcSensoryDataBus::sensoryDataListMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t PoseCalcSensoryDataBus::filterReadDataMutex_ = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------
// Constructor for PoseCalcSensoryDataBus
//-----------------------------------------------------------------------
PoseCalcSensoryDataBus::PoseCalcSensoryDataBus(PoseCalcInterface *poseCalc) : 
    poseCalc_(poseCalc),
    shSesnoryDataListReady_(false),
    shFilterReadData_(true)
{
}

//-----------------------------------------------------------------------
// Destructor for PoseCalcSensoryDataBus
//-----------------------------------------------------------------------
PoseCalcSensoryDataBus::~PoseCalcSensoryDataBus(void)
{
}

//-----------------------------------------------------------------------
// This function clears up the sensory data.
//-----------------------------------------------------------------------
void PoseCalcSensoryDataBus::initialize(void)
{
    pthread_mutex_lock(&sensoryDataListMutex_);
    shSensoryDataList_.clear();
    shSesnoryDataListReady_ = false;
    pthread_mutex_unlock(&sensoryDataListMutex_);
}

//-----------------------------------------------------------------------
// This function sets the flag to filter out the data that was already
// read upon sending it to fusers.
//-----------------------------------------------------------------------
void PoseCalcSensoryDataBus::setFilterReadData(bool filterReadData)
{
    pthread_mutex_lock(&filterReadDataMutex_);
    shFilterReadData_ = filterReadData;
    pthread_mutex_unlock(&filterReadDataMutex_);
}

//-----------------------------------------------------------------------
// This function returns the flag to filter out the data that was already
// read upon sending it to fusers.
//-----------------------------------------------------------------------
bool PoseCalcSensoryDataBus::filterReadData_(void)
{
    bool filterReadData = true;

    pthread_mutex_lock(&filterReadDataMutex_);
    filterReadData = shFilterReadData_;
    pthread_mutex_unlock(&filterReadDataMutex_);

    return filterReadData;
}

//-----------------------------------------------------------------------
// This function wait until the first data is arrived.
//-----------------------------------------------------------------------
void PoseCalcSensoryDataBus::waitForData(void)
{
    double curTime, refTime;

    refTime = getCurrentEpochTime();

    while (true)
    {
        usleep(WAIT_DATA_USLEEP_);

        pthread_testcancel();

        curTime = getCurrentEpochTime();

        if ((curTime - refTime) > WAIT_DATA_SEC_)
        {
            pthread_mutex_lock(&sensoryDataListMutex_);

            if (shSesnoryDataListReady_)
            {
                pthread_mutex_unlock(&sensoryDataListMutex_);
                return;
            }

            pthread_mutex_unlock(&sensoryDataListMutex_);

        }

        if ((curTime - refTime) > WAIT_DATA_WARN_SEC_)
        {
            printfTextWindow("PoseCalcSensoryDataBus: Waiting for the sensory data bus to be filled.\n");
            refTime = curTime;
        }
    }
}

//-----------------------------------------------------------------------
// This function returns all the sensory data including the buffers for
// both location and rotation.
//-----------------------------------------------------------------------
vector<PoseCalcSensoryData_t> PoseCalcSensoryDataBus::getAllSensoryData(int fuserType)
{
    vector<PoseCalcSensoryData_t> sensoryDataList;
    int i, j;

    pthread_mutex_lock(&sensoryDataListMutex_);
    sensoryDataList = shSensoryDataList_;

    // Mark all the data as "read".
    for (i = 0; i < (int)(shSensoryDataList_.size()); i++)
    {
        for (j = 0; j < (int)(shSensoryDataList_[i].locData.size()); j++)
        {
            shSensoryDataList_[i].locData[j].extra.read[fuserType] = true;
        }

        for (j = 0; j < (int)(shSensoryDataList_[i].rotData.size()); j++)
        {
            shSensoryDataList_[i].rotData[j].extra.read[fuserType] = true;
        }
    }

    pthread_mutex_unlock(&sensoryDataListMutex_);

    return sensoryDataList;
}

//-----------------------------------------------------------------------
// This function returns the latest location info for all the available
// sensors.
//-----------------------------------------------------------------------
vector<PoseCalcLocationInfo_t> PoseCalcSensoryDataBus::getLatestLocationData(int fuserType)
{
    PoseCalcLocationInfo_t locInfo;
    vector<PoseCalcLocationInfo_t> locData;
    double refTime;
    int i, index;
    bool filterReadData = true;

    // Get the current reference time for the location.
    poseCalc_->getLocReferenceTime(refTime);

    // Check whether the read data needs to be filtered out.
    filterReadData = filterReadData_();

    // Copy the shared list.
    pthread_mutex_lock(&sensoryDataListMutex_);

    // Extract the location info.
    for (i = 0; i < (int)(shSensoryDataList_.size()); i++)
    {
        index = shSensoryDataList_[i].latestLocIndex;

        if (index >= 0)
        {
            locInfo = shSensoryDataList_[i].locData[index];

            // Return only if the info whose reference time is same as the
            // current reference time.
            if (locInfo.refTime == refTime)
            {
                if (filterReadData &&
                    (shSensoryDataList_[i].locData[index].extra.read[fuserType]))
                {
                    // Filter out the data that was already "read".
                }
                else
                {
                    locData.push_back(locInfo);

                    // Mark this data as "read".
                    shSensoryDataList_[i].locData[index].extra.read[fuserType] = true;
                }
            }
        }
    }

    pthread_mutex_unlock(&sensoryDataListMutex_);

    return locData;
}

//-----------------------------------------------------------------------
// This function returns the latest rotation info for all the available
// sensors.
//-----------------------------------------------------------------------
vector<PoseCalcRotationInfo_t> PoseCalcSensoryDataBus::getLatestRotationData(int fuserType)
{
    PoseCalcRotationInfo_t rotInfo;
    vector<PoseCalcRotationInfo_t> rotData;
    double refTime;
    int i, index;
    bool filterReadData = true;

    // Get the current reference time for the rotation.
    poseCalc_->getRotReferenceTime(refTime);

    // Check whether the read data needs to be filtered out.
    filterReadData = filterReadData_();

    // Copy the shared list.
    pthread_mutex_lock(&sensoryDataListMutex_);

    // Extract the rotation info.
    for (i = 0; i < (int)(shSensoryDataList_.size()); i++)
    {
        index = shSensoryDataList_[i].latestRotIndex;

        if (index >= 0)
        {
            rotInfo = shSensoryDataList_[i].rotData[index];

            // Return only if the info whose reference time is same as the
            // current reference time.
            if (rotInfo.refTime == refTime)
            {
                if (filterReadData &&
                    (shSensoryDataList_[i].rotData[index].extra.read[fuserType]))
                {
                    // Filter out the data that was already "read".
                }
                else
                {
                    rotData.push_back(rotInfo);

                    // Mark this data as "read".
                    shSensoryDataList_[i].rotData[index].extra.read[fuserType] = true;
                }
            }
        }
    }

    pthread_mutex_unlock(&sensoryDataListMutex_);

    return rotData;
}

//-----------------------------------------------------------------------
// This function returns the latest pose info for all the available
// sensors.
//-----------------------------------------------------------------------
vector<PoseCalcPoseInfo_t> PoseCalcSensoryDataBus::getLatestPoseData(int fuserType)
{
    PoseCalcPoseInfo_t poseInfo;
    PoseCalcLocationInfo_t locInfo;
    PoseCalcRotationInfo_t rotInfo;
    vector<PoseCalcPoseInfo_t> poseData;
    double locRefTime, rotRefTime;
    int i, locIndex, rotIndex;
    bool infoCopied = false;
    bool filterReadData = true;

    // Get the current reference time for both location and rotation.
    poseCalc_->getReferenceTimes(locRefTime, rotRefTime);

    // Check whether the read data needs to be filtered out.
    filterReadData = filterReadData_();

    pthread_mutex_lock(&sensoryDataListMutex_);

    for (i = 0; i < (int)(shSensoryDataList_.size()); i++)
    {
        memset((PoseCalcPoseInfo_t *)&poseInfo, 0x0, sizeof(PoseCalcPoseInfo_t));
        infoCopied = false;

        // Copy the sensor class and type.
        poseInfo.sensorClass = shSensoryDataList_[i].sensorClass;
        poseInfo.sensorType = shSensoryDataList_[i].sensorType;

        // Get the location information.
        locIndex = shSensoryDataList_[i].latestLocIndex;

        if (locIndex >= 0)
        {
            locInfo = shSensoryDataList_[i].locData[locIndex];

            // Return only if the info whose reference time is same as the
            // current reference time.
            if (locInfo.refTime == locRefTime)
            {
                if (filterReadData &&
                    (shSensoryDataList_[i].locData[locIndex].extra.read[fuserType]))
                {
                    // Filter out the data that was already "read".
                }
                else
                {
                    // Make up the pose from the location and rotation.
                    poseInfo.pose.loc = locInfo.loc;
                    poseInfo.rawPose.loc = locInfo.rawLoc;
                    poseInfo.locRefTime = locInfo.refTime;

                    // Combine the confidence and variance.
                    poseInfo.confidence.locConfidence = locInfo.confidence;
                    poseInfo.variance.locVariance = locInfo.variance;

                    // Mark those data as "read".	
                    shSensoryDataList_[i].locData[locIndex].extra.read[fuserType] = true;

                    infoCopied = true;
                }
            }
        }

        // Get the rotation information.
        rotIndex = shSensoryDataList_[i].latestRotIndex;

        if (rotIndex >= 0)
        {
            rotInfo = shSensoryDataList_[i].rotData[rotIndex];

            // Return only if the info whose reference time is same as the
            // current reference time.
            if (rotInfo.refTime == rotRefTime)
            {
                if (filterReadData &&
                    (shSensoryDataList_[i].rotData[rotIndex].extra.read[fuserType]))
                {
                    // Filter out the data that was already "read".
                }
                else
                {
                    // Make up the pose from the location and rotation.
                    poseInfo.pose.rot = rotInfo.rot;
                    poseInfo.rawPose.rot = rotInfo.rawRot;
                    poseInfo.rotRefTime = rotInfo.refTime;

                    // Combine the confidence and variance.
                    poseInfo.confidence.rotConfidence = rotInfo.confidence;
                    poseInfo.variance.rotVariance = rotInfo.variance;

                    // Mark those data as "read".
                    shSensoryDataList_[i].rotData[rotIndex].extra.read[fuserType] = true;

                    infoCopied = true;
                }
            }
        }

        if (infoCopied)
        {
            // Put it in the list.
            poseData.push_back(poseInfo);
        }
    }

    pthread_mutex_unlock(&sensoryDataListMutex_);

    return poseData;
}

//-----------------------------------------------------------------------
// This function save the pose info in the sensory data bus.
//-----------------------------------------------------------------------
void PoseCalcSensoryDataBus::savePoseInfo(PoseCalcPoseInfo_t poseInfo)
{
    switch (poseInfo.poseType) {

    case POSECALC_POSE_TYPE_LOCATION:
        saveLocationInfo_(
            PoseCalcUtility::extractLocationInfoFromPoseInfo(poseInfo));
        break;

    case POSECALC_POSE_TYPE_ROTATION:
        saveRotationInfo_(
            PoseCalcUtility::extractRotationInfoFromPoseInfo(poseInfo));
        break;

    case POSECALC_POSE_TYPE_LOCATION_AND_ROTATION:
        saveLocationInfo_(
            PoseCalcUtility::extractLocationInfoFromPoseInfo(poseInfo));
        saveRotationInfo_(
            PoseCalcUtility::extractRotationInfoFromPoseInfo(poseInfo));
        break;
    }
}

//-----------------------------------------------------------------------
// This function save the location info in the sensory data bus.
//-----------------------------------------------------------------------
void PoseCalcSensoryDataBus::saveLocationInfo_(PoseCalcLocationInfo_t locInfo)
{
    PoseCalcSensoryData_t sensoryData;
    double refTime;
    int i, index;
    bool sensorFound = false;

    // Do not save if the data is too old.
    poseCalc_->getLocReferenceTime(refTime);

    if (locInfo.refTime != refTime)
    {
        return;
    }

    memset((PoseCalcSensoryData_t *)&sensoryData, 0x0, sizeof(PoseCalcSensoryData_t));
    memset((bool *)&(locInfo.extra.read), 0x0, sizeof(bool)*NUM_POSECALC_FUSER_TYPES);

    pthread_mutex_lock(&sensoryDataListMutex_);

    for (i = 0; i < (int)(shSensoryDataList_.size()); i++)
    {
        if (shSensoryDataList_[i].sensorType == locInfo.sensorType)
        {
            sensorFound = true;

            if ((int)(shSensoryDataList_[i].locData.size()) > 0)
            {
                // Save the location in a circular buffer.
                index = shSensoryDataList_[i].latestLocIndex;
                index++;

                if (index >= SENSORY_BUFFER_SIZE_)
                {
                    index = 0;

                    if ((int)(shSensoryDataList_[i].rotData.size()) ==
                        SENSORY_BUFFER_SIZE_)
                    {
                        // The buffer for at least one sensor (for both the
                        // location and rotation) is filled.
                        shSesnoryDataListReady_ = true;
                    }
                }

                shSensoryDataList_[i].locData[index] = locInfo;
                shSensoryDataList_[i].latestLocIndex = index;
            }
            else
            {
                // First time location is saved for this sensor.
                shSensoryDataList_[i].locData.resize(SENSORY_BUFFER_SIZE_);
                shSensoryDataList_[i].locData[0] = locInfo;
                shSensoryDataList_[i].latestLocIndex = 0; 
            }

            break;
        }
    }

    if (!sensorFound)
    {
        // Create a new entry for this sensor.
        sensoryData.sensorClass = locInfo.sensorClass;
        sensoryData.sensorType = locInfo.sensorType;
        sensoryData.locData.resize(SENSORY_BUFFER_SIZE_);
        sensoryData.locData[0] = locInfo;
        sensoryData.latestLocIndex = 0; 
        sensoryData.latestRotIndex = -1; 
        shSensoryDataList_.push_back(sensoryData);
    }

    pthread_mutex_unlock(&sensoryDataListMutex_);
}

//-----------------------------------------------------------------------
// This function save the rotation info in the sensory data bus.
//-----------------------------------------------------------------------
void PoseCalcSensoryDataBus::saveRotationInfo_(PoseCalcRotationInfo_t rotInfo)
{
    PoseCalcSensoryData_t sensoryData;
    double refTime;
    int i, index;
    bool sensorFound = false;

    // Do not save if the data is too old.
    poseCalc_->getRotReferenceTime(refTime);

    if (rotInfo.refTime != refTime)
    {
        return;
    }

    memset((PoseCalcSensoryData_t *)&sensoryData, 0x0, sizeof(PoseCalcSensoryData_t));
    memset((bool *)&(rotInfo.extra.read), 0x0, sizeof(bool)*NUM_POSECALC_FUSER_TYPES);

    pthread_mutex_lock(&sensoryDataListMutex_);

    for (i = 0; i < (int)(shSensoryDataList_.size()); i++)
    {
        if (shSensoryDataList_[i].sensorType == rotInfo.sensorType)
        {
            sensorFound = true;

            if ((int)(shSensoryDataList_[i].rotData.size()) > 0)
            {
                // Save the rotation in a circular buffer.
                index = shSensoryDataList_[i].latestRotIndex;
                index++;

                if (index >= SENSORY_BUFFER_SIZE_)
                {
                    index = 0;

                    if ((int)(shSensoryDataList_[i].locData.size()) ==
                        SENSORY_BUFFER_SIZE_)
                    {
                        // The buffer for at least one sensor (for both the
                        // location and rotation) is filled.
                        shSesnoryDataListReady_ = true;
                    }
                }

                shSensoryDataList_[i].rotData[index] = rotInfo;
                shSensoryDataList_[i].latestRotIndex = index;
            }
            else
            {
                // First time rotation is saved for this sensor.
                shSensoryDataList_[i].rotData.resize(SENSORY_BUFFER_SIZE_);
                shSensoryDataList_[i].rotData[0] = rotInfo;
                shSensoryDataList_[i].latestRotIndex = 0; 
            }

            break;
        }
    }

    if (!sensorFound)
    {
        // Create a new entry for this sensor.
        sensoryData.sensorClass = rotInfo.sensorClass;
        sensoryData.sensorType = rotInfo.sensorType;
        sensoryData.rotData.resize(SENSORY_BUFFER_SIZE_);
        sensoryData.rotData[0] = rotInfo;
        sensoryData.latestRotIndex = 0; 
        sensoryData.latestLocIndex = -1; 
        shSensoryDataList_.push_back(sensoryData);
    }

    pthread_mutex_unlock(&sensoryDataListMutex_);
}

/**********************************************************************
# $Log: PCSensoryDataBus.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:53  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1  2004/09/10 19:59:16  endo
# New PoseCalc integrated.
#
#**********************************************************************/
