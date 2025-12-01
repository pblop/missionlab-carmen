/* 

   Universal remote control for hserver

   William Halliburton

*/

#include <panel.h>
#include <stdlib.h>
#include "camera.h"
#include "hserver.h"
#include "message.h"
#include "console.h"
#include "fred.h"
#include "nomad.h"
#include "robot.h"


void runRemoteControl()
{
    bool bDone = false;
    int velocity = 0;
    float uvelocity = 0;
    int rvelocity = 0;
    float urvelocity = 0;
    float armvelocity = 0;
    int cameraInc = 4;
    int robotInc = 16;
    int msgData[7];
    int pan = 0;
    int tilt = 0;
    int refresh = 0;
    int pandegree,tiltdegree;

    disableConsoleRobotControl = true;

    msgData[0] = cameraInc;
    msgData[1] = robotInc;
    msgData[2] = velocity;
    msgData[3] = rvelocity;
    msgData[4] = (int)(armvelocity);
    msgData[5] = false; // teleport
    msgData[6] = 4; // teleport inc
    show_panel(msgPanel);
    redrawWindows();

    messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
    pandegree = int(pan * PAN_TO_DEG);
    tiltdegree = int(tilt * TILT_TO_DEG);
    do
    {
        int c = getch();
        switch(c){
        case 'w':
            tilt += cameraInc;
            tiltdegree = int(tilt * TILT_TO_DEG);
            if (tilt > TILT_ABS_POS_MAX) tilt = TILT_ABS_POS_MAX;
            if (camera!=NULL) camera->moveAbs(pan,tilt);
            break;
        case 's':
            tilt -= cameraInc;
            tiltdegree = int(tilt * TILT_TO_DEG);
            if (tilt < TILT_ABS_POS_MIN) tilt = TILT_ABS_POS_MIN;
            if (camera!=NULL) camera->moveAbs(pan,tilt);
            break;
        case 'a':
            pan -= cameraInc;
            pandegree = int(pan * PAN_TO_DEG);
            if (pan > PAN_ABS_POS_MAX) tilt = PAN_ABS_POS_MAX;
            if (camera!=NULL) camera->moveAbs(pan,tilt);
            break;
        case 'd':
            pan += cameraInc;
            pandegree = int(pan * PAN_TO_DEG);
            if (pan < PAN_ABS_POS_MIN) tilt = PAN_ABS_POS_MIN;
            if (camera!=NULL) camera->moveAbs(pan,tilt);
            break;
        case '0':
            pan = tilt = 0;
            pandegree = tiltdegree = 0;
            if (camera!=NULL) camera->moveAbs(0,0);
            break;
        case 'c':
            cameraInc = cameraInc / 2;
            if (cameraInc == 0) cameraInc = 1;
            msgData[0] = cameraInc; 
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;
        case 'e':
            cameraInc = cameraInc * 2;
            msgData[0] = cameraInc;
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;
        case 'r':
            refreshScreen();
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            break;
        case 'q':
            if (camera!=NULL) camera->zoomInc(cameraInc*10);
            break;
        case 'z':
            if (camera!=NULL) camera->zoomInc(cameraInc*10);
            break;
        case 'Q':
        case 'x':
            bDone = true;
            break;
        case 'i':
            if (msgData[5]) {
                if (robot != NULL) robot->adjustXYT(0,abs(msgData[6]),0);
                break;
            }
            if (robot == NULL) break;
            velocity += robotInc;
            if (robot != NULL) robot->drive(float(velocity)/1000.0);
            msgData[2] = velocity; 
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;
        case 'k':
            if (msgData[5]) {
                robot->adjustXYT(0,-abs(msgData[6]),0);
                break;
            }
            if (robot == NULL) break;
            velocity -= robotInc;
            if (robot != NULL) robot->drive(float(velocity)/1000.0);
            msgData[2] = velocity ; 
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;
        case 'j':
            if (msgData[5]) {
                robot->adjustXYT(-abs(msgData[6]),0,0);
                break;
            }
            if (robot == NULL) break;
            rvelocity += robotInc;
            if (robot != NULL) robot->steer(rvelocity);
            msgData[3] = rvelocity; 
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;
        case 'l':
            if (msgData[5]){
                robot->adjustXYT(abs(msgData[6]),0,0);
                break;
            }
            if (robot == NULL) break;
            rvelocity -= robotInc;
            if (robot != NULL) robot->steer(rvelocity);
            msgData[3] = rvelocity; 
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;
        case ' ':
            if (robot == NULL) break;
            velocity = rvelocity = 0;
            uvelocity = urvelocity = 0;
            armvelocity = 0;
            if (robot != NULL) robot->stop();
            msgData[2] = rvelocity; 
            msgData[3] = rvelocity; 
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;
        case 'o':
            if (msgData[5]){
                robot->adjustXYT(0,0,abs(msgData[6]));
                break;
            }
            if (robot == NULL) break;
            rvelocity = 0;
            if (robot != NULL) robot->steer(0);
            msgData[3] = rvelocity; 
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;
        case 'u':
            if (msgData[5]) {
                robot->adjustXYT(0,0,-abs(msgData[6]));
                break;
            }
            if (robot == NULL) break;
            velocity = 0;
            if (robot != NULL) robot->drive(0);
            msgData[2] = velocity; 
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;
        case 'p':
            if (msgData[5]) {
                msgData[6] = msgData[6] * 2;
                messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
                refresh = true;
                break;
            }
            if (robot == NULL) break;
            robotInc = robotInc * 2;
            msgData[1] = robotInc;
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;
        case ';':
            if (msgData[5]) {
                msgData[6] = msgData[6] / 2;
                if (msgData[6] == 0) msgData[6] = 1;
                messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
                refresh = true;
                break;
            }
            if (robot == NULL) break;
            robotInc = robotInc / 2;
            if (robotInc == 0) robotInc = 1;
            msgData[1] = robotInc;
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;
        case 'n':
            armvelocity += ((float)robotInc);
            robot->arm(armvelocity);
            msgData[3] = (int)(armvelocity);
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;
        case 'm':
            armvelocity -= ((float)robotInc);
            robot->arm(armvelocity);
            msgData[3] = (int)(armvelocity);
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;
        case ',':
            armvelocity = 0;
            robot->arm(armvelocity);
            msgData[3] = (int)(armvelocity);
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;
        case 't':
            msgData[5] = !msgData[5];
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
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
            messageMovePanelKey(c);
            break;
        default:
            printfTextWindow( "key %d", c );
            break;
        }
        if (refresh) {
            pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, (void*) &screenMutex);
            pthread_mutex_lock(&screenMutex);
            wrefresh(msgWindow);
            pthread_cleanup_pop(1);
            refresh = false;
        }
    } while ( !bDone );
    hide_panel(msgPanel);
    redrawWindows();
    disableConsoleRobotControl = false;
}
