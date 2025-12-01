/**********************************************************************
 **                                                                  **
 **                           strip_crs.cc                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: strip_crs.cc,v 1.1.1.1 2008/07/14 16:44:25 endo Exp $ */

#include "utilities.h"

//******************************************************************

// Returns the input string with the carriage returns converted to blanks.
// IT MODIFIES THE INPUT STRING

char *
strip_crs(char *a)
{
   if (a == NULL)
      return NULL;

   char *p = a;
   while( *p )
   {
      if( *p == '\n' )
	 *p = ' ';
      p++;
   }

   return a;
}


///////////////////////////////////////////////////////////////////////
// $Log: strip_crs.cc,v $
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
// Revision 1.1  1996/03/12  17:45:34  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
