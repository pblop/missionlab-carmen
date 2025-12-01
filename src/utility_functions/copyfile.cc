/**********************************************************************
 **                                                                  **
 **                            copyfile.cc                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: copyfile.cc,v 1.1.1.1 2008/07/14 16:44:25 endo Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utilities.h"

bool
copyfile(const char *src, const char *des)
{
   char *cmd = strdup("cp ");
   cmd = strdupcat(cmd, src);
   cmd = strdupcat(cmd, " ");
   cmd = strdupcat(cmd, des);

   int rtn = system(cmd);
   if( rtn & 0xff != 0 )
      return true;

   rtn = (rtn >> 8) & 0xff;
   if( rtn != 0 )
      return true;

   return false;
}


///////////////////////////////////////////////////////////////////////
// $Log: copyfile.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:25  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:52  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:38:01  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 23:00:22  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.4  1995/10/30  21:35:30  doug
// *** empty log message ***
//
// Revision 1.3  1995/10/30  21:13:16  doug
// *** empty log message ***
//
// Revision 1.2  1995/06/29  17:37:17  jmc
// Added header and RCS id and log strings.
///////////////////////////////////////////////////////////////////////
