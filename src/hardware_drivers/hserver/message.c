/**********************************************************************
 **                                                                  **
 **                             message.c                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  Message dialog for HServer                                      **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: message.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include <curses.h>
#include <panel.h>
#include <ctype.h>
#include "message.h"
#include "hserver.h"

WINDOW* msgWindow;
PANEL* msgPanel;

int messagePanelX, messagePanelY;

void messageInitialize()
{
    messagePanelX = MESSAGE_WIN_X;
    messagePanelY = MESSAGE_WIN_Y;
    if ((messagePanelY + MESSAGE_WIN_ROW) > screenY)
    {
        messagePanelY = screenY - MESSAGE_WIN_ROW;
    }
    if ((messagePanelX + MESSAGE_WIN_COL) > screenX)
    {
        messagePanelX = screenX - MESSAGE_WIN_COL;
    }
    msgWindow = newwin(MESSAGE_WIN_ROW, MESSAGE_WIN_COL, messagePanelY, messagePanelX);
    msgPanel = new_panel(msgWindow);
    messageDrawWindow(EnMessageType_INTRO, EnMessageErrType_NONE);
    hide_panel(msgPanel);
}

void messageMove()
{
    if (SilentMode) {
       return;
    }
    if ((messagePanelY + MESSAGE_WIN_ROW) > screenY)
    {
        messagePanelY = screenY - MESSAGE_WIN_ROW;
    }
    if ((messagePanelX + MESSAGE_WIN_COL) > screenX)
    {
        messagePanelX = screenX - MESSAGE_WIN_COL;
    }
    move_panel(msgPanel, messagePanelY, messagePanelX);
}

void capInt(int* i, int l, int u)
{
    if (*i < l)
    {
        *i = l;
    }
    if (*i > u)
    {
        *i = u;
    }
}

void messageMovePanel(int xdif, int ydif)
{
    if (SilentMode) {
       return;
    }
    messagePanelX += xdif;
    messagePanelY += ydif;
    capInt(&messagePanelX, 0, mainPanelX - MESSAGE_WIN_COL);
    capInt(&messagePanelY, 0, mainPanelY - MESSAGE_WIN_ROW);
    move_panel(msgPanel, messagePanelY, messagePanelX);
    redrawWindows();
}

/* Numeric keypad moves message window */
void messageMovePanelKey(int c)
{
    switch(c)
    {
    case '1':
        messageMovePanel(-1, 1);
        break;
    case '2':
    case KEY_DOWN:
        messageMovePanel(0, 1);
        break;
    case '3':
        messageMovePanel(1, 1);
        break;
    case '4':
    case KEY_LEFT:
        messageMovePanel(-1, 0);
        break;
    case '5':
        break;
    case '6':
    case KEY_RIGHT:
        messageMovePanel(1, 0);
        break;
    case '7':
        messageMovePanel(-1, -1);
        break;
    case '8':
    case KEY_UP:
        messageMovePanel(0, -1);
        break;
    case '9':
        messageMovePanel(1, -1);
        break;
    }
}

void messageDrawWindow(EnMessageType msg, EnMessageErrType err, void* data /*=NULL*/)
{
    int* msgData;
    double* dmsgData;
    int line = 1;
    char buf[100];

    if (SilentMode) {
        return;
    }

    dmsgData = (double*) data;
    msgData = (int*) data;

    pthread_cleanup_push((void(*)(void*)) pthread_mutex_unlock, (void *) &screenMutex);
    pthread_mutex_lock(&screenMutex);
    werase(msgWindow);
    wmove(msgWindow, 0, 0);
    wborder(msgWindow, 0, 0, 0, 0, 0, 0, 0, 0);
    switch(msg)
    {
    case EnMessageType_NONE:
        break;
    case EnMessageType_INTRO:
        mvwaddstr(msgWindow, line++, 1, "HServer");
        line++;
        mvwaddstr(msgWindow, line++, 1, "Georgia Institute of Technology");
        mvwaddstr(msgWindow, line++, 1, "Mobile Robot Laboratory");
        break;
    case EnMessageType_CAMERA:
        mvwaddstr(msgWindow, line++, 1, "Enter the camera port string.");
        mvwaddstr(msgWindow, line++, 1, "(i.e. /dev/ttyS0)");
        break;
    case EnMessageType_FRAMEGRABBER_PORT:
        mvwaddstr(msgWindow, line++, 1, "Enter the framegrabber port string.");
        mvwaddstr(msgWindow, line++, 1, "(i.e. /dev/video)");
        break;
    case EnMessageType_LASER:
        mvwaddstr(msgWindow, line++, 1, "Enter the laser port string.");
        mvwaddstr(msgWindow, line++, 1, "(i.e. /dev/ttyS0)");
        break;
    case EnMessageType_ROBOT:
        mvwaddstr(msgWindow, line++, 1, "Enter robot port #");
        mvwaddstr(msgWindow, line++, 1, "/dev/ttyS?");
        break;
    case EnMessageType_TERMINAL:
        mvwaddstr(msgWindow, line++, 1, "Enter terminal port #");
        mvwaddstr(msgWindow, line++, 1, "/dev/ttyS?");
        break;
    case EnMessageType_COG_PORT:
        mvwaddstr(msgWindow, line++, 1, "Enter the cognachrome");
        mvwaddstr(msgWindow, line++, 1, "port string.");
        mvwaddstr(msgWindow, line++, 1, "(i.e. /dev/ttyS0)");
        break;
    case EnMessageType_GPS_PORT:
        mvwaddstr(msgWindow, line++, 1, "Enter the gps port string.");
        mvwaddstr(msgWindow, line++, 1, "(i.e. /dev/ttyS0)");
        break;
    case EnMessageType_SOUND_SELECT:
        mvwaddstr(msgWindow, line++, 1, "Enter sound port #");
        mvwaddstr(msgWindow, line++, 1, "/dev/ttyS?");
        break;
    case EnMessageType_TERMINAL_BAUD:
        mvwaddstr(msgWindow, line++, 1, "Enter terminal baud");
        line++;
        mvwaddstr(msgWindow, line++, 1, "1. 9600");
        mvwaddstr(msgWindow, line++, 1, "2. 19200");
        mvwaddstr(msgWindow, line++, 1, "3. 38400");
        mvwaddstr(msgWindow, line++, 1, "4. 57600");
        mvwaddstr(msgWindow, line++, 1, "5. 115200");
        mvwaddstr(msgWindow, line++, 1, "6. 230400");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_TRACKING:
        mvwaddstr(msgWindow, line++, 1, "Enter tracking mode");
        line++;
        mvwaddstr(msgWindow, line++, 1, "1. none");
        mvwaddstr(msgWindow, line++, 1, "2. center");
        mvwaddstr(msgWindow, line++, 1, "3. sweep X");
        mvwaddstr(msgWindow, line++, 1, "4. sweep Y");
        mvwaddstr(msgWindow, line++, 1, "5. sweep XY");
        mvwaddstr(msgWindow, line++, 1, "6. largest Object [cognachrome]");
        mvwaddstr(msgWindow, line++, 1, "7. largest A [cognachrome]");
        mvwaddstr(msgWindow, line++, 1, "8. largest B [cognachrome]");
        mvwaddstr(msgWindow, line++, 1, "9. largest C [cognachrome]");
        mvwaddstr(msgWindow, line++, 1, "a. closest laser reading");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_REMOTE:
        mvwaddstr(msgWindow, line++, 1, " Camera    Robot            ");
        mvwaddstr(msgWindow, line++, 1, "                  cinc:     ");
        mvwaddstr(msgWindow, line++, 1, "    w        i    inc :     ");
        mvwaddstr(msgWindow, line++, 1, "   a0d      j l   vel :     ");
        mvwaddstr(msgWindow, line++, 1, "    s        k    rvel:     ");
        mvwaddstr(msgWindow, line++, 1, "          n<arm>m avel:     ");
        mvwaddstr(msgWindow, line++, 1, "                            ");
        mvwaddstr(msgWindow, line++, 1, "                            ");
        mvwaddstr(msgWindow, line++, 1, "                            ");
        mvwaddstr(msgWindow, line++, 1, "            arm stop: ,    ");
        mvwaddstr(msgWindow, line++, 1, " zoom: zq   stop: uo space ");
        mvwaddstr(msgWindow, line++, 1, " inc:  ce   inc: ;p");
        sprintf(buf, "%d", msgData[0]);
        mvwaddstr(msgWindow, 2, 25, buf);
        sprintf(buf, "%d", msgData[1]);
        mvwaddstr(msgWindow, 3, 25, buf);
        sprintf(buf, "%d", msgData[2]);
        mvwaddstr(msgWindow, 4, 25, buf);
        sprintf(buf, "%d", msgData[3]);
        mvwaddstr(msgWindow, 5, 25, buf);
        sprintf(buf, "%d", msgData[4]);
        mvwaddstr(msgWindow, 6, 25, buf);
        if (msgData[5])
        {
            sprintf(buf, "Teleport %d", msgData[6]);
            mvwaddstr(msgWindow, 8, 10, buf);
        }
        break;
    case EnMessageType_PIONEER_PORT:
        mvwaddstr(msgWindow, line++, 1, "Enter the pioneer port string.");
        mvwaddstr(msgWindow, line++, 1, "(i.e. /dev/ttyS0)");
        break;
    case EnMessageType_PIONEER:
        mvwaddstr(msgWindow, line++, 1, "Pioneer");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d  disconnect");
        mvwaddstr(msgWindow, line++, 1, "s  sync");
        sprintf(buf, "ab sonar type");
        mvwaddstr(msgWindow, line++, 1, buf);
        if (msgData[0])
        {
            mvwaddstr(msgWindow, line++, 1, "p  range stop");
        }
        else
        {
            mvwaddstr(msgWindow, line++, 1, "p  range start");
        }
        mvwaddstr(msgWindow, line++, 1, "r  refresh");
        mvwaddstr(msgWindow, line++, 1, "x  cancel");
        break;
    case EnMessageType_PIONEER2_PORT:
        mvwaddstr(msgWindow, line++, 1, "Enter the pioneer2 port string.");
        mvwaddstr(msgWindow, line++, 1, "(i.e. /dev/ttyS0)");
        break;
    case EnMessageType_PIONEER2:
        mvwaddstr(msgWindow, line++, 1, "Pioneer2");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d  disconnect");
        mvwaddstr(msgWindow, line++, 1, "s  sync");
        if (msgData[0])
        {
            mvwaddstr(msgWindow, line++, 1, "p  range stop");
        }
        else
        {
            mvwaddstr(msgWindow, line++, 1, "p  range start");
        }
        mvwaddstr(msgWindow, line++, 1, "r  refresh");
        mvwaddstr(msgWindow, line++, 1, "x  cancel");
        break;
    case EnMessageType_AMIGOBOT_PORT:
        mvwaddstr(msgWindow, line++, 1, "Enter the amigobot port string.");
        mvwaddstr(msgWindow, line++, 1, "(i.e. /dev/ttyS0)");
        break;

    case EnMessageType_ROBOTLASER_PORT:
        mvwaddstr(msgWindow, line++, 1, "Introducir la direccion IP del servidor");
        mvwaddstr(msgWindow, line++, 1, "(i.e. 127.0.0.1)");
        break;
    case EnMessageType_CARMEN_PORT:
        mvwaddstr(msgWindow, line++, 1, "Enter CENTRALHOST IP and port.");
        mvwaddstr(msgWindow, line++, 1, "(Blank for 127.0.0.1:1381)");
    	break;
	case EnMessageType_ROOMBA_PORT:
        mvwaddstr(msgWindow, line++, 1, "Introducir el puerto del Roomba");
        mvwaddstr(msgWindow, line++, 1, "(i.e. /dev/ttyS0)");
        break;
	case EnMessageType_ROOMBA560_PORT:
        mvwaddstr(msgWindow, line++, 1, "Introducir el puerto del Roomba560");
        mvwaddstr(msgWindow, line++, 1, "(i.e. /dev/ttyS0)");
        break;
   case EnMessageType_WEBCAM_PORT:
        mvwaddstr(msgWindow, line++, 1, "Enter the webCam port string.");
        mvwaddstr(msgWindow, line++, 1, "(i.e. /dev/video0)");
        break;
    case EnMessageType_JBOX_ID:
        mvwaddstr(msgWindow, line++, 1, "Enter the JBox ID.");
        mvwaddstr(msgWindow, line++, 1, "(i.e. 22)");
        break;
    case EnMessageType_JBOX_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "JBox");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d  disconnect");
        mvwaddstr(msgWindow, line++, 1, "q  quality query");
        mvwaddstr(msgWindow, line++, 1, "p  position query");
        mvwaddstr(msgWindow, line++, 1, "g  GPS query");
        mvwaddstr(msgWindow, line++, 1, "h  HOPS query");
        mvwaddstr(msgWindow, line++, 1, "x  cancel");
        break;
    case EnMessageType_JBOX_SELECT:
        mvwaddstr(msgWindow, line++, 1, "Enter the node number for query");
        mvwaddstr(msgWindow, line++, 1, "(1~128)");
        break;
    case EnMessageType_JBOX_DISPLAY_NSS:
        mvwaddstr(msgWindow, line++, 1, "Signal Strength is:");
        sprintf(buf, "%d", msgData[0]);
        mvwaddstr(msgWindow, line++, 1, buf);
        break;
    case EnMessageType_JBOX_DISPLAY_HOPS:
        mvwaddstr(msgWindow, line++, 1, "Number of hops is:");
        sprintf(buf, "%d", msgData[0]);
        mvwaddstr(msgWindow, line++, 1, buf);
        break;
    case EnMessageType_JBOX_DISPLAY_POS:
        mvwaddstr(msgWindow, line++, 1, "Relative Position is:");
        //sprintf(buf, "%s", (char*)msgData);
        //mvwaddstr(msgWindow, line++, 1, buf);
        //mvwaddstr(msgWindow, line++, 1, (char*)data);
        for (int i=0; i<10; i++)
        {
            sprintf(buf, "%d\n", msgData[i]);
            mvwaddstr(msgWindow, line++, 1, buf);
        }
        break;
    case EnMessageType_JBOX_DISPLAY_GPS:
        mvwaddstr(msgWindow, line++, 1, "GPS reading is:");
        //sprintf(buf, "%s", (char*)msgData);
        //mvwaddstr(msgWindow, line++, 1, buf);
        //mvwaddstr(msgWindow, line++, 1, (char*)data);
        for (int i=0; i<10; i++)
        {
            sprintf(buf, "%f\n", dmsgData[i]);
            mvwaddstr(msgWindow, line++, 1, buf);
        }
        break;
    case EnMessageType_AMIGOBOT:
        mvwaddstr(msgWindow, line++, 1, "AmigoBot");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d  disconnect");
        mvwaddstr(msgWindow, line++, 1, "s  sync");
        if (msgData[0])
        {
            mvwaddstr(msgWindow, line++, 1, "p  range stop");
        }
        else
        {
            mvwaddstr(msgWindow, line++, 1, "p  range start");
        }
        mvwaddstr(msgWindow, line++, 1, "r  refresh");
        mvwaddstr(msgWindow, line++, 1, "x  cancel");
        break;

    case EnMessageType_ROBOTLASER:
        mvwaddstr(msgWindow, line++, 1, "RobotLaser");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d  disconnect");
        mvwaddstr(msgWindow, line++, 1, "s  sync");
        if (msgData[0])
        {
            mvwaddstr(msgWindow, line++, 1, "p  range stop");
        }
        else
        {
            mvwaddstr(msgWindow, line++, 1, "p  range start");
        }
        mvwaddstr(msgWindow, line++, 1, "r  refresh");
        mvwaddstr(msgWindow, line++, 1, "x  cancel");
        break;
	case EnMessageType_ROOMBA:
        mvwaddstr(msgWindow, line++, 1, "Roomba");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d  disconnect");
        mvwaddstr(msgWindow, line++, 1, "s  sync");
        if (msgData[0])
        {
            mvwaddstr(msgWindow, line++, 1, "p  range stop");
        }
        else
        {
            mvwaddstr(msgWindow, line++, 1, "p  range start");
        }
        mvwaddstr(msgWindow, line++, 1, "r  refresh");
        mvwaddstr(msgWindow, line++, 1, "x  cancel");
        break;
	case EnMessageType_ROOMBA560:
        mvwaddstr(msgWindow, line++, 1, "Roomba560");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d  disconnect");
        mvwaddstr(msgWindow, line++, 1, "s  sync");
        if (msgData[0])
        {
            mvwaddstr(msgWindow, line++, 1, "p  range stop");
        }
        else
        {
            mvwaddstr(msgWindow, line++, 1, "p  range start");
        }
        mvwaddstr(msgWindow, line++, 1, "r  refresh");
        mvwaddstr(msgWindow, line++, 1, "x  cancel");
        break;

    case EnMessageType_WEBCAM_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "WebCam Control");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d disconnect");
        mvwaddstr(msgWindow, line++, 1, "r refresh");
        if (msgData[0])
        {
            mvwaddstr(msgWindow, line++, 1, "p power off");
        }
        else
        {
            mvwaddstr(msgWindow, line++, 1, "p power on");
        }
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;

    case EnMessageType_COG:
        mvwaddstr(msgWindow, line++, 1, "Cognachrome Stream");
        line++;
        mvwaddstr(msgWindow, line++, 1, "0. 0 /serial/root");
        mvwaddstr(msgWindow, line++, 1, "1. 1 /serial/gdb");
        mvwaddstr(msgWindow, line++, 1, "2. 2 /serial/dummy");
        mvwaddstr(msgWindow, line++, 1, "3. 3 /serial/pioneer-27_0");
        mvwaddstr(msgWindow, line++, 1, "4. 28 /serial/vision");
        mvwaddstr(msgWindow, line++, 1, "5. 29 /serial/frame_new");
        mvwaddstr(msgWindow, line++, 1, "6. 30 /serial/frame");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_COG_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "Cognachrome Control");
        line++;
        mvwaddstr(msgWindow, line++, 1, "abc  set channel");
        mvwaddstr(msgWindow, line++, 1, "t    train on center color");
        mvwaddstr(msgWindow, line++, 1, "i    incramental train center");
        mvwaddstr(msgWindow, line++, 1, "< >  shrink/grow colors");
        mvwaddstr(msgWindow, line++, 1, "S R  save restore EEPROM");
        mvwaddstr(msgWindow, line++, 1, "h    crosshairs");
        mvwaddstr(msgWindow, line++, 1, "- +  filter level");
        mvwaddstr(msgWindow, line++, 1, "d    disconnect");
        mvwaddstr(msgWindow, line++, 1, "z    tracking");
        sprintf(buf, "[]    adjust object size [%d %d %d]",
                 msgData[0], msgData[1], msgData[2]);
        mvwaddstr(msgWindow, line++, 1, buf);
        mvwaddstr(msgWindow, line++, 1, "x    exit");
        break;
    case EnMessageType_URBAN_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "Urban Control");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d disconnect");
        break;
    case EnMessageType_ATRVJR_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "ATRV-Jr Control");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d disconnect");
        break;
    case EnMessageType_CAMERA_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "Camera Control");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d disconnect");
        mvwaddstr(msgWindow, line++, 1, "t tracking");
        mvwaddstr(msgWindow, line++, 1, "r refresh");
        mvwaddstr(msgWindow, line++, 1, "c center camera");
        if (msgData[0])
        {
            mvwaddstr(msgWindow, line++, 1, "p power off");
        }
        else
        {
            mvwaddstr(msgWindow, line++, 1, "p power on");
        }
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_CAMERA_SWEEP_X:
        mvwaddstr(msgWindow, line++, 1, "sweep X Type");
        line++;
        mvwaddstr(msgWindow, line++, 1, "h high");
        mvwaddstr(msgWindow, line++, 1, "c center");
        mvwaddstr(msgWindow, line++, 1, "l low");
        break;
    case EnMessageType_CAMERA_SWEEP_Y:
        mvwaddstr(msgWindow, line++, 1, "sweep Y Type");
        line++;
        mvwaddstr(msgWindow, line++, 1, "l left");
        mvwaddstr(msgWindow, line++, 1, "c center");
        mvwaddstr(msgWindow, line++, 1, "r right");
        break;
    case EnMessageType_VIDEO_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "Video Control");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d disconnect");
        mvwaddstr(msgWindow, line++, 1, "abc set channel");
        mvwaddstr(msgWindow, line++, 1, "!@#$ set size");
        mvwaddstr(msgWindow, line++, 1, "g capture frame");
        if (msgData[0])
        {
            mvwaddstr(msgWindow, line++, 1, "s streaming off");
        }
        else
        {
            mvwaddstr(msgWindow, line++, 1, "s streaming on");
        }
        mvwaddstr(msgWindow, line++, 1, "r refresh");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_USAGE_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "Usage Control");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d disconnect");
        mvwaddstr(msgWindow, line++, 1, "c cpu usage");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_NOMAD_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "Nomad Control");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d disconnect");
        if (msgData[0])
        {
            mvwaddstr(msgWindow, line++, 1, "s  range stop");
        }
        else
        {
            mvwaddstr(msgWindow, line++, 1, "s  range start");
        }
        mvwaddstr(msgWindow, line++, 1, "+- sonar speed");
        mvwaddstr(msgWindow, line++, 1, "z zero");
        mvwaddstr(msgWindow, line++, 1, "c calibrate compass");
        mvwaddstr(msgWindow, line++, 1, "p ping");
        mvwaddstr(msgWindow, line++, 1, "r refresh");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        sprintf(buf, "%d", msgData[1]);
        mvwaddstr(msgWindow, 5, 25, buf);
        break;
    case EnMessageType_NOMAD:
        mvwaddstr(msgWindow, line++, 1, "Enter nomad type");
        line++;
        mvwaddstr(msgWindow, line++, 1, "1 nomad 150");
        mvwaddstr(msgWindow, line++, 1, "2 nomad 200");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_NOMAD_150_PORT:
        mvwaddstr(msgWindow, line++, 1, "Enter the nomad 150 port string.");
        mvwaddstr(msgWindow, line++, 1, "(i.e. /dev/ttyS0)");
        break;
    case EnMessageType_NOMAD_200_HOST_NAME:
        mvwaddstr(msgWindow, line++, 1, "Enter the nomad 200 host name.");
        break;
    case EnMessageType_FRED_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "Fred Control");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d disconnect");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;

     case EnMessageType_CARRETILLA_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "Carretilla Control");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d disconnect");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_CARRETILLA_SIMULACION_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "Carretilla S Control");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d disconnect");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_CARRETILLA_GAZEBO_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "Carretilla G Control");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d disconnect");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_LASER_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "Laser Control");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d disconnect");
        mvwaddstr(msgWindow, line++, 1, "s steam data");
        mvwaddstr(msgWindow, line++, 1, "c start/stop continual");
        mvwaddstr(msgWindow, line++, 1, "u update laser readings");
        mvwaddstr(msgWindow, line++, 1, "r refresh");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_MAZE_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "Maze Control");
        line++;
        mvwaddstr(msgWindow, line++, 1, "n foreward 1 foot");
        mvwaddstr(msgWindow, line++, 1, "i face north");
        mvwaddstr(msgWindow, line++, 1, "j face west");
        mvwaddstr(msgWindow, line++, 1, "k face south");
        mvwaddstr(msgWindow, line++, 1, "l fase east");
        mvwaddstr(msgWindow, line++, 1, "p face north east");
        mvwaddstr(msgWindow, line++, 1, "u face north west");
        mvwaddstr(msgWindow, line++, 1, ", face south west");
        mvwaddstr(msgWindow, line++, 1, ". fase south east");
        mvwaddstr(msgWindow, line++, 1, "p pause");
        mvwaddstr(msgWindow, line++, 1, "o update obstacles");
        mvwaddstr(msgWindow, line++, 1, "m move robot");
        mvwaddstr(msgWindow, line++, 1, "u update path");
        mvwaddstr(msgWindow, line++, 1, "g run");
        mvwaddstr(msgWindow, line++, 1, "s step");
        mvwaddstr(msgWindow, line++, 1, "d disconnect");
        mvwaddstr(msgWindow, line++, 1, "r refresh");
        if (msgData[0])
        {
            mvwaddstr(msgWindow, line++, 1, "c cont send on");
        }
        else
        {
            mvwaddstr(msgWindow, line++, 1, "c cont send off");
        }
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_MAZE_SIZE_SELECT:
        mvwaddstr(msgWindow, line++, 1, "Maze Size");
        line++;
        mvwaddstr(msgWindow, line++, 1, "1 25x25");
        mvwaddstr(msgWindow, line++, 1, "2 50x50");
        mvwaddstr(msgWindow, line++, 1, "3 75x75");
        mvwaddstr(msgWindow, line++, 1, "4 100x100");
        mvwaddstr(msgWindow, line++, 1, "5 150x100");
        mvwaddstr(msgWindow, line++, 1, "6 300x300");
        mvwaddstr(msgWindow, line++, 1, "7 500x500");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_MAZE_RES_SELECT:
        mvwaddstr(msgWindow, line++, 1, "Maze Resolution");
        line++;
        mvwaddstr(msgWindow, line++, 1, "1 0.05");
        mvwaddstr(msgWindow, line++, 1, "2 0.10");
        mvwaddstr(msgWindow, line++, 1, "3 0.20");
        mvwaddstr(msgWindow, line++, 1, "4 0.40");
        mvwaddstr(msgWindow, line++, 1, "5 0.60");
        mvwaddstr(msgWindow, line++, 1, "6 0.80");
        mvwaddstr(msgWindow, line++, 1, "7 1.00");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_MAZE_TYPE_SELECT:
        mvwaddstr(msgWindow, line++, 1, "Maze Type");
        line++;
        mvwaddstr(msgWindow, line++, 1, "1 Traverse");
        mvwaddstr(msgWindow, line++, 1, "2 Explore");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_CONSOLE_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "Ipt Control");
        line++;
        mvwaddstr(msgWindow, line++, 1, "i information");
        mvwaddstr(msgWindow, line++, 1, "d disconnect");
        mvwaddstr(msgWindow, line++, 1, "c Mlab console");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        mvwaddstr(msgWindow, line++, 1, "r refresh");
        break;
    case EnMessageType_ROBOT_TYPE:
        mvwaddstr(msgWindow, line++, 1, "Robot type");
        line++;
        mvwaddstr(msgWindow, line++, 1, "1 Fred");
        mvwaddstr(msgWindow, line++, 1, "2 Pioneer AT");
        mvwaddstr(msgWindow, line++, 1, "3 Pioneer DXE");
        mvwaddstr(msgWindow, line++, 1, "4 AmigoBot");
        mvwaddstr(msgWindow, line++, 1, "5 Nomad");
        mvwaddstr(msgWindow, line++, 1, "6 Urban");
        mvwaddstr(msgWindow, line++, 1, "7 ATRV-Jr");
		mvwaddstr(msgWindow, line++, 1, "9 Roomba");
		mvwaddstr(msgWindow, line++, 1, "a Roomba560");
		mvwaddstr(msgWindow, line++, 1, "c CARMEN");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_COMPASS_TYPE:
        mvwaddstr(msgWindow, line++, 1, "Compass type");
        line++;
        mvwaddstr(msgWindow, line++, 1, "1 KVH-C100");
        mvwaddstr(msgWindow, line++, 1, "2 3DM-G");
        mvwaddstr(msgWindow, line++, 1, "3 CAN");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_GYRO_TYPE:
        mvwaddstr(msgWindow, line++, 1, "Gyro type");
        line++;
        mvwaddstr(msgWindow, line++, 1, "1 DMU-VGX");
        mvwaddstr(msgWindow, line++, 1, "2 CAN");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_GPS_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "Gps");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d disconnect");
        if (msgData[0])
        {
            mvwaddstr(msgWindow, line++, 1, "s stop saving packets");
        }
        else
        {
            mvwaddstr(msgWindow, line++, 1, "s start saving packets");
        }
        sprintf(buf, "we base x %d", msgData[1]);
        mvwaddstr(msgWindow, line++, 1, buf);
        sprintf(buf, "qa base y %d", msgData[2]);
        mvwaddstr(msgWindow, line++, 1, buf);
        mvwaddstr(msgWindow, line++, 1, "i information");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        mvwaddstr(msgWindow, line++, 1, "r refresh");
        break;
    case EnMessageType_GPS_SELECT:
        mvwaddstr(msgWindow, line++, 1, "Choose Gps");
        line++;
        mvwaddstr(msgWindow, line++, 1, "0 gps base");
        mvwaddstr(msgWindow, line++, 1, "1 gps remote");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_GYRO_DMU_VGX_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "Crossbow Gyro DMU-VGX");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d disconnect");
        mvwaddstr(msgWindow, line++, 1, "r reset angles");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_GYRO_CAN_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "Gyro CAN");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d disconnect");
        mvwaddstr(msgWindow, line++, 1, "r reset angles");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_COMPASS_KVH_C100_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "Compass KVH-C100");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d disconnect");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_COMPASS_3DMG_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "Compass 3DM-G");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d disconnect");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_COMPASS_3DMG_PORT:
        mvwaddstr(msgWindow, line++, 1, "Enter the 3DM-G port string.");
        mvwaddstr(msgWindow, line++, 1, "(i.e. /dev/ttyR0)");
        break;
    case EnMessageType_COMPASS_CAN_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "Compass CAN");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d disconnect");
        mvwaddstr(msgWindow, line++, 1, "x exit");
    	break;
    case EnMessageType_MAZEDISPLAY_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "Maze display");
        line++;
        mvwaddstr(msgWindow, line++, 1, "s save snapshot");
        mvwaddstr(msgWindow, line++, 1, "d disconnect");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        mvwaddstr(msgWindow, line++, 1, "r refresh");
        break;
    case EnMessageType_SOUND_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "Sound Control");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d disconnect");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        mvwaddstr(msgWindow, line++, 1, "r refresh");
        break;
    case EnMessageType_X_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "Xwindow Control");
        line++;
        mvwaddstr(msgWindow, line++, 1, "d disconnect");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        mvwaddstr(msgWindow, line++, 1, "r refresh");
        break;
    case EnMessageType_LASER_TYPE:
        mvwaddstr(msgWindow, line++, 1, "Laser Type");
        line++;
        mvwaddstr(msgWindow, line++, 1, "1 Direct connect");
        mvwaddstr(msgWindow, line++, 1, "2 Direct connect stream ipt");
        mvwaddstr(msgWindow, line++, 1, "3 Listen ipt");
        mvwaddstr(msgWindow, line++, 1, "4 Listen Carmen");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_LASER_USER_ACTION:
        mvwaddstr(msgWindow, line++, 1, "Choose an action");
        line++;
        mvwaddstr(msgWindow, line++, 1, "1 Add a laser");
        mvwaddstr(msgWindow, line++, 1, "2 Control a laser");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    case EnMessageType_ENTER_A_LASER_NAME:
        mvwaddstr(msgWindow, line++, 1, "Laser Name");
        line++;
        mvwaddstr(msgWindow, line++, 1, "Enter a name for the laser.");
        mvwaddstr(msgWindow, line++, 1, "Just hit <Enter> for default.");
        break;
    case EnMessageType_CHOOSE_A_LASER_NAME:
        mvwaddstr(msgWindow, line++, 1, "Choose a laser");
        if (data != NULL)
        {
            char** pszName = (char**) data;
            int i = 1;
            while (*pszName != NULL)
            {
                char szBuffer[256];
                sprintf(szBuffer, "%d %s", i++, *pszName);
                mvwaddstr(msgWindow, line++, 1, szBuffer);
                pszName++;
            }
            mvwaddstr(msgWindow, line++, 1, "x exit");
        }
        break;
    case EnMessageType_LASER_ANGLE_OFFSET:
        mvwaddstr(msgWindow, line++, 1, "Laser Angle Offset");
        line++;
        mvwaddstr(msgWindow, line++, 1, "Enter the laser angle offset in");
        mvwaddstr(msgWindow, line++, 1, "degrees. Just hit <Enter> ");
        mvwaddstr(msgWindow, line++, 1, "for default of 0.0");
        mvwaddstr(msgWindow, line++, 1, "(front is 0.0 degrees)");
        mvwaddstr(msgWindow, line++, 1, "x then <enter> to exit");
        break;
    case EnMessageType_LASER_X_OFFSET:
        mvwaddstr(msgWindow, line++, 1, "Laser X Offset");
        line++;
        mvwaddstr(msgWindow, line++, 1, "Enter the laser's X offset in cm.");
        mvwaddstr(msgWindow, line++, 1, "Just hit <Enter> for default of 0");
        mvwaddstr(msgWindow, line++, 1, "(x=0 is center, positive is front");
        mvwaddstr(msgWindow, line++, 1, "of center)");
        mvwaddstr(msgWindow, line++, 1, "x then <enter> to exit");
        break;
    case EnMessageType_LASER_Y_OFFSET:
        mvwaddstr(msgWindow, line++, 1, "Laser Y Offset");
        line++;
        mvwaddstr(msgWindow, line++, 1, "Enter the laser's Y offset in cm.");
        mvwaddstr(msgWindow, line++, 1, "Just hit <Enter> for default of 0");
        mvwaddstr(msgWindow, line++, 1, "(y=0 is center, positive is left");
        mvwaddstr(msgWindow, line++, 1, "of center)");
        mvwaddstr(msgWindow, line++, 1, "x then <enter> to exit");
        break;

    case EnMessageType_POSECALC_CONTROL:
        mvwaddstr(msgWindow, line++, 1, "PoseCalc Control");
        line++;
        mvwaddstr(msgWindow, line++, 1, "f chose a fuser");
        mvwaddstr(msgWindow, line++, 1, "v chose a variance scheme");
        mvwaddstr(msgWindow, line++, 1, "r reset pose");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;

    case EnMessageType_POSECALC_FUSER_TYPE:
        mvwaddstr(msgWindow, line++, 1, "Choose a fuser");
        line++;
        mvwaddstr(msgWindow, line++, 1, "1 Maximum Confidence");
        mvwaddstr(msgWindow, line++, 1, "2 Extended Kalman Filter");
        mvwaddstr(msgWindow, line++, 1, "3 Particle Filter");
        line++;
        mvwaddstr(msgWindow, line++, 1, "c cancel");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;

    case EnMessageType_POSECALC_VARIANCE_SCHEME:
        mvwaddstr(msgWindow, line++, 1, "Choose a variance scheme");
        line++;
        mvwaddstr(msgWindow, line++, 1, "1 Dynamic Variance");
        mvwaddstr(msgWindow, line++, 1, "2 Static Variance");
        line++;
        mvwaddstr(msgWindow, line++, 1, "c cancel");
        mvwaddstr(msgWindow, line++, 1, "x exit");
        break;
    }

    switch(err)
    {
    case EnMessageErrType_NONE:
        break;
    case EnMessageErrType_PORT:
        mvwaddstr(msgWindow, MESSAGE_WIN_ROW - 2, 1, "Unknown port");
        break;
    case EnMessageErrType_BAUD:
        mvwaddstr(msgWindow, MESSAGE_WIN_ROW - 2, 1, "Unknown baud");
        break;
    case EnMessageErrType_COG:
        mvwaddstr(msgWindow, MESSAGE_WIN_ROW - 2, 1, "Unknown stream");
        break;
    case EnMessageErrType_NOMAD_TYPE:
        mvwaddstr(msgWindow, MESSAGE_WIN_ROW - 2, 1, "Unknown Nomad type");
        break;
    case EnMessageErrType_TRACKING:
        mvwaddstr(msgWindow, MESSAGE_WIN_ROW - 2, 1, "Unknown tracking mode");
        break;
    case EnMessageErrType_MAZE_SIZE:
        mvwaddstr(msgWindow, MESSAGE_WIN_ROW - 2, 1, "Unknown maze size");
        break;
    case EnMessageErrType_MAZE_TYPE:
        mvwaddstr(msgWindow, MESSAGE_WIN_ROW - 2, 1, "Unknown maze type");
        break;
    case EnMessageErrType_MAZE_RES:
        mvwaddstr(msgWindow, MESSAGE_WIN_ROW - 2, 1, "Unknown maze resolution");
        break;
    case EnMessageErrType_ROBOT_TYPE:
        mvwaddstr(msgWindow, MESSAGE_WIN_ROW - 2, 1, "Unknown robot type");
        break;
    case EnMessageErrType_COMPASS_TYPE:
        mvwaddstr(msgWindow, MESSAGE_WIN_ROW - 2, 1, "Unknown compass type");
        break;
    case EnMessageErrType_GYRO_TYPE:
        mvwaddstr(msgWindow, MESSAGE_WIN_ROW - 2, 1, "Unknown gyro type");
        break;
    case EnMessageErrType_JBOX_NOTFOUND:
            mvwaddstr(msgWindow, MESSAGE_WIN_ROW - 2, 1, "No such a neighor");
        break;
    case EnMessageErrType_JBOX_NOTCONNECT:
            mvwaddstr(msgWindow, MESSAGE_WIN_ROW - 2, 1, "Jbox not connected");
        break;
    case EnMessageErrType_GPS:
        mvwaddstr(msgWindow, MESSAGE_WIN_ROW - 2, 1, "Unknown gps type");
        break;
    case EnMessageErrType_LASER_TYPE:
        mvwaddstr(msgWindow, MESSAGE_WIN_ROW - 2, 1, "Unknown laser type");
        break;
    case EnMessageErrType_GENERAL_INVALID:
        mvwaddstr(msgWindow, MESSAGE_WIN_ROW - 2, 1, "Invalid");
        break;
    case EnMessageErrType_INVALID_NUMBER_FORMAT:
        mvwaddstr(msgWindow, MESSAGE_WIN_ROW - 2, 1, "Invalid number format");
        break;
    case EnMessageErrType_CAMERA_SWEEP_X:
    case EnMessageErrType_CAMERA_SWEEP_Y:
        mvwaddstr(msgWindow, MESSAGE_WIN_ROW - 2, 1, "Unknown sweep type");
        break;
    case EnMessageErrType_POSECALC_FUSER_TYPE:
        mvwaddstr(msgWindow, MESSAGE_WIN_ROW - 2, 1, "Invalid fuser type.");
        break;
    case EnMessageErrType_POSECALC_VARIANCE_SCHEME:
        mvwaddstr(msgWindow, MESSAGE_WIN_ROW - 2, 1, "Invalid variance scheme.");
        break;
    }

    show_panel(msgPanel);
    pthread_cleanup_pop(1);
    redrawWindows();
}

/* Filter keypresses */
int messageFilter(int c, EnMessageFilterType filter, void* data = NULL)
{
    if (c == 'x')
    {
        return 0;
    }
    switch(filter)
    {
    case EnMessageFilterType_DIGIT:
        if (isdigit(c)) return 0;
        break;
    case EnMessageFilterType_BAUD:
        if ((c >= '1') && (c <= '6')) return 0;
        break;
    case EnMessageFilterType_COG:
        if ((c >= '0') && (c <= '6')) return 0;
        break;
    case EnMessageFilterType_TRACKING:
        if (((c>='0') && (c <= '9'))||(c=='a')) return 0;
        break;
    case EnMessageFilterType_NOMAD:
        if ((c >= '1') && (c <= '2')) return 0;
        break;
    case EnMessageFilterType_SOUND:
        if ((c >= '0') && (c <= '3')) return 0;
        break;
    case EnMessageFilterType_MAZE_SIZE:
        if ((c >= '1') && (c <= '7')) return 0;
        break;
    case EnMessageFilterType_MAZE_TYPE:
        if ((c >= '1') && (c <= '2')) return 0;
        break;
    case EnMessageFilterType_MAZE_RES:
        if ((c >= '1') && (c <= '7')) return 0;
        break;
    case EnMessageFilterType_ROBOT_TYPE:
        //if ((c >= '1') && (c <= '11'))
        return 0;
        break;
    case EnMessageFilterType_COMPASS_TYPE:
        if ((c >= '1') && (c <= '3')) return 0;
        break;
    case EnMessageFilterType_GYRO_TYPE:
        if ((c >= '1') && (c <= '2')) return 0;
        break;
    case EnMessageFilterType_GPS:
        if ((c >= '0') && (c <= '1')) return 0;
        break;
    case EnMessageFilterType_JBOX:
        break;
    case EnMessageFilterType_LASER_TYPE:
        if ((c >= '1') && (c <= '4')) return 0;
        break;
    case EnMessageFilterType_LASER_USER_ACTION:
        if ((c >= '1') && (c <= '2'))
        {
            return 0;
        }
        break;
    case EnMessageFilterType_LASER_NAME:
        if ((data != NULL) &&
             (c >= '1') &&
             ((c - '0') <= *((int*) data)))
        {
            return 0;
        }
        break;
    case EnMessageFilterType_CAMERA_SWEEP_X:
        if ((c == 'h') || (c == 'c') || (c== 'l')) return 0;
        break;
    case EnMessageFilterType_CAMERA_SWEEP_Y:
        if ((c == 'l') || (c == 'c') || (c == 'r')) return 0;
        break;
    default:
        break;
    }
    return -1;
}

/* Show window and return keypress */
char messageGetChar(
    EnMessageFilterType filter,
    EnMessageType msg,
    EnMessageErrType err,
    void* data,
    bool hide,
    void* pvFilterData)
{
    char c, done;

    if (SilentMode) {
	return 0;
    }

    done = false;
    show_panel(msgPanel);
    messageDrawWindow(msg, EnMessageErrType_NONE, data);
    redrawWindows();

    do
    {
        c = getch();
        if (messageFilter(c, filter, pvFilterData))
        {
            messageDrawWindow(msg,err);
            redrawWindows();
        }
        else
        {
            done = true;
        }
    } while (!done);

    if (hide)
    {
        hide_panel(msgPanel);
    }
    redrawWindows();
    return c;
}

/* Show window and return a string */
string messageGetString(EnMessageType enMsg, unsigned int uEchoLine,
                         void* pvData/*=NULL*/, bool bHide/*=true*/)
{
    string strReturn = "";

   if (SilentMode) {
      return NULL;
   }

   // display the prompt message
    show_panel(msgPanel);
    messageDrawWindow(enMsg, EnMessageErrType_NONE, pvData);
    redrawWindows();

    // make sure we don't try do echo the line outside the window
    if (uEchoLine >= MESSAGE_WIN_ROW)
    {
        uEchoLine = MESSAGE_WIN_ROW - 1;
    }

    nocbreak(); // let Enter be returned as '\n'
    echo();     // print chars as they are entered

    // read the string into the buffer until a newline is entered or until we
    // run out of room in the buffer
    int iPosition = 1;
    char c;
    while ((c = mvwgetch(msgWindow, uEchoLine, iPosition)) != '\n')
    {
        strReturn += c;
        iPosition++;
    }

    noecho(); // turn echoing back off
    cbreak(); // reset curses so we don't wait on Enter

    if (bHide)
    {
        hide_panel(msgPanel);
    }
    redrawWindows();

    return strReturn;
}

void messageShow()
{
    if (SilentMode) {
       return;
    }
    pthread_cleanup_push((void(*)(void*)) pthread_mutex_unlock, (void *) &screenMutex);
    pthread_mutex_lock(&screenMutex);
    show_panel(msgPanel);
    pthread_cleanup_pop(1);
    redrawWindows();
}


void messageHide()
{
    if (SilentMode) {
       return;
    }
    pthread_cleanup_push((void(*)(void*)) pthread_mutex_unlock, (void *) &screenMutex);
    pthread_mutex_lock(&screenMutex);
    hide_panel(msgPanel);
    pthread_cleanup_pop(1);
    redrawWindows();
}

/**********************************************************************
# $Log: message.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.6  2004/09/10 19:41:07  endo
# New PoseCalc integrated.
#
# Revision 1.5  2004/04/24 11:20:53  endo
# 3DM-G added.
#
# Revision 1.4  2004/04/17 12:20:35  endo
# Intalling JBox Sensor Update.
#
# Revision 1.3  2004/04/13 19:25:33  yangchen
# JBox module integrated.
#
# Revision 1.2  2004/04/06 02:07:40  pulam
# Added code for Silent Mode
#
# Revision 1.1.1.1  2004/02/27 22:30:50  endo
# Initial MARS 2020 revision
#
# Revision 1.10  2003/04/26 06:48:31  ebeowulf
# Added the Pioneer2-dxe controller.
#
# Revision 1.9  2003/04/06 13:32:37  endo
# gcc 3.1.1
#
# Revision 1.8  2002/02/18 13:48:21  endo
# AmigoBot added.
#
# Revision 1.7  2001/05/29 22:37:30  blee
# Made changes to incorporate gps data w/ other position info.
#
# Revision 1.6  2001/03/23 21:25:44  blee
# altered to use a config file and allow for a laser's physical
# offset on a robot
#
# Revision 1.5  2000/12/12 23:01:59  blee
# Changed #defines for the user interface to enumerated types.
# added a case in messageDrawWindow for EnMessageType_NONE.
# Added and changed cases in messageDrawWindow pertaining to lasers.
# Added error cases in messageDrawWindow for EnMessageErrType_GENERAL_INVALID
# and EnMessageErrType_INVALID_NUMBER_FORMAT.
# Changed prototype for messageFilter()
# Altered call to messageFilter() in messageGetChar()
# Added messageGetString()
#
# Revision 1.4  2000/11/13 20:09:23  endo
# ARTV-Jr class added to HServer.
#
# Revision 1.3  2000/09/19 03:45:31  endo
# RCS log added.
#
#
#**********************************************************************/
