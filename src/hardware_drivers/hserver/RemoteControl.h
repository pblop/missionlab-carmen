/**********************************************************************
 **                                                                  **
 **                          RemoteControl.h                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: RemoteControl.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef REMOTE_CONTROL_H
#define REMOTE_CONTROL_H

#include <pthread.h>

#include "HServerTypes.h"

class RemoteControl {

protected:
    HSSpeed_t shRobotControlSpeed_; // Shared
    pthread_t controllerThread_;
    bool controllerIsUp_;
    
    static pthread_mutex_t robotControlSpeedMutex_;

    static const int CONTROLLER_THREAD_USLEEP_;

    void processKeyInputs_(void);
    void controllerLoop_(void);
    void resetRobotControlSpeed_(void);
    void setRobotControlDriveSpeed_(double driveSpeed);
    void setRobotControlSteerSpeed_(double steerSpeed);
    void setRobotControlArmSpeed_(double armSpeed);
    void getRobotControlSpeed_(HSSpeed_t &speed);
    void sendRobotControlCommands_(void);

    static void *startControllerThread_(void* controllerInstance);

public:
    RemoteControl(void);
    ~RemoteControl(void);
    void runRemoteControl(void);
};

extern RemoteControl *gRemoteControl;

#endif

/**********************************************************************
# $Log: RemoteControl.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:51  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1  2004/09/10 19:43:28  endo
# New PoseCalc integrated.
#
#**********************************************************************/
