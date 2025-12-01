/////////////////////////////////////////////////////////////////////
//     laser_protocol.h
//
//	Description
//
//
//
//	Modification History:
//	Date		  Author                             Remarks
//  ----------------------------------------------------------------
//      05 Jul,2000       Jonathan Diaz/Alexander Stoytchev
//  Copyright 2000, Georgia Tech Research Corporation
//  Atlanta, Georgia  30332-0415
//  ALL RIGHTS RESERVED, See file COPYRIGHT for details.
//////////////////////////////////////////////////////////////////////




#ifndef _PROTOCOL_LASER_SERVER_H_
#define _PROTOCOL_LASER_SERVER_H_

#define SERV_HSERVER_PORT 31400
#define SERV_ROBOT_EXEC_PORT 31420

extern int REAL_LASER_READ;
extern int COLLECT_LASER_DATA_TO_FILE;
extern int COLLECT_LASER_DATA_TO_FILE_MANUAL;

#define LASER_SYNC_CMD                    0x70
#define LASER_GET_HALLWAY_INFO            0x71
#define LASER_GET_DOORWAY_POS             0x72
#define LASER_CLOSE_CHANNEL_CMD           0x73

#define LASER_GET_DOORWAY_RIGHT  0
#define LASER_GET_DOORWAY_LEFT   1
#define LASER_GET_DOORWAY_EITHER 2

typedef struct {
  unsigned char cmd;
} laser_simple_rec_t;

typedef struct {
  unsigned char cmd;
  int direction;
} laser_doorway_request_t;

#define LASER_HALLWAY_REPLY_INITIALIZER {0, 0, 0, 0, 0, 0, 0}
typedef struct {
  unsigned char cmd;
  double width;
  double forwardptx, forwardpty;
  double backwardptx, backwardpty;
  int time_stamp;
} laser_hallway_reply_t;

#define LASER_DOORWAY_REPLY_INITIALIZER {0, 0, 0, 0, 0, 0, 0}
typedef struct {
  unsigned char cmd;
  double pt1x, pt1y;
  double pt2x, pt2y;
  int isdoor;
  int time_stamp;
} laser_doorway_reply_t;


int startLServer(int hserver_port, int robot_exec_port);


typedef unsigned char t_byte;


#endif



