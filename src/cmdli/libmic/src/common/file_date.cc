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

/* $Id: file_date.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include "mic.h"


namespace sara
{
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
 * Revision 1.1.1.1  2008/07/14 16:44:16  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:46  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2006/07/01 00:30:29  endo
 * CMDLi from MARS 2020 migrated into AO-FNC repository.
 *
 * Revision 1.1.1.1  2006/06/29 20:42:07  endo
 * cmdli local repository.
 *
 * Revision 1.4  2004/05/11 19:34:29  doug
 * massive changes to integrate with USC and GaTech
 *
 * Revision 1.3  1997/12/12 01:09:47  doug
 * *** empty log message ***
 *
 * Revision 1.2  1997/12/11 15:08:03  doug
 * moved to libmic
 *
 * Revision 1.1  1996/02/07 17:51:08  doug
 * Initial revision
 *
 **********************************************************************/
}
