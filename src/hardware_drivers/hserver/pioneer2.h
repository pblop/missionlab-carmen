/**********************************************************************
 **                                                                  **
 **                             pioneer2.h                           **
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

/* $Id: pioneer2.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef PIONEER2_H
#define PIONEER2_H


#include <pthread.h>
#include <string>
#include "robot.h"
#include "sensors.h"
#include "robot_config.h"

using std::string;

#define PIONEER2_NUM_SONAR 16
#define PIONEER2_NUM_IR 2

typedef unsigned char byte_t;

class Pioneer2 : public Robot {

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
    double velocity,avelocity;
    int numIRs,numSonars;
    int sonar_type;
    int fd;
    float sonar[PIONEER2_NUM_SONAR];
    float sonar_loc[PIONEER2_NUM_SONAR][3];
    float ir[PIONEER2_NUM_IR];
    bool syncing,synced,rangeOn;

    static const int SKIP_STATUSBAR_UPDATE_;
    static const int READER_LOOP_USLEEP_;

    void addSensors(void);
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
    byte_t nextByte(void);
    int nextRemainingBytes(byte_t* bytes, int max);
    void getWord(char buf[]);
    int round(double n);
    void set_nibbles(int pos, byte_t buf[]);
    int calc_chksum(byte_t* ptr);
    int shortAngle(int ang1, int ang2);
    void updateStatusBar_(void);
    void setDigOut(int port, int mode);
    void syncerLoop_(void);
    void readerLoop_(void);
    void controlRobot_(void);

    static void *startSyncerThread_(void *pioneer2Instance);
    static void *startReaderThread_(void* pioneer2Instance);

  public:
    void syncer_thread(void);
    void reader_thread(void);
    void initRobot(void);
    void sonarStart(void);
    void sonarStop(void);
    void control(void);
    Pioneer2(Robot** a, const string& strPortString);
    ~Pioneer2(void);
};

#endif

/**********************************************************************
# $Log: pioneer2.h,v $
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
# Revision 1.2  2003/06/19 20:50:20  endo
# *** empty log message ***
#
# Revision 1.1  2003/04/25 21:33:17  ebeowulf
# Initial revision
#
#**********************************************************************/
