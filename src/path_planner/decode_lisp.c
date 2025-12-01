/**********************************************************************
 **                                                                  **
 **                               decode_lisp.c                          **
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

/* $Id: decode_lisp.c,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

/******************************************************************************

  			    decode_lisp

      reads char array from stdin and write it as text to stdout

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
   int             rtn;
   int             i;
   int             len;
   int             chr;

   
   if( (rtn = scanf(" %[^\n\r] ",&line)) == EOF )
      return;

   while(1)
   {
      if( (rtn = scanf("%x",&chr)) == EOF )
      {
	 fprintf(stderr,"eof");
	 break;
      }
      
      if( rtn > 0 )
      {
         putchar(chr);
         chr = getchar();
	 if( chr == '\n' )
	    putchar('\n');
      }
   }

   return 0;
}



///////////////////////////////////////////////////////////////////////
// $Log: decode_lisp.c,v $
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
// Revision 1.1  2000/03/22 04:39:49  saho
// Initial revision
//
///////////////////////////////////////////////////////////////////////

