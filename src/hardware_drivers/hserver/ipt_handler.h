/**********************************************************************
 **                                                                  **
 **                            ipt_handler.h                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  IPT handler for HServer                                         **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: ipt_handler.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef IPTHANDLER_H
#define IPTHANDLER_H

#include <pthread.h>
#include "module.h"
#include "hserver_ipt.h"
#include "gps.h"

#define IPT_MOD_CONSOLE 0
#define IPT_MOD_GPS     1
#define IPT_MOD_LASER   2

class IptHandler : public Module
{
protected:
    IPCommunicator* control_comm;
    IPHandlerCallback* callback;
    Gps** gps;
    pthread_t reader;
    char* ipt_server_name;
    char* ipt_name;
    char* robot_name;
    char* hclient_name;
    char* hclient_host;
    long int totalBytesReceived_;
    long int totalBytesSent_;
    void initIpt();
    void control_connect_callback( IPConnection* conn );
    void control_disconnect_callback( IPConnection* conn );
    void information();
    void callbackModuleId( IPMessage* message );
    void updateStatusBar_();
    bool ipt_server_found, multiple;
    bool gpsMod, laserMod, consoleMod;
    bool consoleConn, gpsConn, laserConn;

    static const double READER_THREAD_SLEEP_SEC_;
    static const int SKIP_STATUSBAR_UPDATE_;
    static const int WATCHDOG_CHECK_INTERVAL_SEC_;

public:
    IptHandler(
        IptHandler** a,
        char* name,
        char* robot_name,
        Gps** gps, 
        bool multiple = FALSE);
    ~IptHandler(void);
    IPCommunicator* communicator(void) { return control_comm; };
    void set_state( int mod, bool listen, bool up );
    void listen_hclient();
    void reader_thread();
    void control();
    void registerNamedFormatters( IPFormatSpec formats[] );
    void registerMessages( IPMessageSpec messages[] );
    void registerHandlers( int num, IPMsgHandlerSpec hndArray[] );
    void registerCallbackHandlers( int num, IPMsgCallbackHandlerSpec hndArray[] );
    void disableHandlers( int num, char* hndNames[] );
    void saveNumBytesReceived(int numBytes);
    void saveNumBytesSent(int numBytes);
};

extern IptHandler* iptHandler;

#endif

/**********************************************************************
# $Log: ipt_handler.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2004/09/10 19:43:26  endo
# New PoseCalc integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:50  endo
# Initial MARS 2020 revision
#
# Revision 1.4  2002/01/16 21:51:53  ernest
# Added WatchDog function
#
# Revision 1.3  2001/05/29 22:37:22  blee
# Made changes to incorporate gps data w/ other position info.
#
# Revision 1.2  2000/09/19 03:14:21  endo
# RCS log added.
#
#
#**********************************************************************/
