/***********************************************************************

	robot_side_com.h

	Description:
		External references for robot_side_comm.c

	Modification History:
		Date		Author		Remarks
		19 MAY 92	Doug MacKenzie 	Original
		19 MAY 96	Tucker Balch 	Revision


        Copyright 1996, Georgia Tech Research Corporation
        Atlanta, Georgia  30332-0415
        ALL RIGHTS RESERVED, See file COPYRIGHT for details.

*************************************************************************/

#include "ipt/ipt.h"

extern int 	ROBIO_FAULTED;
extern int      TTY_NUM;
extern int      USE_LAWN;
extern int      ECHO_TTY;
extern int      LOG_TTY;
extern int      TIME_OUT;

extern IPCommunicator* communicator;
extern IPConnection* console;
extern sensor_t sensor [22];
extern int num_sensors;
