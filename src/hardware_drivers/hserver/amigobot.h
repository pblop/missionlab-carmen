/**********************************************************************
 **                                                                  **
 **                            amigobot.h                            **
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

/* $Id: amigobot.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef AMIGOBOT_H
#define AMIGOBOT_H


#include <pthread.h>
#include <string>
#include "hserver.h"
#include "robot.h"
#include "sensors.h"
#include "robot_config.h"

#define AMIGOBOT_NUM_SONARS  8

class AmigoBot : public Robot
{
    typedef struct RobotInfo_t {
        byte_t status;
        int xPos;
        int yPos;
        int thetaPos;
        int leftVel;
        int rightVel;
        byte_t battery;
        byte_t leftBumper;
        byte_t rightBumper;
        int control;
        int ptu;
        byte_t sonarReadings;
        unsigned int sonarRange[AMIGOBOT_NUM_SONARS];
        int inputTimer;
        byte_t analog;
        byte_t digIn;
        byte_t digOut;
    };

protected:

    Sensor *sensorSonar_;
    RobotPartXyt *partXyt_;
    RobotPartSonar *partSonar_;
    pthread_t readerThread_, syncerThread_;
    pthread_mutex_t serialMutex_;
    char *robotName_;
    char *robotClass_;
    char *subClass_;
    int numSonars_;
    int commFd_;
    bool isSyncing_, isSynced_, isReading_, isRangeOn_;
    float sonar_[AMIGOBOT_NUM_SONARS];
    float sonarLoc_[AMIGOBOT_NUM_SONARS][3];
    double battery_;

    static const int CMD_SYNC0_;
    static const int CMD_SYNC1_;
    static const int CMD_SYNC2_;
    static const int CMD_PULSE_;
    static const int CMD_OPEN_;
    static const int CMD_CLOSE_;
    static const int CMD_POLLING_;
    static const int CMD_ENABLE_;
    static const int CMD_SETO_;
    static const int CMD_VEL_;
    static const int CMD_HEAD_;
    static const int CMD_DHEAD_;
    static const int CMD_SAY_;
    static const int CMD_RVEL_;
    static const int CMD_DIGOUT_;
    static const int READ_BACKOFF_USEC_;
    static const int READER_LOOP_USLEEP_;
    static const int SYNCER_LOOP_USLEEP_;
    static const int MAX_ENCODER_VALUE_;
    static const int MIN_TURN_AVEL_;
    static const int SKIP_STATUSBAR_UPDATE_;
    static const byte_t ARG_INT_;
    static const byte_t ARG_NINT_;
    static const byte_t ARG_STR_;
    static const byte_t PACK_VISIONIO_;
    static const byte_t PACK_VISION_;
    static const byte_t PACK_FRAME24_;
    static const byte_t PACK_DIST_;
    static const float SONAR_ANGLES_[];
    static const float SONAR_POS_[];
    static const double MAX_SONAR_READABLE_DISTANCE_;
    static const double ANGLE_CONV_FACTOR_RAD_;
    static const double DIST_CONV_FACTOR_;
    static const double VELOCITY_CONV_FACTOR_;
    static const double SONAR_RANGE_FACTOR_;
    static const double MIN_ROBOT_VOLTAGE_;

    byte_t nextByte_(void);
    int testRobotConnection_(void); 
    int sendSync_(int s);
    int sendPack_(byte_t pack[]);
    int readPack_(void);
    int calcCheckSum_(byte_t* ptr);
    int nextRemainingBytes_(byte_t* bytes, int max);
    int decodePack_(byte_t pack[]);
    void flushSerialLine_(void);
    void decodeSync2Pack_(byte_t pack[]);
    int sendString_(char str[], byte_t command);
    int sendInt_(int sint, byte_t command);
    void decodeStatusPack_(byte_t pack[]);
    void updateRobotInfo_(RobotInfo_t robotInfo);
    void addSensors_(void);
    void runSyncer_(void);
    void syncerLoop_(void);
    void readerLoop_(void);
    void updateStatusBar_(void);
    void controlRobot_(void);

    static void *startSyncerThread_(void *amigobotInstance);
    static void *startReaderThread_(void *amigoBotInstance);

public:
    AmigoBot(Robot** a, const string& strPortString);
    ~AmigoBot(void);
    void sonarStop(void);
    void sonarStart(void);
    void control(void);
    void initRobot(void);
};

#endif

/**********************************************************************
# $Log: amigobot.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:51  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2004/09/10 19:43:24  endo
# New PoseCalc integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:51  endo
# Initial MARS 2020 revision
#
# Revision 1.1  2002/02/18 13:36:09  endo
# Initial revision
#
#
#**********************************************************************/
