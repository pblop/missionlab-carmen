/**********************************************************************
 **                                                                  **
 **                        edit.cc                                   **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: edit.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <Xm/Xm.h>

#include "load_cdl.h"
#include "reporterror.h"
#include "buildmenu.h"
#include "design.h"
#include "popups.h"
#include "help.h"
#include "prototypes.h"
#include "configuration.hpp"
#include "globals.h"
#include "screen_object.hpp"
#include "page.hpp"
#include "write_cdl.h"
#include "utilities.h"
#include "verify.h"
#include "cfgedit_common.h"

/*-----------------------------------------------------------------------*/

static const char *xterm_string = "xterm -geometry 80x40+%d+%d -e ";

// display a text file using the operators faviorite editor
// Return true if edit runs OK
bool
edit_file(char *filename)
{
	 // Set the cursor to the wait picture
//	 XDefineCursor(XtDisplay(top_level), XtWindow(top_level), wait_cursor);

	 char *full_name = find_file(filename,rc_table.get(CNL_SOURCES));
	 FILE *file = fopen(full_name, "r");

	 if (file == NULL)
	 {
	    // Return the cursor back to normal
//	    XDefineCursor(XtDisplay(top_level), XtWindow(top_level), arrow_cursor);

	    warn_userf("Unable to open file '%s' to display implementation",
		       filename);
	    free(full_name);
	    return false;
	 }
	 fclose(file);

	 // Figure out an editor to run
	 char buf[512];

	 buf[0] = '\0';

	 char *editor = getenv("EDITOR");

	 if (editor == NULL)
	    editor = "vi";


         // Figure out where to put the new window.
	 // Want to overlay the editor work area
	 int x = 0;
	 int y = 0;
	 Widget w = drawing_area;
	 while(w != top_level )
	 {
            Position dx, dy;
            XtVaGetValues(w, XmNx, &dx, XmNy, &dy, NULL);
	    x += dx;
	    y += dy;

	    w = XtParent(w);
	 }


	 if (strstr(editor, "emacs") != NULL)
	 {
	    char *display = getenv("DISPLAY");

	    if (display == NULL)
	    {
	       sprintf(buf, xterm_string,x,y );
	    }
	 }
	 else
	 {
	    sprintf(buf, xterm_string, x,y);
	 }

	 sprintf(&buf[strlen(buf)], "%s %s", editor, full_name);
	 free(full_name);

	 // Run the editor
	 int rtn = system(buf);

	 // Return the cursor back to normal
//	 XDefineCursor(XtDisplay(top_level), XtWindow(top_level), arrow_cursor);

	 if (rtn & 0xff != 0)
	 {
	    warn_userf("Unable to exec editor '%s'", buf);
	    return false;
	 }

	 rtn = (rtn >> 8) & 0xff;
	 if (rtn != 0)
	 {
	    warn_userf("Error exit from editor = %d", rtn);
	    return false;
	 }

   return true;
}

/*-----------------------------------------------------------------------*/

/* move down to the expanded view of the selected agent */
void
edit_string(char **string)
{
  char filename[256];
  sprintf(filename, "/tmp/%d.cmds", getpid());
  FILE *file = fopen(filename, "w");
  if (file == NULL)
  {
     warn_user("Unable to create temporary file to display commands");
     goto leave;
  }
  if( string != NULL )
  {
     char *start = strchr(*string,'\"');
     char *end = strrchr(*string,'\"');
     if( start == NULL )
	start = *string;
     else
	start++;

     if( end == NULL )
     {
	int len = strlen(*string);
	end = &(*string)[len-1];
     }
     else
	end --;

     int len = (int)(end - start) + 1;
     if( len > 0 && (int)fwrite(start,1,len,file) != len )
     {
        warn_user("Unable to write temporary file to display commands");
	goto leave;
     }
  }
  fclose(file);
  file = NULL;

  if( edit_file(filename) )
  {
     FILE *file = fopen(filename, "r");
     if (file == NULL)
     {
        warn_user("Changes lost: Temporary file disapeared during editing!!");
	goto leave;
     }
     if( fseek(file, 0, 2) < 0 )
     {
        warn_user("Changes lost: Unable to seek in temporary file!!");
	goto leave;
     }
     int len = ftell(file);
     if( fseek(file, 0, 0) < 0 )
     {
        warn_user("Changes lost: Unable to seek in temporary file!!");
	goto leave;
     }

     if( len < 0 )
     {
        warn_user("Changes lost: Unable to get length of temporary file!!");
	goto leave;
     }

     if( len == 0 )
     {
	// Special case to delete commands
     }
     else
     {
        char *rtn = new char[len+3];

        int num = fread(&rtn[1],1,len,file);
        if( num != len )
        {
           warn_userf("Changes lost: Tried to read %d bytes but only got %d from temporary file",len,num);
	   goto leave;
        }
	rtn[0] = '\"';
	rtn[len+1] = '\"';
	rtn[len+2] = '\0';
	free(*string);
	*string = rtn;
     }
  }

leave:
   if( file )
      fclose(file);
   unlink(filename);

   // Return the cursor back to normal
   //   XDefineCursor(XtDisplay(top_level), XtWindow(top_level), arrow_cursor);

   return;
}

///////////////////////////////////////////////////////////////////////
// $Log: edit.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 22:59:32  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.12  2000/04/13 21:43:25  endo
// This patch extends MissionLab to allow the user to import read-only
// library code.  When the user attempts to change something which is
// read-only, cfgedit pops up a dialog box and asks if it should import
// the object so it can be edited.  If OK'd, it imports the object.
//
// This fixes the problem with FSA's (and other assemblages) not being
// editable when they are loaded from the library.
//
// Revision 1.11  1997/02/14  16:53:35  zchen
// *** empty log message ***
//
// Revision 1.10  1996/10/04  20:58:17  doug
// changes to get to version 1.0c
//
// Revision 1.10  1996/10/01 13:00:11  doug
// went to version 1.0c
//
// Revision 1.9  1996/05/07 19:17:03  doug
// fixing compile warnings
//
// Revision 1.8  1996/02/06  18:16:42  doug
// *** empty log message ***
//
// Revision 1.7  1996/01/31  03:06:53  doug
// *** empty log message ***
//
// Revision 1.6  1996/01/29  00:07:50  doug
// *** empty log message ***
//
// Revision 1.5  1996/01/17  18:47:01  doug
// *** empty log message ***
//
// Revision 1.4  1995/11/21  23:10:08  doug
// *** empty log message ***
//
// Revision 1.3  1995/11/08  16:50:02  doug
// *** empty log message ***
//
// Revision 1.2  1995/10/31  19:22:01  doug
// handle text field changes in loose focus callback
//
// Revision 1.1  1995/10/31  15:23:57  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
