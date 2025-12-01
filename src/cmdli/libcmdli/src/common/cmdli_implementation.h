#ifndef cmdli_implementation_H
#define cmdli_implementation_H
/**********************************************************************
 **                                                                  **
 **  cmdli_implementation.h                                          **
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

/* $Id: cmdli_implementation.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: cmdli_implementation.h,v $
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
* Revision 1.5  2004/11/12 21:55:16  doug
* tweaks for final Ft. Benning
*
* Revision 1.4  2004/10/29 22:38:38  doug
* working on waittimeout
*
* Revision 1.3  2004/09/21 14:51:49  doug
* add more debugging, remove BlockIsDone, and don't loop if pastBarrier.
*
* Revision 1.2  2004/07/30 13:42:15  doug
* handles comm timeouts and resyncs
*
* Revision 1.1  2004/05/11 19:40:17  doug
* made cmdli an abstract base class to limit the amount of stuff pulled in by users (USC and Mlab)
*
* Revision 1.20  2004/04/30 17:36:01  doug
* works for UPenn
*
* Revision 1.19  2004/04/27 16:19:34  doug
* added support for multicast as unicast
*
* Revision 1.18  2004/04/13 22:29:43  doug
* switched to testing cmdli functions used by upenn
*
* Revision 1.17  2004/04/13 17:57:33  doug
* switch to using internal communications and not passing messages in and out
*
* Revision 1.16  2004/03/18 22:11:33  doug
* coded call and return
*
* Revision 1.15  2004/03/17 22:17:11  doug
* working on procedure calls
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
* Revision 1.11  2004/03/01 00:47:17  doug
* cmdli runs in sara
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
**********************************************************************/

#include "mic.h"
#include <stack>
#include <list>
#include "cmdl.h"
#include "cmdli.h"
#include "CMDLstatus.h"
#include "semaphore.h"
#include "resources.h"

namespace sara
{

// *********************************************************************
class cmdli_implementation : public cmdli
{
   public:
      /// our Constructor 
      cmdli_implementation();

      /// Destructor
      virtual ~cmdli_implementation();

      /// First method to init the cmdl interpreter object.
      /// This instance directly invokes triggers and actions.
      /// returns true on success, false otherwise
      bool init(const defaultActionCompletionTriggers_T &defaultActionCompletionTriggers, Actions_T &actionFncs, Triggers_T &legalTriggers, environment *env, const string &myName);

      /// Second method to init the cmdl interpreter object.
      /// This instance calls a mediating function to invoke triggers and actions.
      /// triggers is the set of legal trigger names
      /// defaultActionCompletionTriggers is a map of action names to the name of their default termination triggers.
      /// returns true on success, false otherwise
      bool init(const stringSet &triggers, const defaultActionCompletionTriggers_T &defaultActionCompletionTriggers, executeActionFnc_T executeAction, void *actionUserData, executeTriggerFnc_T executeTrigger, void *triggerUserData, environment *env, const string &myName);

      /// Third method to init the cmdl interpreter object.
      /// This instance relies on the second form of step
      /// defaultActionCompletionTriggers is a map of action names to the name of their default termination triggers.
      /// returns true on success, false otherwise
      bool init(const stringSet &triggers, const defaultActionCompletionTriggers_T &defaultActionCompletionTriggers, environment *env, const string &myName);

      /// Set the debug mask for the interpreter.
      void set_debug_mask(const uint _debug_mask);

      /// If not using trigger callbacks, then call this function to get the list of triggers that
      /// will be used this cycle, then call the step method that passes in the trigger values.
      bool getTriggersWillUse(triggerInfo_T &triggerInfo);

      /// Run the interpreter once cycle using the specified value of the triggers.
      /// Should have called getTriggersWillUse before this call to get the required triggers.
      /// messages is the string vector of status messages that have arrived since the last step
      /// Returns true on success, false if the cpu had an error
      bool step(const triggerValues_T &triggerValues, string &outputAction, keyValueMap &outputParameters);

      /// Run the interpreter once cycle.
      /// messages is the string vector of status messages that have arrived since the last step
      /// Returns true on success, false if the cpu had an error
      bool step();

      /// Load the cmdl mission from the string buffer.
      /// No changes are made unless it returns true.
      bool load(const string &mission);

      /// Load the cmdl mission from the specified file.
      /// No changes are made unless it returns true.
      bool load_from_file(const string &filename);

      /// Reset the interpreter to the initial state
      void reset();
/*
      /// Return a pointer to the cmdl object
      cmdl *the_cmdl() const {return thecmdl;} 

      /// Is the mission script done?
      bool  isDone() const {return done;}

      /// Set the mission script as done
      void setDone() {done = true;}

      /// Define the status records returned
      class statusRec
      {
         public:
            statusRec(const string &name, double age, const string &block, int cmd, CMDLstatus::statusValues stat): robotName(name), recordAgeInSeconds(age), missionBlock(block), missionCommand(cmd), status(stat) {}

            string robotName;
            double recordAgeInSeconds;
            string missionBlock;
            int    missionCommand; 
            CMDLstatus::statusValues status;
      };
      typedef vector< statusRec > statusRecs;
      statusRecs getStatus();
*/
      // *********************************************************************
   private:

      /// comm callback function to update our status cache as messages arrive
      static void updateStatus(commMsg *msg, void *ptr);

      /// objcomms callback function to process received commands
      static void processCommand(commMsg *msg, void *ptr);

      /// Based on the status message, are we on same command as the other robot?
      bool equivalentStep(const CMDLstatus &msg) const;

      /// Get the value of a trigger
      bool checkTrigger(const string &name, const keyValueMap &options);

      /// Do we need to use a barrier to sync operations with the group before
      /// moving to the next command in the mission?
      bool needSync() const;

      /// Step the mission to the first command for us to execute.
      /// Returns: 
      ///   true on success, command is ready to execute
      ///   false if there are no commands for us in the mission
      bool firstCommand();

      /// Step the mission to the next command for us to execute.
      /// Returns: 
      ///   true on success, command is ready to execute
      ///   false if the old command was the last one in the mission
      bool nextCommand();

      /// If time to send a status message, send it.
      /// returns true on success, false if comms error
      bool sendStatusMessage();

      /// send a resync status message
      /// returns true on success, false if comms error
      bool sendResyncStatusMessage(const string &block, uint cmd);

      /// execute the action
      bool executeAction(const string &name, const keyValueMap &options);

      /// Returns true on success, false if the cpu had an error
      bool internalStep();

      /// returns true if resync is active
      bool handleResyncs();

      /// Get the context-sensitive value to use for the commsTimeout
      TimeOfDay getCommsTimeout() const;

      /// Get the context-sensitive value to use for the waitTimeout
      TimeOfDay getWaitTimeout() const;

      /// a convience type for defining flags
      typedef map<string, bool> Flags;

      /// The list of legal actions
      stringSet actions;

      /// The actions callbacks are specified with a map.
      /// The key is the name of the action (1:1 correspondance with actionRecs_T)
      /// The value is a callback which executes the action
      Actions_T actionFncs;

      /// The list of legal triggers
//      typedef set<string> stringSet;
      stringSet triggers;

      /// list of default completion triggers.
      /// key is action name
      /// value is trigger name
      typedef map<string, string> completionTriggers_T;
      completionTriggers_T defaultCompletionTriggers;

      /// The trigger callbacks
      Triggers_T triggerCallbacks;

      /// Maintains the debug selections.
      uint  debug_mask;

      /// The cmdl state and transition information.
      cmdl *thecmdl;

      /// The set of loaded missions
      typedef map< string, cmdl * > missionList;
      missionList loadedMissions;

      /// The environment we are operating within
      class environment *theEnvironment;

      /// Are we done and should exit?
      bool done;

      /// Our name
      static string ourName;

      /// a callback to execute an action
      executeActionFnc_T executeActionFnc;

      /// user data passed along with the executeActionFnc
      void *actionUserData;

      /// a callback to execute a trigger
      executeTriggerFnc_T executeTriggerFnc;

      /// user data passed along with the executeTriggerFnc
      void *triggerUserData;

      // **********************************************************************
      /// The latest status for each robot on the team
      class statusRec
      {
         public:
            statusRec(CMDLstatus *rec);
            ~statusRec() {if( record ) delete record; record = NULL;}
            CMDLstatus *record;
            TimeOfDay   arrivalTime;

            /// Is this robot waiting for us at a barrier?
            bool        waitingForUs;
      };
      typedef map< string, statusRec *, less< string > > latestStatus_T;
      latestStatus_T latestStatus;
      semaphore statusLock;

      /// our status for the active command
      CMDLstatus::statusValues statusValue;

      /// List of any robots we are ignoring
      stringSet forcedIgnoreList;
      semaphore forcedIgnoreListLock;

      /// check if we are ignoring the specified robot
      /// returns true if should ignore robot, false otherwise
      bool areIgnoring(const string &robotName);

      /// Name of the loiter command emitted while blocked waiting at a barrier
      const string loiterCmdName;

      /// Reference to the loiter command callback
      ActionCallback loiterCmd;

      /// A set of operating modes for calling triggers and actions
      typedef enum {PreviewMode, CachedMode, DirectMode, IndirectMode} callModes_T;
      callModes_T callMode;

      /// If true, use callback functions.  If false, execute callbacks.
      bool directMode;

      /// Period between status messages when in normal mode
      TimeOfDay normalPeriod;

      /// Period between status messages when in a syncronization state
      TimeOfDay syncingPeriod;

      /// Last time we sent a status message
      TimeOfDay lastMessage;

      /// Are we in syncing mode for coms?
      bool syncing;

      /// Used by checkTrigger to flag triggers used instead of getting their value
      bool inPreviewMode;

      /// Used to cache the set of triggers that will be used this step.
      triggerInfo_T *triggersUsed;

      /// used to cache action for this cycle in cached mode
      string actionName;
      keyValueMap actionOptions;

      /// cached trigger values
      triggerValues_T inputTriggerValues;

      /// The state record pushed during a procedure call
      class stateInfo
      {
	 public:
	    stateInfo(Block *activeBlock, uint activeCommand) :
	       block(activeBlock),
	       cmdIndex(activeCommand)
	       { /* empty */ };

	    Block *block;
	    uint   cmdIndex;
      };
      
      stack< stateInfo > callStack;

      /// Internal function to initialize the comms subsystem
      /// returns true on success, false otherwise
      bool initComms(bool compression, uint debug);

      // the comm object
      class objcomms *comobj;

      /// our current runstate and lock
      semaphore runStateLock;
      typedef enum {RS_RUNNING=0, RS_PAUSED, RS_STOPPED} runState_T;
      runState_T runState;

      /// list of other nodes we are waiting for at this barrier
      strings blocklist;

      typedef list< stateInfo > Barriers_T;
      typedef map< string, Barriers_T, less< string > > IgnoreList_T;
      IgnoreList_T ignoreList;

      /// Declare the rc table
      resources rc_table;
      string rc_filename;
};

/*********************************************************************/
}
#endif
