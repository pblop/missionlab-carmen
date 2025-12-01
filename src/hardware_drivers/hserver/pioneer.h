/**********************************************************************
 **                                                                  **
 **                             pioneer.h                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  Pioneer robot driver for HServer                                **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: pioneer.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef PIONEER_H
#define PIONEER_H


#include <pthread.h>
#include <string>
#include "robot.h"
#include "sensors.h"
#include "robot_config.h"

#define PIONEER_NUM_SONAR 8
#define PIONEER_NUM_IR 2
#define SONAR_TYPE_A 0
#define SONAR_TYPE_B 1

typedef unsigned char byte_t;

class Pioneer : public Robot {

  protected:
    Sensor* sensorSonar;
    Sensor* sensorIr;
    Sensor* sensorCompass;
    RobotPartSonar* partSonar;
    RobotPartIr* partIr;
    RobotPartXyt* partXyt;
    RobotPartCompass* partCompass;
    pthread_t readerThread_;
    pthread_t syncerThread_;
    pthread_mutex_t serialMutex;
    char *robotName, *robotClass, *subclass;
    float compass[1];
    double battery;
    int numIRs,numSonars;
    int sonar_type;
    int fd;
    float sonar[PIONEER_NUM_SONAR];
    float sonar_loc[PIONEER_NUM_SONAR][3];
    float ir[PIONEER_NUM_IR];
    bool syncing,synced,rangeOn;

    static const int SKIP_STATUSBAR_UPDATE_;
    static const int READER_LOOP_USLEEP_;

    byte_t nextByte(void);
    int sendPack_(byte_t pack[]);
    int sendSync(int s);
    int sendString(char str[], byte_t command);
    int sendStringN(char str[], int len, byte_t command);
    int sendInt_(int sint, byte_t command);
    void useSip(struct t_sip sip);
    void decodeSync2Pack(byte_t pack[]);
    void decodeStatusPack(byte_t pack[]);
    int decodePack(byte_t pack[]);
    void sync(void);
    int readPack_(void);
    int testRobotConnection(void); 
    void flushSerialLine(void);
    int nextRemainingBytes(byte_t* bytes, int max);
    void getWord(char buf[]);
    int round(double n);
    void set_nibbles(int pos, byte_t buf[]);
    int calc_chksum(byte_t* ptr);
    int shortAngle(int ang1, int ang2);
    void updateStatusBar_(void);
    void syncerLoop_(void);
    void readerLoop_(void);
    void controlRobot_(void);
    void addSensors_(void);

    static void *startSyncerThread_(void *pioneerInstance);
    static void *startReaderThread_(void* pioneerInstance);

  public:
    Pioneer(Robot** a, const string& strPortString);
    ~Pioneer(void);
    void initRobot(void);
    void sonarStart(void);
    void sonarStop(void);
    void reset(void);
    void control(void);
};

#endif

/**********************************************************************
# $Log: pioneer.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.3  2004/09/10 19:43:27  endo
# New PoseCalc integrated.
#
# Revision 1.2  2004/04/24 11:20:53  endo
# 3DM-G added.
#
# Revision 1.1.1.1  2004/02/27 22:30:51  endo
# Initial MARS 2020 revision
#
# Revision 1.4  2001/05/29 22:34:55  blee
# Made changes to incorporate gps data w/ other position info.
#
# Revision 1.3  2001/03/23 21:32:26  blee
# altered to use a config file
#
# Revision 1.2  2000/09/19 03:47:30  endo
# Sonar bug fixed.
#
#
#**********************************************************************/
