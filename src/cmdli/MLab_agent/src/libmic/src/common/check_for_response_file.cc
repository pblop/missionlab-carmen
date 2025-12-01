/**********************************************************************
 **                                                                  **
 **  check_for_response_file.cc                                      **
 **                                                                  **
 **  Only useful on DOS systems where the command line is 128 bytes. **
 **                                                                  **
 **  If the first parameter starts with an '@' then it is the name   **
 **  of a file containing the command line parameters.  Load it as   **
 **  argv and argc so the getops calls work as expected.             **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: check_for_response_file.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: check_for_response_file.cc,v $
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
* Revision 1.2  2004/05/11 19:34:29  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.1  1998/02/14 11:10:37  doug
* Initial revision
*
**********************************************************************/

#include "mic.h"
#include <stdio.h>

namespace sara
{
/*************************************************************************/
void
check_for_response_file(int *argc, char ***argv)
{
   // If the user passed the name of a response file, load it as argv.
   if( *argc == 2 && (*argv)[1][0] == '@' )
   {
      FILE *in = fopen( &(*argv)[1][1], "rb");
      if( in == NULL )
      {
         ERROR_with_perror("Unable to open response file %s", (*argv)[1]);
         return;
      }

      // Figure out how big the file is.
      if( fseek(in, 0, SEEK_END) )
      {
         ERROR_with_perror("Unable to seek to end of response file %s", 
			(*argv)[1]);
         return;
      }
      int size = ftell(in);
      if( size < 0 )
      {
         ERROR_with_perror("Unable to get size of response file %s", 
			(*argv)[1]);
         return;
      }
      if( fseek(in, 0, SEEK_SET) )
      {
         ERROR_with_perror("Unable to seek to beginning of response file %s", 
			(*argv)[1]);
         return;
      }

      // Allocate a buffer to hold the file
      static char *buf = (char *)malloc( size+1 );

      int rd = fread(buf, 1, size, in);
      if( rd != size )
      {
         ERROR_with_perror("Only read %d bytes of %d from response file %s", 
			rd, size, (*argv)[1]);

         free(buf);
         return;
      }
      buf[size] = '\0';

      // Now count the number of entries in the new environment.
      int cnt = 1;	// for program name.
      char *p = &buf[0];
      while( *p )
      {
         // Skip to start of parm.
         while( *p && isspace(*p) ) 
            p++;

         // Found start of one.
         if( *p )
            cnt ++;

         // Skip to end.
         while( *p && !isspace(*p) ) 
            p++;
      }

      char **v = (char **)malloc(sizeof(char *) * cnt);
      if( v == NULL )
      {
         ERROR_with_perror("Out of memory allocating space for %d entries from response file %s", 
			cnt, (*argv)[1]);
         return;
      }

      // get the program name.
      cnt = 0;
      v[cnt++] = (*argv)[0];

      // Set the rest of the new argv pointers.
      p = &buf[0];
      while( *p )
      {
         // Skip to start of parm.
         while( *p && isspace(*p) ) 
         {
            *p = '\0';
            p++;
         }

         // Found start of one.
         if( *p )
            v[cnt++] = p;

         // Skip to end.
         while( *p && !isspace(*p) ) 
            p++;
      }

      // Commit
      *argc = cnt;
      *argv = v;
   }
}

/*********************************************************************/
}
