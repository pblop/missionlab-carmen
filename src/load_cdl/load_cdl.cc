/**********************************************************************
 **                                                                  **
 **                            load_cdl.cc                           **
 **                                                                  **
 **  Main entry point for the cdl parser                             **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: load_cdl.cc,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <GetOpt.h>

#include "cdl_defs.h"
#include "load_cdl.h"

int had_error;
Symbol *top_agent; 

static const char    **filenames;

bool load_cdl_debug = false;
RECORD_CLASS current_class;
const char *current_source_file;

void yyinit(bool preload);

// Remember special architectures
Symbol *AuRA_arch;
Symbol *AuRA_urban_arch;
Symbol *AuRA_naval_arch;
Symbol *UGV_arch;

// Remember special types
Symbol *expression_type;
Symbol *member_type;
Symbol *void_type;
Symbol *UTM_type;
Symbol *boolean_type;
Symbol *string_type;
Symbol *commands_type;


int filenum = -1;

int
next_file()
{
   if( filenum > 0 )
      fclose(yyin);

   // Are we done?
   if( filenames[filenum] == NULL )
      return 1;
 
   // No, open the next one
   yyin = fopen(filenames[filenum], "r");
   if( yyin == NULL )
   {  
      char out[256];
         
      sprintf(out,"Fatal: Can't open input file %s\n",filenames[filenum]);
      perror(out);
      exit(1);
   }
   filename = filenames[filenum];
   
   if( current_class == RC_LIBRARY )
   {
      current_source_file = strdup(filename);
   }

   if( load_cdl_debug )
      printf("Reading file %s\n", filenames[filenum]);

   return 1;
}  
      
int
load_first_file(void)
{  
   filenum = 0;
   return next_file();
}  


/*********************************************************************
*                                                                    *
*                    initilizes the parser as well as preloads       *
*                                                                    *
*********************************************************************/

// Returns true if error
bool
init_cdl(bool debug)
{
   filenames = NULL;
   top_agent = NULL;
   current_class = RC_SYSTEM;
   current_source_file = NULL;

   yydebug = debug;
   load_cdl_debug = debug;

//************ call any initilization routines ********************

   init_symtab();

/************** initilize the parser ********************/

   had_error = false;

   // Define the special types in the free architecture
   void_type = AddType(free_arch,"void");
   expression_type = AddType(free_arch,"expression");
   member_type = AddType(free_arch,"member");
   UTM_type = AddType(free_arch,"UTM");
   boolean_type = AddType(free_arch,"boolean");
   commands_type = AddType(free_arch,"commands");

   // Add the AuRA and UGV architecture types
   AuRA_arch = AddArch("AuRA");
   AuRA_urban_arch = AddArch("AuRA.urban");
   AuRA_naval_arch = AddArch("AuRA.naval");
   UGV_arch = AddArch("UGV");

   current_class = RC_SYSTEM;
   yyinit(true);

/************** load the input file ********************/

   // Put the free architecture on the scope stack as the default
   EnterScope(free_arch);

   // Preload from the preload_buffer
   yyparse();

/************** done ********************/

   return had_error;
}

/*********************************************************************
*                                                                    *
*            mark that new objects are in the user class             *
*                                                                    *
*  Call this before the gui lets the user start creating objects     *
*********************************************************************/

void
set_user_class()
{
   current_class = RC_USER;
   current_source_file = NULL;
}


/*********************************************************************
*                                                                    *
*                    main routine                                    *
*                                                                    *
*********************************************************************/

Symbol *
load_cdl(const char **names, bool &errors, bool is_library, bool debug)
{
   extern FILE *yyin;
   current_class = is_library ? RC_LIBRARY : RC_USER;

   filenames = names;

   yydebug = debug;
   load_cdl_debug = debug;

/************** initilize the parser ********************/

   had_error = false;
   top_agent = NULL;


   // Setup the first file
   load_first_file();
   yyinit(false);

/************** load the input file ********************/

   /* Process the files */
   yyparse();

/************** close the last input file ********************/

   fclose(yyin);

/************** done ********************/
   errors = had_error;

   return top_agent;
}


///////////////////////////////////////////////////////////////////////
// $Log: load_cdl.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:22  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:49  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:59  endo
// MissionLab 7.0
//
// Revision 1.2  2005/05/18 21:19:07  endo
// AuRA.naval added.
//
// Revision 1.1.1.1  2005/02/06 23:00:07  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.16  1999/12/16 22:27:23  endo
// rh-6.0 porting.
//
// Revision 1.15  1999/09/03 20:04:08  endo
// AuRA_urban_arch stuff added.
//
// Revision 1.14  1996/10/04 21:10:59  doug
// changes to get to version 1.0c
//
// Revision 1.14  1996/10/01 13:01:32  doug
// got library writes working
//
// Revision 1.13  1995/11/29  23:12:30  doug
// *** empty log message ***
//
// Revision 1.12  1995/10/09  20:06:46  doug
// added an errors parameter to the load_cdl call so can load files
// with partial errors
//
// Revision 1.11  1995/09/15  15:37:40  doug
// add commands_type
//
// Revision 1.10  1995/09/07  14:23:14  doug
// works
//
// Revision 1.9  1995/06/29  14:46:17  jmc
// Added RCS log string.
///////////////////////////////////////////////////////////////////////
