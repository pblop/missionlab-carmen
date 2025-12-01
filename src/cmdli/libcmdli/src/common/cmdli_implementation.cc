/**********************************************************************
 **                                                                  **
 **  cmdli_implementation.cc                                                        **
 **                                                                  **
 **  The cmdl interpreter                                            **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: cmdli_implementation.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: cmdli_implementation.cc,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:45  endo
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
* Revision 1.12  2004/11/12 21:55:16  doug
* tweaks for final Ft. Benning
*
* Revision 1.11  2004/11/06 17:35:12  doug
* cleanup
*
* Revision 1.10  2004/11/06 01:11:59  doug
* snap
*
* Revision 1.9  2004/11/04 22:33:36  doug
* working on mlab
*
* Revision 1.8  2004/10/29 22:38:38  doug
* working on waittimeout
*
* Revision 1.7  2004/10/22 21:40:31  doug
* added goto and command exception support
*
* Revision 1.6  2004/09/21 18:30:58  doug
* fixed typo
*
* Revision 1.5  2004/09/21 17:27:44  doug
* added more debug
*
* Revision 1.4  2004/09/21 14:51:49  doug
* add more debugging, remove BlockIsDone, and don't loop if pastBarrier.
*
* Revision 1.3  2004/09/14 21:47:37  doug
* fixed rc names to make more meaningful
*
* Revision 1.2  2004/07/30 13:42:15  doug
* handles comm timeouts and resyncs
*
* Revision 1.1  2004/05/11 19:40:17  doug
* made cmdli an abstract base class to limit the amount of stuff pulled in by users (USC and Mlab)
*
* Revision 1.29  2004/04/30 17:36:01  doug
* works for UPenn
*
* Revision 1.28  2004/04/27 18:36:07  doug
* fixed build problems with win32
*
* Revision 1.27  2004/04/27 16:19:34  doug
* added support for multicast as unicast
*
* Revision 1.26  2004/04/20 20:10:23  doug
* cleanup debug code
*
* Revision 1.25  2004/04/20 20:08:24  doug
* cleanup debug code
*
* Revision 1.24  2004/04/20 19:43:58  doug
* fix comment
*
* Revision 1.23  2004/04/13 22:29:43  doug
* switched to testing cmdli_implementation functions used by upenn
*
* Revision 1.22  2004/04/13 17:57:33  doug
* switch to using internal communications and not passing messages in and out
*
* Revision 1.21  2004/03/22 18:42:16  doug
* coded block transitions
*
* Revision 1.20  2004/03/19 22:48:22  doug
* working on exceptions.
*
* Revision 1.19  2004/03/19 15:30:05  doug
* call and return work
*
* Revision 1.18  2004/03/18 22:11:33  doug
* coded call and return
*
* Revision 1.17  2004/03/17 22:17:11  doug
* working on procedure calls
*
* Revision 1.16  2004/03/15 22:33:34  doug
* working on cutting over to ActionCommand
*
* Revision 1.15  2004/03/12 20:36:59  doug
* snap
*
* Revision 1.14  2004/03/09 22:01:04  doug
* promote Blocks to mission segments and procedures
*
* Revision 1.13  2004/03/08 14:52:35  doug
* cross compiles on visual C++
*
* Revision 1.12  2004/03/01 23:37:53  doug
* working on real overlay
*
* Revision 1.11  2004/03/01 00:47:16  doug
* cmdli_implementation runs in sara
*
* Revision 1.10  2004/02/28 21:59:17  doug
* preload dies
*
* Revision 1.9  2004/02/28 16:35:43  doug
* getting cmdli_implementation to work in sara
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
* working on cmdli_implementation
*
* Revision 1.1.1.1  2003/11/07 20:27:33  doug
* initial import
*
**********************************************************************/

#include <fstream>
#include <stdio.h>
#include "mic.h"
#include "objcomms.h"
#include "rawMsg.h"
#include "MsgTypes.h"
#include "CMDLstatus.h"
#include "CMDLcommand.h"
#include "CMDLcommandAck.h"

//#include "Command_extra.h"
//#include "extra_command.h"
//#include "cmdl_extra.h"
//#include "extra_cmdl.h"
#include "Command.h"
#include "ActionCommand.h"
#include "CallCommand.h"
#include "GotoCommand.h"
#include "SyncCommand.h"
#include "ReturnCommand.h"
#include "environment.h"
#include "cmdli_implementation.h"
#include "TimeOfDay.h"

#if defined(ERROR)
#undef ERROR
#endif

#ifdef WIN32
#include <direct.h>
#define getcwd _getcwd
#endif

namespace sara
{
static const string DEFAULT_MULTICAST_ADDR("224.0.1.150:12345");
static const ushort DEFAULT_UNICAST_PORT(12345);
static bool comms_compression(true);
static uint comms_debug(0);

// Init to default timeout

static TimeOfDay CommTimeout( 10.0 * 60.0 ); // 10 minutes
static TimeOfDay WaitTimeout( 24.0 * 60.0 * 60.0 ); // 24 hours
static TimeOfDay startedWait;

static const string RC_MULTICAST_ADDR("MulticastAddr");
static const string RC_UNICAST_PORT("UnicastPort");
static const string RC_UNICASTADDRS("RobotAddresses");
static const string RC_COMMSDEBUG("CommsDebug");
static const string RC_DEBUG("Debug");
static const string RC_COMMSTIMEOUT("CommsTimeout");
static const string RC_WAITTIMEOUT("WaitTimeout");
static const string RC_SYNCMSGPERIOD("SyncMsgPeriod");
static const string RC_NORMALMSGPERIOD("NormalMsgPeriod");
static const string RC_COMMSCOMPRESSION("CommsCompression");

/*********************************************************************/
/// Our name
string cmdli_implementation::ourName;

/*********************************************************************/
/// Constructor for the cmdl interpreter object.
cmdli_implementation::cmdli_implementation() :
   debug_mask(NONE),
   thecmdl(NULL),
   theEnvironment(NULL),
   done(false),
   statusValue(CMDLstatus::CMDEXECUTING),
   loiterCmdName("Loiter"),
   loiterCmd(NULL),
   normalPeriod(5.0),
   syncingPeriod(1.0),
   comobj(NULL),
   runState(RS_RUNNING),
   rc_filename("cmdli.rc")
{
   // Load the resource file, if there is one
   int rtn = rc_table.load(rc_filename.c_str(), true, false, false);
   if( rtn == 0 )
   {
      INFORM("Loaded resource file %s", rc_filename.c_str());
   }
   else if( rtn == 1 )
   {
      INFORM("Syntax error in resource file %s", rc_filename.c_str());
   }
   else
   {
      char *dirname = getcwd(NULL, 0);
      INFORM("Did not find a response file.  Looked for %s in %s", rc_filename.c_str(), dirname);
      free(dirname);
   }

   // Init the resources we use with default values if they were not loaded.
   if( !rc_table.check_exists(RC_MULTICAST_ADDR) )
      rc_table.set_string(RC_MULTICAST_ADDR, DEFAULT_MULTICAST_ADDR);
   if( !rc_table.check_exists(RC_UNICAST_PORT) )
      rc_table.set_int(RC_UNICAST_PORT, DEFAULT_UNICAST_PORT);
   int tmp;
   if( rc_table.get_int(RC_DEBUG, tmp) )
   {
      debug_mask = tmp;

      stringstream str;
      str << "Setting Debug to " << debug_mask << endl;
      INFORM(str.str().c_str());
   }
   if( rc_table.get_int(RC_COMMSDEBUG, tmp) )
   {
      comms_debug = tmp;
      if( debug_mask & INTERPRETER )
      {
         stringstream str;
         str << "Setting CommsDebug to " << comms_debug;
         INFORM(str.str().c_str());
      }
   }
   double dval;
   if( rc_table.get_double(RC_COMMSTIMEOUT, dval) )
   {
      CommTimeout = (TimeOfDay)dval;
      if( debug_mask & INTERPRETER )
      {
         stringstream str;
         str << "Setting default CommsTimeout to " << CommTimeout << endl;
         INFORM(str.str().c_str());
      }
   }
   if( rc_table.get_double(RC_WAITTIMEOUT, dval) )
   {
      WaitTimeout = (TimeOfDay)dval;
      if( debug_mask & INTERPRETER )
      {
         stringstream str;
         str << "Setting default WaitTimeout to " << WaitTimeout << endl;
         INFORM(str.str().c_str());
      }
   }
   if( rc_table.get_double(RC_NORMALMSGPERIOD, dval) )
   {
      normalPeriod = (TimeOfDay)dval;
      if( debug_mask & INTERPRETER )
      {
         stringstream str;
         str << "Setting " << RC_NORMALMSGPERIOD << " to " << normalPeriod << endl;
         INFORM(str.str().c_str());
      }
   }
   if( rc_table.get_double(RC_SYNCMSGPERIOD, dval) )
   {
      syncingPeriod = (TimeOfDay)dval;
      if( debug_mask & INTERPRETER )
      {
         stringstream str;
         str << "Setting " << RC_SYNCMSGPERIOD << " to " << syncingPeriod << endl;
         INFORM(str.str().c_str());
      }
   }
   tmp = rc_table.check_bool(RC_COMMSCOMPRESSION);
   if( tmp == 0 || tmp == 1 )
   {
      comms_compression = (tmp == 1);
      if( debug_mask & INTERPRETER )
      {
         stringstream str;
         str << "Setting " << RC_COMMSCOMPRESSION << " to " << comms_compression << endl;
         INFORM(str.str().c_str());
      }
   }
}

// *********************************************************************
cmdli_implementation::statusRec::statusRec(CMDLstatus *rec) :
   record(rec),
   waitingForUs(false)
{
   arrivalTime = TimeOfDay::now(); 
   waitingForUs = false;
   if( rec && !rec->blocklist.empty() )
   {
      for(uint i=0; i<rec->blocklist.size(); i++)
      {
         if( !cmp_nocase(rec->blocklist[i], ourName) )
         {
            waitingForUs = true;
            break;
         }
      }
   }
}

// *********************************************************************
/// First method to init the cmdl interpreter object.
/// This instance directly invokes triggers and actions.
/// returns true on success, false otherwise
bool 
cmdli_implementation::init(const defaultActionCompletionTriggers_T &defaultTriggers, Actions_T &actionCallbacks, Triggers_T &legalTriggers, environment *env, const string &name)
{
   ourName = name;
INFORM("cmdli_implementation::init1 - ourName='%s'", ourName.c_str());
   theEnvironment = env;

   // copy over the triggers
   triggerCallbacks = legalTriggers;
   for(Triggers_T::iterator it=triggerCallbacks.begin(); it!=triggerCallbacks.end(); ++it)
      triggers.insert( it->first );

   // copy over the actions, making sure the triggers are valid
   for(defaultActionCompletionTriggers_T::const_iterator it=defaultTriggers.begin(); it!=defaultTriggers.end(); ++it)
   {
      if( triggers.find( it->first ) != triggers.end() )
      {
         return false;
      }

      defaultCompletionTriggers[it->first] = it->second;
      actions.insert( it->first );
   }

   // populate our map of callback functions
   actionFncs = actionCallbacks;

   // make sure we have a loiter command to emit when blocked at barriers
   Actions_T::iterator lit;
   if( (lit = actionFncs.find(loiterCmdName)) == actionFncs.end() )
   {
      ERROR("cmdli_implementation::init - You must define a 'Loiter' action that will be emitted while blocked at barriers");
      return false;
   }
   loiterCmd = lit->second;

   // remember how we were called
   directMode = true;

   return initComms(comms_compression, comms_debug);
}

// *********************************************************************
/// Second method to init the cmdl interpreter object.
/// This instance calls a mediating function to invoke triggers and actions.
/// triggers is the set of legal trigger names
/// defaultActionCompletionTriggers is a map of action names to the name of their default termination triggers.
/// returns true on success, false otherwise
bool 
cmdli_implementation::init(const stringSet &triggersIn, const defaultActionCompletionTriggers_T &defaultActionCompletionTriggers, executeActionFnc_T executeActionIn, void *actionUserDataIn, executeTriggerFnc_T executeTriggerIn, void *triggerUserDataIn, environment *env, const string &name)
{
   ourName = name;
INFORM("cmdli_implementation::init2 - ourName='%s'", ourName.c_str());
   theEnvironment = env;
   executeActionFnc = executeActionIn;
   actionUserData = actionUserDataIn;
   executeTriggerFnc = executeTriggerIn;
   triggerUserData = triggerUserDataIn;

   // copy over the triggers
//   triggerCallbacks = legalTriggers;
//   for(Triggers_T::iterator it=triggerCallbacks.begin(); it!=triggerCallbacks.end(); ++it)
//      triggers.insert( it->first );
   triggers = triggersIn;

   // copy over the actions, making sure the triggers are valid
   for(defaultActionCompletionTriggers_T::const_iterator it=defaultActionCompletionTriggers.begin(); it!=defaultActionCompletionTriggers.end(); ++it)
   {
      if( triggers.find( it->first ) != triggers.end() )
      {
         return false;
      }

      defaultCompletionTriggers[it->first] = it->second;
      actions.insert( it->first );
   }

   // make sure we have a loiter command to emit when blocked at barriers
   stringSet::iterator lit;
   if( (lit = actions.find(loiterCmdName)) == actions.end() )
   {
      ERROR("cmdli_implementation::init - You must define a 'Loiter' action that will be emitted while blocked at barriers");
      return false;
   }

   // remember how we were called
   directMode = false;

   return initComms(comms_compression, comms_debug);
}

// *********************************************************************
/// Third method to init the cmdl interpreter object.
/// This instance relies on the second form of step
/// defaultActionCompletionTriggers is a map of action names to the name of their default termination triggers.
/// returns true on success, false otherwise
bool 
cmdli_implementation::init(const stringSet &triggersIn, const defaultActionCompletionTriggers_T &defaultActionCompletionTriggers, environment *env, const string &myName)
{
   ourName = myName;
INFORM("cmdli_implementation::init3 - ourName='%s'", ourName.c_str());
   theEnvironment = env;
   executeActionFnc = NULL;
   actionUserData = NULL;
   executeTriggerFnc = NULL;
   triggerUserData = NULL;

   triggers = triggersIn;

   // copy over the actions, making sure the triggers are valid
   for(defaultActionCompletionTriggers_T::const_iterator it=defaultActionCompletionTriggers.begin(); it!=defaultActionCompletionTriggers.end(); ++it)
   {
      if( triggers.find( it->first ) != triggers.end() )
      {
         return false;
      }

      defaultCompletionTriggers[it->first] = it->second;
      actions.insert( it->first );
   }

   // make sure we have a loiter command to emit when blocked at barriers
   stringSet::iterator lit;
   if( (lit = actions.find(loiterCmdName)) == actions.end() )
   {
      ERROR("cmdli_implementation::init - You must define a 'Loiter' action that will be emitted while blocked at barriers");
      return false;
   }

   return initComms(comms_compression, comms_debug);
}

// *********************************************************************
/// Internal function to initialize the comms subsystem
/// returns true on success, false otherwise
bool 
cmdli_implementation::initComms(bool compression, uint debug)
{
   // create the comm object
   comobj = new objcomms();

   // select the requested debug level
   comobj->setDebug(debug);
//   comobj->setDebug(7);
   
   // if specified, set multicast as unicast
   if( rc_table.check_exists(RC_UNICASTADDRS) )
   {
      if( !comobj->loadMemberAddresses( rc_table.get(RC_UNICASTADDRS) ) )
      {
         ERROR("cmdli_implementation::initComms - Unable to load the robot addresses from the rc file!");
      }
INFORM("cmdli_implementation::initComms - setting multicast as unicast team member addresses to:");
strings addrs = rc_table.get(RC_UNICASTADDRS);
for(uint i=0; i<addrs.size(); i++)
{
   stringstream str;
   str << addrs[i] << endl;
   PRINT( "   %s", str.str().c_str());
}
   }

   string multicastAddress;
   if( !rc_table.get_string(RC_MULTICAST_ADDR, multicastAddress) )
   {
      ERROR("cmdli_implementation::initComms - Internal Error: multicast address is undefined!");
      return(false);
   }

   int tmp;
   if( !rc_table.get_int(RC_UNICAST_PORT, tmp) )
   {
      ERROR("cmdli_implementation::initComms - Internal Error: unicast port is undefined!");
      return(false);
   }
   if( tmp <= 0 || tmp > 65535 )
   {
      ERROR("cmdli_implementation::initComms - invalid port number %d!", tmp);
      return(false);
   }
   ushort unicastPort = (ushort)tmp;

   // open it
// for laptop
//   if( !comobj->open(multicastIP, port, "", 7) )
   if( !comobj->open(multicastAddress, unicastPort, ourName) )
   {
      ERROR("cmdli_implementation::initComms - Error opening comobj!");
      return(false);
   }

   if( compression )
   {
      // Turn compression on
      comobj->enableCompression();
   }
   else
   {
      // compression off
      comobj->disableCompression();
   }
   
   // start with encription off
   comobj->disableEncryption();

   // turn encryption on
//   comobj->enableEncryption("This Is A Weak Key!");

   // Register the packets we will use
   comobj->attachHandler(MsgTypes::CMDLSTATUS, CMDLstatus::from_stream);
   comobj->attachHandler(MsgTypes::CMDLCOMMAND, CMDLcommand::from_stream);

   // Register our callback function
   comobj->subscribeTo(MsgTypes::CMDLSTATUS, updateStatus, this);
   comobj->subscribeTo(MsgTypes::CMDLCOMMAND, processCommand, this);

   // good
   return true;
}

// *********************************************************************
/// Destructor
cmdli_implementation::~cmdli_implementation()
{
   // Get rid of the cmdl object.
   if( thecmdl )
   {
      delete thecmdl;
      thecmdl = NULL;
   }
}

/*********************************************************************/
/// Set the debug level mask for the interpreter.
///    NONE          : no debugging
///    LOADER_USER   : user messages from the loader.
///    LOADER_YACC   : yacc and lex messages from loader.
///    STATE_MACHINE : state transition debugging messages.
void 
cmdli_implementation::set_debug_mask(const uint _debug_mask)
{
   debug_mask = _debug_mask;
}

/*********************************************************************/
/// Load the cmdl from the string buffer
/// No changes are made unless it returns true.
bool
cmdli_implementation::load(const string &cmdlDesc)
{
   // Create a temp object, in case the load fails.
   stringSet triggerNames;
   stringSet::iterator it; 
   for(it =triggers.begin(); it != triggers.end(); ++it)
      triggerNames.insert(*it);

   cmdl *p = new cmdl(actions, triggerNames, theEnvironment);

   p->set_loader_debug((cmdl::cmdl_DEBUG_T)debug_mask);

   bool ok = p->load(ourName, cmdlDesc );
   if( !ok )
   {
      if( debug_mask & LOADER_USER )
      {
         ERROR("cmdli_implementation::load - Unable to load the cmdl.");
         ERROR("cmdli_implementation::load - Previous cmdl unchanged.");
      }
      
      delete p;
      return false;
   }

   if( debug_mask & LOADER_USER )
   {
      stringstream out;
      out << "cmdli_implementation::load - Available actions:" << endl;
      out << p->listActions() << endl << endl;

      out << "cmdli_implementation::load - Available triggers:" << endl;
      out << p->listTriggers() << endl << endl;

      out << "cmdli_implementation::load - The cmdl is:" << endl;
      out << p->print() << endl;

      INFORM(out.str().c_str());
   }

   // Fixup the version numbers.
   // We always want them to get bigger or for the filename to change.
   if( thecmdl != NULL )
   {   
      if( p->getMissionName() == thecmdl->getMissionName() &&
	  p->getMissionVersion() <= thecmdl->getMissionVersion() )
      {
	 p->setMissionVersion( thecmdl->getMissionVersion() + 1 );
      }
   }

   // Switch to the new cmdl
   if( thecmdl != NULL )
      delete thecmdl;
   thecmdl = p; 

   // Skip to the first instruction for us
   if( !firstCommand() )
   {
      // That was the the last command
      stringstream out;
      out << "cmdli_implementation::load(" << ourName << ") - cmdl invalid: we are not listed in the mission!.";
      ERROR(out.str().c_str());
      return false;
   } 

   return true;
}

/*********************************************************************/
/// Load the cmdl from the specified file.
/// No changes are made unless it returns true.
bool 
cmdli_implementation::load_from_file(const string &filename)
{
   if( debug_mask & LOADER_USER)
   {
      INFORM("cmdli_implementation::load_from_file - starting to load file '%s'", filename.c_str());
   }

   // Try to open the file.
   ifstream in;
   in.open( filename.c_str() );
   stringstream buf;

   char c;
   while( in.get(c) )
   {
      buf.put(c);
   }

   // Call the loader
   bool rtn = load(buf.str());

   if( debug_mask & LOADER_USER)
   {
      INFORM("cmdli_implementation::load_from_file - %s", filename.c_str(), rtn ? "SUCCEEDED" : "FAILED");
   }

   return rtn;
}

// ********************************************************************
/// Based on the status message, are we on same command as the other robot?
bool
cmdli_implementation::equivalentStep( const CMDLstatus &msg ) const
{
   return cmp_nocase(thecmdl->getMissionName(), msg.mission) == 0 &&
          thecmdl->getMissionVersion() == msg.version &&
          thecmdl->getActiveBlock()->get_label() == msg.activeBlock &&
          thecmdl->getActiveCommand() == msg.activeCommand;
}

// ********************************************************************
/// Get the value of a trigger
bool 
cmdli_implementation::checkTrigger(const string &name, const keyValueMap &options)
{
   bool val;

   if( debug_mask & INTERPRETER )
   {
      stringstream out;
      for(keyValueMap::const_iterator it = options.begin(); it != options.end(); ++it)
      {
         out << "   " << it->first << " = " << it->second << endl;
      }
      sara::INFORM("Checking trigger %s with parameters:\n%s", name.c_str(), out.str().c_str());
   }

   switch( callMode )
   {
      case PreviewMode:
         {
            (*triggersUsed)[name] = options;
            val = false;
         }
         break;

      case CachedMode:
         {
            triggerValues_T::iterator it = inputTriggerValues.find( name );
            if( it == inputTriggerValues.end() )
            {
               stringstream out;
               out << "cmdli_implementation::checkTrigger - Internal Error: No defined value trigger '" << name << "'";
               ERROR(out.str().c_str());
               return false;
            }
   
            val = it->second == TriggerTrue;
         }
         break;

      case DirectMode:
         {
            // get the callback functions
            Triggers_T::iterator it = triggerCallbacks.find( name );
            if( it == triggerCallbacks.end() )
            {
               stringstream out;
               out << "cmdli_implementation::checkTrigger - Internal Error: No defined callback function for trigger '" << name << "'";
               ERROR(out.str().c_str());
               return false;
            }
   
            TriggerCallback fnc = it->second;
            if( !fnc ) 
            {
               stringstream out;
               out << "cmdli_implementation::checkTrigger - Internal Error: NULL callback function for trigger '" << name << "'";
               ERROR(out.str().c_str());
               return false;
            }
      
            // run it
            TriggerReturnValue_T rtn = (*fnc)( options );
            if( rtn == TriggerFailed )
            {
               stringstream out;
               out << "cmdli_implementation::checkTrigger - Fatal error reported by trigger '" << name << "'";
               ERROR(out.str().c_str());
               return false;
            }


            val = rtn == TriggerTrue;
            if( debug_mask & (INTERPRETER | SYNCHRONIZATION) )
            {
               if( val )
                  sara::INFORM("cmdli - trigger returned true!");
            }
         }
         break;

      case IndirectMode:
         {
            if( !executeTriggerFnc )
            {
               stringstream out;
               out << "cmdli_implementation::checkTrigger - Internal Error: No defined trigger callback function";
               ERROR(out.str().c_str());
               return false;
            }

            // run it
            TriggerReturnValue_T rtn = (*executeTriggerFnc)( triggerUserData, name, options );
            if( rtn == TriggerFailed )
            {
               stringstream out;
               out << "cmdli_implementation::checkTrigger - Fatal error reported by trigger '" << name << "'";
               ERROR(out.str().c_str());
               return false;
            }
      
            val = rtn == TriggerTrue;
         }
         break;
   };

   // send them back the output that the trigger returned
   return val;
}

// ********************************************************************
/// execute the action
bool 
cmdli_implementation::executeAction(const string &name, const keyValueMap &options)
{
   if( debug_mask & INTERPRETER )
   {
      stringstream out;
      for(keyValueMap::const_iterator it = options.begin(); it != options.end(); ++it)
      {
         out << "   " << it->first << " = " << it->second << endl;
      }
      sara::INFORM("Executing action %s with parameters:\n%s", name.c_str(), out.str().c_str());
   }

   switch( callMode )
   {
      case PreviewMode:
         {
            /* empty */
         }
         break;

      case CachedMode:
         {
            actionName = name;
            actionOptions = options;
         }
         break;

      case DirectMode:
         {
            // get the callback functions
            Actions_T::iterator it = actionFncs.find( name );
            if( it == actionFncs.end() )
            {
               stringstream out;
               out << "cmdli_implementation::executeAction - Internal Error: No defined callback function for action '" << name << "'";
               ERROR(out.str().c_str());
               return false;
            }
   
            ActionCallback fnc = it->second;
            if( !fnc ) 
            {
               stringstream out;
               out << "cmdli_implementation::executeAction - Internal Error: NULL callback function for action '" << name << "'";
               ERROR(out.str().c_str());
               return false;
            }
      
            // run it
            if( ! (*fnc)( options ) )
            {
               stringstream out;
               out << "cmdli_implementation::executeAction - Fatal error reported by action '" << name << "'";
               ERROR(out.str().c_str());
               return false;
            }
         }
         break;

      case IndirectMode:
         {
            if( !executeActionFnc )
            {
               ERROR("cmdli_implementation::executeAction - Internal Error: No defined action callback function");
               return false;
            }

            // run it
            if( ! (*executeActionFnc)( actionUserData, name, options ) )
            {
               stringstream out;
               out << "cmdli_implementation::executeAction - Fatal error reported by action '" << name << "'";
               ERROR(out.str().c_str());
               return false;
            }
         }
         break;
   };

   return true;
}

// ********************************************************************
/// Do we need to use a barrier to sync operations with the group before
/// moving to the next command in the mission?
bool 
cmdli_implementation::needSync() const
{
   // get the block 
   Block *block = thecmdl->getActiveBlock();
   if( !block )
   {
      ERROR("cmdli_implementation::needSync - cmdl invalid: NULL active block.");
      return false;
   }

   // get the command 
   uint cmdIndex = thecmdl->getActiveCommand();
   Command *cmd = block->getCommand( cmdIndex );
   if( !cmd )
   {
      ERROR("cmdli_implementation::needSync - cmdl invalid: NULL active command.");
      return false;
   }

   // are there multiple robots on this command?
   if( cmd->robotNameExists( ourName ) && cmd->numRobotNames() > 1 )
   {
      // Yup, need to sync
      return true;
   }

   // are we at the end of the block?
   if( !block->isLastCommand( cmdIndex ) )
   {
      // nope, get on with it.
      return false;
   }

   // are there multiple robots on this block?
   if( block->robotNameExists( ourName ) && block->numRobotNames() > 1 )
   {
      // Yup, need to sync
      return true;
   }

   return false;
}

// ********************************************************************
/// Step the mission to the first command for us to execute.
/// Returns: 
///   true on success, command is ready to execute
///   false if there are no commands for us in the mission
bool 
cmdli_implementation::firstCommand()
{
   bool forus;
   do
   {
      // get the block 
      Block *block = thecmdl->getActiveBlock();
      if( !block )
      {
         ERROR("cmdli_implementation::firstCommand - cmdl invalid: NULL active block.");
         return false;
      }

      // get the command 
      uint cmdIndex = thecmdl->getActiveCommand();
      Command *cmd = block->getCommand( cmdIndex );
      if( !cmd )
      {
         ERROR("cmdli_implementation::firstCommand - cmdl invalid: NULL active command.");
         return false;
      }

      // Are we mentioned?
      forus = cmd->robotNameExists( ourName );

      if( !forus )
      {
         // increment to the next command
         if( !thecmdl->incActiveCommand() )
         {
            // oops, off the mission
            return false;
         }
      }
   } while( !forus );

   return true;
}

// ********************************************************************
/// Step the mission to the next command for us to execute.
/// Returns: 
///   true on success, command is ready to execute
///   false if the old command was the last one in the mission
bool 
cmdli_implementation::nextCommand()
{
   if( debug_mask & INTERPRETER )
   {
      stringstream out;
      out << "cmdli_implementation::nextCommand - old: block=" << thecmdl->getActiveBlock()->get_label() << " cmd=" << thecmdl->getActiveCommand();
      INFORM(out.str().c_str());
   }

   bool forus;
   do
   {
      // increment to the next command
      if( !thecmdl->incActiveCommand() )
      {
         // oops, off the end of the mission
         return false;
      }

      // get the (maybe just changed, above) block 
      Block *block = thecmdl->getActiveBlock();
      if( !block )
      {
         ERROR("cmdli_implementation::needSync - cmdl invalid: NULL active block.");
         return false;
      }

      if( debug_mask & INTERPRETER )
      {
         stringstream out;
         out << "cmdli_implementation::nextCommand - after inc: block=" << thecmdl->getActiveBlock()->get_label() << " cmd=" << thecmdl->getActiveCommand();
         INFORM(out.str().c_str());
      }

      // get the new command 
      uint cmdIndex = thecmdl->getActiveCommand();
      Command *cmd = block->getCommand( cmdIndex );
      if( !cmd )
      {
         ERROR("cmdli_implementation::needSync - cmdl invalid: NULL active command.");
         return false;
      }

      // Are we mentioned?
      forus = cmd->robotNameExists( ourName );

      if( !forus && debug_mask & INTERPRETER )
      {
         stringstream out;
         out << "cmdli_implementation::nextCommand - NOT FOR US (" << ourName << "): List=";
	 stringSet names(cmd->getRobotNames());
	 for( stringSet::iterator it = names.begin(); it != names.end(); ++it)
	 {
	    out << " " << *it;
	 }
         INFORM(out.str().c_str());
      }
   } while( !forus );

   if( debug_mask & INTERPRETER )
   {
      stringstream out;
      out << "cmdli_implementation::nextCommand - new: block=" << thecmdl->getActiveBlock()->get_label() << " cmd=" << thecmdl->getActiveCommand();
      INFORM(out.str().c_str());
   }

   return true;
}

// ********************************************************************
/// If not using trigger callbacks, then call this function to get the list of triggers that
/// will be used this cycle, then call the step method that passes in the trigger values.
/// Note: triggerInfo should be empty before the call.
typedef map<string, keyValueMap> triggerInfo_T; 
bool 
cmdli_implementation::getTriggersWillUse(triggerInfo_T &triggerInfo)
{
   // cache a handle
   triggersUsed = &triggerInfo;

   // remember how we were called
   callMode = PreviewMode;

   // calculate them
   internalStep();

   return true;
}

// ********************************************************************
/// Run the interpreter one cycle using the specified value of the triggers.
/// Should have called getTriggersWillUse before this call to get the required triggers.
/// messages is the string vector of status messages that have arrived since the last step
/// Returns true on success, false if the cpu had an error
typedef map<string, bool> triggerValues_T; 
bool 
cmdli_implementation::step(const triggerValues_T &triggerValues, string &outputAction, keyValueMap &outputParameters)
{
   // remember how we were called
   callMode = CachedMode;
/*
   if( debug_mask & (INTERPRETER | SYNCHRONIZATION) )
   {
      stringstream out;
      out << "cmdli_implementation::step(" << ourName << ") - starting" << endl;
      latestStatus_T::iterator it;

      // get access
      statusLock.p();

      for(it=latestStatus.begin(); it != latestStatus.end(); ++it)
      {
         out << "   " << *it->second->record << " Age=" << TimeOfDay::now() - it->second->arrivalTime << endl;
      }

      // release access
      statusLock.v();

      out << endl;
      INFORM(out.str().c_str());
   }
*/
   // update our triggers
   inputTriggerValues = triggerValues;

   // run the guts of the step
   internalStep();

   // load the action
   outputAction = actionName;
   outputParameters = actionOptions;

   // maybe send an output message
   if( !sendStatusMessage() )
   {
      ERROR("cmdli_implementation::step - Communications error!");
      return false;
   }

   return true;
}

// ********************************************************************
/// Run the interpreter one cycle.
/// messages is the string vector of status messages that have arrived since the last step
/// Returns true on success, false if the cpu had an error
bool 
cmdli_implementation::step()
{
   // setup our callback method.
   callMode = directMode ? DirectMode : IndirectMode;
/*
   if( debug_mask & (INTERPRETER | SYNCHRONIZATION) )
   {
      stringstream out;
      out << "cmdli_implementation::step(" << ourName << ") - starting" << endl;
      latestStatus_T::iterator it;

      // get access
      statusLock.p();

      for(it=latestStatus.begin(); it != latestStatus.end(); ++it)
      {
         out << "   " << *it->second->record << " Age=" << TimeOfDay::now() - it->second->arrivalTime << endl;
      }

      // release access
      statusLock.v();

      out << endl;
      INFORM(out.str().c_str());
   }
*/
   // run the guts of the step
   internalStep();

   // maybe send an output message
   if( !sendStatusMessage() )
   {
      ERROR("cmdli_implementation::step - Communications error!");
      return false;
   }
/*
   if( debug_mask & (INTERPRETER | SYNCHRONIZATION) )
   {
      stringstream out;
      out << "cmdli_implementation::step(" << ourName << ") - done";
      INFORM(out.str().c_str());
   }
*/   
   return true;
}

/*********************************************************************/
/// Reset the interpreter to the initial state
void
cmdli_implementation::reset()
{
   if( ! thecmdl->setActiveBlock( thecmdl->getMainBlock() ) )
   {
      ERROR("cmdli_implementation::reset - The Main Block %s does not exist!");
   }
}

// ********************************************************************
/// objcomms callback function to update our status cache as messages arrive
void 
cmdli_implementation::updateStatus(commMsg *msg, void *ptr)
{
   if( msg == NULL )
   {
      ERROR("cmdli_implementation::updateStatus - Internal Error: Null msg in cmdli_implementation::updateStatus");
      return;
   }

   if( ptr == NULL )
   {
      ERROR("cmdli_implementation::updateStatus - Internal Error: Null this ptr in cmdli_implementation::updateStatus");
      return;
   }

   // Check if it is a status message
   CMDLstatus *rec = dynamic_cast<CMDLstatus *>(msg);
   if( rec == NULL )
   {
      ERROR("cmdli_implementation::updateStatus - Internal Error: Unable to upcast msg to CMDLstatus!");
      return;
   }

   // get access to our record
   cmdli_implementation *thisrec = (cmdli_implementation *)ptr;

   if( thisrec->debug_mask & SYNCHRONIZATION )
   {
      stringstream out;
      out << "Received status message: " << *rec;
      INFORM(out.str().c_str());
   }

   if( rec->status != CMDLstatus::MSGINVALID )
   {

      // get access
      thisrec->statusLock.p();

      // carefully free the last record, if one exists.
      latestStatus_T::iterator it = thisrec->latestStatus.find(rec->name);
      if( it != thisrec->latestStatus.end() )
      {
	 delete it->second;
	 it->second = new statusRec( rec );
      }
      else
      {
	 // first time, just save it.
	 thisrec->latestStatus[rec->name] = new statusRec( rec );
      }

      // release access
      thisrec->statusLock.v();

   }
   else
   {
      ERROR("Discarding corrupt status message");
   }
}

// ********************************************************************
/// objcomms callback function to process received commands
void 
cmdli_implementation::processCommand(commMsg *msg, void *ptr)
{
   if( msg == NULL )
   {
      ERROR("cmdli_implementation::processCommand - Internal Error: Null msg in cmdli_implementation::processCommand");
      return;
   }

   if( ptr == NULL )
   {
      ERROR("cmdli_implementation::processCommand - Internal Error: Null this ptr in cmdli_implementation::processCommand");
      delete msg;
      return;
   }

   // Check that it really is a command
   CMDLcommand *rec = dynamic_cast<CMDLcommand *>(msg);
   if( rec == NULL )
   {
      ERROR("cmdli_implementation::processCommand - Internal Error: Unable to upcast msg to CMDLcommand!");
      delete msg;
      return;
   }
/*
{
stringstream ss;
ss << "cmdli_implementation::processCommand - received command message: " << *rec;
INFORM(ss.str().c_str());
}
*/
   // get access to our record
   cmdli_implementation *thisrec = (cmdli_implementation *)ptr;

   switch( rec->action )
   {
      case CMDLcommand::CMD_INVALID:
         {
            INFORM("cmdli_implementation::processCommand - discarding corrupt command message");
         }
         break;

      case CMDLcommand::CMD_DOWNLOAD:
         {

            // load the new mission and get it ready to run
            bool rtn = thisrec->load(rec->data);

	    if( rtn )
            {
               // force mode to stopped
               thisrec->runStateLock.p();
	       thisrec->runState = RS_STOPPED;
               thisrec->runStateLock.v();

	       thisrec->reset();
            }
            else
            {
               INFORM("cmdli_implementation::processCommand - unable to load mission\n%s", rec->data.c_str());
            }

	    // if they don't yet have our ack, then send one.
	    if( rec->gotAcks.find( ourName ) == rec->gotAcks.end() )
	    {
               // send ack
               CMDLcommandAck ack;
               ack.opcon = rec->opcon;
               ack.opconID = rec->opconID;
	       ack.name = ourName;
               ack.response = rtn ? CMDLcommandAck::OK : CMDLcommandAck::FAILED;
               if( !thisrec->comobj->sendMsg(rec->senderAddr, ack) )
               {
                  INFORM("cmdli_implementation::processCommand - unable to ack START command from %s(%d)", rec->opcon.c_str(), (uint)rec->opconID);
               }
               INFORM("cmdli_implementation::processCommand - sent CMDLcommandAck");
	    }
INFORM("cmdli_implementation::processCommand - DOWNLOAD command from %s(%d)", rec->opcon.c_str(), (uint)rec->opconID);
         }
         break;

      case CMDLcommand::CMD_START:
         {
	    // start out with a clean ignore list
            thisrec->forcedIgnoreListLock.p();
	    thisrec->forcedIgnoreList.empty();
            thisrec->forcedIgnoreListLock.v();

	    // reset the statusvalue so will re-start cleanly.
            thisrec->statusValue = CMDLstatus::CMDEXECUTING;

            // set the run state to "running"
            thisrec->runStateLock.p();
	    thisrec->runState = RS_RUNNING;
            thisrec->runStateLock.v();

	    // if they don't yet have our ack, then send one.
	    if( rec->gotAcks.find( ourName ) == rec->gotAcks.end() )
	    {
               CMDLcommandAck ack;
               ack.opcon = rec->opcon;
               ack.opconID = rec->opconID;
               ack.response = CMDLcommandAck::OK;
	       ack.name = ourName;
               if( !thisrec->comobj->sendMsg(rec->senderAddr, ack) )
               {
                  INFORM("cmdli_implementation::processCommand - unable to ack START command from %s(%d)", rec->opcon.c_str(), (uint)rec->opconID);
               }
               INFORM("cmdli_implementation::processCommand - sent CMDLcommandAck");
	    }

INFORM("cmdli_implementation::processCommand - START command from %s(%d)", rec->opcon.c_str(), (uint)rec->opconID);
         }
         break;

      case CMDLcommand::CMD_STARTAT:
         {
INFORM("cmdli_implementation::processCommand - UNSUPPORTED STARTAT command from %s(%d): Starting mission '%s', version %d at block '%s', command %d", rec->opcon.c_str(), (uint)rec->opconID, rec->mission.c_str(), rec->version, rec->block.c_str(), rec->command);
         }
         break;

      case CMDLcommand::CMD_STOP:
         {
            // set the run state to "stopped"
            thisrec->runStateLock.p();
	    thisrec->runState = RS_STOPPED;
            thisrec->runStateLock.v();

	    thisrec->reset();

	    // if they don't yet have our ack, then send one.
	    if( rec->gotAcks.find( ourName ) == rec->gotAcks.end() )
	    {
               CMDLcommandAck ack;
               ack.opcon = rec->opcon;
               ack.opconID = rec->opconID;
	       ack.name = ourName;
               ack.response = CMDLcommandAck::OK;
               if( !thisrec->comobj->sendMsg(rec->senderAddr, ack) )
               {
                  INFORM("cmdli_implementation::processCommand - unable to ack START command from %s(%d)", rec->opcon.c_str(), (uint)rec->opconID);
               }
               INFORM("cmdli_implementation::processCommand - sent CMDLcommandAck");
	    }
INFORM("cmdli_implementation::processCommand - STOP command: Stopping mission");
         }
         break;

      case CMDLcommand::CMD_ESTOP:
         {
            // get access
            thisrec->runStateLock.p();

            // set the specified mission as active

            // set the run state to "running"
//	    thisrec->;

            // release access
            thisrec->runStateLock.v();
INFORM("cmdli_implementation::processCommand - ESTOP command: Emergency stop of the mission");
         }
         break;

      case CMDLcommand::CMD_PAUSE:
         {
            // get access
            thisrec->runStateLock.p();

	    // can only pause if running
	    if( thisrec->runState == RS_RUNNING )
	    {
	       thisrec->runState = RS_PAUSED;
	    }

            // release access
            thisrec->runStateLock.v();

	    // if they don't yet have our ack, then send one.
	    if( rec->gotAcks.find( ourName ) == rec->gotAcks.end() )
	    {
               CMDLcommandAck ack;
               ack.opcon = rec->opcon;
               ack.opconID = rec->opconID;
	       ack.name = ourName;
               ack.response = CMDLcommandAck::OK;
               if( !thisrec->comobj->sendMsg(rec->senderAddr, ack) )
               {
                  INFORM("cmdli_implementation::processCommand - unable to ack START command from %s(%d)", rec->opcon.c_str(), (uint)rec->opconID);
               }
               INFORM("cmdli_implementation::processCommand - sent CMDLcommandAck to %s", rec->senderAddr.str().c_str() );
	    }
INFORM("cmdli_implementation::processCommand - PAUSE command: Pausing mission");
         }
         break;

      case CMDLcommand::CMD_RESUME:
         {
            // get access
            thisrec->runStateLock.p();

	    // can only resume out of a paused state.
	    if( thisrec->runState == RS_PAUSED )
	    {
               // set the run state to "running"
	       thisrec->runState = RS_RUNNING;
	    }

            // release access
            thisrec->runStateLock.v();

	    // if they don't yet have our ack, then send one.
	    if( rec->gotAcks.find( ourName ) == rec->gotAcks.end() )
	    {
               CMDLcommandAck ack;
               ack.opcon = rec->opcon;
               ack.opconID = rec->opconID;
	       ack.name = ourName;
               ack.response = CMDLcommandAck::OK;
               if( !thisrec->comobj->sendMsg(rec->senderAddr, ack) )
               {
                  INFORM("cmdli_implementation::processCommand - unable to ack START command from %s(%d)", rec->opcon.c_str(), (uint)rec->opconID);
               }
               INFORM("cmdli_implementation::processCommand - sent CMDLcommandAck");
	    }
INFORM("cmdli_implementation::processCommand - RESUME command: Resume mission");
         }
         break;

      case CMDLcommand::CMD_RESUMEAT:
         {
            // get access
            thisrec->runStateLock.p();

            // set the specified mission as active

            // set the run state to "running"
//	    thisrec->;

            // release access
            thisrec->runStateLock.v();
INFORM("cmdli_implementation::processCommand - RESUMEAT command: Resuming mission at block '%s', command %d", rec->block.c_str(), rec->command);
         }
         break;

      case CMDLcommand::CMD_IGNOREROBOT:
         {
	    // Are they ignoring us? if so, we should stop
	    if( rec->data == ourName )
	    {
               // get access
               thisrec->runStateLock.p();

               // set the run state to "running"
	       thisrec->runState = RS_STOPPED;

               // release access
               thisrec->runStateLock.v();
	    }
	    else
	    {
	       // add the dead robot to our ignore list

               // get access
               thisrec->forcedIgnoreListLock.p();

               // add the specified robot to the ignore list
	       thisrec->forcedIgnoreList.insert(rec->data);

               // release access
               thisrec->forcedIgnoreListLock.v();
	    }

	    // if they don't yet have our ack, then send one.
	    if( rec->gotAcks.find( ourName ) == rec->gotAcks.end() )
	    {
               CMDLcommandAck ack;
               ack.opcon = rec->opcon;
               ack.opconID = rec->opconID;
	       ack.name = ourName;
               ack.response = CMDLcommandAck::OK;
               if( !thisrec->comobj->sendMsg(rec->senderAddr, ack) )
               {
                  INFORM("cmdli_implementation::processCommand - unable to ack IGNOREROBOT command from %s(%d)", rec->opcon.c_str(), (uint)rec->opconID);
               }
               INFORM("cmdli_implementation::processCommand - sent CMDLcommandAck");
	    }

INFORM("cmdli_implementation::processCommand - IGNORE command from %s(%d) for robot %s", rec->opcon.c_str(), (uint)rec->opconID, rec->data.c_str());
         }
         break;

   }

   // cleanup
   delete msg;
}

// ********************************************************************
/// returns true if resync is active
bool 
cmdli_implementation::handleResyncs()
{
   bool rtn(false);
   typedef vector< CMDLstatus * > statusRecs;
   statusRecs waiters;

   // get access
   statusLock.p();

   // Is anyone waiting for us?
   latestStatus_T::iterator it;
   for(it=latestStatus.begin(); it != latestStatus.end(); ++it)
   {
      if( it->second->waitingForUs )
      {
         if( debug_mask & (INTERPRETER | SYNCHRONIZATION) )
         {
            INFORM("cmdli_implementation::handleResyncs - %s is waiting for us", it->second->record->name.c_str() );
	 }

         /// If we recently ignored them while crossing a barrier, we better resync
         IgnoreList_T::iterator igdata = ignoreList.find( it->second->record->name );
         if( igdata != ignoreList.end() )
            waiters.push_back( (CMDLstatus *)it->second->record->clone() );
      }
   }

   // release access
   statusLock.v();

   /// anyone waiting for us to resync?
   if( !waiters.empty() )
   {
//      if( debug_mask & (INTERPRETER | SYNCHRONIZATION) )
      {
         INFORM("cmdli_implementation::handleResyncs - %d waiters", waiters.size() );
      }

      for(uint i=0; i<waiters.size(); i++)
      {
         // Figure out where they are at in the mission, based on syncs we have skipped
         CMDLstatus *rec = waiters[i];
         Barriers_T *stateList = &ignoreList[rec->name];
         Barriers_T::iterator state = stateList->begin();
         while( state != stateList->end() )
         {
            if( state->block->get_label() == rec->activeBlock &&
                state->cmdIndex == rec->activeCommand )
            {
               // found the step they are on.  Erase the older ones.
               state++;
               if( state != stateList->end() )
                  stateList->erase(state, stateList->end());

INFORM("cmdli_implementation::handleResyncs - sending resync status message for '%s'", rec->name.c_str());
               // send a special resync status packet to help them out.
               if( !sendResyncStatusMessage(rec->activeBlock, rec->activeCommand) )
               {
                  ERROR("Unable to send resync status messages!");
                  break;
               }
               break;
            }

            state++;
         }
      }

      // free the records
      for(uint i=0; i<waiters.size(); i++)
      {
         delete waiters[i];
      }
      waiters.erase(waiters.begin(), waiters.end());
   }

   return rtn;
}

// ********************************************************************
/// Get the context-sensitive value to use for the commsTimeout
TimeOfDay 
cmdli_implementation::getCommsTimeout() const
{
   Block *blk = thecmdl->getActiveBlock();
   if( !blk )
   {
      ERROR("cmdli_implementation::getCommsTimeout - NULL Block!");
      return TimeOfDay(100000);
   }
   Command *cmd =  blk->getCommand( thecmdl->getActiveCommand() );
   if( !cmd )
   {
      ERROR("cmdli_implementation::getCommsTimeout - NULL Command!");
      return TimeOfDay(100000);
   }

   // is there a command-specific one?
   double dval;
   if( cmd->getOptions()->get_double(RC_COMMSTIMEOUT, dval) )
   {
      return (TimeOfDay)dval;
   }

   // is there a block-specific one?
   if( blk->getOptions()->get_double(RC_COMMSTIMEOUT, dval) )
   {
      return (TimeOfDay)dval;
   }

   // is there a mission-specific one?
   if( thecmdl->getOptions()->get_double(RC_COMMSTIMEOUT, dval) )
   {
      return (TimeOfDay)dval;
   }

   // use the default
   return CommTimeout;
}

// ********************************************************************
/// Get the context-sensitive value to use for the waitTimeout
TimeOfDay 
cmdli_implementation::getWaitTimeout() const
{
   Block *blk = thecmdl->getActiveBlock();
   if( !blk )
   {
      ERROR("cmdli_implementation::getWaitTimeout - NULL Block!");
      return TimeOfDay(100000);
   }
   Command *cmd =  blk->getCommand( thecmdl->getActiveCommand() );
   if( !cmd )
   {
      ERROR("cmdli_implementation::getWaitTimeout - NULL Command!");
      return TimeOfDay(100000);
   }

   // is there a command-specific one?
   double dval;
   if( cmd->getOptions()->get_double(RC_WAITTIMEOUT, dval) )
   {
      return (TimeOfDay)dval;
   }

   // is there a block-specific one?
   if( blk->getOptions()->get_double(RC_WAITTIMEOUT, dval) )
   {
      return (TimeOfDay)dval;
   }

   // is there a mission-specific one?
   if( thecmdl->getOptions()->get_double(RC_WAITTIMEOUT, dval) )
   {
      return (TimeOfDay)dval;
   }

   // use the default
   return WaitTimeout;
}

// ********************************************************************
/// check if we are ignoring the specified robot
/// returns true if should ignore robot, false otherwise
bool
cmdli_implementation::areIgnoring(const string &robotName) 
{
   bool foundIt;

   // get access
   forcedIgnoreListLock.p();

   // try to find it
   foundIt = forcedIgnoreList.find(robotName) != forcedIgnoreList.end();

   // release access
   forcedIgnoreListLock.v();

   return foundIt;
}

// ********************************************************************
/// Returns true on success, false if the cpu had an error
bool 
cmdli_implementation::internalStep()
{
   static bool thecmdl_error_once = true;
   if( !thecmdl )
   {
      if( thecmdl_error_once )
      {
         ERROR("cmdli_implementation::internalStep - No mission loaded!");
         thecmdl_error_once = false;
      }
      return false;
   }
   else
   {
      thecmdl_error_once = true;
   }
      

   bool rtn = true;
/*
   if( debug_mask & (INTERPRETER | SYNCHRONIZATION) )
   {
      stringstream out;
      out << "cmdli_implementation::internalStep - starting" << endl;
      latestStatus_T::iterator it;

      // get access
      statusLock.p();

      for(it=latestStatus.begin(); it != latestStatus.end(); ++it)
      {
         out << "   " << *it->second->record << " Age=" << TimeOfDay::now() - it->second->arrivalTime << endl;
      }

      // release access
      statusLock.v();

      INFORM(out.str().c_str());
   }
*/
   // are we in a syncronization mode?  If so, we send status messages more often.
   syncing = false;

   // get our current state
   runState_T runStateCopy;
   runStateLock.p();
   runStateCopy = runState;
   runStateLock.v();

   // handle any resyncing that is necessary
   handleResyncs();

   bool loop = true;
   if( runStateCopy != RS_RUNNING )
   {
      // oops, we are paused, so don't make any changes
      loop = false;

      if( debug_mask & SYNCHRONIZATION )
      {
         stringstream out;
         out <<  "cmdli_implementation::internalStep(" << ourName << ") - mode: " << CMDLstatus::statusToString(statusValue);
         INFORM(out.str().c_str());
      }
   }

   // we are modal.
   bool sentCmd = false;
   while( loop )
   {
      if( debug_mask & SYNCHRONIZATION )
      {
         stringstream out;
         out <<  "cmdli_implementation::internalStep(" << ourName << ") - mode: " << CMDLstatus::statusToString(statusValue);
         INFORM(out.str().c_str());
      }

      // init 
      loop = false;
      if( !blocklist.empty() )
         blocklist.erase(blocklist.begin(), blocklist.end());

      switch( statusValue )
      {
         // not a valid status message
         case CMDLstatus::MSGINVALID:
            {
               ERROR("cmdli_implementation::internalStep - MSGINVALID statusValue!");
               rtn = false;
            }
            break;
   
         // command executing normally (i.e., still moving towards the waypoint)
         case CMDLstatus::CMDEXECUTING:
	    {
	       if( debug_mask & INTERPRETER )
	       {
		  stringstream out;
		  out << "cmdli - CMDEXECUTING: block=" << thecmdl->getActiveBlock()->get_label() << " cmd=" << thecmdl->getActiveCommand();
		  INFORM(out.str().c_str());
	       }

	       // ***** Execute the action associated with this state *****
	       Block *block = thecmdl->getActiveBlock();
	       uint cmdIndex = thecmdl->getActiveCommand();
	       Command *rawCmd = NULL;
	       if( block )
	       {
		  rawCmd = block->getCommand( cmdIndex );
	       }
	       if( !rawCmd )
	       {
                  rtn = false;
                  break;
	       }

	       // tmps
	       ActionCommand *actionCmd = NULL;
	       CallCommand *callCmd;
	       GotoCommand *gotoCmd;
	       ReturnCommand *returnCmd;
	       SyncCommand *syncCmd;

	       // is it a procedure call?
	       if( (callCmd = dynamic_cast<CallCommand *>(rawCmd)) != NULL )
	       {
		  if( debug_mask & INTERPRETER )
		  {
		     stringstream out;
		     out << "cmdli_implementation::internalStep - current block: " << block->get_label() 
			//			    << " current action: " << callCmd->getAction() 
			<< "  Calling procedure '" << callCmd->getBlockName() << "'";
		     INFORM(out.str().c_str());
		  }

		  // push our state
		  callStack.push( stateInfo(block, cmdIndex) );

		  // jump to the new procedure
		  if( !thecmdl->setActiveBlock( callCmd->getBlockName() ) )
		  {
		     stringstream out;
		     out << "cmdli_implementation::internalStep - Internal Error: Unable to setActiveBlock(" << callCmd->getBlockName() << ") failed!";
		     INFORM(out.str().c_str());
                     rtn = false;
                     break;
		  }

		  // go around again and execute the new block.
		  loop = true;
	       }
	       // is it a return?
	       else if( (returnCmd = dynamic_cast<ReturnCommand *>(rawCmd)) != NULL )
	       {
		  if( debug_mask & INTERPRETER )
		  {
		     stringstream out;
		     out << "cmdli_implementation::internalStep - current block: " << block->get_label() 
			<< "  Returning from call";
		     INFORM(out.str().c_str());
		  }

		  // restore our state
		  Block *rtnBlk = callStack.top().block;
		  uint   rtnCmd = callStack.top().cmdIndex;
		  callStack.pop();

		  // jump back
		  if( !thecmdl->setActiveBlockAndCommand( rtnBlk->get_label(), rtnCmd) )
		  {
		     stringstream out;
		     out << "cmdli_implementation::internalStep - Internal Error: Unable to setActiveBlockAndCommand(" << rtnBlk->get_label() << ", " << rtnCmd << ") failed!";
		     INFORM(out.str().c_str());
		     rtn = false;
                     break;
		  }

		  // that was where we came from.  Now move to the next command.
		  if( !nextCommand() )
		  {
		     // oops, the mission is all done.
		     statusValue = CMDLstatus::MISSIONDONE;
		     continue;
		  }

		  // go around again and execute the new block.
		  loop = true;
	       }
	       // is it a goto?
	       else if( (gotoCmd = dynamic_cast<GotoCommand *>(rawCmd)) != NULL )
	       {
		  if( debug_mask & INTERPRETER )
		  {
		     stringstream out;
		     out << "cmdli_implementation::internalStep - current block: " << block->get_label() 
			<< "  Goto label '" << gotoCmd->getDestination() << "'";
		     INFORM(out.str().c_str());
		  }

		  // jump to the new procedure
		  if( !thecmdl->setActiveCommand( gotoCmd->getDestination() ) )
		  {
		     stringstream out;
		     out << "cmdli_implementation::internalStep - Internal Error: Unable to setActiveCommand(" << gotoCmd->getDestination() << ") failed!";
		     INFORM(out.str().c_str());
                     rtn = false;
                     break;
		  }

		  // go around again and execute the new block.
		  loop = true;
	       }
	       // is it a sync?
	       else if( (syncCmd = dynamic_cast<SyncCommand *>(rawCmd)) != NULL )
	       {
		  if( debug_mask & INTERPRETER )
		  {
		     stringstream out;
		     out << "cmdli_implementation::internalStep - current block: " << block->get_label() 
			<< "  Synchronizing";
		     INFORM(out.str().c_str());
		  }

		  // Sync on the barrier.
		  statusValue = CMDLstatus::ATBARRIER;
		  startedWait = TimeOfDay::now();
		  loop = true;
	       }
	       // Is it an action to execute?
	       else if( (actionCmd = dynamic_cast<ActionCommand *>(rawCmd)) != NULL )
	       {
		  // build a call to execute the action
		  string action = actionCmd->getAction();
		  const keyValueMap *options = actionCmd->getOptions();

		  // do it.
		  executeAction(action, *options);
                  sentCmd = true;

		  // Find our active block
		  Block *activeBlock = thecmdl->getActiveBlock();
		  if( !activeBlock )
		  {
		     ERROR("cmdli_implementation::internalStep - Internal Error: NULL ActiveBlock!");
		     rtn = false;
                     break;
		  }

		  // Find our active command within that block (it might have changed)
		  rawCmd = activeBlock->getCommand( thecmdl->getActiveCommand() );

		  // Is it a normal user command to execute?
		  ActionCommand *activeCmd;
		  if( (activeCmd = dynamic_cast<ActionCommand *>(rawCmd)) != NULL )
		  {
		     if( debug_mask & INTERPRETER )
		     {
			stringstream out;
			out << "cmdli_implementation::internalStep - current block: " << activeBlock->get_label() 
			   << " current action: " << activeCmd->getAction();
			INFORM(out.str().c_str());
		     }

		     // See if the command is done 
		     if( debug_mask & SYNCHRONIZATION )
		     {
			INFORM("starting transition check");
		     }

		     // check the global interrupts
//cerr << "Checking " << thecmdl->transitions.size() << " global exceptions" << endl;
		     cmdl_transition::Transitions_T::const_iterator it;
		     for(it=thecmdl->transitions.begin(); it!=thecmdl->transitions.end(); ++it)
		     {
			cmdl_transition *trans = *it;

			if( debug_mask & SYNCHRONIZATION )
			{
			   stringstream out;
			   out <<  "cmdli_implementation::internalStep(" << ourName << ") - checking global exception trigger '" << trans->getTriggerName() << "'";
			   INFORM(out.str().c_str());
			}

			// Request the value for this trigger.
			bool raised = checkTrigger(trans->getTriggerName(), *options);

			if( debug_mask & SYNCHRONIZATION )
			{
			   stringstream out;
			   out <<  "cmdli_implementation::internalStep(" << ourName << ") - trigger value = " 
			       << raised;
			   INFORM(out.str().c_str());
			}

			if( raised )
			{
			   // is it a subroutine call?
			   if( trans->isCall() )
			   {
			      if( debug_mask & INTERPRETER )
			      {
			         stringstream out;
			         out << "cmdli_implementation::internalStep - current block: " << block->get_label() 
				    << "  Exception raised, calling procedure '" << trans->getTargetName() << "'";
			         INFORM(out.str().c_str());
			      }

			      // yup, push our state so we can return and jump to the new block
			      callStack.push( stateInfo(block, cmdIndex) );

			      if( !thecmdl->setActiveBlock( trans->getTargetName() ) )
			      {
			         stringstream out;
			         out << "cmdli_implementation::internalStep - Internal Error: Unable to setActiveBlock(" << block->get_label() << ") failed!";
			         INFORM(out.str().c_str());
			         rtn = false;
                                 break;
			      }
			   }
			   else
			   {
			      if( debug_mask & INTERPRETER )
			      {
			         stringstream out;
			         out << "cmdli_implementation::internalStep - current block: " << block->get_label() 
				    << "  Exception raised, jumping to label '" << trans->getTargetName() << "'";
			         INFORM(out.str().c_str());
			      }

			      // nope, just a goto
			      if( !thecmdl->setActiveCommand( trans->getTargetName() ) )
			      {
			         stringstream out;
			         out << "cmdli_implementation::internalStep - Internal Error: Unable to setActiveCommand(" << block->get_label() << ") failed!";
			         INFORM(out.str().c_str());
			         rtn = false;
                                 break;
			      }
			   }

			   // go around again and execute the new block.
			   loop = true;
			   break;
			}
		     }
                     // duck out if we had an error or took an exception
                     if( !rtn || loop )
                        break;

		     // check block-wide transitions
//cerr << "Checking " << activeBlock->transitions.size() << " block-wide exceptions" << endl;
		     for(it=activeBlock->transitions.begin(); it!=activeBlock->transitions.end(); ++it)
		     {
			cmdl_transition *trans = *it;

			if( debug_mask & SYNCHRONIZATION )
			{
			   stringstream out;
			   out <<  "cmdli_implementation::internalStep(" << ourName << ") - checking block-wide exception trigger '" << trans->getTriggerName() << "'";
			   INFORM(out.str().c_str());
			}

			// Request the value for this trigger.
			bool raised = checkTrigger( trans->getTriggerName(), *options);

			if( debug_mask & SYNCHRONIZATION )
			{
			   stringstream out;
			   out <<  "cmdli_implementation::internalStep(" << ourName << ") - trigger value = " << raised;
			   INFORM(out.str().c_str());
			}

			if( raised )
			{
			   // is it a subroutine call?
			   if( trans->isCall() )
			   {
			      if( debug_mask & INTERPRETER )
			      {
			         stringstream out;
			         out << "cmdli_implementation::internalStep - current block: " << block->get_label() 
				    << "  Exception raised, calling procedure '" << trans->getTargetName() << "'";
			         INFORM(out.str().c_str());
			      }

			      // yup, push our state so we can return and jump to the new block
			      callStack.push( stateInfo(block, cmdIndex) );

			      if( !thecmdl->setActiveBlock( trans->getTargetName() ) )
			      {
			         stringstream out;
			         out << "cmdli_implementation::internalStep - Internal Error: Unable to setActiveBlock(" << block->get_label() << ") failed!";
			         INFORM(out.str().c_str());
			         rtn = false;
                                 break;
			      }
			   }
			   else
			   {
			      if( debug_mask & INTERPRETER )
			      {
			         stringstream out;
			         out << "cmdli_implementation::internalStep - current block: " << block->get_label() 
				    << "  Exception raised, jumping to label '" << trans->getTargetName() << "'";
			         INFORM(out.str().c_str());
			      }

			      // nope, just a goto
			      if( !thecmdl->setActiveCommand( trans->getTargetName() ) )
			      {
			         stringstream out;
			         out << "cmdli_implementation::internalStep - Internal Error: Unable to setActiveCommand(" << block->get_label() << ") failed!";
			         INFORM(out.str().c_str());
			         rtn = false;
                                 break;
			      }
			   }

			   // go around again and execute the new block.
			   loop = true;
			   break;
			}
		     }
                     // duck out if we had an error or took an exception
                     if( !rtn || loop )
                        break;

		     // check command-specific transitions
//cerr << "Checking " << activeCmd->transitions.size() << " command-specific exceptions" << endl;
		     for(it=activeCmd->transitions.begin(); it!=activeCmd->transitions.end(); ++it)
		     {
			cmdl_transition *trans = *it;

			if( debug_mask & SYNCHRONIZATION )
			{
			   stringstream out;
			   out <<  "cmdli_implementation::internalStep(" << ourName << ") - checking command-specific exception trigger '" << trans->getTriggerName() << "'";
			   INFORM(out.str().c_str());
			}

			// Request the value for this trigger.
			bool raised = checkTrigger( trans->getTriggerName(), *options);

			if( debug_mask & SYNCHRONIZATION )
			{
			   stringstream out;
			   out <<  "cmdli_implementation::internalStep(" << ourName << ") - trigger value = " << raised;
			   INFORM(out.str().c_str());
			}

			if( raised )
			{
			   // is it a subroutine call?
			   if( trans->isCall() )
			   {
			      if( debug_mask & INTERPRETER )
			      {
			         stringstream out;
			         out << "cmdli_implementation::internalStep - current block: " << block->get_label() 
				    << "  Exception raised, calling procedure '" << trans->getTargetName() << "'";
			         INFORM(out.str().c_str());
			      }

			      // yup, push our state so we can return and jump to the new block
			      callStack.push( stateInfo(block, cmdIndex) );

			      if( !thecmdl->setActiveBlock( trans->getTargetName() ) )
			      {
			         stringstream out;
			         out << "cmdli_implementation::internalStep - Internal Error: Unable to setActiveBlock(" << block->get_label() << ") failed!";
			         INFORM(out.str().c_str());
			         rtn = false;
                                 break;
			      }
			   }
			   else
			   {
			      if( debug_mask & INTERPRETER )
			      {
			         stringstream out;
			         out << "cmdli_implementation::internalStep - current block: " << block->get_label() 
				    << "  Exception raised, jumping to label '" << trans->getTargetName() << "'";
			         INFORM(out.str().c_str());
			      }

			      // nope, just a goto
			      if( !thecmdl->setActiveCommand( trans->getTargetName() ) )
			      {
			         stringstream out;
			         out << "cmdli_implementation::internalStep - Internal Error: Unable to setActiveCommand(" << block->get_label() << ") failed!";
			         INFORM(out.str().c_str());
			         rtn = false;
                                 break;
			      }
			   }

			   // go around again and execute the new block.
			   loop = true;
			   break;
			}
		     }
                     // duck out if we had an error or took an exception
                     if( !rtn || loop )
                        break;

		     if( debug_mask & SYNCHRONIZATION )
		     {
		        INFORM("Checking default completion trigger");
		     }

		     completionTriggers_T::iterator cit = defaultCompletionTriggers.find( activeCmd->getAction() );
			if( cit == defaultCompletionTriggers.end() )
			{
			   stringstream out;
			   out << "Warning: No default completion trigger for action '" << activeCmd->getAction() << "'";
			   WARN(out.str().c_str());
			}
			else
			{
			   string trig = cit->second;

			   if( debug_mask & SYNCHRONIZATION )
			   {
			      stringstream out;
			      out <<  "cmdli_implementation::internalStep(" << ourName << ") - using default completion trigger '" << trig << "'";
			      INFORM(out.str().c_str());
			   }

			   // Request the value for this trigger.
			   bool complete = checkTrigger(trig, *options);

			   if( complete )
			   {
			      if( debug_mask & SYNCHRONIZATION )
			      {
			         stringstream out;
			         out <<  "cmdli_implementation::internalStep(" << ourName << ") - trigger is true";
			         INFORM(out.str().c_str());
			      }

			      // do we need to sync before moving on?
			      if( needSync() )
			      {
				 // Yup, sync on the implied barrier.
				 statusValue = CMDLstatus::ATBARRIER;
		                 startedWait = TimeOfDay::now();
				 loop = true;
			      }
			      else
			      {
				 // Nope, move to the next command for the next cycle
				 if( !nextCommand() )
				 {
				    // oops, the mission is all done.
				    statusValue = CMDLstatus::MISSIONDONE;
				    loop = true;
				 }

				 // else, everything is good and we will run the new command next cycle.
			      }
			   }
			}
		  }
	       }
	    }
            break;

         // command failed and will not complete (i.e., waypoint is unreachable)
         case CMDLstatus::CMDFAILED:
            {
	       if( debug_mask & INTERPRETER )
	       {
		  stringstream out;
		  out << "cmdli - CMDFAILED: block=" << thecmdl->getActiveBlock()->get_label() << " cmd=" << thecmdl->getActiveCommand();
		  INFORM(out.str().c_str());
	       }

               // yes we are syncing
               syncing = true;

            }
            break;

         // sender has successfully finished the command (i.e., at the waypoint)
         case CMDLstatus::CMDDONE:
            {
	       if( debug_mask & INTERPRETER )
	       {
		  stringstream out;
		  out << "cmdli - CMDDONE: block=" << thecmdl->getActiveBlock()->get_label() << " cmd=" << thecmdl->getActiveCommand();
		  INFORM(out.str().c_str());
	       }

               // yes we are syncing
               syncing = true;
            }
            break;

         // block is complete and we are waiting for the rest of the group to finish
	 // We block here until we see everyone else either ATBARRIER or PASTBARRIER
	 // for this block/command
         case CMDLstatus::ATBARRIER:
            {
	       if( debug_mask & INTERPRETER )
	       {
		  stringstream out;
		  out << "cmdli - ATBARRIER: block=" << thecmdl->getActiveBlock()->get_label() << " cmd=" << thecmdl->getActiveCommand();
		  INFORM(out.str().c_str());
	       }

               // yes we are syncing
               syncing = true;

               // get the list of our teammates
               Block *block = thecmdl->getActiveBlock();
               Command *cmd = block->getCommand( thecmdl->getActiveCommand() );
               const stringSet group( cmd->getRobotNames() );

               if( debug_mask & SYNCHRONIZATION )
               {
                  stringstream out;
                  out <<  "cmdli_implementation::internalStep(" << ourName << ") - robots in our group: ";
                  stringSet::const_iterator it;
                  for(it=group.begin(); it!=group.end(); ++it)
                  {
                     out << " " << *it;
                  }
                  INFORM(out.str().c_str());
               }

               // is everyone either at the barrier, or past it?
               bool ready = true;
	       bool error = false;

               // has our waitTimeout expired?
               TimeOfDay age = TimeOfDay::now() - startedWait;
               TimeOfDay timeout = getWaitTimeout();
               if( age > timeout )
               {
//                if( debug_mask & SYNCHRONIZATION )
                  {
                     stringstream out;
                     out <<  "cmdli(" << ourName << ") - Leaving barrier because our waitTimeout of " << timeout << " seconds has expired";
                     WARN(out.str().c_str());
                  }
               }
	       else
	       {
		  // check if everyone is at the barrier

	          // !!!!!!! get access to the status messages !!!!!!!!
	          statusLock.p();

                  stringSet::const_iterator it;
                  for(it=group.begin(); it!=group.end(); ++it)
                  {
                     // skip us.
                     if( !cmp_nocase(*it, ourName) )
                     {
                        continue;
                     }

		     // are we ignoring this robot?
                     if( areIgnoring(*it) )
		     {
//                      if( debug_mask & SYNCHRONIZATION )
                        {
                           stringstream out;
                           out <<  "cmdli(" << ourName << ") - IGNORING " << *it << " because on ignore list";
                           WARN(out.str().c_str());
                        }
                     
                        // go on
                        continue;
                     }

                     // do we have a status message from them?
                     latestStatus_T::iterator sit = latestStatus.find(*it);
                     if( sit==latestStatus.end() || sit->second==NULL || sit->second->record==NULL || sit->second->record->status == CMDLstatus::MSGINVALID )
                     {
                        if( debug_mask & SYNCHRONIZATION )
                        {
                           stringstream out;
                           out <<  "cmdli(" << ourName << ") - no status available from " << *it;
                           WARN(out.str().c_str());
                        }
      
                        // no
                        ready = false;
                        blocklist.push_back(*it);
                        continue;
                     }

                     // is it so old we are ignoring them?
                     TimeOfDay age = TimeOfDay::now() - sit->second->arrivalTime;
                     TimeOfDay timeout = getCommsTimeout();
                     if( age > timeout )
                     {
//                      if( debug_mask & SYNCHRONIZATION )
                        {
                           stringstream out;
                           out <<  "cmdli(" << ourName << ") - IGNORING " << *it << " because have not received status for " << age << " seconds";
                           WARN(out.str().c_str());
                        }
                     
                        // go on
                        continue;
                     }

                     // are they at the same place in the mission?
                     CMDLstatus *msg = sit->second->record;
		     if( !msg )
		     {
		        ERROR("cmdli_implementation::internalStep - Internal Error: NULL msg!");
		        error = true;
		        break;
		     }
                     if( !equivalentStep( *msg ) )
                     {  
                        if( debug_mask & SYNCHRONIZATION )
                        {
                           stringstream out;
                           out <<  "cmdli_implementation::internalStep(" << ourName << ") - " << *it << " is not on our mission step yet";
                           INFORM(out.str().c_str());
                        }
      
                        // no
                        ready = false;
                        blocklist.push_back(*it);
                        continue;
                     }

                     // are they at or through the barrier?
                     if( msg->status != CMDLstatus::ATBARRIER && 
                         msg->status != CMDLstatus::PASTBARRIER )
                     {
                        if( debug_mask & SYNCHRONIZATION )
                        {
                           stringstream out;
                           out <<  "cmdli_implementation::internalStep(" << ourName << ") - " << *it << " has status " << msg->status << " so we are waiting";
                           INFORM(out.str().c_str());
                        }
      
                        // no
                        ready = false;
                        blocklist.push_back(*it);
                        continue;
                     }

                     // yes, move on to the next one.
                  }

	          // !!!!!!!!! release access to the status messages !!!!!!!!!!
	          statusLock.v();

	          // duck out if we had an error
	          if( error )
	          {
		     rtn = false;
                     break;
	          }
	       }

	       // are we ready to leave the barrier?
               if( ready )
               {
                  // yes, ready to move on
                  statusValue = CMDLstatus::PASTBARRIER;

//                  loop = true;

                  if( debug_mask & SYNCHRONIZATION )
                  {
                     stringstream out;
                     out <<  "cmdli_implementation::internalStep(" << ourName << ") - everyone is at the barrier.  Releasing";
                     INFORM(out.str().c_str());
                  }
               }
            }
            break;
   
         // Need to wait until the rest of the team clears the barrier
	 // We block here until we see everyone else no longer ATBARRIER 
	 // for this block/command
         case CMDLstatus::PASTBARRIER:
            {
	       if( debug_mask & INTERPRETER )
	       {
		  stringstream out;
		  out << "cmdli - PASTBARRIER: block=" << thecmdl->getActiveBlock()->get_label() << " cmd=" << thecmdl->getActiveCommand();
		  INFORM(out.str().c_str());
	       }

               // yes we are syncing
               syncing = true;

               // get the list of our teammates
               Block *block = thecmdl->getActiveBlock();
               Command *cmd = block->getCommand( thecmdl->getActiveCommand() );
               const stringSet group( cmd->getRobotNames() );


               // is everyone either at the barrier, or past it?
               bool ready = true;
	       bool error = false;

               // has our waitTimeout expired?
               TimeOfDay age = TimeOfDay::now() - startedWait;
               TimeOfDay timeout = getWaitTimeout();
               if( age > timeout )
               {
//                if( debug_mask & SYNCHRONIZATION )
                  {
                     stringstream out;
                     out <<  "cmdli(" << ourName << ") - Leaving barrier because our waitTimeout of " << timeout << " seconds has expired";
                     WARN(out.str().c_str());
                  }
               }
	       else
	       {
		  // check if everyone is at the barrier

	          // !!!!!!! get access to the status messages !!!!!!!!
	          statusLock.p();

                  stringSet::const_iterator it;
                  for(it=group.begin(); it!=group.end(); ++it)
                  {
                     // skip us.
                     if( !cmp_nocase(*it, ourName) )
                     {
                        continue;
                     }

		     // are we ignoring this robot?
                     if( areIgnoring(*it) )
		     {
//                      if( debug_mask & SYNCHRONIZATION )
                        {
                           stringstream out;
                           out <<  "cmdli(" << ourName << ") - IGNORING " << *it << " because on ignore list";
                           WARN(out.str().c_str());
                        }
                     
                        // go on
                        continue;
                     }

                     // do we have a status message from them?
                     latestStatus_T::iterator sit = latestStatus.find(*it);
                     if( sit==latestStatus.end() || sit->second==NULL || sit->second->record==NULL || sit->second->record->status == CMDLstatus::MSGINVALID )
                     {
                        // no
                        ready = false;
                        blocklist.push_back(*it);
                        continue;
                     }

                     // are they at the same place in the mission?
                     CMDLstatus *msg = sit->second->record;
		     if( !msg )
		     {
		        ERROR("cmdli_implementation::internalStep - Internal Error: NULL msg!");
		        error = true;
		        break;
		     }

                     // are they still in the barrier?
                     if( equivalentStep( *msg ) && msg->status == CMDLstatus::ATBARRIER )
                     {
                        // yes, we need to wait for them to notice we are through
                        ready = false;
                        blocklist.push_back(*it);
                        continue;
                     }

                     // yes, move on to the next one.
                  }

	          // !!!!!!!!! release access to the status messages !!!!!!!!!!
	          statusLock.v();

	          // duck out if we had an error
	          if( error )
	          {
                     rtn = false;
                     break;
	          }
	       }

               if( ready )
               {
                  // Need to remember any team members that we ignored
                  stringSet::const_iterator it;
                  for(it=group.begin(); it!=group.end(); ++it)
                  {
                     latestStatus_T::iterator sit = latestStatus.find(*it);
                     if( sit!=latestStatus.end() && sit->second!=NULL && sit->second->record!=NULL && sit->second->record->status != CMDLstatus::MSGINVALID )
                     {
                        TimeOfDay age = TimeOfDay::now() - sit->second->arrivalTime;
                        TimeOfDay timeout = getCommsTimeout();
                        if( age > timeout )
                        {
                           ignoreList[*it].push_front( stateInfo(thecmdl->getActiveBlock(), thecmdl->getActiveCommand()) );
                        }
                     }
                  }

		  // back to running
                  statusValue = CMDLstatus::CMDEXECUTING;

                  // move to the next command for the next cycle.  may cross blocks.
                  if( !nextCommand() )
                  {
                     // oops, the mission is all done.
                     statusValue = CMDLstatus::MISSIONDONE;
                  }

		  // we won't loop here, so there is at least one cycle between commands
               }
            }
            break;

         // we are done
         case CMDLstatus::MISSIONDONE:
            {
               // That was the the last command in the mission
	       bool once = true;
               if( once )
	       {
                  INFORM("Mission Complete");
		  once = false;
	       }
//               done = true;
            }
            break;

	 case CMDLstatus::PAUSED:
	    // shouldn't happen
            ERROR("cmdli_implementation: statusValue = PAUSED");
	    break;

	 case CMDLstatus::STOPPED:
	    // shouldn't happen
            ERROR("cmdli_implementation: statusValue = STOPPED");
	    break;
      }
   }

   if( !sentCmd )
   {
      // if we didn't send anything, send a loiter command
       string action = loiterCmdName;
       const keyValueMap options;  // no options.
       executeAction(action, options);
   }
/*
   if( debug_mask & (INTERPRETER | SYNCHRONIZATION) )
   {
      stringstream out;
      out << "cmdli_implementation::internalStep(" << ourName << ") - done";
      INFORM(out.str().c_str());
   }
*/
   return rtn;
}

// ********************************************************************
/// If time to send a status message, send it.
/// returns true on success, false if comms error
bool 
cmdli_implementation::sendStatusMessage()
{
   // Decide if we want to send a status message.
   TimeOfDay timeSinceLastOne = TimeOfDay::now() - lastMessage;
   bool doit = false;
   if( syncing && timeSinceLastOne > syncingPeriod )
   {
      doit = true;
   }
   else if( !syncing && timeSinceLastOne > normalPeriod )
   {
      doit = true;
   }

   if( doit )
   {
      // send a status message
      CMDLstatus status;
      status.name = ourName;
      status.mission = thecmdl->getMissionName();
      status.version = thecmdl->getMissionVersion();
      status.activeBlock = thecmdl->getActiveBlock()->get_label();
      status.activeCommand = thecmdl->getActiveCommand();

      // the runstate overrides the interpreter status
      if( runState == RS_PAUSED )
         status.status = CMDLstatus::PAUSED;
      else if( runState == RS_STOPPED )
         status.status = CMDLstatus::STOPPED;
      else
         status.status = statusValue;

      for(uint i=0; i<blocklist.size(); i++)
      {
         status.blocklist.push_back( blocklist[i] );
      }
  
      for(IgnoreList_T::iterator it=ignoreList.begin(); it!=ignoreList.end(); ++it)
      {
         status.ignorelist.push_back( it->first );
      }
      for(stringSet::iterator it=forcedIgnoreList.begin(); it!=forcedIgnoreList.end(); ++it)
      {
         status.ignorelist.push_back( *it );
      }

      if( debug_mask & INTERPRETER )
      {
         stringstream out;
         out << "Sending status message: " << status;
         INFORM(out.str().c_str());
      }
   
      // Remember when we sent it
      lastMessage = TimeOfDay::now(); 

      // send it
      if( !comobj->broadcastMsg( status ) )
      {
         ERROR("Error broadcasting msg!");
	 return false;
      }
   }
  
   // good
   return true;
}

// ********************************************************************
/// send a resync status message
/// returns true on success, false if comms error
bool 
cmdli_implementation::sendResyncStatusMessage(const string &block, uint cmd)
{
   // Decide if we want to send a status message.
   TimeOfDay timeSinceLastOne = TimeOfDay::now() - lastMessage;
   bool doit = false;
   if( timeSinceLastOne > syncingPeriod )
   {
      doit = true;
   }

   if( doit )
   {
      // send a status message
      CMDLstatus status;
      status.name = ourName;
      status.mission = thecmdl->getMissionName();
      status.version = thecmdl->getMissionVersion();
      status.activeBlock = block;
      status.activeCommand = cmd;
      status.status = CMDLstatus::PASTBARRIER;
  
      if( debug_mask & INTERPRETER )
      {
         stringstream out;
         out << "Sending resync status message: " << status;
         INFORM(out.str().c_str());
      }
   
      // Remember when we sent it
      lastMessage = TimeOfDay::now(); 

      // send it
      if( !comobj->broadcastMsg( status ) )
      {
         ERROR("Error broadcasting msg!");
	 return false;
      }
   }
  
   // good
   return true;
}

// ********************************************************************
}
