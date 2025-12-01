/**********************************************************************
 **                                                                  **
 **  ipcdefs.x                                                       **
 **                                                                  **
 **  declarations for rpcgen                                         **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: ipc_defs.x,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: ipc_defs.x,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:55  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:25:48  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.20  2004/10/30 00:29:55  doug
* Sending out to MARS2020
*
* Revision 1.19  2004/10/27 19:58:23  doug
* Added new BUD robot type, including steered robot simulator in simserver
*
* Revision 1.18  2004/08/18 22:52:02  doug
* got ugv demo filmed
*
* Revision 1.17  2004/03/26 22:15:04  doug
* for win32, just build the header and skip compilation.
*
* Revision 1.16  2004/03/08 14:52:18  doug
* cross compiles on visual C++
*
* Revision 1.15  2004/03/01 23:37:44  doug
* working on real overlay
*
* Revision 1.14  2003/11/26 15:21:38  doug
* back from GaTech
*
* Revision 1.13  2003/07/07 19:40:00  doug
* adding support for robot terminating
*
* Revision 1.12  2003/06/16 19:51:09  doug
* Adding Tomahawk
*
* Revision 1.11  2002/11/14 13:46:10  doug
* revised ROBOT_TYPES enum
*
* Revision 1.10  2002/10/07 18:17:49  doug
* added Lewk robot type and roll and pitch
*
* Revision 1.9  2002/10/02 18:25:09  doug
* *** empty log message ***
*
* Revision 1.8  1998/06/18 21:14:52  doug
* *** empty log message ***
*
* Revision 1.7  1998/05/12 17:38:01  doug
* added ping_info struct
*
* Revision 1.6  1998/04/07 22:58:23  doug
* renamed flags again
*
* Revision 1.5  1998/04/07 22:16:37  doug
* renamed constants
*
* Revision 1.4  1998/04/07 22:07:57  doug
* renamed STATUS_FLAGS to ROBOT_STATUS_FLAGS
*
* Revision 1.3  1998/04/07 22:04:41  doug
* added status flag field to ROBOT_STATUS record
*
* Revision 1.2  1998/02/18 15:22:55  doug
* *** empty log message ***
*
* Revision 1.1  1998/02/10 08:53:19  doug
* Initial revision
*
*
**********************************************************************/

#define PRG_VERSION 	1

/************************************************************************/
/* Stuff some info in to the various files we generate */

#ifdef RPC_HDR
%/* Is a mission being executed? */
%#define ROBOT_IS_ACTIVE_FLAG (1<<0)
%
%/* Is the robot currently paused? */
%#define ROBOT_IS_PAUSED_FLAG (1<<1)
%
%/* Is the FSA currently locked? */
%#define ROBOT_FSA_IS_LOCKED_FLAG (1<<2)
%
%/* Is this the final message and the robot is terminating? */
%#define ROBOT_IS_TERMINATING_FLAG (1<<3)
%
%
%#if defined(WIN32)
% typedef unsigned long u_long;
% typedef unsigned int u_int;
% typedef struct XDR XDR;
% typedef bool bool_t;
%#endif
#endif

#ifdef RPC_SVC
#endif

/************************************************************************/
/* Define the common types passed around in the rpc packets. */

enum ROBOT_TYPES {RT_HOLONOMIC, RT_LEWK, RT_F16, RT_TOMAHAWK, RT_ROTOCRAFT, RT_ATRV, RT_UGV, RT_BUD};
struct PING_INFO
{     
   ROBOT_TYPES robot_type;
   char        robot_name[80];
};    

struct VECTOR 
{
   double x;
   double y;
   double z;
};

struct ROBOT_XYT 
{
   double x;
   double y;
   double z;
   double r; /* roll */
   double p; /* pitch */
   double t; /* yaw */
};

typedef unsigned long ROBOT_STATUS_FLAGS;

struct ROBOT_STATUS
{     
   double x;
   double y;
   double z;
   double r; /* roll */
   double p; /* pitch */
   double t; /* yaw */
   ROBOT_STATUS_FLAGS status;
};    

enum OBJECT_TYPES {DISK_OBJECT, POLYGON_OBJECT, CONTAINER_OBJECT, MARKER_OBJECT};
struct DISK_READING
{
	unsigned int id; 
	string color<>;
	double Vx; 
	double Vy; 
	double Vz; 
	double r; 
};
struct POLYGON_READING
{
	unsigned int   id; 
	string color<>;
        VECTOR verticies<>;
};
struct CONTAINER_READING
{
	unsigned int   id; 
	string color<>;
	double Vx1;
	double Vy1;
	double Vz1;
	double Vx2;
	double Vy2;
	double Vz2;
};
struct MARKER_READING
{
	unsigned int   id; 
	string color<>;
	string inscription<>;
	double Vx1;
	double Vy1;
	double Vz1;
	double Vx2;
	double Vy2;
	double Vz2;
	double Vx3;
	double Vy3;
	double Vz3;
};

union SENSOR_READING switch (OBJECT_TYPES type)
{
   case DISK_OBJECT:
		DISK_READING disk_reading; 

   case POLYGON_OBJECT:
		POLYGON_READING polygon_reading; 

   case CONTAINER_OBJECT:
		CONTAINER_READING container_reading; 

   case MARKER_OBJECT:
		MARKER_READING marker_reading; 
};

typedef SENSOR_READING SENSOR_READINGS<>;

/**********************************************************************/

