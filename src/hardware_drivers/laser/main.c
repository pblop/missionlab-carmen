/**********************************************************************
 **                                                                  **
 **                               main.c                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  Jonathan Diaz  and                                 **
 **               Alexander Stoytchev                                **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: main.c,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

#include "ezxwin.h"
#include "data_structures.h"
#include "laser_protocol.h"
#include "laser_io.h"

extern void initAlg();
extern void mainAlg();
extern FILE *fp;
extern BOOL gbDoWrite;
extern int giPlayRate;
extern int giPlayMode;

// EZ-XWindows entry point
void ezXWinInit(void)
{  
  startLServer(SERV_HSERVER_PORT, SERV_ROBOT_EXEC_PORT);
}


void ezHandleDrawRequest(Display *display, int screen_num, Window win, 
			 GC gc, unsigned int width, unsigned int height)
{
}

void ezHandleButtonPress(Display *display, int screen_num, Window win, 
			 GC gc, unsigned int width, unsigned int height,
			 int buttonX, int buttonY)
{
  gbDoWrite ^= TRUE;
}

void ezHandleKeyPress(Display *display, int screen_num, Window win, 
		      GC gc, unsigned int width, unsigned int height, char c)
{
  if(REAL_LASER_READ)
	return;

  switch(c)
	{
	  //Forward
      case 'f':
	  case 'F':
		if(giPlayMode != PLAY_FORWARD){
		  giPlayMode= PLAY_FORWARD;
		  giPlayRate= 1;
		} 
		else
		  giPlayRate*=2;
		break;

	  // Reverse
	  case 'r':
	  case 'R':
		if(giPlayMode != PLAY_REVERSE){
		  giPlayMode= PLAY_REVERSE;
		  giPlayRate= 1;
		} 
		else
		  giPlayRate*= 2;
		break;
		
	case 'd':
	case 'D':	
	     giPlayMode=PLAY_SINGLE_FORWARD_FRAME;
		 break;

	case 'e':
	case 'E':	
	     giPlayMode=PLAY_SINGLE_REVERSE_FRAME;
		 break;

	  // Stop
	  case ' ':
		giPlayMode= PLAY_PAUSE;
		break;

      // Start from the beginning of the file
	  case 'b':
	  case 'B':
		giPlayMode= PLAY_FROM_START;
        giPlayRate=1;
		break;

	default:
	   break;
	}

   return;
}

void ezHandleIdleEvent(Display *display, int screen_num, Window win, 
		       GC gc, unsigned int width, unsigned int height)
{
  // Algorithm goes here

  mainAlg();
  return;
}





/**********************************************************************
 * $Log: main.c,v $
 * Revision 1.1.1.1  2006/07/12 13:37:57  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 22:59:39  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.3  2000/09/19 01:17:23  jdiaz
 * playback added
 *
 * Revision 1.2  2000/09/19 01:02:53  jdiaz
 * rockville modifications
 *
 * Revision 1.1  2000/08/12 18:45:31  saho
 * Initial revision
 *
 **********************************************************************/

