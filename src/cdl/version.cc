/**********************************************************************
 **                                                                  **
 **                           version.c                              **
 **                                                                  **
 ** Compile the version and compile time strings separately so they  **
 ** can be updated efficiently everytime console is compiled.        **
 **                                                                  **
 ** By: Jonathan M. Cameron                                          **
 **                                                                  **
 ** Copyright 1995, Georgia Tech Research Corporation                **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **********************************************************************/

/* $Id: version.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include <stdio.h>

char *version_str = "6.0.01";

char compile_time_str[40];


char *compile_time()
{
   sprintf(compile_time_str, "%s", DATE);
   return compile_time_str;
}



/**********************************************************************
 * $Log: version.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:54  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 22:59:29  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.37  2003/06/19 20:38:15  endo
 * 6.0.01
 *
 * Revision 1.36  2003/04/06 13:40:29  endo
 * 6.0.00
 *
 * Revision 1.35  2003/04/06 10:44:44  endo
 * 5.0.09
 *
 * Revision 1.34  2003/04/03 23:06:44  endo
 * 5.0.08
 *
 * Revision 1.33  2002/11/03 20:43:13  endo
 * 5.0.07
 *
 * Revision 1.32  2002/09/26 14:36:23  endo
 * 5.0.06
 *
 * Revision 1.31  2002/08/22 14:42:39  endo
 * 5.0.05
 *
 * Revision 1.30  2002/07/18 17:16:57  endo
 * 5.0.04
 *
 * Revision 1.29  2002/07/04 23:29:48  endo
 * *** empty log message ***
 *
 * Revision 1.28  2002/04/01 22:02:51  endo
 * 5.0.02
 *
 * Revision 1.27  2002/01/31 10:57:55  endo
 * 5.0.01
 *
 * Revision 1.26  2001/12/23 21:55:10  endo
 * 5.0.00
 *
 * Revision 1.25  2001/11/26 19:19:20  endo
 * 4.1.00
 *
 * Revision 1.24  2001/06/15 22:51:10  endo
 * 4.0.09
 *
 * Revision 1.23  2001/03/09 05:30:03  endo
 * 4.0.08
 *
 * Revision 1.22  2001/01/10 19:41:50  endo
 * 4.0.07
 *
 * Revision 1.21  2000/12/02 23:24:44  endo
 * 4.0.06
 *
 * Revision 1.20  2000/10/16 21:58:15  endo
 * 4.0.05
 *
 * Revision 1.19  2000/09/19 10:20:46  endo
 * 4.0.04
 *
 * Revision 1.18  2000/08/15 21:03:35  endo
 * 4.0.03
 *
 * Revision 1.17  2000/08/12 21:55:47  endo
 * 4.0.02
 *
 * Revision 1.16  2000/08/01 19:31:43  endo
 * *** empty log message ***
 *
 * Revision 1.15  2000/07/02 06:05:47  endo
 * 4.0.00
 *
 * Revision 1.14  2000/06/20 00:57:53  endo
 * 3.1.05 -> 3.1.06
 *
 * Revision 1.13  2000/05/30 20:02:09  endo
 * 3.1.04 -> 3.1.05
 *
 * Revision 1.12  2000/04/25 08:55:33  endo
 * 3.1.03 -> 3.1.04
 *
 * Revision 1.11  2000/03/30 20:35:26  endo
 * 3.1.02 -> 3.1.03
 *
 * Revision 1.10  2000/03/20 00:27:58  endo
 * 1.0 --> 3.1.02
 *
 * Revision 1.9  1996/05/30 18:57:29  doug
 * 1.0 is the version released in June 1996
 *
 * Revision 1.8  1996/04/02  23:55:45  doug
 * handle value macros
 *
 * Revision 1.7  1996/02/18  22:44:44  doug
 * handle pushup parms in cnl_codege
 *
 * Revision 1.6  1996/02/14  17:14:57  doug
 * 0.8 adds support for descriptions to cnl_codegen
 *
 * Revision 1.5  1996/01/16  15:11:05  doug
 * version 0.7 adds the library list support
 *
 * Revision 1.4  1995/10/10  15:41:56  doug
 * Version 0.6 has functional FSA support
 *
 * Revision 1.3  1995/08/22  15:29:06  doug
 * Modified so is callable from cfgedit
 *
 * Revision 1.2  1995/05/09  19:55:00  doug
 * *** empty log message ***
 *
 * Revision 1.1  1995/04/21  22:54:21  doug
 * Initial revision
 *
 * Revision 1.1  1995/04/21  22:54:21  doug
 * Initial revision
 *
 **********************************************************************/
