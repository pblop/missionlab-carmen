/**********************************************************************
 **                                                                  **
 **                    file_date.cc                                  **  
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: file_date.cc,v 1.1.1.1 2008/07/14 16:44:25 endo Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "utilities.h"


/*-----------------------------------------------------------------------*/
// Return the date the file was last modified
// or -1 if not found

time_t
file_date(const char *name)
{
   struct stat status;
   if( stat(name, &status) )
   {
      // Didn't find it
      return (time_t)-1;
   }

   // Return modification time
   return status.st_mtime;
}


/**********************************************************************
 * $Log: file_date.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:25  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:22  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
// Revision 1.1  1996/02/07  17:51:08  doug
// Initial revision
//
 **********************************************************************/
