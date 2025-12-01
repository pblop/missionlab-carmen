/**********************************************************************
 **                                                                  **
 **                           version.c                              **
 **                                                                  **
 ** Compile the version and compile time strings separately so they  **
 ** can be updated efficiently every time console is compiled.       **
 **                                                                  **
 **                                                                  **
 **  Written by:  Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: version.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#include <stdio.h>


char *version_str = "7.0.00";

char compile_time_str[40];


char *compile_time()
{
   sprintf(compile_time_str, "%s", DATE);
   return compile_time_str;
}



/**********************************************************************
 * $Log: version.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/07/12 09:06:41  endo
 * Preparing for MissionLab 7.0 release.
 *
 * Revision 1.1.1.1  2005/02/06 23:00:11  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.44  2003/06/19 20:37:41  endo
 * 6.0.01
 *
 * Revision 1.43  2003/04/06 13:39:51  endo
 * 6.0.00
 *
 * Revision 1.42  2003/04/06 10:44:13  endo
 * 5.0.09
 *
 * Revision 1.41  2003/04/03 23:04:44  endo
 * 5.0.08
 *
 * Revision 1.40  2002/11/03 20:42:43  endo
 * 5.0.07
 *
 * Revision 1.39  2002/09/26 14:35:59  endo
 * 5.0.06
 *
 * Revision 1.38  2002/08/22 14:41:48  endo
 * 5.0.05
 *
 * Revision 1.37  2002/07/18 17:16:16  endo
 * 5.0.04
 *
 * Revision 1.36  2002/07/04 23:29:02  endo
 * 5.0.03
 *
 * Revision 1.35  2002/04/01 21:59:47  endo
 * 5.0.02
 *
 * Revision 1.34  2002/01/31 10:58:26  endo
 * 5.0.01
 *
 * Revision 1.33  2001/12/23 21:55:10  endo
 * 5.0.00
 *
 * Revision 1.32  2001/11/26 19:19:20  endo
 * 4.1.00
 *
 * Revision 1.31  2001/06/15 22:53:07  endo
 * 4.0.09
 *
 * Revision 1.30  2001/03/09 05:27:59  endo
 * 4.0.08
 *
 * Revision 1.29  2001/01/10 19:43:48  endo
 * 4.0.07
 *
 * Revision 1.28  2000/12/02 23:26:22  endo
 * 4.0.06
 *
 * Revision 1.27  2000/10/16 21:58:47  endo
 * 4.0.05
 *
 * Revision 1.26  2000/09/19 10:21:19  endo
 * 4.0.04
 *
 * Revision 1.25  2000/08/15 21:04:32  endo
 * 4.0.03
 *
 * Revision 1.24  2000/08/12 21:56:47  endo
 * 4.0.02
 *
 * Revision 1.23  2000/08/01 19:29:37  endo
 * 4.0.00 -> 4.0.01
 *
 * Revision 1.22  2000/07/02 06:03:20  endo
 * 4.0.00
 *
 * Revision 1.21  2000/06/20 01:02:22  endo
 * 3.1.05 -> 3.1.06
 *
 * Revision 1.20  2000/05/30 19:59:55  endo
 * 3.1.04 -> 3.1.05
 *
 * Revision 1.19  2000/04/25 08:52:46  endo
 * 3.1.03 -> 3.1.04
 *
 * Revision 1.18  2000/03/30 20:33:18  endo
 * 3.1.02 -> 3.1.03
 *
 * Revision 1.17  2000/03/02 21:16:33  endo
 * 3.1.01 --> 3.1.02
 *
 * Revision 1.16  2000/02/18 23:26:17  endo
 * 3.1 -> 3.1.01
 *
 * Revision 1.15  1999/12/18 00:03:39  endo
 * 3.0 -> 3.1
 *
 * Revision 1.14  1999/07/22 19:41:34  endo
 * v2.0 -> v3.0
 *
 * Revision 1.13  1997/02/12 05:42:50  zchen
 * *** empty log message ***
 *
 * Revision 1.12  1997/02/07  16:41:05  tucker
 * now 2.0
 * q
 *
 * Revision 1.11  1996/03/13  03:03:12  doug
 * version 1.0 is what started usability testing
 *
 * Revision 1.10  1995/11/12  22:07:44  doug
 * Object handling has been cleaned so there is a single detect_object
 * call and a classification type returned with the object list.
 *
 * Revision 1.9  1995/07/07  15:58:58  jmc
 * Update to version 0.85 (the release version).
 *
 * Revision 1.8  1995/05/08  20:26:39  jmc
 * Increased the version number to 0.8.
 *
 * Revision 1.7  1995/04/20  18:38:09  jmc
 * Corrected spelling.
 *
 * Revision 1.6  1995/04/06  19:07:42  jmc
 * Increased version number to 0.7.
 *
 * Revision 1.5  1995/04/03  20:42:20  jmc
 * Added copyright notice.   Updated version to 0.65.
 *
 * Revision 1.4  1995/03/09  20:53:49  jmc
 * Updated to version 0.6.
 *
 * Revision 1.3  1994/11/18  15:03:20  jmc
 * Changed the version number to 0.5.
 *
 * Revision 1.2  1994/09/09  21:40:16  jmc
 * Converted interface to compile time string to a function.
 *
 * Revision 1.1  1994/09/09  19:31:19  jmc
 * Initial revision
 **********************************************************************/
