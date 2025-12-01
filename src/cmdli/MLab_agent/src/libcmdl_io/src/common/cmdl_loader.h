#ifndef cmdl_LOADER_H
#define cmdl_LOADER_H
/**********************************************************************
 **                                                                  **
 **  cmdl_loader.h                                                   **
 **                                                                  **
 **  An object which will load cmdl missions                         **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: cmdl_loader.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

/**********************************************************************
* $Log: cmdl_loader.h,v $
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
* Revision 1.11  2004/11/04 21:36:39  doug
* moved test.cc to util
*
* Revision 1.10  2004/10/25 22:58:16  doug
* working on mars2020 integrated demo
*
* Revision 1.9  2004/05/11 19:34:49  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.8  2004/03/19 22:48:21  doug
* working on exceptions.
*
* Revision 1.7  2004/03/09 21:36:08  doug
* promote Blocks to mission segments and procedures
*
* Revision 1.6  2004/03/08 14:52:34  doug
* cross compiles on visual C++
*
* Revision 1.5  2004/01/09 18:46:03  doug
* back from Florida
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

#include "cmdl.h"

namespace sara 
{
/*********************************************************************/

class cmdl_loader
{
   public:
      // Constructor
      cmdl_loader(class cmdl *thecmdl, class environment *env);

      // Destructor
      ~cmdl_loader() {};

      // Load an cmdl from the buffer
      // Returns true if successful.
      // If ourName is set, then checks that actions are defined
      bool load(const string &ourName, const char *buf, const int buflen);

      // Set the debug flags for the loader (bitmask).
      /// Create the mask by ORing flags together, or pass NONE to disable.
      ///    NONE          : no debugging
      ///    LOADER_USER   : user messages from the loader.
      ///    LOADER_YACC   : yacc and lex messages from loader.
      void set_debug_mask(const uint debugMask) {debug_mask = debugMask;}

      /// Called by the loader if it has an error.
      void set_error() {load_error = true;}

      /// Did the loader have an error yet?
      bool had_error() const {return load_error;}

      /*********************************************************************/
      // the cmdl_parser.y support functions follow
      /*********************************************************************/
 
      // return a pointer to the Block record, or NULL if not found.
      Block *lookupBlockName(const string &name)
	{ return thecmdl->lookupBlockName(name); }

      // has the name been defined as a Block?
      bool isBlockName(const string &name) const
	{ return thecmdl->isBlockName(name); }

      // has the name been defined as a trigger?
      bool isTriggerName(const string &name) const
	{ return thecmdl->isTriggerName(name); }

      // has the name been defined as an action?
      bool isActionName(const string &name) const
	{ return thecmdl->isActionName(name); }

      // define the name as the active Block
      void setActiveBlock(const string &name)
      { 
         set_active_Block = true; 
         thecmdl->setActiveBlock(name); 
      }

      // Set the mission name
      void setMissionName(const string &name)
      { 
         thecmdl->setMissionName(name);
      }

      // Set the mission version
      void setMissionVersion(const uint ver)
      { 
         thecmdl->setMissionVersion(ver); 
      }

      /// Add the block to the current mission
      /// takes over ownership of rec
      void addBlock(class Block *block);

      /// Add the block to the current mission and mark it as the main block
      /// takes over ownership of rec
      void addMainBlock(Block *block);

      /// adds the list of global transitions 
      void addExceptions(const cmdl_transition::Transitions_T &exceptions)
	{ thecmdl->addExceptions(exceptions); }

      /*********************************************************************/
   private:

      bool  load_error;	// true if yacc found an error
      cmdl *thecmdl;		// Pointer to the cmdl record.
      bool  set_active_Block;   // Set when the loader adds an active Block.

      /// Maintains the debug selections.
      uint  debug_mask;

      /// Our operating environment
      class environment *env;
};

/*********************************************************************/
}
#endif
