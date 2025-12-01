#ifndef cmdli_H
#define cmdli_H
/**********************************************************************
 **                                                                  **
 **  cmdli.h                                                          **
 **                                                                  **
 **  The cmdl interpreter                                             **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: cmdli.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: cmdli.h,v $
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
* Revision 1.21  2004/05/11 19:34:50  doug
* massive changes to integrate with USC and GaTech
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
#include "environment.h"
//#include <stack>
//#include "cmdl.h"
//#include "CMDLstatus.h"
//#include "semaphore.h"
//#include "resources.h"

namespace sara
{
// *********************************************************************
class cmdli
{
   public:
      /// Destructor
      virtual ~cmdli() {};

      /// A triple used to return true or false if the trigger executes and failed otherwise.
      typedef enum {TriggerFalse=0, TriggerTrue, TriggerFailed} TriggerReturnValue_T;

      /// prototype for a trigger callback function
      /// parms are the parameters for this invocation, passed as a map of name/value pairs
      /// returns true if the trigger is true, false otherwise
      typedef TriggerReturnValue_T (* TriggerCallback)(const keyValueMap &parms);

      /// prototype for an action callback function
      /// parms are the parameters for this invocation
      /// returns true if the action ran, false if it encountered fatal configuration errors.
      typedef bool (* ActionCallback)(const keyValueMap &parms);

      /// The triggers are specified with a map.
      /// The key is the name of the trigger (case sensitive)
      /// The value is a callback which returns the value of the trigger
      /// Only the relevant triggers will be checked on each step of the interpreter
      typedef std::map<std::string, TriggerCallback> Triggers_T;

      /// Each legal action must have a default completion trigger associated with it.
      /// The key is the name of the action (case sensitive)
      /// The value is the name of the default completion trigger (case sensitive)
      typedef std::map<std::string, std::string> defaultActionCompletionTriggers_T;

      /// The actions callbacks are specified with a map.
      /// The key is the name of the action (1:1 correspondance with actionRecs_T)
      /// The value is a callback which executes the action
      typedef std::map<std::string, ActionCallback> Actions_T;

      /// Constructor for the cmdl interpreter object.
      static cmdli *createInstance();

      /// First method to init the cmdl interpreter object.
      /// This instance directly invokes triggers and actions.
      /// returns true on success, false otherwise
      virtual bool init(const defaultActionCompletionTriggers_T &defaultActionCompletionTriggers, Actions_T &actionFncs, Triggers_T &legalTriggers, environment *env, const std::string &myName) = 0;

      /// prototype for function the interpreter can use to invoke an action
      /// name is the name of the action to invoke
      /// parms are the parameters for this invocation
      /// Returns true if action will be or was executed, false if permanent failure
      typedef bool (* executeActionFnc_T)(void *userData, const std::string &name, const keyValueMap &parms);

      /// prototype for function the interpreter can use to invoke a trigger
      /// name is the name of the trigger to invoke
      /// Returns true if action will be or was executed, false if permanent failure
      typedef TriggerReturnValue_T (* executeTriggerFnc_T)(void *userData, const std::string &name, const keyValueMap &parms);

      /// Second method to init the cmdl interpreter object.
      /// This instance calls a mediating function to invoke triggers and actions.
      /// triggers is the set of legal trigger names
      /// defaultActionCompletionTriggers is a map of action names to the name of their default termination triggers.
      /// returns true on success, false otherwise
      virtual bool init(const stringSet &triggers, const defaultActionCompletionTriggers_T &defaultActionCompletionTriggers, executeActionFnc_T executeAction, void *actionUserData, executeTriggerFnc_T executeTrigger, void *triggerUserData, environment *env, const std::string &myName) = 0;

      /// Third method to init the cmdl interpreter object.
      /// This instance relies on the second form of step
      /// defaultActionCompletionTriggers is a map of action names to the name of their default termination triggers.
      /// returns true on success, false otherwise
      virtual bool init(const stringSet &triggers, const defaultActionCompletionTriggers_T &defaultActionCompletionTriggers, environment *env, const std::string &myName) = 0;

      /// Create the debug mask by ORing flags together, or pass NONE to disable.
      typedef enum {
         NONE            = 0,               // no debugging
//         LOADER_USER     = cmdl::USER_MSGS, // user messages from the loader.
         LOADER_USER     = 1 << 0, // user messages from the loader.
         LOADER_YACC     = 1 << 1, // yacc messages from loader.
//         LOADER_YACC     = cmdl::YACC_MSGS, // yacc messages from loader.
//         LOADER_LEX      = cmdl::LEX_MSGS,  // lex messages from loader.
         LOADER_LEX      = 1 << 2,  // lex messages from loader.
         INTERPRETER     = 1 << 3,          // execution debugging messages.
         SYNCHRONIZATION = 1 << 4           // detailed synchronization debugging messages.
      } cmdli_DEBUG_T;

      /// Set the debug mask for the interpreter.
      virtual void set_debug_mask(const uint _debug_mask) = 0;

      /// If not using trigger callbacks, then call this function to get the list of triggers that
      /// will be used this cycle, then call the step method that passes in the trigger values.
      typedef std::map<std::string, keyValueMap> triggerInfo_T; 
      virtual bool getTriggersWillUse(triggerInfo_T &triggerInfo) = 0;

      /// Run the interpreter once cycle using the specified value of the triggers.
      /// Should have called getTriggersWillUse before this call to get the required triggers.
      /// messages is the string vector of status messages that have arrived since the last step
      /// Returns true on success, false if the cpu had an error
      typedef std::map<std::string, bool> triggerValues_T; 
      virtual bool step(const triggerValues_T &triggerValues, std::string &outputAction, keyValueMap &outputParameters) = 0;

      /// Run the interpreter once cycle.
      /// messages is the string vector of status messages that have arrived since the last step
      /// Returns true on success, false if the cpu had an error
      virtual bool step() = 0;

      /// Load the cmdl mission from the string buffer.
      /// No changes are made unless it returns true.
      virtual bool load(const std::string &mission) = 0;

      /// Load the cmdl mission from the specified file.
      /// No changes are made unless it returns true.
      virtual bool load_from_file(const std::string &filename) = 0;

      /// Reset the interpreter to the initial state
      virtual void reset() = 0;

   private:
      /// expose the constructor to the base class
      friend class cmdli_implementation;

      /// Hide the constructor for the cmdl interpreter object.
      /// use createInstance() instead to create a new object.
      cmdli() {};
};

/*********************************************************************/
}
#endif
