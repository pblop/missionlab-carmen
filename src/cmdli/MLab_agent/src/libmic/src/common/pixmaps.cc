/**********************************************************************
 **                                                                  **
 **  pixmaps.cc                                                      **
 **                                                                  **
 **  Puts the bitmap files into a container.                         **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: pixmaps.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: pixmaps.cc,v $
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
**********************************************************************/

#include "mic.h"
#include "pixmaps.h"

using namespace std;
namespace sara
{
// **********************************************************************
// singleton design pattern
pixmaps *pixmaps::theobj = NULL;

// **********************************************************************
// Returns the pixmap matching the name 
// or prints a warning and returns a default pixmap if the name is not found.
Pixmap 
pixmaps::get(const string &name)
{
   if( !theobj )
   {
      theobj = new pixmaps();
   }

   pixmaps_T::iterator it = theobj->loaded_maps.find(name);
   if( it == theobj->loaded_maps.end() )
   {
      ERROR("Pixmap %s was referenced but not loaded", name.c_str());

      it = theobj->loaded_maps.find("default");
      if( it == theobj->loaded_maps.end() )
      {
         FATAL_ERROR("No default Pixmap available!");
      }
   }

   return (*it).second;
}

// **********************************************************************
// Add a pixmap into the container
bool 
pixmaps::put(const string &name, Pixmap pm)
{
   if( !theobj )
   {
      theobj = new pixmaps();
   }

   theobj->loaded_maps[name] = pm;

   return true;
}

// **********************************************************************
}
