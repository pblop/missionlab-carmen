/**********************************************************************
 **                                                                  **
 **                        ContrilInterfaces.c                       **
 **                                                                  **
 **                                                                  **
 **  Written by:  Brian Lee                                          **
 **                                                                  **
 **  Copyright 2002 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: ControlInterfaces.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include <string>

#include "ControlInterfaces.h"
#include "message.h"
#include "fred.h"
#include "nomad.h"
#include "pioneer.h"
#include "pioneer2.h"
#include "amigobot.h"
#include "roomba.h"
#include "roomba560.h"
#include "carmenRobot.h"
#include "compass.h"
#include "compass_3DM-G.h"
#include "gyro.h"
#include "ipc_client.h"

#ifndef NO_MOBILITY
#include "urban.h"
#include "atrvjr.h"
#include "compass_kvh-c100.h"
#include "gyro_dmu-vgx.h"
#endif

#include "PoseCalculatorInterface.h"

using std::string;

void robotConnect(Robot*& robot)
{
    char c;
    string strHost;
    string strPortString;
    int poseCalcID;

    char robot_type = messageGetChar(EnMessageFilterType_ROBOT_TYPE,
                                      EnMessageType_ROBOT_TYPE, EnMessageErrType_ROBOT_TYPE);
    if (robot_type != 'x')
    {
        const unsigned int uPIONEER_PORT_ECHO_LINE = 4;
        const unsigned int uPIONEER2_PORT_ECHO_LINE = 4;
        const unsigned int uAMIGOBOT_PORT_ECHO_LINE = 4;
		const unsigned int uROBOTLASER_PORT_ECHO_LINE = 4;
		const unsigned int uROOMBA_PORT_ECHO_LINE = 4;
		const unsigned int uROOMBA560_PORT_ECHO_LINE = 4;
		const unsigned int uCARMEN_PORT_ECHO_LINE = 4;

        //const unsigned int uCARRETILLA_PORT_ECHO_LINE = 4;

        switch(robot_type) {

        case '1': // Emulated Robot (fred)
            new Fred(&robot);
            break;

        case '2': // Pioneer AT
            strPortString = messageGetString(EnMessageType_PIONEER_PORT, uPIONEER_PORT_ECHO_LINE);
            new Pioneer(&robot, strPortString);
            break;

	    case '3': // Pioneer2 DXE
            strPortString = messageGetString(EnMessageType_PIONEER2_PORT, uPIONEER2_PORT_ECHO_LINE);
            new Pioneer2(&robot, strPortString);
            break;

        case '4': // AmigoBot
            strPortString = messageGetString(EnMessageType_AMIGOBOT_PORT, uAMIGOBOT_PORT_ECHO_LINE);
            new AmigoBot(&robot, strPortString);
            break;

        case '5': // Nomad 150/200
            c = messageGetChar(EnMessageFilterType_NOMAD, EnMessageType_NOMAD,
                                EnMessageErrType_NOMAD_TYPE);
            if (c != 'x')
            {
                const unsigned int uNOMAD_150_PORT_ECHO_LINE = 4;
                const unsigned int uNOMAD_200_HOST_NAME_ECHO_LINE = 3;

                Nomad::EnNomadType enType;

                switch(c - '0')
                {
                case 1:
                    enType = Nomad::EnNomadType_150;
                    strPortString = messageGetString(EnMessageType_NOMAD_150_PORT,
                                                      uNOMAD_150_PORT_ECHO_LINE);
                    strHost = "";
                    break;
                case 2:
                    enType = Nomad::EnNomadType_200;
                    strPortString = "";
                    strHost = messageGetString(EnMessageType_NOMAD_200_HOST_NAME,
                                                uNOMAD_200_HOST_NAME_ECHO_LINE);
                    break;
                default:
                    printTextWindow("Bad Nomad type in robotConnect()");
                    return;
                }
		        new Nomad(&robot, enType, strPortString, strHost);
            }
            break;

        case '6': // Urban Robot
#ifndef NO_MOBILITY
            new Urban(&robot);
#else
            printTextWindow("No Mobility support");
#endif
            break;

        case '7': // ATRV-Jr Robot
#ifndef NO_MOBILITY
	        new ATRVJr(&robot);
#else
            printTextWindow("No Mobility support");
#endif
            break;

	case '9': // Roomba
            strPortString = messageGetString(EnMessageType_ROOMBA_PORT, uROOMBA_PORT_ECHO_LINE);
            new Roomba(&robot, strPortString);
            break;
	case 'a': // Roomba560
            strPortString = messageGetString(EnMessageType_ROOMBA560_PORT, uROOMBA560_PORT_ECHO_LINE);
            new Roomba560(&robot, strPortString);
            break;

	case 'c':
    case 'C': //Carmen
    		if(ipc_preferred_server_name() == NULL)
    			strPortString = messageGetString(EnMessageType_CARMEN_PORT, uCARMEN_PORT_ECHO_LINE);
    		else
    			strPortString = "";

			new CarmenRobot(&robot, strPortString);
			break;
        }


        if (robot != NULL)
        {
            poseCalcID = gPoseCalc->addRobot(robot);
            robot->savePoseCalcID(poseCalcID);
        }
    }
}

void compassConnect(Compass*& compass)
{
    char compassType;
    string portString;
    const unsigned int uCOMPASS_3DMG_PORT_ECHO_LINE = 4;
    int poseCalcID;

    compassType = messageGetChar(
        EnMessageFilterType_COMPASS_TYPE,
        EnMessageType_COMPASS_TYPE,
        EnMessageErrType_COMPASS_TYPE);

    if (compassType != 'x')
    {
        switch(compassType)
        {

        case '1': // KVH-C100
#ifndef NO_MOBILITY
            new CompassKVHC100(&compass);
#else
            printTextWindow("No Mobility support");
#endif
            break;

        case '2': // 3DM-G
            portString = messageGetString(
                EnMessageType_COMPASS_3DMG_PORT,
                uCOMPASS_3DMG_PORT_ECHO_LINE);
            new Compass3DMG(&compass, portString);
            break;

        }

        if (compass != NULL)
        {
            poseCalcID = gPoseCalc->addCompass(compass);
            compass->savePoseCalcID(poseCalcID);
        }
    }
}

void gyroConnect(Gyro*& gyro)
{
    char gyroType;
    int poseCalcID;

    gyroType = messageGetChar(
        EnMessageFilterType_GYRO_TYPE,
        EnMessageType_GYRO_TYPE,
        EnMessageErrType_GYRO_TYPE);

    if (gyroType != 'x')
    {
        switch(gyroType) {

        case '1': // DMU-VGX
#ifndef NO_MOBILITY
            new GyroDMUVGX(&gyro);
#else
            printTextWindow("No Mobility support");
#endif
            break;

        default:
            break;
        }

        if (gyro != NULL)
        {
            poseCalcID = gPoseCalc->addGyro(gyro);
            gyro->savePoseCalcID(poseCalcID);
        }
    }
}

void gpsConnect(
    Gps*& gps,
    double base_lat,
    double base_lon,
    double base_x,
    double base_y,
    double mperlat,
    double mperlon)
{
    int c, poseCalcID;
    bool base = false;
    unsigned const int uPORT_ECHO_LINE = 4;

    printfTextWindow("\n");
    printfTextWindow("GPS can only be started through the command line option.\n");
    printfTextWindow("Restart hserver with '-g <gps_section in .hserverrc>'.\n");

    return;

    // Note: Below is an obsolete code.

    c = messageGetChar(EnMessageFilterType_GPS, EnMessageType_GPS_SELECT, EnMessageErrType_GPS);

    if (c != 'x')
    {
        base = (c == '0');

        // get the connection port from the user
        string strPort = messageGetString(EnMessageType_GPS_PORT, uPORT_ECHO_LINE);

        new GpsDefault(&gps, strPort, base_lat, base_lon, base_x, base_y, mperlat, mperlon, base);

        if (gps != NULL)
        {
            poseCalcID = gPoseCalc->addGps(gps);
            gps->savePoseCalcID(poseCalcID);
        }
    }
}

void controlGps(Gps* gps)
{
    bool bDone = false;
    int msgData[4];

    msgData[0] = gps->GetSavingPackets();
    msgData[1] = (int) gps->GetBaseX();
    msgData[2] = (int) gps->GetBaseY();

    messageDrawWindow(EnMessageType_GPS_CONTROL, EnMessageErrType_NONE, msgData);
    do
    {
        int c = getch();
        switch(c)
        {
        case 'd':
            messageHide();
            delete gps;
            return;
            break;

        case 's':
            gps->SetSavePackets(!gps->GetSavingPackets());
            msgData[0] = gps->GetSavingPackets();
            messageDrawWindow(EnMessageType_GPS_CONTROL, EnMessageErrType_NONE, msgData);
            break;
        case 'q':
            gps->AlterBaseYBy(1.0);
            msgData[2] = (int) gps->GetBaseY();
            messageDrawWindow(EnMessageType_GPS_CONTROL, EnMessageErrType_NONE, msgData);
            break;
        case 'a':
            gps->AlterBaseYBy(-1.0);
            msgData[2] = (int) gps->GetBaseY();
            messageDrawWindow(EnMessageType_GPS_CONTROL, EnMessageErrType_NONE, msgData);
            break;
        case 'w':
            gps->AlterBaseXBy(-1.0);
            msgData[1] = (int) gps->GetBaseX();
            messageDrawWindow(EnMessageType_GPS_CONTROL, EnMessageErrType_NONE, msgData);
            break;
        case 'e':
            gps->AlterBaseXBy(1.0);
            msgData[1] = (int) gps->GetBaseX();
            messageDrawWindow(EnMessageType_GPS_CONTROL, EnMessageErrType_NONE, msgData);
            break;
            gps->information();
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

void controlJbox(Jbox* jbox)
{
    int c;
    int i,j;
    bool bDone = false;
    const unsigned int JBOX_ECHO_LINE = 4;
    int j_pos[7];
    double j_gps[7];

    string strJboxString;
    messageDrawWindow(EnMessageType_JBOX_CONTROL, EnMessageErrType_NONE);
    redrawWindows();

    do
    {
        c = getch();
        switch(c)
        {
            case 'd':
                messageHide();
                delete jbox;
                //useJbox = false;
                bDone = true;
                break;

            case 'q':
                messageHide();
                messageDrawWindow (EnMessageType_JBOX_SELECT, EnMessageErrType_NONE);
                redrawWindows();
                strJboxString = messageGetString(EnMessageType_JBOX_SELECT, JBOX_ECHO_LINE);
                i=atoi(strJboxString.c_str());
                messageHide();

                if (jbox != NULL)
                {
                    j = jbox->quality_to_neighbor(i);

                    if (j == -1)
                    {
                        messageDrawWindow (EnMessageType_NONE, EnMessageErrType_JBOX_NOTFOUND);
                    }
                    else
                    {
                        messageDrawWindow (EnMessageType_JBOX_DISPLAY_NSS, EnMessageErrType_NONE, &j);
                    }
                }
                else
                {
                    messageDrawWindow (EnMessageType_NONE, EnMessageErrType_JBOX_NOTCONNECT);
                }

                redrawWindows();

                break;

            case 'h':
                messageHide();
                messageDrawWindow (EnMessageType_JBOX_SELECT, EnMessageErrType_NONE);
                redrawWindows();
                strJboxString = messageGetString(EnMessageType_JBOX_SELECT, JBOX_ECHO_LINE);
                i=atoi(strJboxString.c_str());
                messageHide();
                if (jbox != NULL)
                {
                    j = jbox->HOPS_of_node(i);
                    if (j == -1)
                    {
                        messageDrawWindow (EnMessageType_NONE, EnMessageErrType_JBOX_NOTFOUND);
                    }
                    else
                    {
                        messageDrawWindow (EnMessageType_JBOX_DISPLAY_HOPS, EnMessageErrType_NONE, &j);
                    }
                }
                else
                {
                    messageDrawWindow (EnMessageType_NONE, EnMessageErrType_JBOX_NOTCONNECT);
                }
                redrawWindows();
                break;

            case 'p':
                messageHide();
                messageDrawWindow (EnMessageType_JBOX_SELECT, EnMessageErrType_NONE);
                redrawWindows();
                strJboxString = messageGetString(EnMessageType_JBOX_SELECT, JBOX_ECHO_LINE);
                i=atoi(strJboxString.c_str());
                messageHide();

                if (jbox != NULL)
                {
                    j = jbox->POS_of_node(j_pos, i);

                    if (j == -1)
                    {
                        messageDrawWindow (EnMessageType_NONE, EnMessageErrType_JBOX_NOTFOUND);
                    }
                    else
                    {
                        messageDrawWindow (EnMessageType_JBOX_DISPLAY_POS, EnMessageErrType_NONE, j_pos);
                    }
                }
                else
                {
                    messageDrawWindow (EnMessageType_NONE, EnMessageErrType_JBOX_NOTCONNECT);
                }

                redrawWindows();

                break;

            case 'g':
                messageHide();
                messageDrawWindow (EnMessageType_JBOX_SELECT, EnMessageErrType_NONE);
                redrawWindows();
                strJboxString = messageGetString(EnMessageType_JBOX_SELECT, JBOX_ECHO_LINE);
                i=atoi(strJboxString.c_str());
                messageHide();

                if (jbox != NULL)
                {
                    j = jbox->GPS_of_node(j_gps, i);

                    if (j == -1)
                    {
                        messageDrawWindow (EnMessageType_NONE, EnMessageErrType_JBOX_NOTFOUND);
                    }
                    else
                    {
                        messageDrawWindow (EnMessageType_JBOX_DISPLAY_GPS, EnMessageErrType_NONE, j_gps);
                    }
                }
                else
                {
                    messageDrawWindow (EnMessageType_NONE, EnMessageErrType_JBOX_NOTCONNECT);
                }

                redrawWindows();
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

/**********************************************************************
# $Log: ControlInterfaces.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:51  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.6  2004/09/22 03:59:34  endo
# *** empty log message ***
#
# Revision 1.5  2004/09/10 19:41:06  endo
# New PoseCalc integrated.
#
# Revision 1.4  2004/04/24 11:20:53  endo
# 3DM-G added.
#
# Revision 1.3  2004/04/17 12:20:34  endo
# Intalling JBox Sensor Update.
#
# Revision 1.2  2004/04/13 19:25:33  yangchen
# JBox module integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:51  endo
# Initial MARS 2020 revision
#
# Revision 1.5  2003/06/19 20:52:45  endo
# *** empty log message ***
#
# Revision 1.4  2003/04/26 06:48:31  ebeowulf
# Added the Pioneer2-dxe controller.
#
# Revision 1.3  2002/02/18 13:48:21  endo
# AmigoBot added.
#
#
#Añadido el 2008/10/09 RobotLaser added
#
#Añadido el 2008/10/28 Roomba added
#Añadido el 2009/02/18 Roomba560 added
#Añadido el 2009/06/19 Carretilla added
#Añadido el 2009/11/03 CarretillaSimulacion added
#**********************************************************************/
