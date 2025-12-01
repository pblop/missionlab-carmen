/**********************************************************************
 **                                                                  **
 **                           version.cc                             **
 **                                                                  **
 ** Compile the version and compile time strings separately so they  **
 ** can be updated efficiently everytime console is compiled.        **
 **                                                                  **
 **  By: Jonathan M. Cameron                                         **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: version.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#include <stdio.h>
#include <time.h>

char *version_str = "7.0.00";

char compile_time_str[40];


char *compile_time()
{
   time_t timeSec = DATE;

   sprintf(compile_time_str, "%s", ctime((time_t *)&timeSec));
   return compile_time_str;
}



///////////////////////////////////////////////////////////////////////
// $Log: version.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.2  2006/07/12 09:06:05  endo
// Preparing for MissionLab 7.0 release.
//
// Revision 1.1.1.1  2005/02/06 22:59:34  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.54  2003/06/19 20:38:43  endo
// 6.0.01
//
// Revision 1.53  2003/04/06 20:00:08  endo
// 6.0.00
//
// Revision 1.52  2003/04/06 10:45:09  endo
// 5.0.09
//
// Revision 1.51  2003/04/03 23:07:11  endo
// 5.0.08
//
// Revision 1.50  2002/11/03 20:43:45  endo
// 5.0.07
//
// Revision 1.49  2002/09/26 14:37:07  endo
// 5.0.06
//
// Revision 1.48  2002/08/22 14:43:10  endo
// 5.0.05
//
// Revision 1.47  2002/07/18 17:17:38  endo
// 5.0.04
//
// Revision 1.46  2002/07/04 23:30:43  endo
// 5.0.03
//
// Revision 1.45  2002/04/01 22:01:46  endo
// 5.0.02
//
// Revision 1.44  2002/01/31 10:56:51  endo
// 5.0.00 -> 5.0.01
//
// Revision 1.43  2002/01/12 23:23:16  endo
// Mission Expert functionality added.
//
// Revision 1.42  2001/12/23 21:55:10  endo
// 5.0.00
//
// Revision 1.41  2001/11/26 19:19:20  endo
// 4.1.00
//
// Revision 1.40  2001/06/15 22:52:35  endo
// 4.0.09
//
// Revision 1.39  2001/03/09 05:28:44  endo
// 4.0.08
//
// Revision 1.38  2001/01/10 19:43:08  endo
// 4.0.07
//
// Revision 1.37  2000/12/02 23:27:13  endo
// 4.0.06
//
// Revision 1.36  2000/10/16 21:59:30  endo
// 4.0.05
//
// Revision 1.35  2000/09/19 10:20:11  endo
// 4.0.04
//
// Revision 1.34  2000/08/15 21:02:11  endo
// 4.0.03
//
// Revision 1.33  2000/08/12 21:56:21  endo
// 4.0.02
//
// Revision 1.32  2000/08/01 19:30:45  endo
// 4.0.00 -> 4.0.01
//
// Revision 1.31  2000/07/02 06:04:27  endo
// 4.0.00
//
// Revision 1.30  2000/06/20 01:00:39  endo
// 3.1.05 -> 3.1.06
//
// Revision 1.29  2000/05/30 20:01:06  endo
// 3.1.04 -> 3.1.05
//
// Revision 1.28  2000/04/25 08:53:53  endo
// 3.1.03 -> 3.1.04
//
// Revision 1.27  2000/03/30 20:34:50  endo
// 3.1.02 -> 3.1.03
//
// Revision 1.26  2000/03/02 21:18:19  endo
// 3.1.01 -> 3.1.02
//
// Revision 1.25  2000/02/18 23:27:19  endo
// 3.1 -> 3.1.01
//
// Revision 1.24  1999/12/18 00:05:42  endo
// 3.0 -> 3.1
//
// Revision 1.23  1999/07/22 19:43:37  endo
// v1.0c -> v3.0 (Sync with mlab)
//
// Revision 1.22  1997/02/14 16:53:35  zchen
// *** empty log message ***
//
// Revision 1.21  1996/10/04  20:58:17  doug
// changes to get to version 1.0c
//
// Revision 1.21  1996/09/25 20:00:41  doug
// 1.0c adds the ability to write library files
//
// Revision 1.20  1996/06/02 16:25:30  doug
// added ability to group nodes, got SAUSAGES code generator working
//
// Revision 1.19  1996/03/20  15:34:39  doug
// 1.0a fixes privilege problems
//
// Revision 1.18  1996/03/13  03:02:29  doug
// version 1.0 is what started the usability testing
//
// Revision 1.17  1996/02/15  22:50:26  doug
// 0.9b continues development of the new interface
//
// Revision 1.16  1996/02/15  19:28:57  doug
// 0.9a fixes the convert constant bug
//
// Revision 1.15  1996/02/09  13:59:51  doug
// 0.9 uses the new button style user interface
//
// Revision 1.14  1996/01/19  20:55:22  doug
// Version 0.8f will fix the split code
//
// Revision 1.13  1996/01/17  18:49:59  doug
// Now supports using multiple CNL and CDL directories and libraries
// Fixed bind/unbind architecture so writes and reloads file to correctly
//   relink the instance records to the correct definition records.
//
// Revision 1.12  1996/01/10  19:33:11  doug
// version 0.8d fixed a problem with binding points not showing up
//
// Revision 1.11  1995/12/14  21:53:29  doug
// *** empty log message ***
//
// Revision 1.10  1995/12/05  15:47:09  doug
// Fixed new so it really works now
//
// Revision 1.9  1995/11/27  16:47:55  doug
// 8a fixes so output arrow isn't shown on void objects
// and also fixes verify so allows multiple void objects (and robots)
// on the same page
//
// Revision 1.8  1995/11/18  23:06:41  doug
// have fixed problems with splitting nodes
//
// Revision 1.7  1995/11/12  21:14:09  doug
// version 0.7 has the type checking fixed
// You can delete states
// the split code handles first children of states
// verifies the names that are entered in text fields
//
// Revision 1.6  1995/08/17  22:33:11  doug
// bump to 0.6
//
// Revision 1.5  1995/06/29  18:20:39  jmc
// Improved header and RCS log string.
//
// Revision 1.4  1995/03/07  14:54:06  doug
// *** empty log message ***
//
// Revision 1.3  1995/01/11  15:17:59  doug
// *** empty log message ***
//
// Revision 1.3  1995/01/11  15:17:59  doug
// *** empty log message ***
//
// Revision 1.2  1994/12/07  18:47:26  doug
// *** empty log message ***
//
// Revision 1.2  1994/12/07  18:47:26  doug
// *** empty log message ***
//
// Revision 1.3  1994/11/18  15:03:20  jmc
// Changed the version number to 0.5.
//
// Revision 1.2  1994/09/09  21:40:16  jmc
// Converted interface to compile time string to a function.
//
// Revision 1.1  1994/09/09  19:31:19  jmc
// Initial revision
////////////////////////////////////////////////////////////////////////
