/**********************************************************************
 **                                                                  **
 **                           strdupcat.cc                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: strdupcat.cc,v 1.1.1.1 2008/07/14 16:44:25 endo Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utilities.h"

//******************************************************************

// Returns an allocated block of mem with a//b and frees a.
// b is not freed.
// Handles case where a is NULL, but b must be valid

char *
strdupcat(char *a, const char *b)
{
   assert(b);
   if (a == NULL)
      return strdup(b);

   int len1 = strlen(a);
   int len2 = strlen(b);
   char *out = (char *) malloc(len1 + len2 + 1);

   strcpy(out, a);
   strcpy(&out[len1], b);
   free(a);

   return out;
}

//******************************************************************

// Returns an allocated block of mem with a//b//c and frees a.
// b and c are not freed.
// Handles case where a is NULL, but b and c must be valid.

char *
strdupcat(char *a, const char *b, const char *c)
{
   assert(b);
   assert(c);
   if (a == NULL)
      return strdupcat(strdup(b),c);

   int len1 = strlen(a);
   int len2 = strlen(b);
   int len3 = strlen(c);
   char *out = (char *) malloc(len1 + len2 + len3 + 1);

   strcpy(out, a);
   strcpy(&out[len1], b);
   strcpy(&out[len1+len2], c);
   free(a);

   return out;
}

///////////////////////////////////////////////////////////////////////
// $Log: strdupcat.cc,v $
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
// Revision 1.3  1995/10/31  19:23:46  doug
// added three arg version
//
// Revision 1.2  1995/10/30  21:35:30  doug
// *** empty log message ***
//
// Revision 1.1  1995/10/30  21:13:16  doug
// Initial revision
//
// Revision 1.2  1995/06/29  17:37:17  jmc
// Added header and RCS id and log strings.
///////////////////////////////////////////////////////////////////////
