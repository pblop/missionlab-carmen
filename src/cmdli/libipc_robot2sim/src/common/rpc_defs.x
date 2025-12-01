/**********************************************************************
 **                                                                  **
 **  rpcdefs.x                                                       **
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

/* $Id: rpc_defs.x,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: rpc_defs.x,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:24:20  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.11  2004/05/11 19:34:35  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.10  2003/06/30 21:11:04  doug
* Support altitiude
*
* Revision 1.9  1998/11/08 04:08:19  doug
* fixed comment
*
* Revision 1.8  1998/06/18 20:13:09  doug
* *** empty log message ***
*
* Revision 1.7  1998/02/10 15:48:17  doug
* Added code for Win32
*
* Revision 1.6  1998/02/10 09:15:27  doug
* moved data types to libipc_support
*
* Revision 1.5  1997/12/31 02:59:53  doug
* fixing for win32
*
* Revision 1.4  1997/12/12 00:59:25  doug
* *** empty log message ***
*
* Revision 1.3  1997/12/11 12:00:24  doug
* working
*
* Revision 1.2  1997/12/10 15:52:03  doug
* *** empty log message ***
*
* Revision 1.1  1997/11/26 13:05:57  doug
* Initial revision
*
**********************************************************************/

#define PRG_VERSION 	1
#define PRG_NUMBER	0x5E5E4E53	/* "SSIM" */

/************************************************************************/
/* Stuff some info in to the various files we generate */

#ifdef RPC_HDR
%#include "ipc_defs.h"
%#ifdef WIN32
%extern bool gotRPCmsg;
%#else
%#define PASCAL
%#endif
%void PASCAL rpcprogram_1(struct svc_req *rqstp, register SVCXPRT *transp);
#endif

#ifdef RPC_SVC
%#include "robot2sim.h"
%using namespace sara;
#endif

/************************************************************************/
/* 
* Procedures are numbered from 1 to ...
* Convert procedure names to upper case, since rpcgen makes a define
* with this name.  Also drop the asterix in the return type. They 
* look like they are return by value here, instead of pointer to val.
*/

program RPCprogram { 	/* defined as program number */
   version RPCversion {	/* defined as program version */

     int ATTACH_ROBOT(int mykey, ROBOT_TYPES robot_type, 
		double startX, double startY, double startZ, double startT) = 1;

     int DETACH_ROBOT(int robot_id) = 2;

     int MOVE_ROBOT(int robot_id, double steer, double Hmps, double Vmps) = 3;

     ROBOT_XYT GET_XYZT(int robot_id) = 4;

     SENSOR_READINGS GET_SENSOR_READINGS(int robot_id, double range) = 5;

   } = PRG_VERSION;
 
} = PRG_NUMBER;

/**********************************************************************/

