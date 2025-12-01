#ifndef MsgTypes_H
#define MsgTypes_H
/**********************************************************************
 **                                                                  **
 **  MsgTypes.h                                                      **
 **                                                                  **
 **  Define the MsgType numbers used to identify packets             **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: MsgTypes.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: MsgTypes.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:17  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:55  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:31:54  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.25  2004/11/07 03:02:36  doug
// updated budmon
//
// Revision 1.24  2004/11/03 15:56:46  doug
// working on firewire video driver
//
// Revision 1.23  2004/09/16 15:01:02  doug
// added the rest of the bud message types
//
// Revision 1.22  2004/09/01 19:03:32  doug
// added BudGet
//
// Revision 1.21  2004/08/31 20:08:51  doug
// cleanup debug messages
//
// Revision 1.20  2004/08/31 20:08:26  doug
// cleanup debug messages
//
// Revision 1.19  2004/08/20 21:45:39  doug
// working on bud_monitor
//
// Revision 1.18  2004/08/20 20:46:46  doug
// wrap to allow compiling in C
//
// Revision 1.17  2004/08/20 20:46:20  doug
// wrap to allow compiling in C
//
// Revision 1.16  2004/08/20 18:28:05  doug
// change to a struct so can use in C programs too.
//
// Revision 1.15  2004/08/20 17:37:41  doug
// added bud packets
//
// Revision 1.14  2004/08/20 17:29:42  doug
// added bud packets
//
// Revision 1.13  2004/05/11 19:34:34  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.12  2004/04/27 16:19:32  doug
// added support for multicast as unicast
//
// Revision 1.11  2004/04/20 18:35:17  doug
// added CMDLCOMMAND
//
// Revision 1.10  2004/02/28 16:35:22  doug
// getting cmdli to work in sara
//
// Revision 1.9  2003/11/26 22:50:41  doug
// working on state machines
//
// Revision 1.8  2003/11/26 15:21:37  doug
// back from GaTech
//
// Revision 1.7  2003/03/21 15:55:33  doug
// moved libccl_code to the sara namespace
//
// Revision 1.6  2003/02/26 14:55:48  doug
// controller now runs with objcomms
//
// Revision 1.5  2003/02/17 23:00:13  doug
// assigns task lead
//
// Revision 1.4  2003/02/14 21:47:13  doug
// links again
//
// Revision 1.3  2003/02/13 22:13:18  doug
// Added AckLead
//
// Revision 1.2  2003/02/11 20:40:49  doug
// libdtasker compiles
//
// Revision 1.1  2003/01/31 19:30:13  doug
// test1 builds and links!
//
///////////////////////////////////////////////////////////////////////

#if defined(__cplusplus) || defined(c_plusplus)
namespace sara
{
#endif
// *******************************************************************
struct MsgTypes
{
   enum MsgType_Numbers
   {
	INVALID_MSG_TYPE=0, 

	RAWMSG,
	SOLICITATION, 
	NEWTASK, 
	BID, 
	ASSIGNTASK, 
	ACKACCEPT, 
	ACCEPT, 
	CLAIMLEAD, 
	ACKBID, 
	TASKSTATUS, 
	PERIODIC_STATUS, 
	ACKLEAD,
	ACCEPTLEAD,
	ACCEPTTASK,
	ACTIONSTATUS, 
	SOLICITPEER, 
	ACKPEER, 
	CLAIMPEER, 
	PEERACCEPT, 
        ACKACTIONSTATUS,
        REQACTIONSTATUS,
        CMDLSTATUS,
	CMDLCOMMAND,
	CMDLCOMMANDACK,
	CMDLDOWNLOAD,
	CMDLDOWNLOADACK,
	BUDCOMMAND,
	BUDGET,
	BUDSET,
	BUDACK,
	BUDPING,
	BUDLOGMSG,
	BUD_NOTUSED_,
	BUDPOSITION,

	// marker for the number of message types
	NUM_MESSAGE_TYPES
   } MsgType_Numbers_enum;
};

/*********************************************************************/
#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
