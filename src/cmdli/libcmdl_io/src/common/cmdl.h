#ifndef cmdl_H
#define cmdl_H
/**********************************************************************
 **                                                                  **
 **  cmdl.h                                                          **
 **                                                                  **
 **  The cmdl object implements an interpreted cmdl machine          **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: cmdl.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

/**********************************************************************
* $Log: cmdl.h,v $
* Revision 1.1.1.1  2008/07/14 16:44:15  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:45  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.2  2006/07/11 10:30:45  endo
* Compiling error fixed.
*
* Revision 1.1  2006/07/01 00:19:15  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.17  2004/11/04 21:36:39  doug
* moved test.cc to util
*
* Revision 1.16  2004/10/29 22:38:36  doug
* working on waittimeout
*
* Revision 1.15  2004/10/22 21:40:29  doug
* added goto and command exception support
*
* Revision 1.14  2004/05/11 19:34:49  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.13  2004/03/19 22:48:21  doug
* working on exceptions.
*
* Revision 1.12  2004/03/18 22:11:17  doug
* coded call and return
*
* Revision 1.11  2004/03/09 21:36:08  doug
* promote Blocks to mission segments and procedures
*
* Revision 1.10  2004/03/08 14:52:34  doug
* cross compiles on visual C++
*
* Revision 1.9  2004/03/01 23:37:51  doug
* working on real overlay
*
* Revision 1.8  2004/02/28 16:35:42  doug
* getting cmdli to work in sara
*
* Revision 1.7  2004/02/20 14:39:35  doug
* cmdl interpreter runs a bit
*
* Revision 1.6  2004/01/09 18:46:03  doug
* back from Florida
*
* Revision 1.5  2003/11/13 22:29:41  doug
* adding transitions
*
* Revision 1.4  2003/11/11 22:04:25  doug
* getting it to execute
*
* Revision 1.3  2003/11/10 22:48:30  doug
* working on cmdli
*
* Revision 1.2  2003/11/07 22:54:51  doug
* working on cmdl_io
*
* Revision 1.1  2003/11/07 20:31:00  doug
* adding interpreter
*
**********************************************************************/

#include "Block.h"
#include "cmdl_transition.h"
#include "cmdl_Root.h"

namespace sara 
{
/*********************************************************************/
   
// Declare a type to hold our Block records.
//typedef map<string, Block *, less<string> > BlockS_T;

/*********************************************************************/
/// Magic numbers
const string DefaultBlockAction = "pause";
const uint   DefaultBlockX      = 0;
const uint   DefaultBlockY      = 0;
/*********************************************************************/

class cmdl
{
   public:
      typedef enum {
         NONE        = 0,        // no debugging
         USER_MSGS   = 1 << 0,   // print user messages from the loader.
         YACC_MSGS   = 1 << 1,   // print yacc messages from loader.
         LEX_MSGS    = 1 << 2    // print lex messages from loader.
      } cmdl_DEBUG_T;


      /// Constructor
      cmdl(const stringSet &legalActions, const stringSet &legalTriggers, class environment *env);

      /// Destructor
      ~cmdl();

      // ********** load, print, and unload the mission *******************
      // Set the debug level for the loader
      void set_loader_debug(cmdl_DEBUG_T debug_level);

      /// Load an cmdl from a text buffer
      /// if ourName is not empty, then checks that actions are defined for us.
      /// Returns true if successful.
      bool load(const string &ourName, const string &buf);

      /// Return the current cmdl as text suitable for the load function.
      string print() const;

      /// Reset the cmdl to an empty configuration.
      void unload();

      // ********** maintain the available actions and triggers **********
      /// Return the list of available triggers
      string listTriggers();

      /// has the name been defined as a trigger?
      bool isTriggerName(const string &name) const;

      /// Return the list of available actions 
      string listActions() const;

      /// has the name been defined as an action?
      bool isActionName(const string &name) const;

      /// ********* Maintain the mission ********************
      /// Set the root of the mission
      void setRoot(cmdl_Root *theRoot) {root = theRoot;}

      /// return the list of robots named in this mission
      stringSet getRobotNames();

      /// return a pointer to the Block record, or NULL if not found.
      Block *lookupBlockName(const string &name) const;
      
      /// has the name been defined as a Block?
      bool isBlockName(const string &name) const;

      /// Get reference to the main Block
      Block * getMainBlock() const;

      /// Define the active Block (in case restarting already underway).
      bool setActiveBlock(const string &name);

      /// Define the active Block 
      bool setActiveBlock(Block *block) {activeBlock = block; return true;}

      /// Define the active Block and command 
      /// returns true if the block and command were changed
      ///         false if one is invalid and nothing was changed.
      bool setActiveBlockAndCommand(const string &blockName, const uint cmdIndex);

      /// Get the index of the active command within the active Block
      Block *getActiveBlock() const {return activeBlock;}

      /// Define the index of the active command within the active Block
      /// returns true on success
      /// returns false if no changes were made because index was out of range
      bool setActiveCommandIndex(const uint index);

      /// Move us to the specified command
      /// returns true on success
      /// returns false if no changes were made because the label was not found
      bool setActiveCommand(const string &label);

      /// Increment the index of the active command within the active Block
      /// returns true on success
      /// returns false if no changes were made because new index was out of range
      bool incActiveCommand();

      /// Get the index of the active command within the active Block
      uint getActiveCommand() const {return activeCommandIndex;}

      /// Add the block to the mission
      /// takes over ownership of rec
      /// Returns true on success, false if the label is already used
      bool addBlock(class Block *block);

      /// Add the block to the mission and define it as the main Block
      /// takes over ownership of rec
      /// Returns true on success, false if the label is already used
      bool addMainBlock(class Block *block);

      // ********** naming the mission **************
      /// Get the mission name
      const string &getMissionName() const {return missionName;}

      /// Set the mission name
      void setMissionName(const string &name) {missionName = name;}

      /// Get the mission version number
      uint getMissionVersion() const {return missionVersion;}

      /// Set the mission version number
      void setMissionVersion(uint version) {missionVersion = version;}

      // ********** misc **************
      /// Reset the cmdl to the start Block.
      void reset();

      /// Stream out in a form suitable for the loader
      friend ostream & operator << (ostream & out, const cmdl &m);

      /// get the extra record
      class cmdl_extra *get_extra() const {return extra;}
      
      // Get a pointer to the current environment
      class environment *currentEnvironment() { return theEnvironment;}

      /// adds the list of global transitions 
      void addExceptions(const cmdl_transition::Transitions_T &exceptions);

      /// The list of global interupt transitions
      cmdl_transition::Transitions_T transitions;

      /// Set this robot's name
      void set_ourName(const string &name) {root->set_ourName(name);}

      /// Get a reference to the set of default options
      /// NOTE: The caller must not delete the returned object!
      resources *getOptions() {return root->getOptions();}

      // ********************************************************************
   private:
      // our debug flags
      cmdl_DEBUG_T  loader_debug;

      /// Name of the mission
      string        missionName;	

      /// Version number of this instance of the mission.
      uint          missionVersion;        

      /// blocks in this mission, indexed by their label
      typedef std::map<const std::string, class Block *> Blocks_T;
      Blocks_T        blocks;
 
      /// The main block for this mission
      Block          *mainBlock;

      /// The currently active block
      Block          *activeBlock;

      /// The index of the active command within the active block
      uint            activeCommandIndex; 

      // Declare containers to hold the actions and trigger names
      stringSet       actions;		// The available actions
      stringSet       triggers;		// The available triggers

      /// The environment we are working against
      class environment *theEnvironment;

      /// Pointer to implementation of the extra.
      class cmdl_extra *extra;

      /// The root of the cmdl mission
      cmdl_Root *root;
};

/*********************************************************************/
}
#endif
