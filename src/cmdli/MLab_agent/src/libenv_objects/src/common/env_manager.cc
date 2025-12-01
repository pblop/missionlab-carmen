/**********************************************************************
 **                                                                  **
 **  env_manager.cc                                                  **
 **                                                                  **
 **  Manage the environment objects for the operator console         **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: env_manager.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: env_manager.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:16  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:21:03  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.2  2004/05/11 19:34:39  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.1  2002/11/05 17:23:53  doug
// compiles
//
// Revision 1.8  2002/11/04 22:35:12  doug
// snapshot
//
// Revision 1.7  2002/10/08 20:15:54  doug
// working on scaling robots
//
// Revision 1.6  2002/09/30 20:12:40  doug
// scaling works
//
// Revision 1.5  2002/09/23 21:41:50  doug
// *** empty log message ***
//
// Revision 1.4  2002/06/21 14:47:34  doug
// snapshot
//
// Revision 1.3  1999/03/11 22:21:08  doug
// Obstacles are working
//
// Revision 1.2  1999/03/10 13:00:02  doug
// compiles
//
// Revision 1.1  1999/03/10 01:35:57  doug
// Initial revision
//
//
///////////////////////////////////////////////////////////////////////

#include <mic.h>
#include "environment.h"
#include "env_object.h"
#include "env_manager.h"

namespace sara
{
// *******************************************************************
// Declare the Static variables
  
// The env_manager object
env_manager *env_manager::the_env_manager = NULL;
	 
// *******************************************************************
/// Create the env_manager and load the environment file.
env_manager *
env_manager::load_env(const string &filename, 
                      const bool no_path_search,
                      const bool verbose,
		      const bool debug)
{
   if( the_env_manager != NULL )
   {
      ERROR("Called env_manager::load_environment twice.\n");
   }
   else
   {
      the_env_manager =  new env_manager(debug);

      // Load the simulated environment
      the_env_manager->theEnvironment = new environment();
      if( the_env_manager->theEnvironment->load_environment(filename.c_str(), no_path_search, verbose, debug) )
      {
         ERROR_with_perror("Unable to load environment file '%s'", filename.c_str());
      }
   }

   return the_env_manager;
}

/*********************************************************************/
// Return a pointer to the single env_manager object
env_manager *
env_manager::get_handle()
{
   if( the_env_manager == NULL )
   {
      FATAL_ERROR("Called env_manager::get_handle before load_environment.\n");
   }

   return the_env_manager;
}

/*********************************************************************/
// The constructor
env_manager::env_manager(const bool debug_) : 
   theEnvironment(NULL),
   debug(debug_)
{
   /* empty */
}

/*********************************************************************/
// Take down the env_manager
env_manager::~env_manager()
{
   if( the_env_manager != NULL )
   {
      delete the_env_manager;
      the_env_manager = NULL;
   }
}

// *******************************************************************
}
