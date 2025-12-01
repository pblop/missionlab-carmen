/**********************************************************************
 **                                                                  **
 **  r2cdefs.x                                                       **
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

/* $Id: r2c_defs.x,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: r2c_defs.x,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:22:47  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.13  2004/05/11 19:34:36  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.12  1998/11/08 04:10:04  doug
* *** empty log message ***
*
* Revision 1.11  1998/05/12 15:31:27  doug
* added locking
*
* Revision 1.10  1998/05/12 15:10:26  doug
* Add locking to the mission plan manip functions
*
* Revision 1.9  1998/05/04 20:53:29  doug
* *** empty log message ***
*
* Revision 1.8  1998/02/10 15:38:03  doug
* final win32 fix
*
* Revision 1.7  1998/02/10 14:41:33  doug
* *** empty log message ***
*
* Revision 1.6  1998/02/10 09:12:50  doug
* *** empty log message ***
*
* Revision 1.5  1997/12/17 15:50:09  doug
* removed includes of mic.h and Vector.h which were no longer used
*
* Revision 1.4  1997/12/15 11:46:57  doug
*  added pause, singlestep and resume
*
* Revision 1.3  1997/12/13 23:20:56  doug
* *** empty log message ***
*
* Revision 1.2  1997/12/13 21:00:08  doug
* *** empty log message ***
*
* Revision 1.1  1997/12/11 15:55:48  doug
* Initial revision
*
**********************************************************************/

#define PRG_VERSION 	1
#define PRG_NUMBER	0x5E435554	/* "SCON" */

/************************************************************************/
/* Stuff some info in to the various files we generate */

#ifdef RPC_HDR
%#include "ipc_defs.h"
%#ifdef WIN32
%extern bool gotR2Cmsg;
%#else
%#define PASCAL 
%#endif
%void PASCAL r2cprogram_1(struct svc_req *rqstp, register SVCXPRT *transp);
#endif

#ifdef RPC_SVC
%#include "robot2con.h"
%using namespace sara;
#endif

/************************************************************************/
/* 
* Procedures are numbered from 1 to ...
* Convert procedure names to upper case, since rpcgen makes a define
* with this name.  Also drop the asterix in the return type. They 
* look like they are return by value here, instead of pointer to val.
*/

program R2Cprogram { 	/* defined as program number */
   version R2Cversion {	/* defined as program version */

     PING_INFO PING_ROBOT() = 1;

     ROBOT_STATUS GET_ROBOT_STATUS() = 2;

     int ATTACH_TO_ROBOT(int seqnum) = 3;

     int DETACH_FROM_ROBOT(int robot_id) = 4;

     int ROBOT_PAUSE() = 5;
     int ROBOT_SINGLE_STEP() = 6;
     int ROBOT_RESUME() = 7;
     string GET_FSA() = 8;
     string LIST_PRIMS() = 9;
     string GET_STATE_INFO() = 10;
     int PUT_FSA(string newfsa,int key) = 11;
     int LOCK_FSA(int seqnum) = 12;
     int UNLOCK_FSA(int key) = 13;

   } = PRG_VERSION;
} = PRG_NUMBER;

/**********************************************************************/
