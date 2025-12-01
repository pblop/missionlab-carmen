/****************************************************************************
*                                                                           *
*                                                                           *
*                      Doug MacKenzie                                       *
*                                                                           *
* gt_com - manage communication between operator console and robots         *
*                                                                           *
*   Copyright 1995, Georgia Tech Research Corporation                       *
*   Atlanta, Georgia  30332-0415                                            *
*   ALL RIGHTS RESERVED, See file COPYRIGHT for details.                    *
*                                                                           *
****************************************************************************/

/* $Id: gt_com.h,v 1.1.1.1 2008/07/14 16:44:20 endo Exp $ */

#define MAX_PARM_NAME_LEN 32
#define MAX_VALUE_LEN 128

/****************************************************************************
*                                                                           *
* int update(int robotid,char *parm_name,char *value);                      *
*                                                                           *
* robotid:                                                                  *
*    -1  : all robots                                                       *
*    else: a specific robot's id                                            *
*                                                                           *
* parm_name:                                                                *
*    Any text string parameter name.  It is case and white space sensitive. *
*                                                                           *
* value:                                                                    *
*    Any text string.                                                       *
*                                                                           *
* Returns:                                                                  *
*    0: success                                                             *
*   -1: failure                                                             *
*                                                                           *
****************************************************************************/
/*
 * add the parameter to the symbol table if it isn't already there.
 * If it is found then update its value.
 */

// ENDO - gcc 3.4
//int gt_update(int robotid, char *parm_name, char *value);
int gt_update(int robotid, char *parm_name, const char *value);

/****************************************************************************
*                                                                           *
* int delete(int robotid,char *parm_name);                                  *
*                                                                           *
* robotid:                                                                  *
*    -1  : all robots                                                       *
*    else: a specific robot's id                                            *
*                                                                           *
* parm_name:                                                                *
*    Any text string parameter name.  It is case and white space sensitive. *
*                                                                           *
* Returns:                                                                  *
*    0: success                                                             *
*   -1: failure                                                             *
*                                                                           *
****************************************************************************/
/*
 * delete the parameter from the symbol table if it is there.
 */

int gt_delete(int robotid, char *parm_name);

/****************************************************************************
*                                                                           *
* char *inquire(int robotid,char *parm_name);                               *
*                                                                           *
* robotid:                                                                  *
*    -1  : all robots (dealer's choice)                                     *
*    else: a specific robot's id                                            *
*                                                                           *
* parm_name:                                                                *
*    Any text string parameter name.  It is case and white space sensitive. *
*                                                                           *
* Returns:                                                                  *
*    NULL: failure                                                          *
*    else: pointer to the string value                                      *
*                                                                           *
****************************************************************************/

char *gt_inquire(int robotid, char *parm_name);



char *gt_get_value(int robotid, char *parm_name);

/***************************************************************************/
#define BROADCAST_ROBOTID  -1

#define MLAB_CONSOLE       -1
#define THIS_ROBOT          0
/***************************************************************************/
/* well known parameters */

#define JOYSTICK 		"joystick"
#define KEYPRESS 		"keypress"
#define LEFT_MOUSE_CLICK 	"left_mouse_click"
#define COMMAND 		"command"
#define RESPONSE 		"response"



/**********************************************************************
 * $Log: gt_com.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:20  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:48  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:57  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/05/14 06:23:36  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.1.1.1  2005/02/06 22:59:46  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.5  2000/06/13 16:48:57  endo
 * MLAB_CONSOLE, THIS_ROBOT defined.
 *
 * Revision 1.4  1995/03/31 22:03:57  jmc
 * Added a copyright notice.
 *
 * Revision 1.3  1994/10/04  20:17:46  doug
 * *** empty log message ***
 *
 * Revision 1.2  1994/07/14  13:20:08  jmc
 * Added RCS automatic id and log strings
 **********************************************************************/
