/**********************************************************************
 **                                                                  **
 **  cmdl.cc                                                         **
 **                                                                  **
 **  The cmdl object implements and interpreted cmdl machine         **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: cmdl.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

/**********************************************************************
* $Log: cmdl.cc,v $
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
* Revision 1.19  2004/11/04 21:36:39  doug
* moved test.cc to util
*
* Revision 1.18  2004/10/22 21:40:29  doug
* added goto and command exception support
*
* Revision 1.17  2004/05/11 19:34:49  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.16  2004/03/19 22:48:21  doug
* working on exceptions.
*
* Revision 1.15  2004/03/18 22:11:17  doug
* coded call and return
*
* Revision 1.14  2004/03/17 21:53:33  doug
* working on subroutine calls
*
* Revision 1.13  2004/03/11 19:43:42  doug
* splitting off from Command
*
* Revision 1.12  2004/03/09 21:36:08  doug
* promote Blocks to mission segments and procedures
*
* Revision 1.11  2004/03/08 14:52:34  doug
* cross compiles on visual C++
*
* Revision 1.10  2004/03/01 23:37:51  doug
* working on real overlay
*
* Revision 1.9  2004/02/28 16:35:42  doug
* getting cmdli to work in sara
*
* Revision 1.8  2004/02/20 14:39:35  doug
* cmdl interpreter runs a bit
*
* Revision 1.7  2004/01/15 00:24:09  doug
* working on cmdl
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

#include "mic.h"
#include <sstream>
#include <stdio.h>
#include "Block.h"
//#include "cmdl_transition.h"
#include "cmdl_loader.h"
#include "cmdl_extra.h"
#include "cmdl_parser.h"
#include "environment.h"
#include "cmdl.h"

namespace sara 
{
/*********************************************************************/
/// Constructor
cmdl::cmdl(const stringSet &legalActions, const stringSet &legalTriggers, environment *theEnv):
   loader_debug(NONE),
   mainBlock(NULL),
   activeBlock(NULL),
   activeCommandIndex(0),
   theEnvironment(theEnv),
   root(NULL)
{
   // Stuff the list of actions into our container.
   stringSet::const_iterator it;
   for(it = legalActions.begin(); it != legalActions.end(); ++it)
   {
      if( actions.find( *it ) != actions.end() )
      {
	 char buf[2048];
	 sprintf(buf,"Duplicate action '%s'", (*it).c_str() );

#ifdef USE_EXCEPTIONS
	 throw already(buf);
#else
         FATAL_ERROR(buf);
#endif
      }

      // Map the names to their index.
      actions.insert( *it );
   }

   // Stuff the list of triggers into our container.
   for(it = legalTriggers.begin(); it != legalTriggers.end(); ++it)
   {
      if( triggers.find( *it ) != triggers.end() )
      {
	 char buf[2048];
	 sprintf(buf,"Duplicate trigger '%s'", (*it).c_str() );
#ifdef USE_EXCEPTIONS
	 throw already(buf);
#else
         FATAL_ERROR(buf);
#endif
      }

      // Map the names to their index.
      triggers.insert( *it );
   }

   // Set a default name and version.
   missionName = "Unknown";
   missionVersion = 1;

   // Create the users extra data record.
   extra = cmdl_extra::create(this);
}

/*********************************************************************/
/// Destructor
cmdl::~cmdl()
{
   // Delete the blocks.
   for(Blocks_T::iterator it=blocks.begin(); it != blocks.end(); ++it)
   {
      delete it->second;
   }
   blocks.erase(blocks.begin(), blocks.end());

   // clear references (activeBlock and mainBlock are in blocks and deleted above)
   mainBlock = NULL;
   activeBlock = NULL;
   theEnvironment = NULL;

   // Delete the extra data
   if( extra )
   {
      // Delete the users extra data record.
      delete extra;
      extra = NULL;
   }

   // delete the root
   if( root )
   {
      delete root;
      root = NULL;
   }
}

/*********************************************************************/
/// Load an cmdl from a text buffer
/// if ourName is not empty, then checks that actions are defined for us.
/// Returns true if successful.
bool 
cmdl::load(const string &ourName, const string &buf)
{
   // Create a loader object.
   cmdl_loader loader(this, theEnvironment);

   // Pass on the debug level
   loader.set_debug_mask(loader_debug);

   // Try to load the buffer
   const char *cbuf = buf.c_str();
   bool ok = loader.load(ourName, cbuf, strlen(cbuf) );

   return ok;
}

/*********************************************************************/
/// Return the current cmdl as text suitable for the load function.
string 
cmdl::print() const
{
   ostringstream outs;

   // Dump the mission name 
   outs << "Mission Name \"" << missionName << "\"" << endl;; 
   outs << endl;

   // Define the robots
   outs << "// ----- Robot definitions ------" << endl;; 
   for(cmdl_Root::Robots_T::iterator it=root->robots.begin(); it != root->robots.end(); ++it)
   {
      outs << *(*it).second << endl;
   }

   // Define the robots
   outs << endl << "// ----- Unit definitions ------" << endl;; 
   for(cmdl_Root::Units_T::iterator it=root->units.begin(); it != root->units.end(); ++it)
   {
      outs << *(*it).second << endl;
   }

   outs << endl << "// ----- Mission definition ------" << endl;; 
   for(Blocks_T::const_iterator it=blocks.begin(); it != blocks.end(); ++it)
   {
      // do the main block last
      if( (*it).second != mainBlock )
         outs << *(*it).second << endl;
   }

   outs << *mainBlock;

   return outs.str();
}

// *********************************************************************
/// return the list of robots named in this mission
stringSet 
cmdl::getRobotNames() 
{
   if( root )
      return root->getRobotNames();
   else
      return stringSet();
}

// *********************************************************************
/// Return the list of available triggers 
string 
cmdl::listTriggers()
{
   ostringstream outs;

   // Dump the trigger names one per line

   for(stringSet::const_iterator it=triggers.begin(); it !=triggers.end(); ++it)
   {
      outs << '\"' << *it << '\"' << endl;
   }

   return outs.str();
}

/*********************************************************************/
/// Return the list of available actions 
string 
cmdl::listActions() const
{
   ostringstream outs;

   // Dump the action names one per line

   for(stringSet::const_iterator it=actions.begin(); it != actions.end(); ++it)
   {
      outs << '\"' << *it << '\"' << endl;
   }

   return outs.str();
}

/*********************************************************************/
/// Set the debug flags for the loader
void 
cmdl::set_loader_debug(cmdl_DEBUG_T debug_level)
{
   loader_debug = debug_level;
}

/*********************************************************************/
/// Is the name a legal action?
bool 
cmdl::isActionName(const string &name) const
{
   return actions.find(name) != actions.end();
}

/*********************************************************************/
/// Is the name a legal trigger?
bool 
cmdl::isTriggerName(const string &name) const
{
   return triggers.find(name) != triggers.end();
}

// ***************************************************************************
/// Add the block to the mission
/// takes over ownership of rec
/// Returns true on success, false if the label is already used
bool 
cmdl::addBlock(class Block *block)
{
   // make sure we have a label
   if( !block->is_labeled() )
   {
      ERROR("cmdl::addBlock - Internal Error: Ignoring unlabeled block!");
      return false;
   }

   // make sure the label is not a dup
   string label = block->get_label();
   if( blocks.find( label ) != blocks.end() )
   {
      ERROR("cmdl::addBlock - Internal Error: Ignoring duplicate definition of block '%s'!", label.c_str());
      return false;
   }

   // add it.
   blocks[ label ] = block;

   // good one
   return true;
}


// ***************************************************************************
/// Add the block to the mission and define it as the main Block
/// takes over ownership of rec
/// Returns true on success, false if there already is a main block
bool 
cmdl::addMainBlock(class Block *block)
{
   if( mainBlock )
   {
      ERROR("cmdl::addBlock - Internal Error: Ignoring redefinition of main block!");
      return false;
   }

   // if it isn't labeled, set it to Main
   if( !block->is_labeled() )
   {
      block->set_label("Main");
   }

   if( !addBlock(block) )
   {
      ERROR("cmdl::addBlock - Internal Error: Ignoring main block with invalid label!");
      return false;
   }

   mainBlock = block;

   return true;
}

// ********************************************************************
// Define the active Block by name
// returns true if Block exists, false otherwise
bool
cmdl::setActiveBlock(const string &name)
{
   // find the block
   Block *tmp = lookupBlockName(name);

   // make sure block was found
   if( !tmp )
      return false;

   // Good one, do it.
   activeBlock = tmp;
   activeCommandIndex = 0;

   return true;
}

// ********************************************************************
// Define the active Block by name
// returns true if Block exists, false otherwise
bool
cmdl::setActiveBlockAndCommand(const string &name, const uint cmdIndex)
{
   // find the block
   Block *tmp = lookupBlockName(name);

   // make sure block was found
   if( !activeBlock )
      return false;

   // make sure the command is valid
   if( !tmp->isValidCommandIndex(cmdIndex) )
      return false;

   // Good one, do it.
   activeBlock = tmp;
   activeCommandIndex = cmdIndex;

   return true;
}

// *********************************************************************
/// Is the name a legal Block?
bool 
cmdl::isBlockName(const string &name) const
{
   return blocks.find(name) != blocks.end();
}

/*********************************************************************/
/// Return a pointer to the Block info record, or NULL if not found.
Block *
cmdl::lookupBlockName(const string &name) const
{
   Blocks_T::const_iterator it=blocks.find(name);
   if( it != blocks.end() )
      return it->second;

   // Didn't find it.
   return NULL;
}

/*********************************************************************/
/// Get reference to the main Block
Block *
cmdl::getMainBlock() const
{
   return mainBlock;
}

/*********************************************************************/

/// Get ready to execute the cmdl from the Block Block.
void 
cmdl::reset()
{
   setActiveBlock( getMainBlock() );
}

// *********************************************************************
/// Define the index of the active command within the active Block
/// returns true on success
/// returns false if no changes were made because index was out of range
bool 
cmdl::setActiveCommandIndex(const uint index) 
{
   bool rtn(false);

   Block *block = getActiveBlock();
   if( block && block->isValidCommandIndex( index ) )
   {
      activeCommandIndex = index;
      rtn = true;
   }

   return rtn;
}

// *********************************************************************
/// Move us to the specified command
/// returns true on success
/// returns false if no changes were made because the label was not found
bool 
cmdl::setActiveCommand(const string &label) 
{
   Block *block = getActiveBlock();
   if( !block )
   {
      ERROR("cmdl::setActiveCommand - NULL activeBlock!");
      return false;
   }

   int index = block->getCommandIndex( label );
   if( index < 0 )
   {
      ERROR("cmdl::setActiveCommand - Invalid command label '%s'!", label.c_str());
      return false;
   }

   activeCommandIndex = index;
   return true;
}

// *********************************************************************
/// Increment the index of the active command within the active Block
/// returns true on success
/// returns false if no changes were made because new index was out of range
bool 
cmdl::incActiveCommand()
{
   bool rtn(false);

   Block *block = getActiveBlock();
   if( block && block->isValidCommandIndex( activeCommandIndex + 1 ) )
   {
      activeCommandIndex ++;
      rtn = true;
   }

   return rtn;
}

// ***************************************************************************
/// adds the list of global transitions 
void 
cmdl::addExceptions(const cmdl_transition::Transitions_T &exceptions)
{
   cmdl_transition::Transitions_T::const_iterator it;
   for(it=exceptions.begin(); it!=exceptions.end(); ++it)
   {
      transitions.push_back( *it );
   }
}

// *********************************************************************
// Stream out in a form suitable for the loader
ostream & operator << (ostream & out, const cmdl &m)
{
   out << m.print();
   return out;
}

//**************************************************************************
}
