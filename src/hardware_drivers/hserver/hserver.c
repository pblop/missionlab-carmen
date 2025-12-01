/**********************************************************************
 **                                                                  **
 **                             hserver.c                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  Hardware Server                                                 **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: hserver.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

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
#include <getopt.h>
#include <curses.h>
#include <panel.h>
#include <ctype.h>
#include <semaphore.h>
#include <vector>
#include <string>
#include <sys/resource.h>

#include "hserver.h"
#include "camera.h"
#include "laser.h"
#include "terminal.h"
#include "message.h"
#include "cognachrome.h"
#include "ipt_handler.h"
#include "RemoteControl.h"
#include "statusbar.h"
#include "video.h"
#include "apm_module.h"
#include "robot_config.h"
#include "sensors.h"
#include "fred.h"
#include "pioneer.h"
#include "pioneer2.h"
#include "amigobot.h"
#include "roomba.h"
#include "roomba560.h"
#include "carmenRobot.h"
#include "webCam.h"
#include "report_level.h"
#include "robot.h"
#include "fred.h"
#include "gps.h"
#include "gps_default.h"
#include "gps_CARMEN.h"
#include "nomad.h"
#include "console.h"
#include "HSRCFile.h"
#include "HServerConfigurator.h"
#include "ControlInterfaces.h"
#include "PoseCalculatorInterface.h"
#include "PoseCalculator/PoseCalculator.h"
#include "jbox.h"
#include "compass.h"
#include "compass_3DM-G.h"
#include "gyro.h"
#include "Watchdog.h"
#include "LogManager.h"
#include "ipc_client.h"

#ifndef NO_MOBILITY
#include "urban.h"
#include "atrvjr.h"
#include "gyro_dmu-vgx.h"
#include "compass_kvh-c100.h"
#endif

// X11's Xlib.h (and may be others), which is called in xwindow.h
// defines "Status" as a macro. This name conflicts with the
// "Status" type defined in the CORBA namespace. To avoid the
// error, xwindow.h needs to be called after mobility/CORBA-related
// headers are called.
#include "xwindow.h"

const string DEFAULT_POSECALC_RC_SECTION_NAME = "posecalc1";

struct SuOption
{
    ClHServerConfig::EnHServerConfigSectionType enSectionType;
    string strSectionName;

    SuOption() {}
    SuOption(const SuOption& suOriginal) { *this = suOriginal; }
    SuOption& operator=(const SuOption& suOriginal);
};

SuOption& SuOption::operator=(const SuOption& suOriginal)
{
    if (this != &suOriginal)
    {
        enSectionType = suOriginal.enSectionType;
        strSectionName = suOriginal.strSectionName;
    }

    return *this;
}

bool gWatchdogEnabled = true;

bool useApm;
bool useUrban;
bool useATRVJr;
bool useIpt;
bool useHclient;
bool useX, useFred;
bool multipleHservers;
bool noSplash = true;
bool useJbox = false;
bool useGps = false;
bool logData = false;
bool jboxDisableNetworkQuery = false;
int jboxID = 22;

// lists to hold initialization info
vector<SuLaserInitData> g_clLaserInitList;
vector<SuCameraInitData> g_clCameraInitList;
vector<SuWebCamInitData> g_clWebCamInitList;
vector<SuCognachromeInitData> g_clCognachromeInitList;
vector<SuNomadInitData> g_clNomadInitList;
vector<SuPioneerInitData> g_clPioneerInitList;
vector<SuPioneer2InitData> g_clPioneer2InitList;
vector<SuAmigoBotInitData> g_clAmigoBotInitList;
vector<SuRoombaInitData> g_clRoombaInitList;
vector<SuRoomba560InitData> g_clRoomba560InitList;
vector<SuCarmenInitData> g_clCarmenInitList;
vector<SuFrameGrabberInitData> g_clFrameGrabberInitList;
vector<SuGpsInitData> g_clGpsInitList;
vector<SuJboxInitData> g_clJboxInitList;
vector<SuCompassInitData> g_clCompassInitList;
vector<SuGyroInitData> g_clGyroInitList;
vector<SuPoseCalcInitData> g_clPoseCalcInitList;

double gps_base_lat, gps_base_lon, gps_base_x, gps_base_y,
       gps_got_data, gps_mperlon, gps_mperlat;

char* ipt_server_name = NULL;

WINDOW *mainWindow = NULL;
WINDOW *backWindow,*textWindow,*helpWindow;

PANEL *backPanel,*textPanel,*helpPanel;

int statusbarDevideLine;

int mainPanelX,mainPanelY;
int screenX,screenY;

pthread_mutex_t screenMutex;	// mutex for all curses functions

char robot_name[100] = "fred";

pthread_t signal_handler;
sem_t exit_sem;

bool text_paused = false;
bool SilentMode = false;

PoseCalcInterface *gPoseCalc = NULL;
Robot *gRobot = NULL;
Gps *gps = NULL;
Jbox *jbox = NULL;
Compass *gCompass = NULL;
Gyro *gGyro = NULL;
RemoteControl *gRemoteControl = NULL;
LogManager *gLogManager = NULL;
Apm *apm = NULL;
WebCam *webCam = NULL;

// This function returns the current time (since Epoch) in second.
double getCurrentEpochTime(void)
{
    timeval tvp;
    double currentEpochTime;

    gettimeofday(&tvp, NULL);
    currentEpochTime = (double)tvp.tv_sec + (((double)tvp.tv_usec)/1000000.0);

    return currentEpochTime;
}


HSDateString_t getCurrentDateString(void)
{
    HSDateString_t dateString;
    time_t cur_time;
    tm local_time;
    string prefix;
    char date[100], month[100], day[100], year[100], hour[100], min[100], sec[100];

    // Get the current date and time.
    cur_time = time(NULL);
    strncpy(date,ctime(&cur_time),sizeof(date));
    local_time = *localtime(&cur_time);
    strftime(month, sizeof(month), "%m", &local_time);
    strftime(day, sizeof(day), "%d", &local_time);
    strftime(year, sizeof(year), "%Y", &local_time);
    strftime(hour, sizeof(hour), "%H", &local_time);
    strftime(min, sizeof(min), "%M", &local_time);
    strftime(sec, sizeof(sec), "%S", &local_time);

    dateString.year = year;
    dateString.month = month;
    dateString.day = day;
    dateString.hour = hour;
    dateString.min = min;
    dateString.sec = sec;

    return dateString;
}

// if the last character in the string is a '/', get rid of it
string TruncateSlash(string str)
{
    if ((str != "") && (str[str.length() - 1] == '/'))
    {
        str.erase(str.length() - 1, str.length() - 1);
    }

    return str;
}

// extract the list of colon seperated directories from an environment variable
vector<string> ExtractDirectories(string strName)
{
    vector<string> clDirList;

    // Get a copy of the environment variable
    const char* szPathList = getenv(strName.c_str());
    string strPathList = (szPathList != NULL) ? szPathList : "";

    unsigned int i;
    string strDirectory = "";
    char c;

    // iterate through the characters in the path list
    for (i = 0; i < strPathList.length(); i++)
    {
        c = strPathList[i];

        // add non-colon characters to the current path directory
        if (c != ':')
        {
            strDirectory += c;
        }
        // if we've hit a colon, we've completed a directory
        else
        {
            if ((strDirectory = TruncateSlash(strDirectory)) != "")
            {
                // add the directory to the list and reset our local variable
                clDirList.push_back(strDirectory);
                strDirectory = "";
            }
        }
    }

    // if the last character wasn't a colon, then we never added the last
    // directory in the path list to our list. do this now.
    if (c != ':')
    {
        if ((strDirectory = TruncateSlash(strDirectory)) != "")
        {
            clDirList.push_back(strDirectory);
        }
    }

    return clDirList;
}

void redrawWindows(void)
{
    pthread_cleanup_push((void(*)(void*)) pthread_mutex_unlock, (void *) &screenMutex);
    pthread_mutex_lock(&screenMutex);
    update_panels();
    doupdate();
    pthread_cleanup_pop(1);
}

void refreshScreen(void)
{
    pthread_cleanup_push((void(*)(void*)) pthread_mutex_unlock, (void *) &screenMutex);
    pthread_mutex_lock(&screenMutex);
    clear();
    pthread_cleanup_pop(1);
    redrawWindows();
}

/* Print a formatted text string to the text window */
void printfTextWindow(const char* szFormat, ...)
{
    va_list args;
    char szMessage[512];

    va_start(args, szFormat);
    vsprintf(szMessage, szFormat, args);
    va_end(args);

    printTextWindow(szMessage);
}

/* Print string to text window. Filter will remove all non-alphanumerics.
   nl will scroll window. clear will clear window and return. */
void printTextWindow(char *str, int nl, int filter, int clear)
{
    static int y = 0;
    static int x = 0;
    static int firsttime = 0;
    static int ff = false;
    int i, len;
    char buf[HS_BUF_SIZE];

    if (text_paused || SilentMode)
    {
        return;
    }

    if (clear)
    {
        y = 0;
        x = 0;
        firsttime = 0;
        ff = false;
        werase(textWindow);
        redrawWindows();
        return;
    }

    if (filter)
    {
        len = strlen(str);
        for (i = 0; i < len; i++)
        {
            if (isalnum(str[i]))
            {
                buf[i] = str[i];
            }
            else
            {
                buf[i] = '^';
            }
        }
        buf[len] = 0;
    }

    pthread_cleanup_push((void(*)(void*)) pthread_mutex_unlock, (void *) &screenMutex);
    pthread_mutex_lock(&screenMutex);
    if (ff)
    {
        scroll(textWindow);
        ff = false;
    }

    if ((y == (screenY - 14)) && firsttime && (x == 0))
    {
        scroll(textWindow);
        firsttime = true;
    }
    wmove(textWindow, y, x);
    if (!filter)
    {
        waddnstr(textWindow, str, screenX - x - 1);
    }
    else
    {
        waddnstr(textWindow, buf, screenX - x - 1);
    }
    if (nl)
    {
        if (y < (screenY - 13))
        {
            y++;
        }
        else
        {
            ff = true;
        }
        x = 0;
    }
    else
    {
        getyx(textWindow, y, x);
    }
    pthread_cleanup_pop(1);
    redrawWindows();
}

/* Initialize serial port */
int setupSerial(
    int* fd,
    const char* port,
    long baud,
    int report_level,
    char* caller,
    int timeout,
    int nonblock)
{
    struct termios oldtp;
    struct termios tp;

    if ((*fd = open(port, O_RDWR | O_NOCTTY  | O_NONBLOCK)) < 0)
    {
        if (report_level >= HS_REP_LEV_ERR)
        {
            printfTextWindow("%s Error opening serial port %s", caller, port);
        }
        return -1;
    }
    if (tcgetattr(*fd, &oldtp) < 0)
    {
        close(*fd);
        if (report_level >= HS_REP_LEV_ERR)
        {
            printfTextWindow("%s Error Couldn't get term attributes.", caller);
        }
        return -1;
    }
    bzero(&tp, sizeof(tp));
    tp.c_cflag = CS8 | CLOCAL | CREAD | baud;
    tp.c_oflag = 0;
    tp.c_iflag = IGNBRK | IGNPAR;
    tp.c_lflag = 0;
    tp.c_cc[VTIME] = timeout;
    tp.c_cc[VMIN] = 0;
    if (nonblock)
    {
        fcntl(*fd, F_SETFL ,O_NONBLOCK);
    }
    tcflush(*fd, TCIFLUSH);
    if (tcsetattr(*fd, TCSANOW, &tp) < 0)
    {
        if (report_level >= HS_REP_LEV_ERR)
        {
            printfTextWindow("%s Error Couldn't set term attributes.", caller);
        }
        return -1;
    }

    close(*fd);
    if ((*fd = open(port, O_RDWR | O_NOCTTY)) < 0)
    {
        if (report_level >= HS_REP_LEV_ERR)
        {
            printfTextWindow("%s Error opening serial port %s", caller, port);
        }
        return -1;
    }

    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printfTextWindow("%s serial opened on %s", caller, port);
    }
    return 0;
}

/* Shutdown curses */
void closeCurses()
{
  curs_set(1);
  endwin();
}

void drawHelpWindow(void)
{
    int line = 1;
    pthread_cleanup_push((void(*)(void*)) pthread_mutex_unlock, (void *) &screenMutex);
    pthread_mutex_lock(&screenMutex);
    wmove(helpWindow, 0, 0);
    wborder(helpWindow, 0, 0, 0, 0, 0, 0, 0, 0);
    mvwaddstr(helpWindow, line++, 1, "HServer Hardware Control");
    mvwaddstr(helpWindow, line++, 1, "h help");
    mvwaddstr(helpWindow, line++, 1, "r refresh screen");
    mvwaddstr(helpWindow, line++, 1, "c clear text window");
    mvwaddstr(helpWindow, line++, 1, "p stop text window");
    mvwaddstr(helpWindow, line++, 1, "Q quit");
    mvwaddstr(helpWindow, line++, 1, "R robot");
    mvwaddstr(helpWindow, line++, 1, "C camera");
    mvwaddstr(helpWindow, line++, 1, "E sensors");
    mvwaddstr(helpWindow, line++, 1, "B battery usage");
    mvwaddstr(helpWindow, line++, 1, "L laser");
    mvwaddstr(helpWindow, line++, 1, "M cognachrome");
    mvwaddstr(helpWindow, line++, 1, "F remote control");
    mvwaddstr(helpWindow, line++, 1, "T terminal");
    mvwaddstr(helpWindow, line++, 1, "V video");
    mvwaddstr(helpWindow, line++, 1, "X xwindow");
    mvwaddstr(helpWindow, line++, 1, "I ipt");
    mvwaddstr(helpWindow, line++, 1, "D report level");
    mvwaddstr(helpWindow, line++, 1, "W robot config");
    mvwaddstr(helpWindow, line++, 1, "G gps");
    mvwaddstr(helpWindow, line++, 1, "Y gyroscope");
    mvwaddstr(helpWindow, line++, 1, "K compass");
    mvwaddstr(helpWindow, line++, 1, "J Jbox");
    mvwaddstr(helpWindow, line++, 1, "w webCam");
    pthread_cleanup_pop(1);
}


/* Draw borders and lines */
void drawBackWindow(void)
{
    char buf[100];
    pthread_cleanup_push((void(*)(void*)) pthread_mutex_unlock, (void *) &screenMutex);
    pthread_mutex_lock(&screenMutex);
    wclear(backWindow);
    wmove(backWindow, 0, 0);
    waddch(backWindow, ACS_ULCORNER);
    whline(backWindow, ACS_HLINE, screenX - 2);
    wmove(backWindow, 0, screenX - 1);
    waddch(backWindow, ACS_URCORNER);
    wmove(backWindow, 1, 0);
    wvline(backWindow, ACS_VLINE, screenY - 2);
    wmove(backWindow, 1, screenX - 1);
    wvline(backWindow, ACS_VLINE, screenY - 2);
    wmove(backWindow, screenY - 1, 0);
    waddch(backWindow, ACS_LLCORNER);
    whline(backWindow, ACS_HLINE, screenX - 2);
    wmove(backWindow, screenY - 1, screenX - 1);
    waddch(backWindow, ACS_LRCORNER);
    wmove(backWindow, statusbarDevideLine, 0);
    waddch(backWindow, ACS_LTEE);
    whline(backWindow, ACS_HLINE, screenX - 2);
    wmove(backWindow, statusbarDevideLine, screenX - 1);
    waddch(backWindow, ACS_RTEE);
    wmove(backWindow, 1, 1);
    wattron(backWindow, A_UNDERLINE);
    sprintf(buf, "HServer %s", robot_name);
    waddstr(backWindow, buf);
    wattroff(backWindow, A_UNDERLINE);
    pthread_mutex_unlock(&screenMutex);
    pthread_cleanup_pop(1);
}

void initializeCurses(void)
{
    statusbarDevideLine = 4;
    if (mainWindow == NULL)
    {
        mainWindow = (WINDOW*)malloc (sizeof (WINDOW));
    }
    mainWindow = initscr();	// initialize the curses library
    nonl();			        // tell curses not to do NL->CR/NL on output
    cbreak();			    // take input chars one at a time, no wait for \n
    noecho();			    // don't echo input
    keypad(stdscr, TRUE);



    if (has_colors())
    {
        start_color();
        init_pair(COLOR_BLACK, COLOR_BLACK, COLOR_BLACK);
        init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
        init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
        init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
        init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
        init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
        init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
    }

    getmaxyx(mainWindow, screenY, screenX);
    backWindow = newwin(screenY, screenX, 0, 0);
    backPanel = new_panel(backWindow);
    drawBackWindow();
    show_panel(backPanel);
    mainPanelX = screenX;
    mainPanelY = screenY;

    textWindow = newwin(screenY - statusbarDevideLine - 2, screenX - 2,
                        statusbarDevideLine + 1, 1);
    textPanel = new_panel(textWindow);
    scrollok(textWindow, TRUE);
    show_panel(textPanel);

    helpWindow = newwin(HS_HELP_ROW, HS_HELP_COL, HS_HELP_Y, HS_HELP_X);
    helpPanel = new_panel(helpWindow);
    drawHelpWindow();
    hide_panel(helpPanel);

    statusbar = new Statusbar(statusbarDevideLine - 3);

    terminalInitialize();
    messageInitialize();

    displayTerm = false;
    curs_set(0);
}

void exit_hserver(void)
{
    printTextWindow("Exiting");

    // delete all lasers
    while (Laser::m_clLaserList.size() > 0)
    {
        if (Laser::m_clLaserList[0] != NULL)
        {
            // just delete the laser -- the laser being deleted
            // will remove itself from the list
            delete Laser::m_clLaserList[0];
        }
        else
        {
            Laser::m_clLaserList.erase(Laser::m_clLaserList.begin());
        }
    }

    if (gLogManager != NULL)
    {
        delete gLogManager;
        gLogManager = NULL;
    }

    if (gRemoteControl != NULL)
    {
        delete gRemoteControl;
        gRemoteControl = NULL;
    }

    if (gGyro != NULL)
    {
        delete gGyro;
    }

    if (camera != NULL)
    {
        delete camera;
    }
    if (cognachrome != NULL)
    {
        delete cognachrome;
    }
    if (gps != NULL)
    {
        delete gps;
    }

    if (iptHandler != NULL)
    {
        delete iptHandler;
    }

    if (jbox != NULL)
    {
        delete jbox;
    }

    if (video != NULL)
    {
        delete video;
    }
    if (webCam != NULL)
    {
        delete webCam;
    }
    if (apm != NULL)
    {
        delete apm;
    }

    if (gRobot != NULL)
    {
        delete gRobot;
    }

    if (gCompass != NULL)
    {
        delete gCompass;
    }

    if (xwindow != NULL)
    {
        delete xwindow;
    }

    if (gPoseCalc != NULL)
    {
        delete gPoseCalc;
        gPoseCalc = NULL;
    }

    closeCurses();
    sem_post(&exit_sem);
    printf("\n");
}

void perform_laser_action()
{
    char cUserChoice;

    if (Laser::m_clLaserList.size() == 0)
    {
        Laser::ConnectLaser();
    }
    else
    {
        cUserChoice = messageGetChar(EnMessageFilterType_LASER_USER_ACTION,
                                      EnMessageType_LASER_USER_ACTION,
                                      EnMessageErrType_GENERAL_INVALID);
        if (cUserChoice != 'x')
        {
            if (cUserChoice == '1')
            {
                Laser::ConnectLaser();
            }
            else
            {
                Laser* pLaser = Laser::GetLaserByName();
                if (pLaser != NULL)
                {
                    pLaser->control();
                }
            }
        }
    }
}

// **** KEYBOARD READER
void keyboard_reader_thread(void)
{
    int c = 0;
    bool displayHelp = false;
    while(true)
    {
        pthread_testcancel();
        c = getch();
        switch (c) {

        case 'S':
            SilentMode = !SilentMode;
            break;

        case 'F':
            if (gRemoteControl != NULL)
            {
                gRemoteControl->runRemoteControl();
            }
            break;

        case 'Q':
            exit_hserver();
            break;

        case 'h':
            displayHelp = !displayHelp;
            if (displayHelp)
            {
                show_panel(helpPanel);
            }
            else
            {
                hide_panel(helpPanel);
            }
            redrawWindows();
            break;

        case 'C':
            if (camera != NULL)
            {
                camera->control();
            }
            else
            {
                cameraConnect();
            }
            break;

        case 'L':
            perform_laser_action();
            break;

        case 'J':
            if (jbox != NULL)
            {
                controlJbox(jbox);
            }
            else
            {
                Jbox::connectToJbox(jbox);
            }
            break;

        case 'r':
            refreshScreen();
            break;

        case 'c':
            printTextWindow(NULL, false, false, true);
            break;

        case 'T':
            if (terminalConnected)
            {
                terminalShow();
            }
            else
            {
                terminalReconnect();
            }
            break;

        case 'M':
            if (cognachrome != NULL)
            {
                cognachrome->control();
            }
            else
            {
                cognachromeConnect();
            }
            break;

        case 'V':
            if (video != NULL)
            {
                video->control();
            }
            else
            {
                framegrabberConnect();
            }
            break;

        case 'X':
            if (xwindow != NULL)
            {
                xwindow->control();
            }
            else
            {
                new Xwindow(&xwindow);
            }
            break;

        case 'B':
            if (apm != NULL)
            {
                apm->control();
            }
            else
            {
                new Apm(&apm);
            }
            break;

        case 'W':
            robot_config->show();
            break;

        case 'E':
            sensors->show();
            break;

        case 'R':
            if (gRobot != NULL)
            {
                gRobot->control();
            }
            else
            {
                robotConnect(gRobot);
            }
            break;

        case 'I':
            if (iptHandler != NULL)
            {
                iptHandler->control();
            }
            else
            {

                new IptHandler(
                    &iptHandler,
                    ipt_server_name,
                    robot_name,
                    &gps,
                    multipleHservers);

                sleep(1);
                if (useHclient)
                {
                    iptHandler->listen_hclient();
                }
            }
            break;

        case 'D':
            reportlevel->show();
            break;

        case 'G':
            if (gps != NULL)
            {
                controlGps(gps);
            }
            else
            {
                gpsConnect(
                    gps,
                    gps_base_lat,
                    gps_base_lon,
                    gps_base_x,
                    gps_base_y,
                    gps_mperlat,
                    gps_mperlon);
            }
            break;

        case 'Y':
            if (gGyro != NULL)
            {
                gGyro->control();
            }
            else
            {
                gyroConnect(gGyro);
            }
            break;

        case 'K':
            if (gCompass != NULL)
            {
                gCompass->control();
            }
            else
            {
                compassConnect(gCompass);
            }
            break;

        case 'P':
            if (gPoseCalc != NULL)
            {
                gPoseCalc->control();
            }
            else
            {
                printfTextWindow("Error: PoseCalc not running.\n");
            }
            break;

        case 'p':
            text_paused = !text_paused;
            break;

        case 'w':
            if (webCam != NULL)
            {
                webCam->control();
            }
            else
            {
                webCamConnect(webCam);
            }
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

        case KEY_ESC:
        case 'x':
            messageHide();
            break;
        }
    }
}


void resizeWindows(bool erase)
{
    struct winsize win;

    if ((ioctl(1, TIOCGWINSZ, &win) >= 0) ||       // 1 is stdout
         (ioctl(0, TIOCGWINSZ, &win) >= 0))        // 0 is stdin
    {
        screenX = win.ws_col;
        screenY = win.ws_row;
        mainPanelX = screenX;
        mainPanelY = screenY;
        pthread_cleanup_push((void(*)(void*)) pthread_mutex_unlock, (void *) &screenMutex);
        pthread_mutex_lock(&screenMutex);
        resizeterm(screenY, screenX);
        mvwin(textWindow, statusbarDevideLine + 1, 1);
        wresize(textWindow, screenY - statusbarDevideLine - 2, screenX - 2);
        statusbar->resize();
        messageMove();
        pthread_cleanup_pop(1);
        drawBackWindow();
        if (erase)
        {
            printTextWindow(NULL, true, false, true);
        }
    }
    if (erase)
    {
        printfTextWindow("Window resized %dx%d", win.ws_row, win.ws_col);
    }
}

char* signalName(int signal)
{
    static char name[10];

    switch (signal) {
    case 1: return "SIGHUP";
    case 2: return "SIGINT";
    case 3: return "SIGQUIT";
    case 4: return "SIGILL";
    case 6: return "SIGABRT";
    case 8: return "SIGFPE";
    case 9: return "SIGKILL";
    case 11: return "SIGSEGV";
    case 13: return "SIGPIPE";
    case 14: return "SIGALRM";
    case 15: return "SIGTERM";
    case 10: return "SIGUSR1";
    case 12: return "SIGUSR2";
    case 17: return "SIGCHLD";
    case 18: return "SIGCONT";
    case 19: return "SIGSTOP";
    case 20: return "SIGTSTP";
    case 21: return "SIGTTIN";
    case 22: return "SIGTTOU";
    case 23: return "SIGURG";
    case 24: return "SIGXCPU";
    case 25: return "SIGXFSZ";
    case 26: return "SIGVTALRM";
    case 27: return "SIGPROF";
    case 28: return "SIGWINCH";
    case 29: return "SIGPOLL";
    case 30: return "SIGPWR";
    case 31: return "SIGSYS";
    default:
        sprintf(name, "signal %d", signal);
        return name;
    }
}

/* Thread to wait for signals. All other threads ignore all signals */
void signal_handler_thread(void)
{
    sigset_t sigset;
    int signal;

    sigfillset(&sigset);
    while(true)
    {
        sigwait(&sigset, &signal);
        switch(signal)
        {
        case SIGINT:
            exit_hserver();
            break;
        case SIGWINCH:
            resizeWindows();
            break;
        default:
            printfTextWindow("Signal handler recieved %s", signalName(signal));
        }
    }
}

int startHServer(void)
{
    PoseCalcInterface *poseCalc = NULL;
    PoseCalcConstants_t poseCalcConstants;
    pthread_t keyboard_reader;
    sigset_t sigset;
    int poseCalcID;
    unsigned int i;

    pthread_mutex_init(&screenMutex, NULL);

    sem_init(&exit_sem, 0, 0);

    initializeCurses();
    if (!noSplash)
    {
        messageShow();
    }
    sensors = new Sensors(10);
    robot_config = new RobotConfiguration(10, 1, robot_name);

    reportlevel = new ReportLevel;

    printfTextWindow("HServer Version %s", HSERVER_VERSION);
    printTextWindow("Generic Robot Contoller");
    printTextWindow("");
    printTextWindow("Press h for help");

    pthread_create(&signal_handler, NULL, (void * (*)(void *)) &signal_handler_thread, NULL);
    sigfillset(&sigset);
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    printTextWindow("I know how to connect to:");
    printTextWindow("   Pioneer AT robot via serial");
    printTextWindow("   Pioneer2 DXE robot via serial");
    printTextWindow("   AmigoBot robot via serial");
	printTextWindow("   Roomba robot via serial or BlueTooth");
	printTextWindow("   Roomba560 robot via serial or BlueTooth");
    printTextWindow("   GROUSAL forklift via CAN");
    printTextWindow("   GROUSAL forklift via Gazebo");
    printTextWindow("   Sony evi-d30 camera via serial");
    printTextWindow("   SICK LMS200 laser scanner via serial");
    printTextWindow("   Missionlab console via tcp [ipt]");
    printTextWindow("   RWI Urban robot via tcp [mobility]");
    printTextWindow("   RWI ATRV-Jr robot via tcp [mobility]");
    printTextWindow("   Nomadic technologies Nomad 150/200 robot via serial/tcp");
    printTextWindow("   Congnachrome vision system via serial");
    printTextWindow("   Bt848 Video Framegrabber via /dev/video");
    printTextWindow("   X11 server via local or tcp");
    printTextWindow("   WebCam via USB");
    printTextWindow("");
    printfTextWindow("HServer name is %s", robot_name);

    if (logData)
    {
        gLogManager = new LogManager();
    }

    if (useIpt)
    {

        new IptHandler(
            &iptHandler,
            ipt_server_name,
            robot_name,
            &gps,
            multipleHservers);

        sleep(2);

        if (useHclient)
        {
            iptHandler->listen_hclient();
            sleep(2);
        }
    }

    if (g_clCognachromeInitList.size() > 0)
    {
        new Cognachrome(&cognachrome, g_clCognachromeInitList[0].strPortString);
        sleep(4);
    }

    if (g_clCameraInitList.size() > 0)
    {
        new Camera(&camera, g_clCameraInitList[0].strPortString);
        sleep(2);
    }

    if (g_clWebCamInitList.size() > 0)
    {
        new WebCam(&webCam, g_clWebCamInitList[0].strPortString);
        sleep(2);
    }

    if (useATRVJr)
    {
#ifndef NO_MOBILITY

        new ATRVJr(&gRobot);

        sleep(2);
#else
        printTextWindow("No Mobility support");
#endif
    }

    bool bUseStream = false;
    for (i = 0; i < g_clLaserInitList.size(); i++)
    {
		Laser::EnConnectType enConnectType;

    	if(strcmp(g_clLaserInitList[i].strPort.c_str(), "CARMEN"))
    	{
    		enConnectType = Laser::EnConnectType_DIRECT;
			if (g_clLaserInitList[i].bListen)
			{
				enConnectType = Laser::EnConnectType_LISTEN_IPT;
			}
			else if (g_clLaserInitList[i].bSend)
			{
				enConnectType = Laser::EnConnectType_DIRECT_AND_STREAM_IPT;
			}
    	}
    	else
    	{
    		enConnectType = Laser::EnConnectType_CARMEN;
    	}

        new Laser(
            enConnectType,
            g_clLaserInitList[i].strPort,
            g_clLaserInitList[i].strName,
            g_clLaserInitList[i].fAngleOffset,
            g_clLaserInitList[i].fXOffset,
            g_clLaserInitList[i].fYOffset);


        if (g_clLaserInitList[i].strStreamAddress != "")
        {
            bUseStream = true;
            Laser::SetStreamAddress(g_clLaserInitList[i].strStreamAddress);
        }

        sleep(2);
    }
    if (bUseStream)
    {
		Laser::streamStart();
        sleep(2);
    }

    if (g_clFrameGrabberInitList.size() > 0)
    {
        new Video(&video, g_clFrameGrabberInitList[0].strPortString);
        sleep(2);
    }

    if (useUrban) {
#ifndef NO_MOBILITY
        new Urban(&gRobot);
        sleep(2);
#else
        printTextWindow("No Mobility support");
#endif
    }

    if (g_clNomadInitList.size() > 0)
    {
        new Nomad(
            &gRobot,
            g_clNomadInitList[0].enType,
            g_clNomadInitList[0].strPortString,
            g_clNomadInitList[0].strHost);

        sleep(2);
    }

    if (useApm)
    {
        new Apm(&apm);
        sleep(2);
    }

    if (useX)
    {
        new Xwindow(&xwindow);
        sleep(2);
    }

    if (g_clPioneerInitList.size() > 0)
    {
        new Pioneer(&gRobot, g_clPioneerInitList[0].strPortString);
        sleep(2);
    }

    if (g_clPioneer2InitList.size() > 0)
    {
        new Pioneer2(&gRobot, g_clPioneer2InitList[0].strPortString);
        sleep(2);
    }

    if (g_clAmigoBotInitList.size() > 0)
    {
        new AmigoBot(&gRobot, g_clAmigoBotInitList[0].strPortString);
        sleep(2);
    }

	if (g_clRoombaInitList.size() > 0)
    {
        new Roomba(&gRobot, g_clRoombaInitList[0].strPortString);
        sleep(2);
    }

	if (g_clRoomba560InitList.size() > 0)
    {
        new Roomba560(&gRobot, g_clRoomba560InitList[0].strPortString);
        sleep(2);
    }

	if (g_clCarmenInitList.size() > 0)
    {
        new CarmenRobot(&gRobot, g_clCarmenInitList[0].strCentralServer);
        sleep(2);
    }

    if (useFred)
    {
        new Fred(&gRobot);
        sleep(2);
    }

    if (useJbox)
    {
        Jbox::connectToJbox(
            jbox,
            jboxID,
            jboxDisableNetworkQuery,
            gps_base_lat,
            gps_base_lon,
            gps_base_x,
            gps_base_y,
            gps_mperlat,
            gps_mperlon);

        sleep (2);
    }

    if (g_clGyroInitList.size() > 0)
    {
        switch (g_clGyroInitList[0].type)
        {

        case (int)(Gyro::TYPE_DMUVGX):
#ifndef NO_MOBILITY
            new GyroDMUVGX(&gGyro);
            sleep(2);
#else
            printTextWindow("No Mobility support");
#endif
            break;

        default:
            break;
        }
    }

    if (g_clCompassInitList.size() > 0)
    {
        switch (g_clCompassInitList[0].type) {

        case (int)(Compass::TYPE_KVHC100):
#ifndef NO_MOBILITY
            new CompassKVHC100(&gCompass);
            sleep(2);
#else
            printTextWindow("No Mobility support");
#endif
            break;

        case (int)(Compass::TYPE_3DMG):
            new Compass3DMG(&gCompass, g_clCompassInitList[0].portString);
            sleep(2);
            break;

        }
    }

    //if (g_clGpsInitList.size() > 0)
    if (useGps)
    {
    	{
    		if(strcmp(g_clGpsInitList[0].strGpsType.c_str(), "CARMEN"))
    		{
    			new GpsDefault(
    				&gps,
    				g_clGpsInitList[0].strPortString,
    				g_clGpsInitList[0].dBaseLat,
    				g_clGpsInitList[0].dBaseLong,
    				g_clGpsInitList[0].dXDiff,
    				g_clGpsInitList[0].dYDiff,
    				g_clGpsInitList[0].dMPerLat,
    				g_clGpsInitList[0].dMPerLong,
    				g_clGpsInitList[0].bUseBase);
    		}
    		else
    		{
    			new GpsCARMEN(&gps,
    					g_clGpsInitList[0].strPortString);
    		}
    	}

        sleep(2);
    }

    if (g_clPoseCalcInitList.size() > 0)
    {
        poseCalcConstants.compass2MlabHeadingFacor =
            g_clPoseCalcInitList[0].compass2MlabHeadingFacor;
        poseCalcConstants.compass2MlabHeadingOffset =
            g_clPoseCalcInitList[0].compass2MlabHeadingOffset;
        poseCalcConstants.compassMaxValidAngSpeed4Heading =
            g_clPoseCalcInitList[0].compassMaxValidAngSpeed4Heading;
        poseCalcConstants.gps2MlabHeadingFacor =
            g_clPoseCalcInitList[0].gps2MlabHeadingFacor;
        poseCalcConstants.gps2MlabHeadingOffset =
            g_clPoseCalcInitList[0].gps2MlabHeadingOffset;
        poseCalcConstants.gpsMinValidTransSpeed4Heading =
            g_clPoseCalcInitList[0].gpsMinValidTransSpeed4Heading;
        poseCalcConstants.gpsMaxValidAngSpeed4Heading =
            g_clPoseCalcInitList[0].gpsMaxValidAngSpeed4Heading;

        if ((poseCalc = new PoseCalc(
                 poseCalcConstants,
                 g_clPoseCalcInitList[0].fuserType,
                 g_clPoseCalcInitList[0].varianceScheme)) == NULL)
        {
            printTextWindow("Fatal error: Couldn't allocate memory for PoseCalculator.");
            exit(-1);
        }

        gPoseCalc = poseCalc;
    }
    else
    {
        printTextWindow(
            "Fatal error: Section name for PoseCalculator not found in the RC file.");
        exit(-1);
    }

    refreshScreen();

    if (gRobot == NULL)
    {
        // Connect to a robot if not done yet.
        robotConnect(gRobot);
    }

    if (gRobot != NULL)
    {
        poseCalcID = gPoseCalc->addRobot(gRobot);
        gRobot->savePoseCalcID(poseCalcID);
    }
    else
    {
        printTextWindow("\n");
        printTextWindow("Error: HServer has to be running with a robot.");
        exit(-1);
    }


    if (gps != NULL)
    {
        poseCalcID = gPoseCalc->addGps(gps);
        gps->savePoseCalcID(poseCalcID);
    }

    if (gGyro != NULL)
    {
        poseCalcID = gPoseCalc->addGyro(gGyro);
        gGyro->savePoseCalcID(poseCalcID);
    }

    if (gCompass != NULL)
    {
        poseCalcID = gPoseCalc->addCompass(gCompass);
        gCompass->savePoseCalcID(poseCalcID);
    }

    if (gRemoteControl == NULL)
    {
        gRemoteControl = new RemoteControl();
    }

    gPoseCalc->resetPose();

    pthread_create(&keyboard_reader, NULL, (void* (*)(void*)) &keyboard_reader_thread, NULL);

    sem_wait(&exit_sem);
    return 0;
}



string FindRCFile(string strOriginalFileName)
{
    unsigned int i;
    vector<string> clDirList;
    FILE* pfFile;
    string strCurrentFileName;
    string strRCFileName = "";

    // first, try the raw file name that's passed in to check the current directory
    if ((pfFile = fopen(strOriginalFileName.c_str(), "r")) != NULL)
    {
        fclose(pfFile);
        strRCFileName = strOriginalFileName;
    }

    // if the file hasn't been found yet, check the home directory
    if (strRCFileName == "")
    {
        const char* szHomeDir = getenv("HOME");
        if (szHomeDir != NULL)
        {
            strCurrentFileName = TruncateSlash(szHomeDir) + '/' + strOriginalFileName;
            if ((pfFile = fopen(strCurrentFileName.c_str(), "r")) != NULL)
            {
                fclose(pfFile);
                strRCFileName = strCurrentFileName;
            }
        }
    }

    // if the file hasn't been found yet, check the directories in the PATH
    // environment variable
    if (strRCFileName == "")
    {
        clDirList = ExtractDirectories("PATH");
        for (i = 0; i < clDirList.size(); i++)
        {
            strCurrentFileName = clDirList[i] + '/' + strOriginalFileName;
            if ((pfFile = fopen(strCurrentFileName.c_str(), "r")) != NULL)
            {
                fclose(pfFile);
                strRCFileName = strCurrentFileName;
                break;
            }
        }
    }

    return strRCFileName;
}

void usage(void)
{
    printf("If ran with no arguments hserver starts up with no hardware connections.\n");
    printf("Hardware can be then connected via keyboard command.\n");
    printf("\n");
    printf("Hardware can be started up automatically with command line arguments.\n");
    printf("\n");
    printf("-C config_file_name Use this configuration file\n");
    printf("-c camera_section   Connect to a Sony evi-d20 described in the configuration\n");
    printf("                    file under camera section <start camera ...>\n");
    printf("-l laser_section    Connect to a SICK LMS200 laser scanner using settings\n");
    printf("                    described in the configuration file under laser\n");
    printf("                    section <start laser ...>\n");
    printf("-m cognachrome_section\n");
    printf("                    Connect to cognachrome vision system described in the\n");
    printf("                    configuration file under cognachrome section\n");
    printf("                    <start cognachrome ...>\n");
    printf("-n nomad_section    Connect to a Nomad 150 or Nomad 200 described in the\n");
    printf("                    configuration file under nomad section <nomad_section>\n");
    printf("-p pioneer_section  Connect a Pioneer described in the configuration\n");
    printf("                    file under pioneer section <start pioneer ...>\n");
    printf("-b amigobot_section Connect an AmigoBot described in the configuration\n");
    printf("                    file under amigobot section <start amigobot ...>\n");
	printf("-R Roomba_section Connect a Roomba described in the configuration\n");
    printf("                    file under Roomba section <start Roomba ...>\n");
	printf("-T Roomba560_section Connect a Roomba560 described in the configuration\n");
    printf("                    file under Roomba560 section <start Roomba560 ...>\n");
    printf("-u                  Connect urban via mobility\n");
    printf("-j                  Connect ATRV-Jr via mobility\n");
    printf("-v framegrabber_section\n");
    printf("                    Connect to framegrabber described in the configuration\n");
    printf("                    file under framegrabber section <start framegrabber ...>\n");
    printf("-g gps_section      Connect to the GPS unit described in the configuration file\n");
    printf("                    under gps section <start gps ...>\n");
    printf("-y gyro_section     Connect to a gyroscope.\n");
    printf("-k compass_section  Connect a compass described in the configuration\n");
    printf("                    file under compass section <start compass ...>\n");
    printf("\n");
    printf("Other options\n");
    printf("\n");
    printf("-r                  hserver ipt name  defaults to fred\n");
    printf("-B                  Display battery information\n");
    printf("-f                  Connect Fred\n");
    printf("-i hostname         Start ipt. Set ipthost to [hostname] (optional)\n");
    printf("-a                  Start ipt and register hclient messages\n");
    printf("-o                  Setup ipt handler for multiple hservers\n");
    printf("-x                  Open X window\n");
    printf("-S                  no spash screen\n");
    printf("-J                  Connect to JBox\n");
    printf("-w                  Connect to webCam via USB\n");
    printf("-P posecalc_section Pose Calculator configuration\n");
    printf("\n");
    printf("-h                  this help message\n");
}

int main(int argc, char* argv[])
{
    int c;
    unsigned int i, j;
    char* szEnvironment;
    string strRCFileName = ".hserverrc";
    vector<SuOption> clOptionList;
    SuOption suOption;
    bool posecalcSectionSpecified = false;

    useTerminal = false;
    useUrban = false;
    useATRVJr = false;
    useIpt = false;
    useHclient = false;
    useX = false;
    useFred = false;
    multipleHservers = false;


    szEnvironment = getenv("HSERVER_USE_IPT");
    if (szEnvironment != NULL)
    {
        useIpt = true;
    }

    while((c = getopt (argc, argv, "C:p:d:s:q:b:t:c:i::t:l:m:n:r:g:k:Sv:eujaBfoxhy:J:P:L:R:T:A:Z")) != -1)
    {
        switch (c) {

        case 'L':
            logData = true;
            break;

        case 'C':
            strRCFileName = optarg;
            break;

        case 'S':
            noSplash = true;
            break;

        case 's':
        	ipc_preferred_server_name(strdup(optarg));
        	break;

        case 'q':
        	suOption.enSectionType = ClHServerConfig::EnHServerConfigSectionType_CARMEN;
			suOption.strSectionName = optarg;
			clOptionList.push_back(suOption);
			break;

        case 'g':
            suOption.enSectionType = ClHServerConfig::EnHServerConfigSectionType_GPS;
            suOption.strSectionName = optarg;
            clOptionList.push_back(suOption);
            useGps = true;
            break;

        case 'u':
            useUrban = true;
            break;

        case 'j':
            useATRVJr = true;
            break;

        case 'p':
            suOption.enSectionType = ClHServerConfig::EnHServerConfigSectionType_PIONEER;
            suOption.strSectionName = optarg;
            clOptionList.push_back(suOption);
            break;

        case 'd':
            suOption.enSectionType = ClHServerConfig::EnHServerConfigSectionType_PIONEER2;
            suOption.strSectionName = optarg;
            clOptionList.push_back(suOption);
            break;

        case 'b':
            suOption.enSectionType = ClHServerConfig::EnHServerConfigSectionType_AMIGOBOT;
            suOption.strSectionName = optarg;
            clOptionList.push_back(suOption);
            break;

	case 'R':
            suOption.enSectionType = ClHServerConfig::EnHServerConfigSectionType_ROOMBA;
            suOption.strSectionName = optarg;
            clOptionList.push_back(suOption);
            break;
	case 'T':
            suOption.enSectionType = ClHServerConfig::EnHServerConfigSectionType_ROOMBA560;
            suOption.strSectionName = optarg;
            clOptionList.push_back(suOption);
            break;

        case 'c':
            suOption.enSectionType = ClHServerConfig::EnHServerConfigSectionType_CAMERA;
            suOption.strSectionName = optarg;
            clOptionList.push_back(suOption);
            break;

        case 'l':
            suOption.enSectionType = ClHServerConfig::EnHServerConfigSectionType_LASER;
            suOption.strSectionName = optarg;
            clOptionList.push_back(suOption);
            break;

        case 'm':
            suOption.enSectionType = ClHServerConfig::EnHServerConfigSectionType_COGNACHROME;
            suOption.strSectionName = optarg;
            clOptionList.push_back(suOption);
            break;

        case 'v':
            suOption.enSectionType = ClHServerConfig::EnHServerConfigSectionType_FRAMEGRABBER;
            suOption.strSectionName = optarg;
            clOptionList.push_back(suOption);
            break;

        case 'n':
            suOption.enSectionType = ClHServerConfig::EnHServerConfigSectionType_NOMAD;
            suOption.strSectionName = optarg;
            clOptionList.push_back(suOption);
            break;

        case 'i':
            useIpt = true;
            if (optarg)
            {
                ipt_server_name = strdup(optarg);
            }
            break;

        case 'a':
            useIpt = true;
            useHclient = true;
            break;

        case 'B':
            useApm = true;
            break;

        case 'y':
            suOption.enSectionType = ClHServerConfig::EnHServerConfigSectionType_GYRO;
            suOption.strSectionName = optarg;
            clOptionList.push_back(suOption);
            break;

        case 'J':
            useJbox = true;
            suOption.enSectionType = ClHServerConfig::EnHServerConfigSectionType_JBOX;
            suOption.strSectionName = optarg;
            clOptionList.push_back(suOption);
            break;

        case 'P':
            suOption.enSectionType = ClHServerConfig::EnHServerConfigSectionType_POSECALC;
            suOption.strSectionName = optarg;
            clOptionList.push_back(suOption);
            posecalcSectionSpecified = true;
            break;

        case 'k':
            suOption.enSectionType = ClHServerConfig::EnHServerConfigSectionType_COMPASS;
            suOption.strSectionName = optarg;
            clOptionList.push_back(suOption);
            break;

        case 'f':
            useFred = true;
            break;

        case 'r':
            strcpy(robot_name, optarg);
            break;

        case 'o':
            multipleHservers = true;
            break;

        case 'x':
            useX = true;
            break;

        case 'h':
            usage();
            return 0;
        case 'w':
            suOption.enSectionType = ClHServerConfig::EnHServerConfigSectionType_WEBCAM;
            suOption.strSectionName = optarg;
            clOptionList.push_back(suOption);
            break;

        default:
            usage();
            exit(-1);
        }
    }


    if (!posecalcSectionSpecified)
    {
        suOption.enSectionType = ClHServerConfig::EnHServerConfigSectionType_POSECALC;
        suOption.strSectionName = DEFAULT_POSECALC_RC_SECTION_NAME;
        clOptionList.push_back(suOption);
        posecalcSectionSpecified = true;
    }

    string strOriginalRCFileName = strRCFileName; // save this for error msg
    strRCFileName = FindRCFile(strRCFileName);  // try to find the RC file using
                                                  //  PATH and HOME environment vars
    // make sure we have a valid RC file name
    if (strRCFileName == "")
    {
        fprintf(stderr, "hserver error: couldn't find RC file: %s\n",
                 strOriginalRCFileName.c_str());
        exit(-1);
    }

    // parse the RC file into a ClHServerConfig object
    ClHSRCFile clRCFile(strRCFileName);
    ClHServerConfig clConfig;
    if (!clRCFile.CreateConfiguration(clConfig))
    {
        fprintf(stderr, "hserver error: error with configuration file\n");
        exit(-1);
    }

    ClHServerConfigurator clConfigurator(clConfig);

    bool bFoundSection = false;

    // look at all options from the command line that could specify sections from the config file
    for (i = 0; i < clOptionList.size(); i++)
    {
        bFoundSection = false;
        for (j = 0; j < clConfig.GetSectionCount(); j++)
        {
            string strSectionName;

            // check to see if the current option matches any sections in the config file
            if ((clConfig.GetSectionType(j) == clOptionList[i].enSectionType) &&
                 clConfig.GetSectionName(j, strSectionName) &&
                 (clOptionList[i].strSectionName == strSectionName))
            {
                SuCameraInitData suCameraData;
                SuLaserInitData suLaserData;
                SuCognachromeInitData suCognachromeData;
                SuNomadInitData suNomadData;
                SuPioneerInitData suPioneerData;
                SuPioneer2InitData suPioneer2Data;
                SuAmigoBotInitData suAmigoBotData;
				SuRoombaInitData suRoombaData;
				SuRoomba560InitData suRoomba560Data;
				SuCarmenInitData suCarmenData;
                SuFrameGrabberInitData suFrameGrabberData;
                SuGpsInitData suGpsData;
                SuJboxInitData suJboxData;
                SuCompassInitData suCompassData;
                SuGyroInitData suGyroData;
                SuPoseCalcInitData suPoseCalcData;
                SuWebCamInitData suWebCamData;

                // get device-specific info from a general option
                switch (clOptionList[i].enSectionType)
                {
                case ClHServerConfig::EnHServerConfigSectionType_CAMERA:
                    suCameraData = clConfigurator.GetCameraOptions((int) j);
                    g_clCameraInitList.push_back(suCameraData);
                    break;
                case ClHServerConfig::EnHServerConfigSectionType_LASER:
                    suLaserData = clConfigurator.GetLaserOptions((int) j);
                    g_clLaserInitList.push_back(suLaserData);
                    break;
                case ClHServerConfig::EnHServerConfigSectionType_COGNACHROME:
                    suCognachromeData = clConfigurator.GetCognachromeOptions((int) j);
                    g_clCognachromeInitList.push_back(suCognachromeData);
                    break;
                case ClHServerConfig::EnHServerConfigSectionType_NOMAD:
                    suNomadData = clConfigurator.GetNomadOptions((int) j);
                    g_clNomadInitList.push_back(suNomadData);
                    break;
                case ClHServerConfig::EnHServerConfigSectionType_PIONEER:
                    suPioneerData = clConfigurator.GetPioneerOptions((int) j);
                    g_clPioneerInitList.push_back(suPioneerData);
                    break;
                case ClHServerConfig::EnHServerConfigSectionType_PIONEER2:
                    suPioneer2Data = clConfigurator.GetPioneer2Options((int) j);
                    g_clPioneer2InitList.push_back(suPioneer2Data);
                    break;
                case ClHServerConfig::EnHServerConfigSectionType_AMIGOBOT:
                    suAmigoBotData = clConfigurator.GetAmigoBotOptions((int) j);
                    g_clAmigoBotInitList.push_back(suAmigoBotData);
                    break;

				case ClHServerConfig::EnHServerConfigSectionType_ROOMBA:
                    suRoombaData = clConfigurator.GetRoombaOptions((int) j);
                    g_clRoombaInitList.push_back(suRoombaData);
                    break;
				case ClHServerConfig::EnHServerConfigSectionType_ROOMBA560:
                    suRoomba560Data = clConfigurator.GetRoomba560Options((int) j);
                    g_clRoomba560InitList.push_back(suRoomba560Data);
                    break;
				case ClHServerConfig::EnHServerConfigSectionType_CARMEN:
					suCarmenData = clConfigurator.GetCarmenOptions((int) j);
					g_clCarmenInitList.push_back(suCarmenData);
					break;
                case ClHServerConfig::EnHServerConfigSectionType_FRAMEGRABBER:
                    suFrameGrabberData = clConfigurator.GetFrameGrabberOptions((int) j);
                    g_clFrameGrabberInitList.push_back(suFrameGrabberData);
                    break;
                case ClHServerConfig::EnHServerConfigSectionType_GPS:
                    suGpsData = clConfigurator.GetGpsOptions((int) j);
                    g_clGpsInitList.push_back(suGpsData);
                    if (!suGpsData.bUseBase) // this needs to go away some time, along w/ the
                                               // global vars, but for time's sake now... jbl
                    {
                        gps_base_lat = suGpsData.dBaseLat;
                        gps_base_lon = suGpsData.dBaseLong;
                        gps_base_x = suGpsData.dXDiff;
                        gps_base_y = suGpsData.dYDiff;
                        gps_mperlat = suGpsData.dMPerLat;
                        gps_mperlon = suGpsData.dMPerLong;
                    }
                    break;
                case ClHServerConfig::EnHServerConfigSectionType_JBOX:
                    suJboxData = clConfigurator.GetJboxOptions((int) j);
                    g_clJboxInitList.push_back(suJboxData);
                    jboxID = suJboxData.jboxID;
                    jboxDisableNetworkQuery = suJboxData.disableNetworkQuery;
                    if (!useGps)
                    {
                        suOption.enSectionType = ClHServerConfig::EnHServerConfigSectionType_GPS;
                        suOption.strSectionName = suJboxData.gpsSection;
                        clOptionList.push_back(suOption);
                    }
                    break;
                case ClHServerConfig::EnHServerConfigSectionType_COMPASS:
                    suCompassData = clConfigurator.GetCompassOptions((int) j);
                    g_clCompassInitList.push_back(suCompassData);
                    break;
                case ClHServerConfig::EnHServerConfigSectionType_GYRO:
                    suGyroData = clConfigurator.GetGyroOptions((int) j);
                    g_clGyroInitList.push_back(suGyroData);
                    break;
                case ClHServerConfig::EnHServerConfigSectionType_POSECALC:
                    suPoseCalcData = clConfigurator.GetPoseCalcOptions((int) j);
                    g_clPoseCalcInitList.push_back(suPoseCalcData);
                    break;
                case ClHServerConfig::EnHServerConfigSectionType_WEBCAM:
                    suWebCamData = clConfigurator.GetWebCamOptions((int) j);
                    g_clWebCamInitList.push_back(suWebCamData);
                    break;
                default:
                    fprintf(stderr, "hserver error: encountered an invalid option type.\n");
                    exit(-1);
                }
                bFoundSection = true;
                break;
            }
        }
        if (!bFoundSection)
        {
            fprintf(stderr, "hserver error: option with name \"%s\" could not be found in\n",
                     clOptionList[i].strSectionName.c_str());
            fprintf(stderr, "   the configuration file.\n");
            exit(-1);
        }
    }

    startHServer();
}

/**********************************************************************
# $Log: hserver.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.3  2006/06/24 21:51:20  endo
# gcc 4.1 upgrade
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.11  2004/11/22 00:18:37  endo
# Network query of JBox can now be disabled.
#
# Revision 1.10  2004/09/22 03:59:34  endo
# *** empty log message ***
#
# Revision 1.9  2004/09/10 19:41:06  endo
# New PoseCalc integrated.
#
# Revision 1.8  2004/04/24 11:20:53  endo
# 3DM-G added.
#
# Revision 1.7  2004/04/19 08:49:45  endo
# Particle filter fixed. Visualization option added.
#
# Revision 1.6  2004/04/17 12:20:35  endo
# Intalling JBox Sensor Update.
#
# Revision 1.5  2004/04/13 19:25:33  yangchen
# JBox module integrated.
#
# Revision 1.4  2004/04/12 01:14:36  endo
# noSplash meant to be true.
#
# Revision 1.3  2004/04/12 01:12:19  endo
# Default noSplash is now "false".
#
# Revision 1.2  2004/04/06 02:07:39  pulam
# Added code for Silent Mode
#
# Revision 1.1.1.1  2004/02/27 22:30:50  endo
# Initial MARS 2020 revision
#
# Revision 1.14  2003/04/26 06:48:31  ebeowulf
# Added the Pioneer2-dxe controller.
#
# Revision 1.13  2003/04/06 13:32:37  endo
# gcc 3.1.1
#
# Revision 1.12  2002/04/01 22:20:44  endo
# sound DSP removed.
#
# Revision 1.11  2002/02/18 13:48:21  endo
# AmigoBot added.
#
# Revision 1.10  2002/01/16 21:46:27  ernest
# Added WatchDog functionality
#
# Revision 1.9  2001/05/29 22:36:30  blee
# Made changes to incorporate gps data w/ other position info.
#
# Revision 1.8  2001/03/23 21:19:44  blee
# massive modifications to use a config file
#
# Revision 1.7  2000/12/12 22:54:18  blee
# Updates were added to support multiple lasers.
# Added printfTextWindow.
# Fixed usage() and added the -h command line option for help.
#
# Revision 1.6  2000/11/13 20:09:23  endo
# ARTV-Jr class added to HServer.
#
# Revision 1.5  2000/09/19 09:06:31  endo
# robot name -> hserver name.
#
# Revision 1.4  2000/09/19 03:14:21  endo
# RCS log added.
#
#
#**********************************************************************/
