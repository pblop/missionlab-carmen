/**********************************************************************
**                                                                  **
**                              webCam.h                            **
**                                                                  **
**                                                                  **
**  Written by:  Alberto Rodríguez Valle                            **
**                                                                  **
**  webCam control para HServer                                     **
**                                                                  **
**                                                                  **
 **********************************************************************/


#ifndef WEBCAM_H_
#define WEBCAM_H_

#include <pthread.h>
#include <string>
#include <linux/videodev2.h>

#include "module.h"
#include "sensors.h"
#include "robot_config.h"

#define CAPTURE_IMAGE_WIDTH 640     /* image width */
#define CAPTURE_IMAGE_HEIGHT    480     /* image height */

#define RGB         3       /* number of bit plane */


using std::string;

class WebCam: public Module
{
    protected:
        Sensor* sensorWebCam;
        RobotPart* partWebCam;
        pthread_mutex_t camQMutex;
//         int qTop, qBot, qEmpty, qNext;
//         int haveMove;
        int devfd;
        unsigned char *frame_buf;
        int err;
        struct video_capability vcap;
        struct video_mbuf vm;
        struct video_picture vp;
        struct v4l2_format fmt;
        struct video_mmap vmap;
        int quality;
        bool threads_running;
        pthread_t reader;
        float updateSpeed;
        float current_readings[ 4 ];
        unsigned char disp_image[CAPTURE_IMAGE_WIDTH*CAPTURE_IMAGE_HEIGHT];
        void addSensor();
        void updateStatusBar_(void);
        void decodeMessage( unsigned char* msg );
        void getPacket();

    public:
        void reader_thread();
//         void tracker_thread();
//         void setTrackerMode( int mode );
//         void MDModeOnOff( bool on );
        void control();
        WebCam(WebCam** a, const string& strPort );
        ~WebCam();
};


//extern WebCam *webCam;

void webCamConnect(WebCam  *a);

#endif
