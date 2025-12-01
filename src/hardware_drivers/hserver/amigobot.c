/**********************************************************************
 **                                                                  **
 **                            amigobot.c                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  Antonio Sgorbissa and Yoichiro Endo                **
 **                                                                  **
 **  AmigoBot robot driver for HServer                               **
 **                                                                  **
 **  Copyright 2002, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: amigobot.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include <stdio.h>
#include <termio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <ctype.h>
#include <string>

#include "HServerTypes.h"
#include "message.h"
#include "amigobot.h"
#include "hserver.h"
#include "camera.h"
#include "statusbar.h"
#include "robot_config.h"
#include "sensors.h"
#include "robot.h"

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------

//---------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------
const int AmigoBot::CMD_SYNC0_ = 0;
const int AmigoBot::CMD_SYNC1_ = 1;
const int AmigoBot::CMD_SYNC2_ = 2;
const int AmigoBot::CMD_PULSE_ = 0;
const int AmigoBot::CMD_OPEN_ = 1;
const int AmigoBot::CMD_CLOSE_ = 2;
const int AmigoBot::CMD_POLLING_ = 3;
const int AmigoBot::CMD_ENABLE_ = 4;
const int AmigoBot::CMD_SETO_ = 7;
const int AmigoBot::CMD_VEL_ = 11;
const int AmigoBot::CMD_HEAD_ = 12;
const int AmigoBot::CMD_DHEAD_ = 13;
const int AmigoBot::CMD_SAY_ = 15;
const int AmigoBot::CMD_RVEL_ = 21;
const int AmigoBot::CMD_DIGOUT_ = 30;
const int AmigoBot::READ_BACKOFF_USEC_ = 100;
const int AmigoBot::READER_LOOP_USLEEP_ = 1000;
const int AmigoBot::SYNCER_LOOP_USLEEP_ = 500000;
const int AmigoBot::MAX_ENCODER_VALUE_ = 65536;
const int AmigoBot::MIN_TURN_AVEL_ = 2;
const int AmigoBot::SKIP_STATUSBAR_UPDATE_ = 10;
const byte_t AmigoBot::ARG_INT_ = 0x3B;
const byte_t AmigoBot::ARG_NINT_ = 0x1B;
const byte_t AmigoBot::ARG_STR_ = 0x2B;
const byte_t AmigoBot::PACK_VISIONIO_ = 0x80;
const byte_t AmigoBot::PACK_VISION_ = 0x50;
const byte_t AmigoBot::PACK_FRAME24_ = 0xb0;
const byte_t AmigoBot::PACK_DIST_ = 0xd0;
const float AmigoBot::SONAR_ANGLES_[] = {90,44,12,-12,-44,-90,-144,-216};
const float AmigoBot::SONAR_POS_[] = {0.18,0.28,0.27,0.27,0.27,0.28,0.18,0.16};
const double AmigoBot::MAX_SONAR_READABLE_DISTANCE_ = 5.0;
const double AmigoBot::ANGLE_CONV_FACTOR_RAD_ = 0.001534;
const double AmigoBot::DIST_CONV_FACTOR_ = 0.5083;
const double AmigoBot::VELOCITY_CONV_FACTOR_ = 0.6154;
const double AmigoBot::SONAR_RANGE_FACTOR_ = 0.521;
const double AmigoBot::MIN_ROBOT_VOLTAGE_ = 10.8;

//---------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------
AmigoBot::AmigoBot(Robot** a, const string& strPortString) : 
    Robot(a, HS_ROBOT_TYPE_AMIGOBOT, HS_MODULE_NAME_ROBOT_AMIGOBOT)
{
    char commandBuf[6];

    partXyt_ = NULL;
    partSonar_ = NULL;
    sensorSonar_ = NULL;
    robotName_ = NULL;
    isSyncing_ = false;
    isSynced_ = false;
    isReading_ = false;
    numSonars_ = AMIGOBOT_NUM_SONARS;

    pthread_mutex_init(&serialMutex_, NULL);

    if ((report_level > HS_REP_LEV_NONE) && (report_level != HS_REP_LEV_DEBUG))
    {
        printTextWindow("Connecting to AmigoBot... ", false);
    }

    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printTextWindow("Connecting to AmigoBot");
        printfTextWindow("robot serial port: %s", strPortString.c_str());
        printfTextWindow("%d sonar sensors", numSonars_);
    }

    setupSerial(&commFd_, strPortString.c_str(), B9600, report_level, "Robot", 1);
    strcpy(commandBuf, "WMS2");
    commandBuf[4] = 13;
    write(commFd_, commandBuf, 6);
    sleep(1);
    sendSync_(CMD_SYNC2_);

    flushSerialLine_();

    if (!testRobotConnection_())
    {
        redrawWindows();
        updateStatusBar_();
        redrawWindows();
        sleep(1);
        sendInt_(1, (byte_t)CMD_ENABLE_);
    }
    else
    {
        if ((report_level > HS_REP_LEV_NONE) && (report_level != HS_REP_LEV_DEBUG))
        {
            printTextWindow("failed", true);
        }
        else if (report_level > HS_REP_LEV_NONE)
        {
            printTextWindow("AmigoBot connection failed", true);
        }

        delete this;
        return;
    }

    if ((report_level > HS_REP_LEV_NONE) && (report_level != HS_REP_LEV_DEBUG))
    {
        printTextWindow("connected", true);
    }
    else if (report_level > HS_REP_LEV_NONE)
    {
        printTextWindow("AmigoBot connected", true);
    }

    pthread_create(&readerThread_, NULL, &startReaderThread_,(void*)this);

    addSensors_();

    runSyncer_();
}

//---------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------
AmigoBot::~AmigoBot(void)
{
    if (isSynced_)
    {
        sendSync_(CMD_CLOSE_);
        sendSync_(CMD_CLOSE_);
    }

    if (isSyncing_)
    {
        pthread_cancel(syncerThread_);
        pthread_join(syncerThread_, NULL);
        isSyncing_ = false;
    }

    if (isReading_)
    {
        pthread_cancel(readerThread_);
        pthread_join(readerThread_, NULL);
        isReading_ = false;
    }

    if (partXyt_ != NULL)
    {
        delete partXyt_;
        partXyt_ = NULL;
    }

    if (partSonar_ != NULL)
    {
        delete partSonar_;
        partSonar_ = NULL;
    }

    if (sensorSonar_ != NULL)
    {
        delete sensorSonar_;
        sensorSonar_ = NULL;
    }

    printTextWindow("AmigoBot disconnected");

    close(commFd_);

    if(robotName_ != NULL)
    {
        free(robotName_);
        free(robotClass_);
        free(subClass_);
    }

    redrawWindows();
}

//---------------------------------------------------------------------
// This function flushes the buffer in the serial port.
//---------------------------------------------------------------------
void AmigoBot::flushSerialLine_(void)
{
    byte_t buf[100];
    int cnum;
  
    do
    {
        cnum=read(commFd_, buf, 100);
        if (cnum == -1)
        {
            cnum = 0;
        }
    } while(cnum);
}

//---------------------------------------------------------------------
// This function tests the robot connection.
//---------------------------------------------------------------------
int AmigoBot::testRobotConnection_(void)
{
    int rtn;

    sendSync_(CMD_SYNC2_);
    sendSync_(CMD_SYNC0_);
    sendSync_(CMD_SYNC0_);
    sendSync_(CMD_SYNC0_);

    rtn = readPack_();

    if (rtn != -1)
    {
        return 0;
    }

    return -1;
}

//---------------------------------------------------------------------
// This function reads the next byte from the fd
//---------------------------------------------------------------------
byte_t AmigoBot::nextByte_(void)
{
    int cnum;
    byte_t byte;
  
    while(true)
    {
        cnum = read(commFd_, &byte, 1);
        if (cnum)
        {
            break;
        }
        // NEED TO SLEEP, DO NOT REMOVE
        usleep(READ_BACKOFF_USEC_);
    }
    return byte;
}

//---------------------------------------------------------------------
// This function reads the next remaining bytes from the fd
//---------------------------------------------------------------------
int AmigoBot::nextRemainingBytes_(byte_t *bytes, int max)
{
    int cnum;
    int numTries = 25;

    do
    {
        cnum = read(commFd_, bytes, max);
        if (cnum == 0)
        {
            numTries--;
            if (numTries == 0)
            {
                return -1;
            }
        }
        if (cnum == -1)
        {
            return -1;
        }
    } while ((cnum == 0) || (cnum == -1));

    return cnum;
}

//---------------------------------------------------------------------
// This function calculates the check sum.
//---------------------------------------------------------------------
int AmigoBot::calcCheckSum_(byte_t* ptr)
{
    int n;
    int c = 0;
    n = *(ptr++);
    n -= 2;

    while(n > 1)
    {
        c += (*(ptr) << 8) | *(ptr + 1);
        c = c & 0xffff;
        n -= 2;
        ptr += 2;
    }

    if (n > 0)
    {
        c = c ^ (int)*(ptr++);
    }

    return c;
}

//---------------------------------------------------------------------
// This function updates the status bar
//---------------------------------------------------------------------
void AmigoBot::updateStatusBar_(void)
{
    HSPose_t pose;
    char buf[100];

    statusStr[0] = 0;
  
    strcat(statusStr, "AmigoBot: ");

    if (isSyncing_)
    {
        strcat(statusStr, "Syncing");
    }
    else if(!isSynced_)
    {
        strcat(statusStr, "Not synced");
    }
    else
    {      
        getPose(pose);

        sprintf(
            buf,
            "%c %.2fV  x: %.2f  y: %.2f  t: %.2f ",
            statusbarSpinner_->getStatus(),
            battery_,
            pose.loc.x,
            pose.loc.y,
            pose.rot.yaw);

        strcat(statusStr, buf);
    }
    statusbar->update(statusLine);
}

//---------------------------------------------------------------------
// This function sends a packet to the communication port.
//---------------------------------------------------------------------
int AmigoBot::sendPack_(byte_t pack[])
{
    int checksum;
    byte_t header[2];
    byte_t chksum[2];

    header[0] = 0xFA;
    header[1] = 0xFB;
    checksum = calcCheckSum_(pack);
    chksum[0] = (checksum & 0x0000ff00) >> 8;
    chksum[1] = checksum & 0x000000ff;

    pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, (void *) &serialMutex_);
    pthread_mutex_lock(&serialMutex_);
    write(commFd_, header, 2);
    write(commFd_, pack, pack[0] - 1);
    write(commFd_, chksum, 2);
    pthread_cleanup_pop(1);

    return 0;
}

//---------------------------------------------------------------------
// This function sends a sync command to the robot.
//---------------------------------------------------------------------
int AmigoBot::sendSync_(int s)
{
    byte_t pack[6];
  
    pack[0] = 3;
    pack[1] = s;

    sendPack_(pack);

    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printfTextWindow("Send Sync: %d", s);
    }
    return 0;
}

//---------------------------------------------------------------------
// This function sends a command string to the robot
//---------------------------------------------------------------------
int AmigoBot::sendString_(char str[], byte_t command)
{
    byte_t pack[255];
    int i;

    pack[0] = 5 + strlen(str);
    pack[1] = command;
    pack[2] = ARG_STR_;
    pack[3] = strlen(str);

    for(i = 0; i < (int) strlen(str); i++)
    {
        pack[4 + i] = str[i];
    }
    sendPack_(pack);

    return 0;
}

//---------------------------------------------------------------------
// This function sends an integer to the robot.
//---------------------------------------------------------------------
int AmigoBot::sendInt_(int sint, byte_t command)
{
    byte_t pack[40];
  
    pack[0] = 6;
    pack[1] = command;

    if (sint < 0)
    {
        pack[2] = ARG_NINT_;
        sint = abs(sint);
    }
    else
    {
        pack[2] = ARG_INT_;
    }

    pack[4] = (sint&0x0000ff00) >> 8;
    pack[3] = sint&0x000000ff;
    sendPack_(pack);

    return 0;
}

//---------------------------------------------------------------------
// This function updates the robot info.
//---------------------------------------------------------------------
void AmigoBot::updateRobotInfo_(RobotInfo_t robotInfo)
{
    HSPose_t pose;
    HSPose_t deltaPose;
    double tdif;
    static RobotInfo_t oldRobotInfo;
    static int batteryWarn = 0;
    int i, xdif, ydif;
    static bool initialized = 0;

    if (!initialized)
    {
        oldRobotInfo = robotInfo;
        initialized = true;
        return;
    }

    xdif = (int)((float)(robotInfo.xPos - oldRobotInfo.xPos) * DIST_CONV_FACTOR_);
    ydif = (int)((float)(robotInfo.yPos - oldRobotInfo.yPos) * DIST_CONV_FACTOR_);
    tdif = (float)(robotInfo.thetaPos - oldRobotInfo.thetaPos) * ANGLE_CONV_FACTOR_RAD_*180/M_PI;;

    if (xdif < -500)
    {
        xdif = (int)((MAX_ENCODER_VALUE_ - oldRobotInfo.xPos + robotInfo.xPos) * DIST_CONV_FACTOR_);
    }

    if (xdif > 500)
    {
        xdif = -(int)((MAX_ENCODER_VALUE_ - robotInfo.xPos + oldRobotInfo.xPos) * DIST_CONV_FACTOR_);
    }

    if (ydif < -500)
    {
        ydif = (int)((MAX_ENCODER_VALUE_ - oldRobotInfo.yPos + robotInfo.yPos) * DIST_CONV_FACTOR_);
    }

    if (ydif > 500)
    {
        ydif = -(int)((MAX_ENCODER_VALUE_ - robotInfo.yPos + oldRobotInfo.yPos) * DIST_CONV_FACTOR_);
    }

    ROBOT_CRAMPDEG(tdif, -200, 200);

    // Account for a reset robot
    if ((robotInfo.xPos == 0) && (robotInfo.yPos == 0) && (robotInfo.thetaPos == 0))
    {
        xdif = 0;
        ydif = 0;
        tdif = 0;
    }

    memset((HSPose_t *)&deltaPose, 0x0, sizeof(HSPose_t));
    deltaPose.loc.x = (double)xdif / (1000.0);
    deltaPose.loc.y = (double)ydif / (1000.0);
    deltaPose.rot.yaw = tdif;
    deltaPose.extra.time = getCurrentEpochTime();

    advancePose_(deltaPose);
    getPose(pose);

    for(i = 0; i < numSonars_; i++)
    {
        if (robotInfo.sonarRange[i] > 0)
        {
            sonar_[i] = (robotInfo.sonarRange[i] * SONAR_RANGE_FACTOR_) / 1000.0;

            // If the distance being read was bigger than sonar's detectable range, mark it as
            // no reading. Otherwise, adjust the readings by considering the width of the robot.
            if (sonar_[i] > MAX_SONAR_READABLE_DISTANCE_)
            {
                sonar_[i] = SONAR_READING_OUTSIDE_RANGE;
            }
            else
            {
                sonar_[i] += SONAR_POS_[i];
            }

            sonarLoc_[i][0] = pose.loc.x;
            sonarLoc_[i][1] = pose.loc.y;
            sonarLoc_[i][2] = pose.rot.yaw;
        }
    }

    battery_level = (double)(robotInfo.battery);

    battery_ = (double)(robotInfo.battery) / 10.0;

    if (battery_ < MIN_ROBOT_VOLTAGE_)
    {
        if ((batteryWarn % 30) == 0)
        {
            printTextWindow("Warning: Battery low");
            printf("\a");
        }

        batteryWarn++;
    }

    oldRobotInfo = robotInfo;
}

//---------------------------------------------------------------------
// This function enables the sonar
//---------------------------------------------------------------------
void AmigoBot::sonarStart(void)
{
    char polling[12] = {8, 7, 6, 5, 4, 3, 2, 1, 0};
    sendString_(polling, (byte_t)CMD_POLLING_);

    if (report_level >= HS_REP_LEV_DEBUG)
    { 
        printTextWindow("AmigoBot sonar start");
    }

    isRangeOn_ = true;
}

//---------------------------------------------------------------------
// This function disables the sonar
//---------------------------------------------------------------------
void AmigoBot::sonarStop(void)
{
    char polling[12] = {0};

    sendString_(polling, (byte_t)CMD_POLLING_);

    if (report_level >= HS_REP_LEV_DEBUG)
    { 
        printTextWindow("AmigoBot sonar stop");
    }

    isRangeOn_ = false;
}

//---------------------------------------------------------------------
// This function decodes the Sync to a packet
//---------------------------------------------------------------------
void AmigoBot::decodeSync2Pack_(byte_t pack[])
{
    isSyncing_ = false;
    isSynced_ = true;
    redrawWindows();
    robotName_ = strdup((char*) &(pack[2]));
    robotClass_ = strdup((char*) &(pack[3 + strlen(robotName_)]));
    subClass_ = strdup((char*) &(pack[4 + strlen(robotName_) + strlen(robotClass_)]));

    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printfTextWindow("Sync Completed  Name: %s  Class: %s  Sub-Class: %s", 
                          robotName_,
			  robotClass_,
			  subClass_);
    }

    sendSync_(CMD_OPEN_);  
    sonarStop();
    sendInt_(1, (byte_t)CMD_ENABLE_);
    //sendInt_(0x0101, (byte_t)CMD_DIGOUT_);  
}

//---------------------------------------------------------------------
// This function decodes the status packet.
//---------------------------------------------------------------------
void AmigoBot::decodeStatusPack_(byte_t pack[])
{
    static RobotInfo_t robotInfo;
    int num, i, base;

    robotInfo.status = pack[1];
    robotInfo.xPos = ((int)pack[3]<<8)|(int)pack[2];
    robotInfo.yPos = ((int)pack[5]<<8)|(int)pack[4];
    robotInfo.thetaPos = ((int)pack[7]<<8)|(int)pack[6];
    robotInfo.leftVel = ((int)pack[9]<<8)|(int)pack[8];
    robotInfo.rightVel = ((int)pack[11]<<8)|(int)pack[10];
    robotInfo.battery = pack[12];
    robotInfo.leftBumper = pack[13];
    robotInfo.rightBumper = pack[14];
    robotInfo.control = ((int) pack[16]<<8)|(int)pack[15];
    robotInfo.ptu     = ((int) pack[18]<<8)|(int)pack[17];
    robotInfo.sonarReadings = pack[20];

    memset(robotInfo.sonarRange, 0x0, sizeof(unsigned int)*numSonars_);

    if (robotInfo.sonarReadings > numSonars_)
    {
        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printTextWindow("Bad packet recieved");
        }
        return;
    }

    for(i = 0; i < robotInfo.sonarReadings; i++)
    {
        num = pack[21+(i*3)];
        robotInfo.sonarRange[num] = ((int)pack[23 + (i*3)] << 8) | (int)pack[22 + (i*3)];
    }

    base = 21 + (robotInfo.sonarReadings*3);
    robotInfo.inputTimer = ((int) pack[base+1] << 8) | (int) pack[base];
    robotInfo.analog = pack[base+2];
    robotInfo.digIn  = pack[base+3];
    robotInfo.digOut = pack[base+4];

    updateRobotInfo_(robotInfo);
    updateStatusBar_();
}

//---------------------------------------------------------------------
// This function decodes the packet content.
//---------------------------------------------------------------------
int AmigoBot::decodePack_(byte_t pack[])
{
    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printTextWindow("AmigoBot packet: ", false);
    }

    switch(pack[1]) {

    case 0:
        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printTextWindow("SYNC0", true);
        }
        sendSync_(1);
        break;

    case 1:
        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printTextWindow("SYNC1", true);
        }
        sendSync_(2);  
        break;

    case 2:
        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printTextWindow("SYNC2", true);
        }
        decodeSync2Pack_(pack);
        break;

    case PACK_VISION_:
        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printTextWindow("VISION Packet", true);
        }
        break;

    case PACK_VISIONIO_:
        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printTextWindow("VISIONIO Packet", true);
        }
        break;

    case PACK_FRAME24_:
        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printTextWindow("FRAME24 Packet", true);
        }
        break;

    case PACK_DIST_:
        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printTextWindow("DIST Packet", true);
        }
        break;

    case 0x32:
    case 0x33:
        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printTextWindow("Status Packet", true);
        }
        decodeStatusPack_(pack);
        break;

    default:
        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printfTextWindow("Unknown: %2x", pack[1]);
        }
        break;
    }

    return pack[1];
}

//-----------------------------------------------------------------------
// This function controls the robot with the control speed.
//-----------------------------------------------------------------------
void AmigoBot::controlRobot_(void)
{
    HSSpeed_t controlSpeed;

    if (!isSynced_)
    {
        return;
    }

    getControlSpeed_(controlSpeed);

    // Note: Drive command is in mm/s.
	sendInt_((int)(ROBOT_M2MM(controlSpeed.driveSpeed.value)), (byte_t)CMD_VEL_);
    sendInt_((int)(controlSpeed.steerSpeed.value), (byte_t)CMD_RVEL_);
}

//---------------------------------------------------------------------
// This function runs the syncer in a loop
//---------------------------------------------------------------------
void AmigoBot::syncerLoop_(void)
{
    while(true)
    {
        if (isSynced_)
        {
            pthread_exit(0);
        }

        usleep(SYNCER_LOOP_USLEEP_);
        sendSync_(CMD_PULSE_);
    }
}


//---------------------------------------------------------------------
// This function runs the syncer
//---------------------------------------------------------------------
void AmigoBot::runSyncer_(void)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printTextWindow("AmigoBot: Sync");
    }

    isSyncing_ = true;
    isSynced_ = false;

    updateStatusBar_();

    pthread_create(&syncerThread_, &attr, &startSyncerThread_, (void*)this);
}

//---------------------------------------------------------------------
// This function intialize the x, y, and theta
//---------------------------------------------------------------------
void AmigoBot::initRobot(void)
{
    //resetPose_();
    sendInt_(0, (byte_t)CMD_SETO_);
}

//---------------------------------------------------------------------
// This function reads a packet from the port.
//---------------------------------------------------------------------
int AmigoBot::readPack_(void)
{
    int count, checksum, rchecksum, n, len;
    byte_t pack[1024], hd[2], *data;

    n = nextRemainingBytes_(hd, 2);

    if (n == -1)
    {
        return -1; // Read timeout in testing
    }

    if (n < 2)
    {
        n = nextRemainingBytes_(hd + 1, 1);
    }

    if (n == -1)
    {
        return -1; // Read timeout in testing
    }

    if ((hd[0] != 0xFA) || (hd[1] != 0xFB))
    {
        if (report_level >= HS_REP_LEV_ERR)
        {
            printTextWindow("AmigoBot: bad packet");
        }
        return -1;
    }

    len = count = pack[0] = nextByte_();
    data = pack + 1;

    while (len)
    {
        n = nextRemainingBytes_(data, len);

        if (n == -1) return -1; // Read timeout in testing

        len -= n;
        data += n;
    }

    checksum = (pack[count - 1] << 8) | (pack[count]);
    rchecksum = calcCheckSum_(pack);

    if (checksum != rchecksum)
    {
        printfTextWindow("AmigoBot: Checksum failed\n");
        return -1;
    }

    if (isSynced_ || isSyncing_)
    {
        decodePack_(pack);
    }

    return pack[1];
}


//---------------------------------------------------------------------
// This function runs the reader in a loop
//---------------------------------------------------------------------
void AmigoBot::readerLoop_(void)
{
    int count = 0;
    
    isReading_ = true;

    while(true)
    {
        // Make sure termination was not requested.
        pthread_testcancel();

        // Read the data from the robot.
        readPack_();

        // Compute the control speed.
        compControlSpeed_();

        // Control the robot.
        controlRobot_();

        // Check for the pthread termination again.
        pthread_testcancel();

        // Compute the frequency.
        compFrequency_();

        // Compute the velocity.
        updateVelocity_();

        if (count > SKIP_STATUSBAR_UPDATE_)
        {
            updateStatusBar_();
            count = 0;
        }
        count++;

        usleep(READER_LOOP_USLEEP_);
    }
}

//---------------------------------------------------------------------
// This function configure the sensors
//---------------------------------------------------------------------
void AmigoBot::addSensors_(void)
{

    partXyt_ = new RobotPartXyt();

    partSonar_ = new RobotPartSonar(numSonars_, (float *)SONAR_ANGLES_);

    sensorSonar_ = new Sensor(
	SENSOR_SONAR,
	sizeof(sonarLoc_),
	(char*)sonarLoc_, 
	numSonars_,
	(float *)SONAR_ANGLES_,
	numSonars_,
	sonar_);
}


//---------------------------------------------------------------------
// This function sends commands to the robot based on the user input
//---------------------------------------------------------------------
void AmigoBot::control()
{
    int c;
    int done = 0;
    int msgData[3];

    msgData[0] = isRangeOn_;
    msgData[1] = 0; // "speed_factor" is obsolete.
    msgData[2] = 0; // "angular_speed_factor" is obsolete.
    messageDrawWindow(EnMessageType_AMIGOBOT, EnMessageErrType_NONE, msgData);

    redrawWindows();

    do
    {
        c = getch();
        switch(c) {

        case 'd':
            messageHide();
            delete this;
            return;
            break;

        case 's':
            if (!isSynced_ && !isSyncing_)
            {
                runSyncer_();
            }
            break;

        case 'p':
            isRangeOn_ = !isRangeOn_;
            msgData[0] = isRangeOn_;
            if (isRangeOn_)
            {
                sonarStart();
            }
            else
            {
                sonarStop();
            }
            messageDrawWindow(EnMessageType_AMIGOBOT, EnMessageErrType_NONE, msgData);
            break;

        case 'r':
            refreshScreen();
            break;

        case 'x':
        case 'Q':
        case KEY_ESC:
            done = true;
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
            messageMovePanelKey(c);
            break;
        }
    } while (!done);
    messageHide();
}

//---------------------------------------------------------------------
// This function starts the reader thread
//---------------------------------------------------------------------
void *AmigoBot::startReaderThread_(void *amigoBotInstance)
{
    ((AmigoBot*)amigoBotInstance)->readerLoop_();
    return NULL;
}

//---------------------------------------------------------------------
// This function starts the syncer thread
//---------------------------------------------------------------------
void *AmigoBot::startSyncerThread_(void *amigobotInstance)
{
    ((AmigoBot*)amigobotInstance)->syncerLoop_();
    return NULL;
}

/**********************************************************************
# $Log: amigobot.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:51  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.3  2005/03/26 00:30:54  endo
# check for isSynced_ added in controlRobot_().
#
# Revision 1.2  2004/09/10 19:41:05  endo
# New PoseCalc integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:51  endo
# Initial MARS 2020 revision
#
# Revision 1.2  2003/04/02 23:16:16  zkira
# Added battery level
#
# Revision 1.1  2002/02/18 13:36:09  endo
# Initial revision
#
#
#**********************************************************************/
