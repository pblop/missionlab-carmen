#ifndef CARMENROBOT_H
#define CARMENROBOT_H

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
#include <sys/poll.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
#include "HServerTypes.h"
#include "message.h"
#include "hserver.h"
#include "camera.h"
#include "statusbar.h"
#include "robot_config.h"
#include "sensors.h"
#include "robot.h"
#include "PoseCalculatorInterface.h"
#include <string>
#include <time.h>
#include "ipt_handler.h"
#include "hserver_ipt.h"
#include "ipc_client.h"

class CarmenRobot : public Robot
{
public:

	CarmenRobot(Robot** a, const string& strPortString);
    virtual ~CarmenRobot();
    virtual void control(void);

    void callbackSonarScan(carmen_base_sonar_message* message);
    void callbackOdometryScan(carmen_base_odometry_message* message);
    void callbackTruePosScan(carmen_simulator_truepos_message* message);

protected:
    IPCommunicator* m_communicator;
    RobotPartXyt *partXyt_;
    RobotPartSonar *partSonar_;
    Sensor *sensorSonar_;
    IPHandlerCallback* m_sonarIptCallback;
    IPHandlerCallback* m_odometryIptCallback;
    IPHandlerCallback* m_localizeIptCallback;
    IPHandlerCallback* m_truePosIptCallback;

    IPConnection *m_carmenConnection;
    pthread_t robotThread_;
    bool isReading_;
    float *sonarAngles;
    float (*sonarLoc)[6];
    float *sonar;

    const char *server;

    void callbackLocalizeScan(IPMessage* message);
    void callbackNavigationStartScan(IPMessage* message);
    void callbackNavigationStopScan(IPMessage* message);

    virtual void controlRobot_();
    void updateStatusBar_(void);
    void robotLoop_(void);
    static void *startRobotThread_(void* carmenRobotInstance);

    static const int SKIP_STATUSBAR_UPDATE_;
    static const int READER_LOOP_USLEEP_;

    HSPose_t newPose;

    static pthread_mutex_t deltaPoseMutex_;

};

#endif //CARMENROBOT_H
