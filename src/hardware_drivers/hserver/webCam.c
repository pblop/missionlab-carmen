/**********************************************************************
**                                                                  **
**                              webCam.c                            **
**                                                                  **
**                                                                  **
**  Written by:  Alberto Rodríguez Valle                            **
**                                                                  **
**  webCam control para HServer                                     **
**                                                                  **
**                                                                  **
 **********************************************************************/

        /* $Id: camera.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifdef LINUX_VIDEODEV_PRESENT
	#include <linux/videodev.h>
#else
	#include <libv4l1-videodev.h>
#endif

#include <unistd.h>     /* for mmap() */
#include <sys/mman.h>



/*=====================================================
        Definitions for Video4Linux
        =====================================================*/
//#define DEFAULT_DEVICE_NAME "/dev/video"    /* device file name */
#define MAX_NO_CHANNEL      10      /* maximum number of channel of frame grabber */

#define COMPOSITE1      0       /* for IO-DATA GV-VCP2M/PCI */
#define COMPOSITE2      1
#define S_VIDEO         2

#define DOWN_SAMPLING_RATE  2       /* to remove even number field */

#define IMAGE_WIDTH_DS      CAPTURE_IMAGE_WIDTH/DOWN_SAMPLING_RATE  /* image size after down sampling */
#define IMAGE_HEIGHT_DS     CAPTURE_IMAGE_HEIGHT/DOWN_SAMPLING_RATE
#define PREFIX_IMAGE_FILE   "video_image"


#include "HServerTypes.h"
#include "hserver.h"
#include "webCam.h"
#include "message.h"
#include "cognachrome.h"
#include "statusbar.h"
#include "laser.h"
#include "robot_config.h"
#include "sensors.h"





void WebCam::updateStatusBar_(void)
{
    char buf[ 100 ];
    struct timeval actual;
    static struct timeval anterior;
    static int inicio = 0;
    double diferencia;

    if(inicio == 0){
        gettimeofday(&anterior, NULL);
        inicio ++;
    }



    gettimeofday(&actual, NULL);
    diferencia = (actual.tv_sec - anterior.tv_sec) * 1000000 + (actual.tv_usec - anterior.tv_usec);
    statusStr[ 0 ] = '\0';

    strcat( statusStr, "WebCam: " );

    sprintf( buf, "%c %.0f up/sec", statusbarSpinner_->getStatus(),1000000.0 / diferencia);

    strcat( statusStr, buf );
    statusbar->update( statusLine );
    anterior = actual;
}


void WebCam::decodeMessage( unsigned char* msg )
{
}

void WebCam::getPacket()
{
    //static char image_prefix[1024]=PREFIX_IMAGE_FILE;           /* Prefix of output image file */
    static unsigned char image[IMAGE_WIDTH_DS*IMAGE_HEIGHT_DS*RGB]; /* Captured image */
    //static unsigned char skin_map[IMAGE_WIDTH_DS*IMAGE_HEIGHT_DS];  /* Skin color map */
    unsigned char *pixel_pos;
    int i,j;
    int dheight,dwidth;
    int down_sampling_rate = DOWN_SAMPLING_RATE;

    if( ioctl( devfd , VIDIOCSYNC , &(vmap.frame) ) == -1 ) {
        printfTextWindow("Error en VIDIOCSYNC.\n");
        return;
    }
    pthread_mutex_lock(&camQMutex);
    /*======= Change order of data from BGR(capture board order) to RGB for processing ===============*/
    dheight = vmap.height/down_sampling_rate;
    dwidth = vmap.width/down_sampling_rate;
    pixel_pos = frame_buf+vm.offsets[vmap.frame];
    for( i = 0 ; i < dheight ; i++ ) {
        for( j = 0 ; j < dwidth; j++ ) {
            /* image for processing: RGB order */
            image[((dheight-1-i)*dwidth+j)*RGB] = pixel_pos[2];
            image[((dheight-1-i)*dwidth+j)*RGB+1] = pixel_pos[1];
            image[((dheight-1-i)*dwidth+j)*RGB+2] = pixel_pos[0];
            /* image for display: BRG order */
            disp_image[((dheight-1-i)*dwidth+j)*RGB] = pixel_pos[0];
            disp_image[((dheight-1-i)*dwidth+j)*RGB+1] = pixel_pos[1];
            disp_image[((dheight-1-i)*dwidth+j)*RGB+2] = pixel_pos[2];
            pixel_pos += RGB*down_sampling_rate;
        }
        pixel_pos += vmap.width*RGB*(down_sampling_rate-1);
    }
    pthread_mutex_unlock(&camQMutex);
    if( ioctl( devfd , VIDIOCMCAPTURE , &vmap ) == -1 ) {
        printfTextWindow("Error en VIDIOCMCAPTURE.\n");
        return;
    }

    /*======= Change buffer ===================================*/
    if( vmap.frame == 0 ) {
        vmap.frame = 1;
    } else {
        vmap.frame = 0;
    }
    //glutSwapBuffers();

    //frame_no++;

}


static void start_reader_thread( void* webCam_instance )
{
    ( (WebCam*) webCam_instance)->reader_thread();
}

                    /* Thread to read all repies from camera and send from send que */
void WebCam::reader_thread()
{
    pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, NULL );
    while( true )
    {
        pthread_testcancel();
        getPacket();
        updateStatusBar_();
    }
}

void WebCam::addSensor()
{
    partWebCam = new RobotPartWebCam( 0, 0);
    sensorWebCam = new Sensor( WEBCAM, 0, NULL, fmt.fmt.pix.sizeimage, (float *)disp_image );
}

WebCam::WebCam( WebCam** a, const string& strPort ) : Module( (Module**) a, HS_MODULE_NAME_WEBCAM)
{
    int i;
    devfd = -1;
    sensorWebCam = NULL;
    partWebCam = NULL;

    pthread_mutex_init( &camQMutex, NULL );

    printfTextWindow( "WebCam serial port: %s", strPort.c_str() );
    printTextWindow( "Testing camera connection... \n", false );



    if( ( devfd = open(strPort.c_str(), O_RDWR ) ) == -1 ) {
        fprintf(stderr, "Device %s can't be opened.\n", strPort.c_str());
        return;
    }


    if( ioctl( devfd , VIDIOCGCAP , &vcap ) == -1 ) {
        printfTextWindow("Error al obtener propiedades del dispositivo.\n");
        return ;
    }

    printfTextWindow("Maximum width of image: %d\n" , vcap.maxwidth );
    printfTextWindow("Maximum height of image: %d\n", vcap.maxheight );
    printfTextWindow("Minimum width of image: %d\n" , vcap.minwidth );
    printfTextWindow("Minimum height of image: %d\n" , vcap.minheight );

    if( ioctl( devfd , VIDIOCGMBUF , &vm ) == -1 ) {
        printfTextWindow("Error get Memory map.\n");
        return;
    }

    if( ioctl( devfd , VIDIOCGPICT , &vp ) == -1 ) {
        printfTextWindow("Get Picture Info.\n");
        return;
    }

    if( (frame_buf = (unsigned char *)mmap( 0, (size_t)vm.size, PROT_READ | PROT_WRITE , MAP_SHARED , devfd , 0 ) ) == MAP_FAILED ) {
        printfTextWindow("ERROR MMAP.\n");
        return ;
    }

    vmap.width = CAPTURE_IMAGE_WIDTH;
    vmap.height = CAPTURE_IMAGE_HEIGHT;
    vmap.format = VIDEO_PALETTE_RGB24;

    vmap.frame = 0;
    if( ioctl( devfd , VIDIOCMCAPTURE , &vmap ) == -1 ) {
        printfTextWindow("ERROR VIDIOCMCAPTURE.\n");
        return;
    }
    vmap.frame = 1;
    if( ioctl( devfd , VIDIOCMCAPTURE , &vmap ) == -1 ) {
        printfTextWindow("ERROR VIDIOCMCAPTURE.\n");
        return ;
    }
    vmap.frame = 0;
    /* Seteamos formato para la imagen */
//     fmt.fmt.pix.width       = 640;
//     fmt.fmt.pix.height      = 480;
//     fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//     quality = 50;
//     err = ioctl(devfd, VIDIOC_G_FMT, &fmt);
//     if (err)
//     {
//         fprintf(stderr, "1  G_FMT returned error %d\n", errno);
//         return ;
//     }
//     err = ioctl(devfd, VIDIOC_S_FMT, &fmt);
//     if (err)
//     {
//         perror("Primera llamada iotcl(VIDIOC_S_FMT)");
//         fprintf(stderr, "S_FMT returned error %d\n", errno);
//         return ;
//     }

    /* Obtenemos sizeimage para saber el offset de la imagen a leer de la webcam */
//     err = ioctl(devfd, VIDIOC_G_FMT, &fmt);
//     if (err)
//     {
//         perror("Segunda llamada iotcl(VIDIOC_S_FMT)");
//         fprintf(stderr, "2  G_FMT returned error %d\n", errno);
//         return ;
//     }
//
//     data = (char *)malloc(fmt.fmt.pix.sizeimage);
//     if (data == NULL)
//     {
//         fprintf(stderr, "malloc(%d) failed\n", fmt.fmt.pix.sizeimage);
//         return ;
//     }

    threads_running = true;
    pthread_create( &reader, NULL,(void * (*)(void *)) &start_reader_thread,(void*) this );
    printTextWindow( "connected\n" );
    updateStatusBar_();
    addSensor();
}

WebCam::~WebCam(){

    if ( threads_running )
    {
        pthread_cancel( reader );
        pthread_join( reader, NULL );
        pthread_mutex_destroy( &camQMutex );
        close( devfd );
    }
    delete sensorWebCam;
    delete partWebCam;
    printTextWindow( "Camera disconnected\n" );
}


void WebCam::control()
{
    int c, ct, cs;
    int done = 0, done_track_select = 0;
    int msgData[ 1 ];

    //msgData[ 0 ] = powerOn;

    messageDrawWindow( EnMessageType_WEBCAM_CONTROL, EnMessageErrType_NONE, msgData );
    do
    {
        c = getch();
        switch( c )
        {
            case 'd':
                messageHide();
                delete this;
                return;
                break;
            case 'r':
                refreshScreen();
                break;

            case 'x':
            case 'Q':
            case KEY_ESC:
                done = true;
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
                messageMovePanelKey( c );
                break;
        }
    } while ( !done );
    messageHide();
}

void webCamConnect(WebCam *a)
{
    unsigned const int uPORT_ECHO_LINE = 4;

    // get the connection port from the user
    string strPort = messageGetString( EnMessageType_WEBCAM_PORT, uPORT_ECHO_LINE );

    new WebCam(&a, strPort );
}

