/**********************************************************************
 **                                                                  **
 **                            ipt_handler.c                         **
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

/* $Id: ipt_handler.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "HServerTypes.h"
#include "hserver.h"
#include "hserver_ipt.h"
#include "ipt_handler.h"
#include "statusbar.h"
#include "message.h"
#include "console.h"
#include "laser.h"
#include "ipt/callbacks.h"
#include "ipt/connection.h"
#include "Watchdog.h"

IptHandler* iptHandler = NULL;

declareHandlerCallback(IptHandler)
implementHandlerCallback(IptHandler)

declareConnectionCallback(IptHandler)
implementConnectionCallback(IptHandler)

const double IptHandler::READER_THREAD_SLEEP_SEC_ = 0.1;
const int IptHandler::SKIP_STATUSBAR_UPDATE_ = 0;
const int IptHandler::WATCHDOG_CHECK_INTERVAL_SEC_ = 5;

void IptHandler::control_connect_callback(IPConnection* conn)
{
    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printfTextWindow("IPT: New connection with %s from %s", conn->Name(), conn->Host());
    }
    if (!strncmp("hclient", conn->Name(), 7))
    {
        hclient_name = strdup(conn->Name());
        hclient_host = strdup(conn->Host());
        consoleConn = true;
        updateStatusBar_();
    }
}

void IptHandler::control_disconnect_callback(IPConnection* conn)
{
    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printfTextWindow("IPT: Lost connection with %s from %s", conn->Name(), conn->Host());
    }
    if (!strncmp("hclient", conn->Name(), 7))
    {
        consoleConn = false;
        updateStatusBar_();
    }
}

void IptHandler::updateStatusBar_(void)
{
    char buf[100];

    statusStr[0] = '\0';

    strcat(statusStr, "Ipt:");

    sprintf(buf, " %c", statusbarSpinner_->getStatus());
    strcat(statusStr, buf);

    if (ipt_server_found)
    {
        if (consoleMod)
        {
            strcat(statusStr, " console");
            if (consoleConn) strcat(statusStr, "[up]");
            else strcat(statusStr, "[down]");
        }
        if (gpsMod)
        {
            strcat(statusStr, " gps");
            if (gpsConn) strcat(statusStr, "[up]");
            else strcat(statusStr, "[down]");
        }
        if (laserMod)
        {
            strcat(statusStr, " laser");
            if (laserConn) strcat(statusStr, "[up]");
            else strcat(statusStr, "[down]");
        }
        strcat(statusStr, " Bytes Received: ");
        sprintf(buf, "%ld", totalBytesReceived_);
        strcat(statusStr, buf);

        strcat(statusStr, "  Bytes Sent: ");
        sprintf(buf, "%ld", totalBytesSent_);
        strcat(statusStr, buf);
    }
    else
    {
        sprintf(buf, " looking for ipt server on %s", ipt_server_name);
        strcat(statusStr, buf);
    }
    statusbar->update(statusLine);
}

void IptHandler::set_state(int mod, bool listen, bool up)
{
    switch(mod)
    {
    case IPT_MOD_CONSOLE:
        consoleMod = listen;
        consoleConn = up;
        break;

    case IPT_MOD_GPS:
        gpsMod = listen;
        gpsConn = up;
        break;

    case IPT_MOD_LASER:
        laserMod = listen;
        laserConn = up;
        break;
    }
    updateStatusBar_();
}

void IptHandler::information()
{
    printfTextWindow("Iptserver is running on %s", control_comm->ServerHostName());
    printfTextWindow("Hserver module %s running on %s", control_comm->ModuleName(), 
                     control_comm->ThisHost());
    if (control_comm->DomainName() != NULL)
    {
        printfTextWindow("Domain %s", control_comm->DomainName());
    }
}

void IptHandler::registerNamedFormatters(IPFormatSpec formats[])
{
    control_comm->RegisterNamedFormatters(formats);
}

void IptHandler::registerMessages(IPMessageSpec messages[])
{
    control_comm->RegisterMessages(messages);
}

void IptHandler::registerHandlers(int num, IPMsgHandlerSpec hndArray[])
{
    int i;
    IPMessageType* msg_type;
    for(i = 0; i < num; i++)
    {
        msg_type = control_comm->LookupMessage(hndArray[i].msg_name);
        control_comm->RegisterHandler(msg_type, hndArray[i].callback, 
                                       hndArray[i].data, hndArray[i].context);
    }
}
    
void IptHandler::disableHandlers(int num, char* hndNames[])
{
    int i;
    IPMessageType* msg_type;
    for(i = 0; i < num; i++)
    {
        msg_type = control_comm->LookupMessage(hndNames[i]);
        control_comm->DisableHandler(msg_type);
    }
}

void IptHandler::registerCallbackHandlers(int num, IPMsgCallbackHandlerSpec hndArray[])
{
    int i;
    IPMessageType* msg_type;
    for(i = 0; i < num; i++)
    {
        msg_type = control_comm->LookupMessage(hndArray[i].msg_name);
        control_comm->RegisterHandler(msg_type, hndArray[i].callback, hndArray[i].context);
    }
}

void IptHandler::callbackModuleId(IPMessage* message)
{
    REC_ModuleId* rec = (REC_ModuleId*) message->FormattedData();
    REC_ModuleId rply;
    char* modname;

    switch(rec->id)
    {
    case MODULE_ID_GPS:    
        modname = "gps";
        break;

    case MODULE_ID_LASER:
        modname = "laser";
        break;

    default:
        modname = "unknown";
    }
    printfTextWindow("Module connecting: %s", modname);
    
    switch(rec->id) {

    case MODULE_ID_GPS:
        if (*gps != NULL)
        {
            (*gps)->connectRemote();
            rply.id = MODULE_ID_GPS;
        }
        else
        {
            rply.id = MODULE_NOT_CONNECTED;
        }
        break;
    case MODULE_ID_LASER:
        // jbl -- laser stuff -- COME BACK TO THIS!!!
        // This will only work with one laser being in existence, but because
        // of time I couldn't spend time making this more general.
        if (Laser::m_clLaserList.size() > 0)
        {
            Laser::m_clLaserList[0]->connectRemote();
            rply.id = MODULE_ID_LASER;
        }
        else
        {
            rply.id = MODULE_NOT_CONNECTED;
        }
        break;

    default:
        rply.id = MODULE_UNKNOWN;
    }
    control_comm->Reply(message, MSG_ModuleId, &rply);
}

void IptHandler::initIpt(void)
{
    setenv("IPTOUTPUT", "none", 1);

    if (multiple)
    {
        sprintf(ipt_name, "control");
    }
    else
    {
        sprintf(ipt_name, "hserver_%s", robot_name);
    }
    
    control_comm = IPCommunicator::Instance(ipt_name, ipt_server_name);

    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printfTextWindow("IPT  module name: %s  server: %s  thishost: %s  domain: %s", 
                          control_comm->ModuleName(), control_comm->ServerHostName(), 
                          control_comm->ThisHost(), control_comm->DomainName());
    }
    else if (report_level > HS_REP_LEV_NONE)
    {
        printfTextWindow("Connected to IPT server on %s", control_comm->ServerHostName());
    }
    registerMessages(iptHandlerMessageArray);
    callback = new HandlerCallback(IptHandler)(this, &IptHandler::callbackModuleId);
    IPMsgCallbackHandlerSpec hndArray[] = {
        { MSG_ModuleId, callback, IPT_HNDL_STD }
    };
    registerCallbackHandlers(1, hndArray);

    updateStatusBar_();
}

static void start_thread(void* ipth_instance)
{
    ((IptHandler*) ipth_instance)->reader_thread();
}

// Thread for main ipt loop
void IptHandler::reader_thread(void)
{
    static int count = 0;

    updateStatusBar_();
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    while(true)
    {
        pthread_testcancel();
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        control_comm->ScanForMessages(1);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);


        if (count > SKIP_STATUSBAR_UPDATE_)
        {
            updateStatusBar_();
            count = 0;
        }
        count++;

        if (watchdog_ != NULL)
        {
            watchdog_->notifyUpdate();
        }

        control_comm->Sleep(READER_THREAD_SLEEP_SEC_);
    }
}  

void IptHandler::listen_hclient()
{
    consoleConnect();
}

IptHandler::IptHandler(
    IptHandler** a,
    char* name,
    char* robot_name,
    Gps** gps,
    bool multiple) :
    Module((Module**) a, HS_MODULE_NAME_IPT),
    totalBytesReceived_(0),
    totalBytesSent_(0)
{
    IPConnectionCallback* callback;
    ipt_name = new char[100];
    this->gps = gps;
    gpsMod = false;
    laserMod = false;
    consoleMod = false;
    consoleConn = false;
    this->robot_name = strdup(robot_name);
    this->multiple = multiple;

    if (name != NULL)
    {
        ipt_server_name = strdup(name);
    }
    else if (getenv("IPTHOST") != NULL)
    {
        ipt_server_name = strdup(getenv("IPTHOST"));
    }
    else if (getenv("HOST") != NULL)
    {
        ipt_server_name = strdup(getenv("HOST"));
    }
    else
    {
        ipt_server_name = strdup("localhost");
    }
    ipt_server_found = false;

    initIpt();
    ipt_server_found = true;
    callback = new ConnectionCallback(IptHandler)(this, &IptHandler::control_connect_callback);
    control_comm->AddConnectCallback(callback);
    callback = new ConnectionCallback(IptHandler)(this, &IptHandler::control_disconnect_callback);
    control_comm->AddDisconnectCallback(callback);

    updateStatusBar_();
    pthread_create(&reader, NULL, (void * (*)(void *)) &start_thread, (void*) this);
    printTextWindow("Ipt handler started");

    if (gWatchdogEnabled)
    {
        watchdog_ = new Watchdog(NAME_, WATCHDOG_CHECK_INTERVAL_SEC_, getpid(), reader);
    }
}

IptHandler::~IptHandler()
{
    pthread_cancel(reader);
    pthread_join(reader, NULL);
    free(control_comm);
    delete [] ipt_name;
    free(robot_name);
    free(ipt_server_name);
    printTextWindow("Ipt hander disconnected");
}

void IptHandler::control()
{
    bool bDone = false;

    messageDrawWindow(EnMessageType_CONSOLE_CONTROL, EnMessageErrType_NONE);
    do
    {
        int c = getch();
        switch(c)
        {
        case 'd':
            messageHide();
            delete this;
            return;
            break;
        case 'i':
            information();
            break;
        case 'c':
            consoleConnect();
            break;
        case 'r':
            refreshScreen();
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
            messageMovePanelKey(c);
            break;
        }
    } while (!bDone);
    messageHide();
}

void IptHandler::saveNumBytesReceived(int numBytes)
{
    totalBytesReceived_ += numBytes;
}

void IptHandler::saveNumBytesSent(int numBytes)
{
    totalBytesSent_ += numBytes;
}

/**********************************************************************
# $Log: ipt_handler.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.3  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.4  2004/09/10 19:41:06  endo
# New PoseCalc integrated.
#
# Revision 1.3  2004/04/24 11:20:53  endo
# 3DM-G added.
#
# Revision 1.2  2004/04/12 01:13:05  endo
# Core dump upon exit fixed.
#
# Revision 1.1.1.1  2004/02/27 22:30:50  endo
# Initial MARS 2020 revision
#
# Revision 1.6  2003/04/06 13:32:37  endo
# gcc 3.1.1
#
# Revision 1.5  2002/01/16 21:50:16  ernest
# Added WatchDog function
#
# Revision 1.4  2001/05/29 22:37:16  blee
# Made changes to incorporate gps data w/ other position info.
#
# Revision 1.3  2000/12/12 22:57:03  blee
# Altered the case for MODULE_ID_LASER in callbackModuleId().
#
# Revision 1.2  2000/09/19 03:14:21  endo
# RCS log added.
#
#
#**********************************************************************/
