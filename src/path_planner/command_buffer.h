/**********************************************************************
 **                                                                  **
 **                               command_buffer.h                          **
 **                                                                  **
 **                                                                  **
 **                                                                  **
 **                                                                  **
 **  Written by:                                                     **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: command_buffer.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

/*--------------------------------------------------------------------------

		COMMAND_BUFFER.H

		Sets up for command to robot
	 command buffer - assumes clipstruct.h already included

-------------------------------------------------------------------------*/


char            command_filename[50] = "command.dat";
struct ascid    command_gsddesc;

struct
{
   char            message[80];	/* message to robot */
   char            response[350];	/* response from robot */

   /* flags for interprocess coordination */
   int             message_read;
   int             message_deposited;
   int             response_deposited;
   int             response_read;

   int             read_ultrasonics;	/* Boolean */
   int             read_encoders;
}               command_buffer;



///////////////////////////////////////////////////////////////////////
// $Log: command_buffer.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:24  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:51  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:38:00  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 23:00:23  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.1  2000/03/22 04:39:48  saho
// Initial revision
//
///////////////////////////////////////////////////////////////////////

