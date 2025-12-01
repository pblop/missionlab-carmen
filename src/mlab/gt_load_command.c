/**********************************************************************
 **                                                                  **
 **                        gt_load_command.c                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_load_command.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

extern "C" {
#include <stdio.h>
#include <malloc.h>
}

#include "console.h"
#include "gt_load_command.h"
#include "gt_console_db.h"
#include "gt_world.h"
#include "gt_sim.h"
#include "file_utils.h"


extern FILE *cmdl_in;
extern int cmdl_parse();
extern void cmdl_file_reset();
extern char cmdl_filename[];
extern int next_robot_id;


char *command_filename_directory = NULL;


/**********************************************************************
 **          gt_load_commands          **
 ****************************************/

int gt_load_commands(char *filename)
{
   /* returns 0 for success, nonzero otherwise */

   char msg[256];
   char dir[256];
   int result;

   /* try to open the file */
   if ((cmdl_in = fopen(filename, "r")) == NULL) {
      sprintf(msg, "Error! Unable to open command file '%s'.", filename);
      warn_user(msg);
      cmdl_in = NULL;
      return GT_FAILURE;
      }

   /* note the filename */
   strcpy(cmdl_filename, filename);

   /* extract the directory, if any */
   command_filename_directory = NULL;
   if (extract_directory(filename, dir) != NULL) {
      command_filename_directory = (char *)malloc(strlen(dir)+1);
      strcpy(command_filename_directory, dir);
      }

   /* clear out everything first */
   clear_simulation();
   gt_initialize_robot_info();
   scroll_drawing_area_to_bottom();
   next_robot_id = 1;

   /* parse that file! */
   result = cmdl_parse();
   cmdl_file_reset();

   /* restore the old file */
   fclose(cmdl_in);
   cmdl_in = NULL;
   cmdl_filename[0] = '\0';
   if (command_filename_directory)
      free(command_filename_directory);
   command_filename_directory = NULL;

   return result;
}



/**********************************************************************
 * $Log: gt_load_command.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/06/15 15:30:36  pulam
 * SPHIGS Removal
 *
 * Revision 1.1.1.1  2005/02/06 23:00:10  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.23  1997/02/12  05:42:50  zchen
 * *** empty log message ***
 *
 * Revision 1.22  1995/10/23  18:21:57  doug
 * Extend object stuff to support obstacles and also add 3d code
 *
 * Revision 1.21  1995/05/08  16:00:12  jmc
 * Minor fix to potential free() problem in line 84.
 *
 * Revision 1.20  1995/04/24  20:02:55  jmc
 * Changed the clear_* before loading new commands to
 * clear_simulation() so everything is reset.
 *
 * Revision 1.19  1995/04/19  19:53:12  jmc
 * Added a call to scroll_drawing_area_to_bottom to reposition the
 * drawing appropriately each time before a new scenario is loaded.
 *
 * Revision 1.18  1995/04/14  17:25:38  jmc
 * Renamed clear_world() to clear_obstacles().
 *
 * Revision 1.17  1995/04/12  21:13:32  jmc
 * Modified gt_load_command to extract and save the directory, if
 * present so it can be used to locate the overlay files.  Defined
 * global variable command_filename_directory.
 *
 * Revision 1.16  1995/04/03  20:20:49  jmc
 * Added copyright notice.
 *
 * Revision 1.15  1994/11/21  17:26:12  jmc
 * Added gt_initialize_robot_info() before loading/reloading command
 * file.
 *
 * Revision 1.14  1994/11/15  15:52:45  jmc
 * Added code to reset the next_robot_id to 1 before loading the
 * command file.
 *
 * Revision 1.13  1994/11/14  20:14:41  jmc
 * Clear the obstacles before loading a new command file!
 *
 * Revision 1.12  1994/11/11  21:59:03  jmc
 * Modified function to clear database and map before loading new
 * info.
 *
 * Revision 1.11  1994/11/10  17:01:52  jmc
 * Modified error message to one line.
 *
 * Revision 1.10  1994/09/02  20:15:40  jmc
 * Fixed a minor #include ordering problem.
 *
 * Revision 1.9  1994/08/18  20:03:50  doug
 * moved gt_std.h to doug.h
 *
 * Revision 1.8  1994/08/04  14:18:48  jmc
 * Modified includes because prototypes for warn_user... were
 * moved to console.h.
 *
 * Revision 1.7  1994/07/26  18:41:39  jmc
 * Save filename while reading commands for better syntax errors.
 * Got rid of stuff for saving previous version of file cmdl_in.
 *
 * Revision 1.6  1994/07/26  16:09:34  jmc
 * Changed explicit "extern void warn_user" to include "gt_sim.h".
 *
 * Revision 1.5  1994/07/25  22:15:22  jmc
 * Tweaked error message.
 *
 * Revision 1.4  1994/07/25  21:27:35  jmc
 * Changed error messages to use the error dialog box.
 *
 * Revision 1.3  1994/07/14  20:30:11  jmc
 * Removed definition of FAILURE
 *
 * Revision 1.2  1994/07/12  19:25:11  jmc
 * Added RCS automatic identification strings
 **********************************************************************/
