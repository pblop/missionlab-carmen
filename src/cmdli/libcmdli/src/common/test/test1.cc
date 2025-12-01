/**********************************************************************
 **                                                                  **
 **  test1: Test harness for cmdli                                   **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1997, Mobile Intelligence Corporation                 **
 **  33150 Schoolcraft Road, Suite 108                               **
 **  Livonia, Michigan  48150-1646                                   **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: test1.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: test1.cc,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:14:58  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.13  2004/04/13 22:29:43  doug
* switched to testing cmdli functions used by upenn
*
* Revision 1.12  2004/04/13 18:00:06  doug
* renamed test.cc to test1.cc to prevent it shadowing system test command
*
* Revision 1.11  2004/04/13 17:57:33  doug
* switch to using internal communications and not passing messages in and out
*
* Revision 1.10  2004/02/28 21:59:17  doug
* preload dies
*
* Revision 1.9  2004/02/28 16:35:43  doug
* getting cmdli to work in sara
*
* Revision 1.8  2004/02/20 20:54:48  doug
* barriers work
*
* Revision 1.7  2004/02/20 14:39:36  doug
* cmdl interpreter runs a bit
*
* Revision 1.6  2004/01/15 00:24:10  doug
* working on cmdl
*
* Revision 1.5  2004/01/09 18:46:04  doug
* back from Florida
*
* Revision 1.4  2003/11/13 22:29:33  doug
* adding transitions
*
* Revision 1.3  2003/11/11 22:04:13  doug
* getting it to execute
*
* Revision 1.2  2003/11/10 22:48:33  doug
* working on cmdli
*
* Revision 1.1.1.1  2003/11/07 20:27:33  doug
* initial import
*
* Revision 1.3  1998/01/24 13:51:57  doug
* Loading is working
*
* Revision 1.2  1998/01/24 10:47:09  doug
* now fsa is the root object and loader is used by it.
*
* Revision 1.1  1998/01/23 13:20:48  doug
* Initial revision
*
**********************************************************************/

#include "mic.h"
#include "myGetOpt.h"
#include "environment.h"
#include "cmdli.h"

const double defaultStepSize = 100.0;

// Exit codes:
//   0   : Successful test
//   <>0 : Test failed

/**********************************************************************/
void failed()
{
   cout << "FAILED cmdli test\n";
   exit(1);
}

// **********************************************************************
/*
typedef class robotRec
{
public:
   /// constructor
   robotRec(cmdli *rec) 
   {
      cpu = rec;
   }

   /// destructor
   ~robotRec() 
   {
      if(cpu) 
      {
         delete cpu;
      } 
      cpu=NULL;
   }

   /// the robot's interpreter
   cmdli *cpu;

};
*/
   /// The robots current location
   Vector location;
/*
typedef map< string, robotRec *, less< string > > robots;
robots robotRecs;
*/

// **********************************************************************
cmdli::TriggerReturnValue_T
trigger_immediate(const keyValueMap &parms)
{
cerr << "trigger_immediate - returning true" << endl;
   return cmdli::TriggerTrue;
}

// **********************************************************************
cmdli::TriggerReturnValue_T
trigger_atLocation(const keyValueMap &parms)
{
cerr << "trigger_atLocation - running with options:" << endl;
   keyValueMap::const_iterator it;
   for(it=parms.begin(); it!=parms.end(); ++it)
   {
      cerr << "   " << it->first << " = " << it->second << endl;
   }

   it = parms.find("location");
   if( it == parms.end() )
   {
      cerr << "trigger_atLocation - Missing 'location' option parameter to denote the destination!" << endl;
      return cmdli::TriggerFailed;
   }

   Vector *dest = Vector::load( it->second );
   if( !dest )
   {
      cerr << "trigger_atLocation - Unable to parse location value '" << it->second << "'" << endl;
      return cmdli::TriggerFailed;
   }

   Vector distance = *dest - location;

   bool atGoal = distance.length_2d() <= defaultStepSize;

   return atGoal ? cmdli::TriggerTrue : cmdli::TriggerFalse;
}

// **********************************************************************
bool action_start(const keyValueMap &parms)
{
cerr << "action_start - running " << endl;
   return true;
}

// **********************************************************************
bool action_moveTo(const keyValueMap &parms)
{
cerr << "action_moveTo - starting at " << location << " with options:" << endl;
   keyValueMap::const_iterator it;
   for(it=parms.begin(); it!=parms.end(); ++it)
   {
      cerr << "   " << it->first << " = " << it->second << endl;
   }

   it = parms.find("location");
   if( it == parms.end() )
   {
      cerr << "action_moveTo - Missing 'location' option parameter to denote the destination!" << endl;
      return false;
   }

   Vector *dest = Vector::load( it->second );
   if( !dest )
   {
      cerr << "action_moveTo - Unable to parse location value '" << it->second << "'" << endl;
      return false;
   }

   Vector step = *dest - location;
   if( step.length_2d() > defaultStepSize )
   {
      step.make_unit_2d();
      step *= defaultStepSize;
   }
   location += step;

cerr << "action_moveTo - ended at " << location << endl;

   return true;
}

// **********************************************************************
bool action_loiter(const keyValueMap &parms)
{
cerr << "action_loiter - running " << endl;
   return true;
}

// **********************************************************************
bool action_stop(const keyValueMap &parms)
{
cerr << "action_stop - running " << endl;
   return true;
}

// **********************************************************************
bool cpuExited;
bool action_exit(const keyValueMap &parms)
{
cerr << "action_Exit - running " << endl;
   cpuExited = true;
   return true;
}

// *********************************************************************

int 
main(int argc, char **argv)
{
   // load the environment
   const char ENV_FILENAME[] = "default.env";
   const char *env_filename = ENV_FILENAME;
   bool verbose = false;
   bool debugLoad = false;
   bool no_env_path_search = false;
   int missionLoaderDebug = cmdli::INTERPRETER | cmdli::SYNCHRONIZATION;

   GetOpt getopt(argc, argv, "n:l");
   int c;
   string ourName = "robot";
   while((c=getopt()) != EOF)
   {
      switch(c)
      {
         // set our robot name
         case 'n':
            ourName = getopt.optarg;
            INFORM("setting robot name to %s", ourName.c_str());
            break;

         // enable mission loading debug
         case 'l':
            missionLoaderDebug |= cmdli::LOADER_YACC | cmdli::LOADER_LEX;
            INFORM("enabling lex and yacc messages during mission loading");
            break;

	 default:
	   cerr << "Unknown parameter: " << c << endl;
	   exit(1);
      }
   }

   string cmdl_filename("mission.cmdl");
   int optIndex = getopt.optind;
   if( optIndex == argc-1 )
   {
      cmdl_filename = argv[optIndex];
   }
   else if( optIndex < argc-1 )
   {
      cerr << endl << "ERROR: Extra parameters after mission filename!" << endl << endl;
//      usage(argv[0]);
      exit(1);
   }

   environment *env = new environment();
   int rtn = env->load_environment(env_filename, no_env_path_search, verbose, debugLoad);
   if( rtn != 0 )
   {
      cerr << endl << "ERROR: Unable to load environment file " << env_filename << endl << endl;
   }

   // build the list of defined triggers
   cmdli::Triggers_T triggers;
   triggers["immediate"] = trigger_immediate;
   triggers["atLocation"] = trigger_atLocation;
   triggers["exit"] = trigger_immediate;

   stringSet triggerNames;
   triggerNames.insert("immediate");
   triggerNames.insert("atLocation");
   triggerNames.insert("exit");

   // build the list of defined actions, along with their default completion triggers
   cmdli::defaultActionCompletionTriggers_T defaultActionCompletionTriggers;
   defaultActionCompletionTriggers["Start"] = "immediate";
   defaultActionCompletionTriggers["MoveTo"] = "atLocation";
   defaultActionCompletionTriggers["Stop"] = "immediate";
   defaultActionCompletionTriggers["Loiter"] = "immediate";
   defaultActionCompletionTriggers["Exit"] = "immediate";

   // build list of action callbacks
   cmdli::Actions_T actionFncs;
   actionFncs["Start"] = action_start;
   actionFncs["MoveTo"] = action_moveTo;
   actionFncs["Stop"] = action_stop;
   actionFncs["Loiter"] = action_loiter;
   actionFncs["Exit"] = action_exit;

   // setup our robot
   cmdli *cpu = new cmdli();
   if( !cpu->init(triggerNames, defaultActionCompletionTriggers, env, ourName) )
//   if( !cpu->init(defaultActionCompletionTriggers, actionFncs, triggers, env, ourName) )
   {
      cerr << "Fatal error initializing the cmdl interpreter!" << endl;
      exit(1);
   }

   cpu->set_debug_mask( missionLoaderDebug );

//   bool ok = cpu->load( buf1 );
   bool ok = cpu->load_from_file( cmdl_filename );
   if( !ok )
   {
//         cout << "Unable to load buf1:\n" << buf1 << endl << "FAILED cmdli test" << endl;;
      cout << "Unable to load mission from '" << cmdl_filename << "'" << endl;
      failed();
   }


   int step = 0;
   int max_steps = 120;
   while( !cpu->isDone() && ++step < max_steps )
   {
      /// If not using trigger callbacks, then call this function to get the list of triggers that
      /// will be used this cycle, then call the step method that passes in the trigger values.
      typedef map<string, keyValueMap> triggerInfo_T; 
      bool getTriggersWillUse(triggerInfo_T &triggerInfo);

      // get the list of trigges that will be used this cycle
      triggerInfo_T RequiredTriggers; 
      if( !cpu->getTriggersWillUse(RequiredTriggers) )
      {
         cout << "Unable to get list of required triggers" << endl;
	 break;
      }

      // load the trigger values
      cmdli::triggerValues_T triggerValues;
      triggerInfo_T::iterator it;
      for(it=RequiredTriggers.begin(); it!=RequiredTriggers.end(); ++it)
      {
         cmdli::Triggers_T::iterator trig = triggers.find(it->first);
	 if( trig == triggers.end() )
	 {
            cout << "Request for unknown trigger '" << it->first << "' from getTriggersWillUse!" << endl;
	    break;
	 }

         cmdli::TriggerReturnValue_T rtn = (*trig->second)(it->second);

         cout << "setting trigger '" << it->first << "' to " << rtn << endl;
	 triggerValues[ it->first ] = rtn == cmdli::TriggerTrue;
      }

      // step it
      string outputAction;
      keyValueMap outputParameters;

      if( !cpu->step(triggerValues, outputAction, outputParameters) )
      {
	 // oops, had an error.
	 cerr << "cpu step returned an error!" << endl;
	 break;
      }

      // run the action
      cmdli::Actions_T::iterator act = actionFncs.find(outputAction);
      if( act == actionFncs.end() )
      {
	 cout << "Request for unknown action '" << outputAction << "' from step!" << endl;
	 break;
      }

      if( !(*act->second)(outputParameters) )
      {
	 cout << "action '" << outputAction << "' return error!" << endl;
	 break;
      }


#if defined(linux)
      sleep(2);
#elif defined(WIN32)
      Sleep(2*1000);
#else
#error "undefined architecture!"
#endif
   }

   if( step >= max_steps )
   {
      cout << "FAILED cmdli test - timed out after " << max_steps << " steps!" << endl;
      return 1;
   }

   cout << "SUCESSFUL cmdli test" << endl;
   return 0;
}

