/**********************************************************************
 **                                                                  **
 **  test.cc                                                         **
 **                                                                  **
 **  Test harness to allow exercising libcmdl_io                     **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: test_loader.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

/**********************************************************************
* $Log: test_loader.cc,v $
* Revision 1.1.1.1  2008/07/14 16:44:15  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:45  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:19:15  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.1  2004/11/12 21:56:59  doug
* renamed loader so doesn't conflict with missionlab
*
* Revision 1.2  2004/11/04 21:39:33  doug
* moved test to util
*
* Revision 1.1  2004/11/04 21:36:39  doug
* moved test.cc to util
*
* Revision 1.10  2004/05/11 19:34:49  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.9  2004/03/09 21:36:08  doug
* promote Blocks to mission segments and procedures
*
* Revision 1.8  2004/02/28 16:35:42  doug
* getting cmdli to work in sara
*
* Revision 1.7  2004/02/20 14:39:35  doug
* cmdl interpreter runs a bit
*
* Revision 1.6  2003/11/10 22:48:31  doug
* working on cmdli
*
* Revision 1.5  2003/11/07 22:54:51  doug
* working on cmdl_io
*
* Revision 1.4  2003/11/07 20:29:27  doug
* added cmdli interpreter library
*
* Revision 1.3  2003/11/03 17:48:13  doug
* compiles and links
*
* Revision 1.2  2003/11/03 16:43:31  doug
* snapshot
*
* Revision 1.1  2003/10/27 17:45:37  doug
* added a test harness
*
**********************************************************************/

#include "mic.h"
#include <fstream>
#include "myGetOpt.h"
#include "environment.h"
#include "cmdl_Root.h"
#include "cmdl_parser.h"
#include "cmdl_loader.h"

using namespace sara;

// *********************************************************************
const char ENV_FILENAME[] = "default.env";

// *********************************************************************
int
main(int argc, char **argv)
{
   const char *env_filename = ENV_FILENAME;
   int cmdl_loader_debuglevel = 0;
   GetOpt getopt(argc, argv, "d:e:n:");
   int c;
   bool verbose = false;
   bool debugLoad = false;
   bool no_env_path_search = false;
   string ourName = "scout";
   while((c=getopt()) != EOF)
   {
      switch(c)
      {
         case 'd':
            cmdl_loader_debuglevel = atoi(getopt.optarg);
            fprintf(stderr,"CCL Loader debug level set to %d\n", cmdl_loader_debuglevel);
            break;

         // Load a non-standard environment file
         case 'e':
            env_filename = getopt.optarg;
            INFORM("Using environment file %s", env_filename);
            no_env_path_search = true;
            break;

         // set our robot name
         case 'n':
            ourName = getopt.optarg;
            INFORM("setting robot name to %s", ourName.c_str());
            break;

	 default:
	   cerr << "Unknown parameter: " << c << endl;
	   exit(1);
      }
   }

   int optIndex = getopt.optind;
   if( optIndex >= argc )
   {
      cerr << endl << "ERROR: Missing controller filename!" << endl << endl;
      exit(1);
   }
   if( optIndex < argc-1 )
   {
      cerr << endl << "ERROR: Extra parameters after controller filename!" << endl << endl;
      exit(1);
   }

   // load the environment
   environment *env = new environment();
   int rtn = env->load_environment(env_filename, no_env_path_search, verbose, debugLoad);
   if( rtn != 0 )
   {
      cerr << endl << "ERROR: Unable to load environment file " << env_filename << endl << endl;
   }

   const string cmdl_filename(argv[optIndex]);
   cerr << "Loading controller from " << cmdl_filename << endl;

   // Create an input stream from the file.
   std::ifstream input( cmdl_filename.c_str() );
   if( !input )
   {
      cerr << "Unable to open the CMDL file " << cmdl_filename << " for reading!" << endl;
      exit(1);
   }

   // Read it in.
   string cmdl_file_buffer;
   char ch;
   while( input.get(ch) )
      cmdl_file_buffer += ch;

   // Process it.
   stringSet legalActions;
   legalActions.insert("Start");
   legalActions.insert("MoveTo");
   legalActions.insert("Loiter");
   legalActions.insert("Recon");

   stringSet legalTriggers;
   legalTriggers.insert("atLocation");

   cmdl *thecmdl = new cmdl(legalActions, legalTriggers, env);

   thecmdl->set_loader_debug((cmdl::cmdl_DEBUG_T)cmdl_loader_debuglevel);

   thecmdl->load(ourName, cmdl_file_buffer );

   thecmdl->set_ourName( ourName );

   // dump it.
   cout << thecmdl->print();
}

//********************************************************************
