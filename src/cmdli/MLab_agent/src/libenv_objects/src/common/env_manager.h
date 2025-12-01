#ifndef ENV_MANAGER_H
#define ENV_MANAGER_H
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
 *********************************************************************/

/* $Id: env_manager.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: env_manager.h,v $
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
// Revision 1.5  2002/09/30 20:12:40  doug
// scaling works
//
// Revision 1.4  2002/09/23 21:41:50  doug
// *** empty log message ***
//
// Revision 1.3  1999/03/11 22:21:08  doug
// Obstacles are working
//
// Revision 1.2  1999/03/10 13:00:02  doug
// compiles
//
// Revision 1.1  1999/03/10 01:35:57  doug
// Initial revision
///////////////////////////////////////////////////////////////////////

#include <mic.h>
#include <list>
#include "env_object.h"
#include "environment.h"

namespace sara
{
class env_manager
{
   public:
      /// Create the env_manager and load the environment file.
      static env_manager *load_env(const string &filename, const bool no_path_search, const bool verbose, const bool debug);

      /// Take down the env_manager
      ~env_manager();

      /// Return a pointer to the singleton env_manager object.
      static env_manager *get_handle();

      /// The set of environmental object
      environment *theEnvironment;

   private:
      /// Hide the constructor: using singleton paradigm
      env_manager(const bool debug); 

      /// Keep track of the singleton object.
      static env_manager *the_env_manager;

      /// Enable debug mode
      bool debug;
};

}
// ********************************************************************
#endif
