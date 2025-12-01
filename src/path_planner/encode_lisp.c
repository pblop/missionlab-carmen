/**********************************************************************
 **                                                                  **
 **                               encode_lisp.c                          **
 **                                                                  **
 **                                                                  **
 **                                                                  **
 **                                                                  **
 **  Written by:                                                     **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: encode_lisp.c,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

/******************************************************************************

  			    encode_lisp

      reads lisp file from stdin and write it as a char array to stdout

******************************************************************************/


/*--------------------------------------------------------------------------*/

/* preliminaries                   */

/*------------------------------------------------------------------*/
#include <stdio.h>
#include "std.h"

/*---------------------------------------------------------------------------

				Main program

	Accepts a text file as input and writes it as a char array to stdout

---------------------------------------------------------------------------*/

int
main(int argc, char *argv[])
{
   char            line[10000];
   char           *comment;
   char           *strchr(char *,char);
   int             rtn;
   int             i;
   int             len;

   printf("char LISP_TEXT[]={\n");

   while(1)
   {
      line[0] = '\0';
      if( (rtn = scanf(" %[^\n\r] ",&line)) == EOF )
	 break;
      
      if( rtn > 0 )
      {
	 if((comment = strchr(line,';')) != NULL)
	    *comment = '\0';

         len = strlen(line);
         if( len > 0 )
	 {
            for(i=0; i<len; i++)
               printf("0x%2.2X,",line[i]);
            putchar('\n');
	 }
      }
   }

   puts("\n0x00 };\n");

   return 0;
}



///////////////////////////////////////////////////////////////////////
// $Log: encode_lisp.c,v $
// Revision 1.1.1.1  2008/07/14 16:44:24  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:51  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:38:00  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 23:00:23  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.1  2000/03/22 04:39:53  saho
// Initial revision
//
///////////////////////////////////////////////////////////////////////

