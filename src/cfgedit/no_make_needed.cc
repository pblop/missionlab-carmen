/**********************************************************************
 **                                                                  **
 **                     no_make_needed.cc                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: no_make_needed.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include <Xm/Xm.h>

#include "list.hpp"
#include "load_cdl.h"
#include "configuration.hpp"
#include "globals.h"
#include "renumber_robots.h"
#include "utilities.h"
#include "popups.h"
#include "verify.h"

/*-----------------------------------------------------------------------*/

/* called when binding button is clicked in glyph */
bool
no_make_needed(void)
{
   // Must verify before we will try to run it.
   if( !verify(true, true) )
      return false;

   // Get time cfg file was last modified
   time_t cfg_time = file_date(config->configuration_filename());

   if( cfg_time == (time_t)-1 )
   {
      warn_userf("Didn't find the configuration file '%s' after loading\n",
		config->configuration_filename());
      return false;
   }

   // Check the date on each robot executable
   for(int i=1; i <= num_robots; i++)
   {
      char *name = RobotName(i);
      time_t t = file_date(name);
      free(name);

      if( t == (time_t)-1 )
      {
	 return false;
      }

      if( t < cfg_time )
      {
	 return false;
      }
   }

   return true;
}

///////////////////////////////////////////////////////////////////////
// $Log: no_make_needed.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 22:59:33  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.2  1997/02/14  16:53:35  zchen
// *** empty log message ***
//
// Revision 1.1  1996/02/28  03:56:52  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
