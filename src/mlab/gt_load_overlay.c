/**********************************************************************
 **                                                                  **
 **                       gt_load_overlay.c                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_load_overlay.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#include <stdio.h>
#include <string.h>
#include <X11/Intrinsic.h>
#include <string>

#include "console.h"
#include "mission_design.h"
#include "gt_scale.h"
#include "gt_simulation.h"
#include "gt_load_overlay.h"
#include "gt_load_command.h"
#include "gt_sim.h"
#include "cfgedit_common.h"
#include "file_utils.h"
#include "EventLogging.h"

extern string g_strEnvChangeFilename;
void LoadEnvChangeFile(string strFileName);

/****************************************
 **          gt_load_overlay           **
 ****************************************/

int fileLoaded = 0;      // Used to determine whether a load_file command
                         //   should actually be ignored.... 
                         //   see <interrupt_load_ovlay>
char currentFile[256]; // The name of the alternate overlay 

void interrupt_load_overlay(char* filename)
{
    // Sets the value of the global variables (fileLoaded, currentFile) so
    //  that "gt_load_overlay" will load the specified file instead of the
    //  default file.  This does not change the name of the default file,
    //  which is specified when cfgedit creates a mission.

    fileLoaded = 1;
    sprintf(currentFile,"%s",filename);
} //  NEEDED BY RESET WORLD ERIC

int gt_load_overlay(char* filename)
{
    // returns 0 for success, nonzero otherwise 


    // gt_load_overlay is called by the yac code for parsing
    //    the mission cdl file.  Immeadiately afterwards, the start 
    //    positions are set for each of the robots.  In order to 
    //    specify the correct start positions, the desired overlay
    //    has to be loaded the first time. 
    // The following function checks to see if there is an alternate
    //    file to the default overlay, and recursively calls 
    //    gt_load_overlay on the correct file, instead of continuing
    //    execution on the wrong file.
    if (fileLoaded)
    {
        fileLoaded = 0;
        return gt_load_overlay(currentFile);
    }

    char full_filename[256];
    int result;

    // First, find the file
    if (find_full_filename(filename, command_filename_directory,
                           full_filename) == NULL)
    {
        // If there is a resource variable set, look there instead.
        const rc_chain* dirs = rc_table.get(OVERLAY_FILES);
        char* str;
        if ((dirs == NULL) ||
            (dirs->first(&str) == NULL) ||
            (find_full_filename(filename, str, full_filename) == NULL))
        {
        	if(find_full_filename(filename, "/usr/src/overlays/", full_filename) == NULL)
        	{
        		warn_userf("Error! Unable to find overlay file\n      '%s'.", filename);
        		odl_in = NULL;
        		return GT_FAILURE;
        	}
        }
    }

    // Open the file
    gEventLogging->start("LoadOverlay %s", full_filename);
    scroll_drawing_area_to_bottom();
    if ((odl_in = fopen(full_filename, "r")) == NULL)
    {
        warn_userf("Error! Unable to open overlay file\n       '%s'.",
                   full_filename);
        odl_in = NULL;
        return GT_FAILURE;
    }
    gEventLogging->end("LoadOverlay");

    // note the filename
    strcpy(odl_filename, full_filename);

    // parse that file!
    result = odl_parse();
    odl_file_reset();

    // close the file
    fclose(odl_in);
    odl_in = NULL;
    odl_filename[0] = '\0';

    // Make sure the new control measures get drawn immediately
    XFlush(XtDisplay(drawing_area));

    if (gUseMlabMissionDesign)
    {
        gMMD->saveOverlayName(filename);
        gMMD->drawOverlayMeasures(meters_per_pixel);
    }

    LoadEnvChangeFile(g_strEnvChangeFilename);

    return result;
}

/**********************************************************************
 * $Log: gt_load_overlay.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.2  2007/01/28 21:00:54  endo
 * MEXP_FEATURE_OPTION_POLYGONS added.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/01/30 02:43:17  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.1.1.1  2005/02/06 23:00:10  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.22  2002/11/05 19:18:16  blee
 * made changes to call LoadEnvChangeFile() at the end of gt_load_overlay()
 *
 * Revision 1.21  2002/10/31 21:36:37  ebeowulf
 * Added interrupt_load_overlay, to fix Start Place problem.
 *
 * Revision 1.20  2002/01/12 23:00:58  endo
 * Mission Expert functionality added.
 *
 * Revision 1.19  2000/07/07 18:22:29  endo
 * variable declearation moved to gt_load_overlay.h .
 *
 * Revision 1.18  2000/03/15 17:03:24  sapan
 * Added event logging calls
 *
 * Revision 1.17  1997/02/12 05:42:50  zchen
 * *** empty log message ***
 *
 * Revision 1.16  1996/03/06  23:39:33  doug
 * *** empty log message ***
 *
 * Revision 1.15  1995/06/14  18:22:33  jmc
 * Moved file finding code to find_full_filename() in file_utils.*.
 *
 * Revision 1.14  1995/04/12  21:12:01  jmc
 * Modifed gt_load_overlay to be smarter about finding the file,
 * including looking in the directory in which the invoking command
 * file is.
 *
 * Revision 1.13  1995/04/03  20:23:15  jmc
 * Added copyright notice.
 *
 * Revision 1.12  1994/11/10  17:01:25  jmc
 * Modified error message to one line.
 *
 * Revision 1.11  1994/10/25  22:12:14  jmc
 * Added a XFlush routine to make sure the new control measures are
 * drawn immediately.  Not convinced it helped much.
 *
 * Revision 1.10  1994/09/02  20:15:57  jmc
 * Fixed a minor #include ordering problem.
 *
 * Revision 1.9  1994/08/18  20:03:50  doug
 * moved gt_std.h to doug.h
 *
 * Revision 1.8  1994/08/04  14:18:48  jmc
 * Modified includes because prototypes for warn_user... were
 * moved to console.h.
 *
 * Revision 1.7  1994/07/26  18:21:55  jmc
 * Save filename while reading overlay for better syntax errors.
 * Got rid of stuff for saving previous version of file odl_in.
 *
 * Revision 1.6  1994/07/26  16:09:25  jmc
 * Changed explicit "extern void warn_user" to include "gt_sim.h".
 *
 * Revision 1.5  1994/07/25  22:15:10  jmc
 * Tweaked error message.
 *
 * Revision 1.4  1994/07/25  22:14:33  jmc
 * Tweaked error message.
 *
 * Revision 1.3  1994/07/25  21:27:49  jmc
 * Changed error messages to use the error dialog box.
 *
 * Revision 1.2  1994/07/12  19:25:27  jmc
 * Added RCS automatic identification strings
 **********************************************************************/
