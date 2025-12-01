#ifndef Block_H
#define Block_H
/**********************************************************************
 **                                                                  **
 **  Block.h                                                         **
 **                                                                  **
 **  Keep track of all the information involved with a single robot  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: Block.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: Block.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:19:14  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.11  2004/10/29 22:38:36  doug
// working on waittimeout
//
// Revision 1.10  2004/10/22 21:40:29  doug
// added goto and command exception support
//
// Revision 1.9  2004/05/11 19:34:49  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.8  2004/03/19 22:48:21  doug
// working on exceptions.
//
// Revision 1.7  2004/03/09 21:36:08  doug
// promote Blocks to mission segments and procedures
//
// Revision 1.6  2004/03/01 23:37:51  doug
// working on real overlay
//
// Revision 1.5  2004/02/20 14:39:35  doug
// cmdl interpreter runs a bit
//
// Revision 1.4  2003/11/13 22:29:41  doug
// adding transitions
//
// Revision 1.3  2003/11/10 22:48:30  doug
// working on cmdli
//
// Revision 1.2  2003/11/07 22:54:51  doug
// working on cmdl_io
//
// Revision 1.1  2003/11/03 16:45:16  doug
// snapshot
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include <queue>
#include "cmdl_transition.h"
#include "resources.h"

namespace sara 
{
// ********************************************************************
const string DefaultBlockNameRoot = "L";
/***************************************************************************/
class Block 
{
   public:
      /// Constructor
      Block();

      /// Destructor
      ~Block();

      /// Stream out in a form suitable for the loader
      friend ostream & operator << (ostream & out, const Block &r);

      // maintain the label on this Block.
      bool is_labeled() const {return isLabeled;}
      void set_label(const string &theLabel) {isLabeled = true; label = theLabel;}
      const string &get_label() const {return label;}

      /// adds the command to the end of this block
      void addCommand(class Command *cmd);

      /// Get the specified command
      /// Returns a pointer to the specified command or NULL if index is invalid
      /// NOTE: The user must not delete the returned record!
      class Command *getCommand(const uint index) const;

      /// Is the command index valid?
      bool isValidCommandIndex(const uint index) const {return index < commands.size();}

      /// Is the command label valid?
      bool isValidCommandLabel(const string &label) const;

      /// get the index for the specified command label.
      /// returns -1 if label not found
      int getCommandIndex(const string &label) const;

      /// Is this the last command in the block
      bool isLastCommand(const uint index) const {return index == (commands.size() - 1);}

      /// adds the list of transitions to this block
      void addException(cmdl_transition *exception) { transitions.push_back( exception );}
      void addExceptions(const cmdl_transition::Transitions_T &exceptions);

      /// The list of guard transitions when this block is active
      cmdl_transition::Transitions_T transitions;

      /// Get the list of robots mentioned in this mission
      const stringSet &getRobotNames() const {return names;}

      /// does the specified name exist in the list of robots?
      bool robotNameExists(const string &name) const {return names.find(name) != names.end();}

      /// return the number of robots mentioned
      uint numRobotNames() const {return names.size();}

      /// Get a reference to the set of block-specific options
      /// NOTE: The caller must not delete the returned object!
      resources *getOptions() {return &options;}

   private:
      bool isLabeled;
      string label;

      typedef vector < class Command * > Commands_T;
      Commands_T commands;

      /// The list of robots mentioned in the block
      stringSet names;

      /// Any block-specific execution options
      resources options;
};

// list of Blocks
typedef queue<Block *> BlockList;

/***************************************************************************/
}
#endif
