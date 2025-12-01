/**********************************************************************
 **                                                                  **
 **                           gyro_dmu-vgx.c                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Corssbow DMU-VGX Gyroscope driver for HServer                   **
 **                                                                  **
 **  Note: HServer reads DMU data from mobility. mobility has to be  **
 **        installed in the system.                                  **
 **                                                                  **
 **  Copyright 2001 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gyro_dmu-vgx.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include "HServerTypes.h"
#include "gyro_dmu-vgx.h"
#include "Watchdog.h"

// Name of the DMU server.
const string GyroDMUVGX::DMUSERVER_NAME_ = "crossbow";

// How often the data from mobility should be updated (in microsecond).
const int GyroDMUVGX::READER_THREAD_USLEEP_ = 1000;

// How often the status bar updated with respect to the sampling time
// in the reader thread.
const int GyroDMUVGX::SKIP_STATUSBAR_UPDATE_ = 10;

// Initial values for the parameters to convert from mobility
// DMU data to HServer angles in degree. *calib_factor* multiplies
// and *offset* adds values to the DMU data. The multiplication
// is done first.
const double GyroDMUVGX::INIT_CALIB_FACTOR_YAW_ = -1.0;
const double GyroDMUVGX::INIT_CALIB_FACTOR_PITCH_ = 1.0;
const double GyroDMUVGX::INIT_CALIB_FACTOR_ROLL_ = 1.0;
const double GyroDMUVGX::INIT_OFFSET_YAW_ = 0.0;
const double GyroDMUVGX::INIT_OFFSET_PITCH_ = 0.0;
const double GyroDMUVGX::INIT_OFFSET_ROLL_ = 0.0;

// Interval for watchdog to monitor the status of the gyro thread.
const int GyroDMUVGX::WATCHDOG_CHECK_INTERVAL_SEC_ = 2;

// Constructor for the Gyro DMU-VGX class.
GyroDMUVGX::GyroDMUVGX(Gyro** a) 
    : Gyro((Gyro**)a, TYPE_DMUVGX, HS_MODULE_NAME_GYRO_DMUVGX)
{
    int rtn;

    if (report_level > HS_REP_LEV_NONE)
    {
        printTextWindow("Connecting to Gyro DMU-VGX... ", false);
    }

    strcat(statusStr, "Gyro DMU-VGX: [connecting]");
    statusbar->update(statusLine);

    gyroRotCalibFactor_.yaw = INIT_CALIB_FACTOR_YAW_;
    gyroRotCalibFactor_.pitch = INIT_CALIB_FACTOR_PITCH_;
    gyroRotCalibFactor_.roll = INIT_CALIB_FACTOR_ROLL_;
    gyroRotOffset_.yaw = INIT_OFFSET_YAW_;
    gyroRotOffset_.pitch = INIT_OFFSET_PITCH_;
    gyroRotOffset_.roll = INIT_OFFSET_ROLL_;

    readerThreadUSleep_ = READER_THREAD_USLEEP_;
    skipStatusBarUpdate_ = SKIP_STATUSBAR_UPDATE_;

    // Initialize the mobility.
    rtn = initializeMobility_();

    refreshScreen();

    if (rtn)
    {
        if (report_level > HS_REP_LEV_NONE)
        {
            printTextWindow("failed", true);
        }

        delete this;
        return;
    }

    // Start reading the DMU-VGX data.
    pthread_create(&readerThread_, NULL, &startMainThread_, (void*)this);

    if (gWatchdogEnabled)
    {
        watchdog_ = new Watchdog(NAME_, WATCHDOG_CHECK_INTERVAL_SEC_, getpid(), readerThread_);
    }

    while (!firstDataReceived_)
    {
        usleep(readerThreadUSleep_);
    }

    if (report_level > HS_REP_LEV_NONE)
    {
        printTextWindow( "connected", true );
    }

    // Reset the pose.
    resetAngles_();

    updateStatusBar_();
}

// Distructor for the Gyro DMU-VGX class.
GyroDMUVGX::~GyroDMUVGX()
{
    pthread_cancel(readerThread_);
    pthread_join(readerThread_, NULL );

    printTextWindow( "Gyro DMU-VGX disconnected" );
}

// This function updates the status bar.
void GyroDMUVGX::updateStatusBar_()
{
    HSRotation_t rot;
    double frequency;
    char buf[100];
    const bool DISPLAY_FREQUENCY = true;
  
    statusStr[0] = 0;

    strcat(statusStr, "Gyro DMU-VGX: ");

    getRotation(rot);

    sprintf(
        buf,
        "%c yaw: %.2f pitch: %.2f roll: %.2f [deg]",    
        statusbarSpinner_->getStatus(),
        rot.yaw,
        rot.pitch,
        rot.roll);

    if (DISPLAY_FREQUENCY)
    {
        getFrequency_(frequency);
        sprintf(buf, "%s : %.2f Hz", buf, frequency);
    }

    strcat(statusStr, buf);
    statusbar->update(statusLine);
}

// This function sets up the interface between HServer and mobility. 
int GyroDMUVGX::initializeMobility_(void)
{
    int argc;
    char* argv[4];
    char pathName[255];

    argv[0] = "HServer";
    argv[1] = "DMU";
    argv[2] = "VGX";
    argv[3] = 0;
    argc = 3;

    pHelper_ = new mbyClientHelper(argc, argv);

    sprintf(pathName, "%s/Drive/State", DMUSERVER_NAME_.c_str());

    ptempObj_ = pHelper_->find_object(pathName);

    pMbyGyroState_ = MobilityActuator::ActuatorState::_duplicate(
        MobilityActuator::ActuatorState::_narrow(ptempObj_));

    return 0;
}  

// This function grabs the DMU-VGX data from mobility
Gyro::GyroData_t GyroDMUVGX::readGyroData_(void)
{
    GyroData_t gyroData;
    double curTime;
    
    // Reset the data.
    memset((HSRotation_t *)&(gyroData.rot), 0x0, sizeof(HSRotation_t));
    memset((HSRotation_t *)&(gyroData.angVel), 0x0, sizeof(HSRotation_t));

    // Get the time stamp.
    curTime = getCurrentEpochTime();

    // Grab the latest data set from mobility.
    pMbyGyroData_ = pMbyGyroState_->get_sample(0); 

    // Retrieve the positional data.
    gyroData.rot.roll = pMbyGyroData_->position[3];
    gyroData.rot.pitch = pMbyGyroData_->position[4];
    gyroData.rot.yaw = pMbyGyroData_->position[5];
    gyroData.rot.extra.time = curTime;

    // Retrieve the veloicty data.
    gyroData.angVel.roll = pMbyGyroData_->velocity[3];
    gyroData.angVel.pitch = pMbyGyroData_->velocity[4];
    gyroData.angVel.yaw = pMbyGyroData_->velocity[5];
    gyroData.angVel.extra.time = curTime;

    delete pMbyGyroData_;
    pMbyGyroData_ = NULL;

    return gyroData;
}

void GyroDMUVGX::control(void)
{
    int c;
    bool bDone = false;

    messageDrawWindow(
        EnMessageType_GYRO_DMU_VGX_CONTROL,
        EnMessageErrType_NONE);

    redrawWindows();

    do
    {
        c = getch();

        switch(c) {

        case 'd':
            messageHide();
            delete this;
        case 'r':
            resetAngles_();
            break;
        case 'x':
        case 'Q':
        case KEY_ESC:
            bDone = true;
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
            messageMovePanelKey( c );
            break;
        }
    } while ( !bDone );

    messageHide();
}

/**********************************************************************
# $Log: gyro_dmu-vgx.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.3  2004/09/10 19:41:06  endo
# New PoseCalc integrated.
#
# Revision 1.2  2004/05/13 23:15:11  endo
# Memory leak fixed.
#
# Revision 1.1.1.1  2004/02/27 22:30:51  endo
# Initial MARS 2020 revision
#
# Revision 1.2  2001/12/22 16:35:40  endo
# CALIB_FACTOR changed due to the change in the DMU server.
#
# Revision 1.1  2001/05/18 17:54:47  blee
# Initial revision
#
#**********************************************************************/
