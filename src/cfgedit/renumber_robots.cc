/**********************************************************************
 **                                                                  **
 **                     renumber_robots.cc                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: renumber_robots.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#include <stdio.h>
#include <malloc.h>

#include <Xm/Xm.h>

#include "list.hpp"
#include "load_cdl.h"
#include "configuration.hpp"
#include "globals.h"
#include "cfg_apply.h"
#include "renumber_robots.h"

int num_robots;
static SymbolList robot_records;
static bool changed;
int num_FSAs;
static SymbolList FSA_records;

// NOTE: The robots are named "CfgRobotxxx" where xxx is a 1 based integer.
//        and cfg is the name of the current configuration.

/*-----------------------------------------------------------------------*/

char *
RobotName(const int n)
{
   char buf[256];
   sprintf(buf,"%sRobot%d", config->configuration_name(), n); 
   return strdup(buf);
}


/*-----------------------------------------------------------------------*/

// Renumber this robot (if this is in fact a robot)
// Returns true if successful
static bool
renumber(Symbol * rec, Symbol *page_def, Symbol *robot,
         SymbolList * page_stack, char **message, bool *restart)
{
   // Don't care if isn't a robot
   if (rec == NULL || rec->symbol_type != ROBOT_NAME )
   {
      return true;
   }

   // Already handled
   if( robot_records.exists(rec) )
   {
      return true;
   }

   num_robots ++;
   char *name = RobotName(num_robots);
   if( rec->name == NULL || strcmp(rec->name, name) )
   {
      if( rec->name )
         free( rec->name );
      rec->name = name;
      changed = true;
   }
   else
   {
      free(name);
   }

   // Assign the ID of the robot.
   rec->id = num_robots;

   // Remember we did this one.
   robot_records.append(rec);

   return true;
}

/*-----------------------------------------------------------------------*/
static char *FSAName(const int n)
{
   char buf[256];
   sprintf(buf,"FSA%d", n); 
   return strdup(buf);
}


/*-----------------------------------------------------------------------*/
// Rename this FSA (if this is in fact an FSA)
// Returns true if successful
static bool renameFSA(
    Symbol * rec,
    Symbol *page_def,
    Symbol *robot,
    SymbolList * page_stack,
    char **message, bool *restart)
{
   // Don't care if isn't an FSA
   if (rec == NULL || !rec->is_fsa() )
   {
      return true;
   }

   // Already handled
   if( FSA_records.exists(rec) )
   {
      return true;
   }

   num_FSAs ++;
   char *name = FSAName(num_FSAs);
   if( rec->name == NULL || strcmp(rec->name, name) )
   {
      if( rec->name )
         free( rec->name );
      rec->name = name;
      changed = true;
   }
   else
   {
      free(name);
   }

   // Remember we did this one.
   FSA_records.append(rec);

   return true;
}

/*-----------------------------------------------------------------------*/

/* called when binding button is clicked in glyph */
void
renumber_robots(void)
{
   // Remember if make any changes
   changed = false;

   // Walk through the configuration renaming robots to "cfgRobotxxx",
   // where xxx is a 1 based integer and cfg is the name of the current configuration.
   // This is the name that each robot executable will receive and they are
   // renumbered here to ensure that two robot executables don't have the
   // same name (caused by copy and paste operations).
   num_robots = 0;
   robot_records.clear();
   cfg_apply(config->root(), renumber);

   // Walk through the configuration renaming FSAs to "FSAxxx",
   // where xxx is a 1 based integer
   // This is done to ensure that the FSAs have a unique, real name.
   // The replay functionality requires matching FSA names from the
   // running robots against the copy loaded in cfgedit.
   num_FSAs = 0;
   FSA_records.clear();
   cfg_apply(config->root(), renameFSA);

   // Force a redraw if we made any changes 

   if( changed )
   {
      // Regenerate the drawing to get the changes
      config->regen_page();

      // Mark drawing as changed
      config->made_change();
   }
}

///////////////////////////////////////////////////////////////////////
// $Log: renumber_robots.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.2  2005/02/07 22:25:26  endo
// Mods for usability-2004
//
// Revision 1.1.1.1  2005/02/06 22:59:34  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.6  2003/04/06 12:25:34  endo
// gcc 3.1.1
//
// Revision 1.5  2001/02/01 20:06:55  endo
// Modification to display the robot ID and name in the
// "Individual Robot" glyph was added.
//
// Revision 1.4  1997/02/14 16:53:35  zchen
// *** empty log message ***
//
// Revision 1.3  1996/05/08  15:16:39  doug
// fixing compile warnings
//
// Revision 1.2  1996/05/07  19:53:20  doug
// fixing compile warnings
//
// Revision 1.1  1996/02/28  03:57:00  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
