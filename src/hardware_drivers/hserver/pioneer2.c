/**********************************************************************
 **                                                                  **
 **                             pioneer2.c                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **  Modified by: Eric Martinson (for DXE robot)                     **
 **                                                                  **
 **  Pioneer2 robot driver for HServer                               **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: pioneer2.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

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

#include "HServerTypes.h"
#include "message.h"
#include "pioneer2.h"
#include "hserver.h"
#include "camera.h"
#include "statusbar.h"
#include "robot_config.h"
#include "sensors.h"

#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif

#define AngleConvFactor      0.0057       // radians per encoder count diff 
#define DAngleConvFactor     0.0879       // deg per encoder count diff 
//#define AngleConvFactor      0.0057       // radians per encoder count diff 
//#define DAngleConvFactor     0.3267       // deg per encoder count diff 
#define DistConvFactor       0.969        // (mm/count)
#define RobotRadius          330.0        // radius in mm
#define RobotDiagonal        120.0        // half-height to diagonal of octagon
#define Holonomic            1            // turns in own radius 
#define MaxRVelocity         100.0        // degrees per second
#define MaxVelocity          500.0        // mm per second
#define RangeConvFactor      0.268        // sonar range mm per 2 usec tick

#define MaxSonarReadableDistance 5.0      // usually, the pioneer sonars can detect about 5.6 meters
#define ShaftDegPerRealDeg   180.0/160.0  // fudge factor

#define PIONEER2_PULSE 0
#define PIONEER2_OPEN 1
#define PIONEER2_POLLING 3
#define PIONEER2_SETO 7
#define PIONEER2_VEL 11
#define PIONEER2_HEAD 12
#define PIONEER2_DHEAD 13
#define PIONEER2_SAY 15
#define PIONEER2_RVEL 21
#define PIONEER2_DIGOUT 30
#define PIONEER2_PTZ 42

#define PIONEER2_ARGINT  0x3B
#define PIONEER2_ARGNINT 0x1B
#define PIONEER2_ARGSTR  0x2B

#define VISIONIO_COM 100
#define VISIONIOpac 0x80
#define VISIONpac 0x50
#define FRAME24pac 0xb0
#define DISTpac 0xd0

#define BLOB_MODE 0
#define BLOB_BB_MODE 2
#define LINE_MODE 1

#define TURNACCURACY 2
#define SONARMAX 10000
#define SONARHOLDTIME 10
#define IR_DISTANCE 290

float pioneer2_range_angle[] = {90, 50, 30, 10, -10, -30, -50, -90, -90, -130, -150, -170, 170, 150, 130, 90 }; 

float pioneer2_range_angle_IR[] = { 60, 300 };

float pioneer2_range_loc[PIONEER2_NUM_SONAR] = 
{  0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.20, 0.23, 0.24, 0.24, 0.24, 0.24, 0.23, 0.20 };

int numSonars2 = PIONEER2_NUM_SONAR;
int numIRs2 = 0;
// bool gbAlreadySending = false;



struct t_sip 
{
    byte_t Status;
    int Xpos;
    int Ypos;
    int Th_pos;
    int L_vel;
    int R_vel;
    byte_t Battery;
    byte_t LBumper;
    byte_t RBumper;
    int Control;
    int PTU;
    byte_t Compass;
    byte_t Sonar_readings;
    unsigned int Sonar_range[PIONEER2_NUM_SONAR];
    int Input_timer;
    byte_t Analog;
    byte_t Digin;
    byte_t Digout;
};  


#define READ_BACKOFF 100

const int Pioneer2::READER_LOOP_USLEEP_ = 1000;
const int Pioneer2::SKIP_STATUSBAR_UPDATE_ = 10;

void Pioneer2::flushSerialLine()
{
    byte_t buf[100];
    int cnum;
  
    do
    {
        cnum=read(fd, buf, 100);
        if (cnum == -1)
        {
            cnum = 0;
        }
    } while(cnum);
}


byte_t Pioneer2::nextByte()
{
    int cnum;
    byte_t byte;
  
    while(true)
    {
        cnum = read(fd, &byte, 1);
        if (cnum)
        {
            break;
        }
        // NEED TO SLEEP, DO NOT REMOVE
        usleep(READ_BACKOFF);
    }
    return byte;
}

int Pioneer2::nextRemainingBytes(byte_t *bytes, int max)
{
    int cnum;
    int numTries = 25;

    do
    {
        cnum = read(fd, bytes, max);
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

void Pioneer2::getWord(char buf[])
{
    int i = 0;
    byte_t nchar;

    do
    {
        nchar = nextByte();
        if ((nchar != 191) && (nchar != ' ') && (nchar != 10))
        {
            buf[i++] = nchar;
        }
    } while ((nchar != 191) && (nchar != ' ') && (nchar != 0));
    buf[i] = 0;
}

int Pioneer2::round(double n)
{
    if(n < 0)
    { 
        return((int)(n - 0.5));
    }
    else 
    {
        return((int)(n + 0.5));
    }
}

void Pioneer2::set_nibbles(int pos, byte_t buf[])
{
    buf[0] = (pos & 0xf000) >> 12;
    buf[1] = (pos & 0x0f00) >> 8;
    buf[2] = (pos & 0x00f0) >> 4;
    buf[3] = (pos & 0x000f) >> 0;
}

int Pioneer2::calc_chksum(byte_t* ptr)
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
        c = c ^ ((int) *(ptr++));
    }
    return c;
}

int Pioneer2::shortAngle(int ang1, int ang2)
{
    int a, b;

    if (ang1 < ang2) {
        a = ang2 - ang1;
        b = 360 - ang2 + ang1;
    }
    else {
        a = ang1 - ang2;
        b = 360 - ang1 + ang2;
    }
    return (a < b) ? a : b;
}

void Pioneer2::updateStatusBar_()
{
    HSPose_t pose;
    char buf[100];

    statusStr[0] = 0;
  
    strcat(statusStr, "Pioneer2: ");

    sprintf(buf, "%c ", statusbarSpinner_->getStatus());
    strcat(statusStr, buf);

    if (syncing)
    {
        strcat(statusStr, "Syncing");
    }
    else if(!synced)
    {
        strcat(statusStr, "Not synced");
    }
    else
    {      
        getPose(pose);

        sprintf(
            buf,
            " %.2fV  x: %.2f  y: %.2f  t: %.2f ",
            battery,
            pose.loc.x,
            pose.loc.y,
            pose.rot.yaw);

        strcat(statusStr, buf);
    }
    statusbar->update(statusLine);
}


int Pioneer2::sendPack_(byte_t pack[])
{
    int checksum;
    byte_t header[2];
    byte_t chksum[2];

    header[0] = 0xFA;
    header[1] = 0xFB;
    checksum = calc_chksum(pack);
    chksum[0] = (checksum & 0x0000ff00) >> 8;
    chksum[1] = checksum & 0x000000ff;
    pthread_cleanup_push((void(*)(void*)) pthread_mutex_unlock, (void*) &serialMutex);
    pthread_mutex_lock(&serialMutex);
    write(fd, header, 2);
    write(fd, pack, pack[0] - 1);
    write(fd, chksum, 2);
    pthread_cleanup_pop(1);
    return 0;
}

int Pioneer2::sendSync(int s)
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


int Pioneer2::sendString(char str[], byte_t command)
{
    byte_t pack[255];
    int i;

    pack[0] = 5 + strlen(str);
    pack[1] = command;
    pack[2] = PIONEER2_ARGSTR;
    pack[3] = strlen(str);
    for(i = 0; i < (int) strlen(str); i++)
    {
        pack[4 + i] = str[i];
    }
    sendPack_(pack);
    return 0;
}

int Pioneer2::sendStringN(char str[], int len, byte_t command)
{
    byte_t pack[255];
    int i;

    pack[0] = 5 + len;
    pack[1] = command;
    pack[2] = PIONEER2_ARGSTR;
    pack[3] = len;
    for(i = 0; i < len; i++)
    {
        pack[4 + i] = str[i];
    }
    sendPack_(pack);
    return 0;
}  

int Pioneer2::sendInt_(int sint, byte_t command)
{
    byte_t pack[40];
  
    pack[0] = 6;
    pack[1] = command;
    if (sint < 0)
    {
        pack[2] = PIONEER2_ARGNINT;
        sint = abs(sint);
    }
    else pack[2] = PIONEER2_ARGINT;
    pack[4] = (sint&0x0000ff00) >> 8;
    pack[3] = sint&0x000000ff;
    sendPack_(pack);
    return 0;
}

void Pioneer2::useSip(struct t_sip sip)
{
    HSPose_t pose;
    HSPose_t deltaPose;
    static struct t_sip oldsip;
    static int battery_warn = 0;
    int i, xdif, ydif;
    double tdif;
    const int maxxy = 65535;
    int doIR = 1;
    double tmpSonar;

    xdif = (int)((sip.Xpos - oldsip.Xpos) * DistConvFactor);
    ydif = (int)((sip.Ypos - oldsip.Ypos) * DistConvFactor);
    tdif = (double(sip.Th_pos) - (double)(oldsip.Th_pos)) * DAngleConvFactor;

    if (xdif < -500)
    {
        xdif = (int)((maxxy - oldsip.Xpos + sip.Xpos) * DistConvFactor);
    }
    if (xdif > 500)
    {
        xdif = -(int)((maxxy - sip.Xpos + oldsip.Xpos) * DistConvFactor);
    }
    if (ydif < -500)
    {
        ydif = (int)((maxxy - oldsip.Ypos + sip.Ypos) * DistConvFactor);
    }
    if (ydif > 500)
    {
        ydif = -(int)((maxxy - sip.Ypos + oldsip.Ypos) * DistConvFactor);
    }

    ROBOT_CRAMPDEG(tdif, -200.0, 200.0);

    // Account for a reset robot
    if ((sip.Xpos == 0) && (sip.Ypos == 0) && (sip.Th_pos == 0))
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

    compass[0] = (double)(sip.Compass * 2);
    for(i = 0; i < PIONEER2_NUM_SONAR; i++)
    {
        if (sip.Sonar_range[i] > 0)
        {
            sonar[i] = (sip.Sonar_range[i] * RangeConvFactor) / 1000.0;

            // If the distance being read was bigger than sonar's detectable range, mark it as
            // no reading. Otherwise, adjust the readings by considering the width of the robot.
            if (sonar[i] > MaxSonarReadableDistance)
            {
                sonar[i] = SONAR_READING_OUTSIDE_RANGE;
            }
            else
            {
                sonar[i] += pioneer2_range_loc[i];
            }

            sonar_loc[i][0] = pose.loc.x;
            sonar_loc[i][1] = pose.loc.y;
            sonar_loc[i][2] = pose.rot.yaw;
        }
    }

    battery = (double)(sip.Battery) / 10.0;

    if (battery < 10.8)
    {
        if ((battery_warn % 30) == 0)
        {
            printTextWindow("Warning: Battery low");
            printf("\a");
        }
        battery_warn++;
    }
    oldsip = sip;

    if (numIRs2)
    {
        if (doIR)
        {
            // read the IR values as long as this isn't our
            // first sip received
            int i, which = 0x2, base;
            base = numSonars2 - numIRs2;
            for (i = 0; i < numIRs2; i++)
            {
                if (sip.Digin & which)
                {
                    tmpSonar = SONARMAX;
                }
                else
                {
                    tmpSonar = IR_DISTANCE;
                }
                ir[i] = (double) (tmpSonar) / 1000.0;
                which = which << 1;
            } 
        }
        sendInt_(0x0202, PIONEER2_DIGOUT); // hold the Vin high
        usleep(3000);                    // wait 6 milliseconds
        sendInt_(0x0200, PIONEER2_DIGOUT); // pull the Vin low
        // we need to wait atleast 56 milliseconds so
        // wait till the next sip
    }
}


void Pioneer2::sonarStart()
{
    char polling[20] = { 8, 7, 6, 5, 4, 3, 2, 1, 16, 15, 14, 13 ,12 ,11, 10, 9, 0 }; // ERIC

    sendString(polling, PIONEER2_POLLING);

    if (report_level >= HS_REP_LEV_DEBUG)
    { 
        printTextWindow("Pioneer2 sonar start");
    }

    rangeOn = true;
}

void Pioneer2::sonarStop()
{
    char polling[12] = { 0 };
    sendString(polling, PIONEER2_POLLING);
    if (report_level >= HS_REP_LEV_DEBUG)
    { 
        printTextWindow("Pioneer2 sonar stop");
    }
    rangeOn = false;
}

void Pioneer2::decodeSync2Pack(byte_t pack[])
{
    syncing = false;
    synced = true;
    redrawWindows();
    robotName = strdup((char*) &(pack[2]));
    robotClass = strdup((char*) &(pack[3 + strlen(robotName)]));
    subclass = strdup((char*) &(pack[4 + strlen(robotName) + strlen(robotClass)]));
    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printfTextWindow("Sync Completed  Name: %s  Class: %s  Subclass: %s", 
                          robotName, robotClass, subclass);
    }
    sendSync(1);  
    sonarStop();
  
    sendInt_(0x0101, PIONEER2_DIGOUT);  
}

void Pioneer2::decodeStatusPack(byte_t pack[])
{
    static struct t_sip sip;
    int num, i, base;

    //pioneerSendSync(0);  
    sip.Status = pack[1];
    sip.Xpos   = ((int) pack[3 ] << 8) | (int) pack[2 ];
    sip.Ypos   = ((int) pack[5 ] << 8) | (int) pack[4 ];
    sip.Th_pos = ((int) pack[7 ] << 8) | (int) pack[6 ];
    sip.L_vel  = ((int) pack[9 ] << 8) | (int) pack[8 ];
    sip.R_vel  = ((int) pack[11] << 8) | (int) pack[10];
    sip.Battery = pack[12];
    sip.LBumper = pack[13];
    sip.RBumper = pack[14];
    sip.Control = ((int) pack[16] << 8) | (int) pack[15];
    sip.PTU     = ((int) pack[18] << 8) | (int) pack[17];
    sip.Compass = pack[19];
    sip.Sonar_readings = pack[20];
    for(i = 0; i < 8; i++)
    {
        sip.Sonar_range[i] = 0;
    }
    if (sip.Sonar_readings>8)
    {
        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printTextWindow("Bad packet recieved");
        }
        return;
    }
    for(i = 0; i < 8; i++)
    {
        sip.Sonar_range[i] = 0;
    }
    for(i = 0; i < sip.Sonar_readings; i++)
    {
        num = pack[21 + i * 3];
        sip.Sonar_range[num] = ((int) pack[23 + i * 3] << 8) | (int) pack[22 + i * 3];
    }
    base = 21 + sip.Sonar_readings * 3;
    sip.Input_timer = ((int) pack[base + 1] << 8) | (int) pack[base];
    sip.Analog = pack[base + 2];
    sip.Digin  = pack[base + 3];
    sip.Digout = pack[base + 4];

//    printfTextWindow("DigOut....%x\n",sip.Digout);
    useSip(sip);
    updateStatusBar_();
}

int Pioneer2::decodePack(byte_t pack[])
{
    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printTextWindow("pioneer packet: ", false);
    }
    switch(pack[1])
    {
    case 0:
        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printTextWindow("SYNC0", true);
        }
        sendSync(1);  
        break;
    case 1:
        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printTextWindow("SYNC1", true);
        }
        sendSync(2);  
        break;
    case 2:
        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printTextWindow("SYNC2", true);
        }
        decodeSync2Pack(pack);
        break;
    case VISIONpac:
        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printTextWindow("VISIONpac", true);
        }
        //decodeVisionPack(pack);
        break;
    case VISIONIOpac:
        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printTextWindow("VISIONIOpac", true);
        }
        //decodeVisionIoPack(pack);
        break;
    case FRAME24pac:
        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printTextWindow("FRAME24pac", true);
        }
        //decodeFrame24Pack(pack);
        break;
    case DISTpac:
        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printTextWindow("DISTpac", true);
        }
        break;
    case 0x32:
    case 0x33:
        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printTextWindow("Status Packet", true);
        }
        decodeStatusPack(pack);
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

void Pioneer2::controlRobot_(void)
{
    HSSpeed_t controlSpeed;

    if (!synced)
    {
        return;
    }

    getControlSpeed_(controlSpeed);

    // Note: Drive command is in mm/s.
	sendInt_((int)(ROBOT_M2MM(controlSpeed.driveSpeed.value)), PIONEER2_VEL);
    sendInt_((int)(controlSpeed.steerSpeed.value), PIONEER2_RVEL);
}

void *Pioneer2::startSyncerThread_(void *pioneer2Instance)
{
    ((Pioneer2*) pioneer2Instance)->syncerLoop_();
    return NULL;
}


void Pioneer2::syncerLoop_(void)
{
    while(true)
    {
        if (synced)
        {
            pthread_exit(0);
        }

        usleep(500000);
        sendSync(0);
    }
}

void Pioneer2::sync(void)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printTextWindow("Pioneer2: Sync");
    }

    syncing = true;
    synced = false;

    updateStatusBar_();

    pthread_create(&syncerThread_, &attr, &startSyncerThread_, (void*)this);
}

void Pioneer2::initRobot(void)
{
    //resetPose_();
    sendInt_(0, PIONEER2_SETO);
}

int Pioneer2::readPack_()
{
    int count, checksum, rchecksum, n, len;
    byte_t pack[1024], hd[2], *data;

    n = nextRemainingBytes(hd, 2);
    if (n == -1)
    {
        return -1;	// Read timeout in testing
    }
    if (n < 2)
    {
        n = nextRemainingBytes(hd + 1, 1);
    }
    if (n == -1)
    {
        return -1;	// Read timeout in testing
    }
    if ((hd[0] != 0xFA) || (hd[1] != 0xFB))
    {
        if (report_level >= HS_REP_LEV_ERR)
        {
            printTextWindow("pioneer: bad packet");
        }
        return -1;
    }
    len = count = pack[0] = nextByte();
    data = pack + 1;
    while (len)
    {
        n = nextRemainingBytes(data, len);
        if (n == -1)	// Read timeout in testing
        {
            return -1;
        }
        len -= n;
        data += n;
    }
    //if (report_level >= HS_REP_LEV_DEBUG){ 
    //  fprintf(stderr, "PACKET len: {%d} ", count);
    //  for (i = 0; i <= count; i++) {
    //    fprintf(stderr, "%2x ", pack[i]);
    //  }
    //  fprintf(stderr, "\n");
    //}
    checksum = (pack[count - 1] << 8) | (pack[count]);
    rchecksum = calc_chksum(pack);
    if (checksum != rchecksum)
    {
        fprintf(stderr, "PServer: Checksum failed\n");
        return -1;
    }
    if (synced || syncing)
    {
        decodePack(pack);
    }
    return pack[1];
}

int Pioneer2::testRobotConnection()
{
    int rtn;
    sendSync(2);
    sendSync(0);
    sendSync(0);
    sendSync(0);
    rtn = readPack_();
    if (rtn == 0)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}


/* **** PIONEER2 AT PSOS COMMUNICATION */

Pioneer2::~Pioneer2()
{
    if (synced)
    {
        sendSync(2);
        sendSync(2);
    }

    if (syncing)
    {
        pthread_cancel(syncerThread_);
        pthread_join(syncerThread_, NULL);
    }

    syncing = false;
    pthread_cancel(readerThread_);
    pthread_join(readerThread_, NULL);
    delete sensorSonar;
    delete sensorIr;
    delete sensorCompass;
    delete partSonar;
    delete partIr;
    delete partXyt;
    delete partCompass;
    printTextWindow("Pioneer2 disconnected");
    close(fd);
    if(robotName != NULL)
    {
        free(robotName);
        free(robotClass);
        free(subclass);
    }
    redrawWindows();
}

void *Pioneer2::startReaderThread_(void* pioneer2Instance)
{
    ((Pioneer2*)pioneer2Instance)->readerLoop_();
    return NULL;
}

void Pioneer2::readerLoop_(void)
{
    int count = 0;

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

void Pioneer2::addSensors(void)
{
    partXyt = new RobotPartXyt();

    partSonar = new RobotPartSonar(PIONEER2_NUM_SONAR, pioneer2_range_angle);
    sensorSonar = new Sensor(SENSOR_SONAR, sizeof(sonar_loc), (char*) sonar_loc, PIONEER2_NUM_SONAR, pioneer2_range_angle, PIONEER2_NUM_SONAR, sonar);

    partIr = new RobotPartIr(PIONEER2_NUM_IR, pioneer2_range_angle_IR);
    sensorIr = new Sensor(SENSOR_IR, 0, NULL, PIONEER2_NUM_IR, pioneer2_range_angle_IR, PIONEER2_NUM_IR, ir);

    partCompass = new RobotPartCompass();
    sensorCompass = new Sensor(SENSOR_COMPASS, 0, NULL, 1, compass);
    compass[0] = 0.0;

}

Pioneer2::Pioneer2(Robot** a, const string& strPortString) : 
    Robot(a, HS_ROBOT_TYPE_PIONEER2, HS_MODULE_NAME_ROBOT_PIONEER2)
{
    int num_irs = PIONEER2_NUM_IR;

    robotName = NULL;
    pthread_mutex_init(&serialMutex, NULL);
    syncing = false;
    synced = false;
    numSonars2 = PIONEER2_NUM_SONAR;
    numIRs2 = num_irs;
    if ((report_level > HS_REP_LEV_NONE) && (report_level != HS_REP_LEV_DEBUG))
    {
        printTextWindow("Connecting to pioneer... ", false);
    }
    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printTextWindow("Connecting to pioneer2");
        printfTextWindow("robot serial port: %s", strPortString.c_str());
        printfTextWindow("8 sonar sensors + %d infared sensors", num_irs);
    }
    setupSerial(&fd, strPortString.c_str(), B9600, report_level, "Robot", 1);
    sendSync(2);
    flushSerialLine();
    if (!testRobotConnection())
    {
        redrawWindows();
        updateStatusBar_();
        redrawWindows();
    }
    else
    {
        if ((report_level > HS_REP_LEV_NONE) && (report_level != HS_REP_LEV_DEBUG))
        {
            printTextWindow("failed", true);
        }
        else if (report_level > HS_REP_LEV_NONE)
        {
            printTextWindow("Pioneer2 connection failed", true);
        }
    }
    if ((report_level > HS_REP_LEV_NONE) && (report_level != HS_REP_LEV_DEBUG))
    {
        printTextWindow("connected", true);
    }
    else if (report_level > HS_REP_LEV_NONE)
    {
        printTextWindow("Pioneer2 connected", true);
    }
    pthread_create(&readerThread_, NULL, &startReaderThread_, (void*)this);
    addSensors();
    sync();
    for (int i=0;i<8;i++)
	setDigOut(i,0);
}


void Pioneer2::control(void)
{
    int c;
    int done = 0;
    int msgData[3];

    msgData[0] = rangeOn;
    msgData[1] = 0; // "speed_factor" is obsolete.
    msgData[2] = 0; // "angular_speed_factor" is obsolete.
    messageDrawWindow(EnMessageType_PIONEER2, EnMessageErrType_NONE, msgData);
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
            if (!synced && !syncing)
            {
                sync();
            }
            break;
        case 'p':
            rangeOn = !rangeOn;
            msgData[0] = rangeOn;
            if (rangeOn)
            {
                sonarStart();
            }
            else
            {
                sonarStop();
            }
            messageDrawWindow(EnMessageType_PIONEER2, EnMessageErrType_NONE, msgData);
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

void Pioneer2::setDigOut(int port, int mode)
{
   
    unsigned int myOut = (1<<(port+8))|(mode<<port);
    sendInt_(myOut, PIONEER2_DIGOUT);
}

/**********************************************************************
# $Log: pioneer2.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.3  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.4  2005/03/26 00:41:30  endo
# *** empty log message ***
#
# Revision 1.3  2005/03/26 00:30:55  endo
# check for isSynced_ added in controlRobot_().
#
# Revision 1.2  2004/09/10 19:41:07  endo
# New PoseCalc integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:51  endo
# Initial MARS 2020 revision
#
# Revision 1.1  2003/04/25 21:34:04  ebeowulf
# Initial revision
#
#**********************************************************************/
