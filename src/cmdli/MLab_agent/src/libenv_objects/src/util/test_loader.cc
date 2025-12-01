/**********************************************************************
 **                                                                  **
 **  test_env_loader.cc                                              **
 **                                                                  **
 **  Test harness to allow exercising libenv                         **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: test_loader.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: test_loader.cc,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:21:05  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
**********************************************************************/

#include "mic.h"
#include <fstream>
#include "myGetOpt.h"
#include "environment.h"

using namespace sara;

// *********************************************************************
int
main(int argc, char **argv)
{
   int env_debuglevel = 0;
   GetOpt getopt(argc, argv, "d:");
   int c;
   bool verbose = false;
   bool debugLoad = false;
   bool no_env_path_search = false;
   while((c=getopt()) != EOF)
   {
      switch(c)
      {
         case 'd':
            env_debuglevel = atoi(getopt.optarg);
            fprintf(stderr,"Environment loader debug level set to %d\n", env_debuglevel);
            break;

	 default:
	   cerr << "Unknown parameter: " << c << endl;
	   exit(1);
      }
   }

   int optIndex = getopt.optind;
   if( optIndex >= argc )
   {
      cerr << endl << "ERROR: Missing environment filename!" << endl << endl;
      exit(1);
   }
   if( optIndex < argc-1 )
   {
      cerr << endl << "ERROR: Extra parameters after environment filename!" << endl << endl;
      exit(1);
   }

   const string env_filename(argv[optIndex]);
   cerr << "Loading environment file " << env_filename << endl;

   // load the environment
   environment *env = new environment();
   int rtn = env->load_environment(env_filename, no_env_path_search, true, env_debuglevel);
   if( rtn != 0 )
   {
      cerr << endl << "ERROR: Unable to load environment file " << env_filename << endl << "returned error " << rtn << endl;
   }

   // dump it.
//   cout << env->print();
}

//********************************************************************
