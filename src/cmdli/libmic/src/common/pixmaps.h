/**********************************************************************
 **                                                                  **
 **  pixmaps.h                                                       **
 **                                                                  **
 **  A container for bitmap graphics files.                          **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: pixmaps.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: pixmaps.h,v $
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
* Revision 1.3  2004/05/11 19:34:29  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.2  2003/08/25 22:01:49  doug
* builds against libccl_gui
*
* Revision 1.1  2003/08/25 19:59:33  doug
* moved pixmaps to libmic
*
**********************************************************************/

#include "mic.h"
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <Xm/Xm.h>

namespace sara
{
/**********************************************************************/
// Macros to help build the pixmaps from the bitmaps.
// Usage:   pixmaps.add("add_state", create_pm(add_state));
//          pixmaps.add("add_state", create_ins_pm(add_state));

// Creates a pixmap using the insensitive forground color.
#define create_ins_pm(name) XCreatePixmapFromBitmapData(XtDisplay(top_level),\
	RootWindowOfScreen(XtScreen(top_level)),\
	(char *)name##_bits, name##_width, name##_height, ins_fg, bg, \
	DefaultDepthOfScreen(XtScreen(top_level)))

// Creates a pixmap using the standard forground color.
#define create_pm(name) XCreatePixmapFromBitmapData(XtDisplay(top_level),\
	RootWindowOfScreen(XtScreen(top_level)),\
	(char *)name##_bits, name##_width, name##_height, fg, bg, \
	DefaultDepthOfScreen(XtScreen(top_level)))


/**********************************************************************/
// Define a container for the pixmaps.
class pixmaps
{
   public:
      /// Add a pixmap to the container
      /// Returns true on success, false if the pixmap already existed
      static bool put(const std::string &name, Pixmap pm);

      /// Returns the pixmap matching the name 
      /// or prints a warning and returns a default pixmap if the name is not found.
      static Pixmap get(const std::string &name);

   private:
      /// the container
      typedef std::map<std::string, Pixmap, std::less<std::string> > pixmaps_T;
      pixmaps_T loaded_maps;

      // singleton design pattern
      static pixmaps *theobj;

      /// constructor that does not load a default pixmap
      pixmaps() {};

      /// destructor
      ~pixmaps() {};
};

/**********************************************************************/
}
