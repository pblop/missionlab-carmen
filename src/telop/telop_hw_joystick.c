
/**********************************************************************
 **                                                                  **
 **                            telop_hw_joystick.c                   **
 **                                                                  **
 **  Written by: Michael Langford                                    **
 **                                                                  **
 **  Copyright 2003 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: telop_hw_joystick.c,v 1.2 2009/01/24 02:40:11 zkira Exp $ */

#define _RENTRANT
#include <pthread.h>

#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <linux/joystick.h>
#include "telop_hw_joystick.h"
#include "telop_joystick.h"
#include "status_window.h"
#include "gt_com.h"

#define WAIT_INTERVAL_US 0L
#define MODNUM 2

#define NAME_LENGTH 128
#define STICK_PORT "/dev/input/js0"
#define XMIN  -32767L
#define XMAX  32767L
#define YMIN  -32767L
#define YMAX  32767L

int axial_report_counter=0;

int thread_state=0;

int axis[6];
int button[12];
pthread_t the_thread=0;
int bKeepGoing =0;

void (*axis_callback)(int axis[6]) = 0;

static void *jq_connect_thread(void *unused);

void jq_wait_us(unsigned long time_in_us);


long jq_get_xmax(void)
{
	return XMAX;
}

long jq_get_xmin(void)
{
	return XMIN;
}

long jq_get_ymax(void)
{
	return YMAX;
}

long jq_get_ymin(void)
{
	return YMIN;
}

int jq_connect(void)
{
	return pthread_create(&the_thread,0,jq_connect_thread,0);
}


void *jq_get_axis_callback(void)
{
	return (void *)axis_callback;
}


int jq_register_axis_callback(void (*cb)(int axis[6]))
{
	int bGo;

	bGo = (0==axis_callback);

	axis_callback = cb;
	if(cb)
	{
		bKeepGoing = 1;
		if(bGo)jq_connect();
	}
	else
	{
		jq_kill_js_driver();
	}

	while (thread_state == 0) {
	}
	return (thread_state==2);

}

static void *jq_connect_thread(void *unused)
{
	int argc = 1;
	char value[100];
	int fd;
	unsigned char axes = 2;
	unsigned char buttons = 2;
	int version = 0x000800;
	char name[NAME_LENGTH] = "Unknown";
	int joy_buttons = 0;

	if ((fd = open(STICK_PORT, O_RDONLY)) < 0)
	{
		perror("libjoyQ");
		axis_callback = 0;
		thread_state = 1; /* unsuccessful joystick link */
		return NULL;
	}

	ioctl(fd, JSIOCGVERSION, &version);
	ioctl(fd, JSIOCGAXES, &axes);
	ioctl(fd, JSIOCGBUTTONS, &buttons);
	ioctl(fd, JSIOCGNAME(NAME_LENGTH), name);

	if (argc < 3 || !strcmp("--normal", STICK_PORT)) {

		struct js_event js;


		while (bKeepGoing) {
			jq_wait_us(WAIT_INTERVAL_US);
			if(!bKeepGoing)
			{
				thread_state = 2;
				return NULL;
			}

			if (read(fd, &js, sizeof(struct js_event)) != sizeof(struct js_event)) {
				perror("\nlibjoyQ: error reading");
				axis_callback = 0;
				thread_state = 1; /* unsuccessful joystick link */
				return NULL;

			}
			thread_state = 2; /* successful joystick link */

			switch(js.type & ~JS_EVENT_INIT) {
			case JS_EVENT_BUTTON:

	        button[js.number] = js.value;
			switch(js.number)
            {
            case 0:
            	if(js.value == 1)
					arm_deadman();
            	else
            		disarm_deadman();
            	break;
            case 1:
            	if(js.value == 0)
            		increment_avel_val(10);
            	break;
            case 2:
            	if(js.value == 0)
            		increment_avel_val(-10);
            	break;
			case 4:
			case 6:
				sprintf(value, "%d", button[6]-button[4]);
				gt_update(BROADCAST_ROBOTID, "slider_1", value);
				break;
			case 5:
			case 7:
				sprintf(value, "%d", button[7]-button[5]);
				gt_update(BROADCAST_ROBOTID, "slider_2", value);
				break;
            }

			joy_buttons = 0;
			for(int i=0; i<12; i++)
				joy_buttons += button[i] * (1<<i);
			sprintf(value, "%d", joy_buttons);
			gt_update(BROADCAST_ROBOTID, "joy_buttons", value);
            break;

			case JS_EVENT_AXIS:
				axis[js.number] = js.value;
				if(axis_callback)
				{
					switch(js.number)
					{
						case 0:
						case 1:
							axis_callback(axis);
							break;
						case 2:
							sprintf(value, "%d", -js.value);
							gt_update(BROADCAST_ROBOTID, "joystick2_y", value);
							break;
						case 3:
							sprintf(value, "%d", js.value);
							gt_update(BROADCAST_ROBOTID, "joystick2_x", value);
							break;
					}
				}
				else
				{
				  thread_state = 0; /* uninitialize joystick */
				  return NULL;
				}
				break;
			default:
             break;
			}
		}

	}

    return NULL;
}

int jq_get_info(void)
{
	return axis[0];
}

int jq_kill_js_driver(void)
{
	bKeepGoing = 0;
	axis_callback = 0;
	thread_state = 0; /* uninitialize joystick */
	/* wait for the thread to die */
	return pthread_join(the_thread,0);


}



#include <sys/time.h>     /* struct timeval definition           */
#include <unistd.h>       /* declaration of gettimeofday()       */

void jq_wait_us(unsigned long time_in_us)
{
	struct timeval  now;            /* time when we started waiting        */
	struct timespec timeout;        /* timeout value for the wait function */
	pthread_mutex_t a_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t request_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t got_request = PTHREAD_COND_INITIALIZER;
	int             done;           /* are we done waiting?                */

	/* first, lock the mutex */
	int rc = pthread_mutex_lock(&a_mutex);
	if (rc) { /* an error has occurred */
	    perror("pthread_mutex_lock");
	    pthread_exit(NULL);
	}
	/* mutex is now locked */

	/* get current time */
	if(0!=gettimeofday(&now,0)) printf("Failed time aquire!");

	/* prepare timeout value */
	timeout.tv_sec = now.tv_sec;
	timeout.tv_nsec = (now.tv_usec + time_in_us) * 1000L;

	/* wait on the condition variable. */
	/* we use a loop, since a Unix signal might stop the wait before the timeout */
	done = 0;
	while (!done)
	{
	    /* remember that pthread_cond_timedwait() unlocks the mutex on entrance */
	    rc = pthread_cond_timedwait(&got_request, &request_mutex, &timeout);
	    switch(rc)
	    {
	        case ETIMEDOUT: /* our time is up */
        	    done = 1;
	            break;
	        default:        /* some error occurred (e.g. we got a Unix signal) */
	            break;      /* break this switch, but re-do the while loop.   */
	    }

	}

	/* finally, unlock the mutex */
	pthread_mutex_unlock(&request_mutex);
}

/**********************************************************************
 * $Log: telop_hw_joystick.c,v $
 * Revision 1.2  2009/01/24 02:40:11  zkira
 * Aerial telop
 *
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/05/02 04:20:20  endo
 * TrackTask improved for the experiment.
 *
 * Revision 1.1.1.1  2005/02/06 23:00:21  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.1  2003/04/02 21:24:55  zkira
 * Initial revision
 *
 **********************************************************************/
