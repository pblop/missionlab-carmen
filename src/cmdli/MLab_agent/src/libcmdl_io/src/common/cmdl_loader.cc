/**********************************************************************
 **                                                                  **
 **  cmdl_loader.cc                                                  **
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

/* $Id: cmdl_loader.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

/**********************************************************************
* $Log: cmdl_loader.cc,v $
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
* Revision 1.11  2004/11/12 21:56:59  doug
* renamed loader so doesn't conflict with missionlab
*
* Revision 1.10  2004/11/04 21:36:39  doug
* moved test.cc to util
*
* Revision 1.9  2004/05/11 19:34:49  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.8  2004/03/09 21:36:08  doug
* promote Blocks to mission segments and procedures
*
* Revision 1.7  2004/03/08 14:52:34  doug
* cross compiles on visual C++
*
* Revision 1.6  2004/02/20 14:39:35  doug
* cmdl interpreter runs a bit
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

#include "mic.h"
#include "cmdl_parser.h"
#include "cmdl_scanner.h"
#include "cmdl_Root.h"
#include "cmdl_loader.h"

namespace sara 
{
/*********************************************************************/
/// Constructor: Save pointer to cmdl we are loading and set debug level to 0.
cmdl_loader::cmdl_loader(cmdl *_thecmdl, environment *theEnv) :
   thecmdl(_thecmdl),
   debug_mask(cmdl::NONE),
   env(theEnv)
{ /* empty */ }

// *********************************************************************
// Add the block to the current mission
// takes over ownership of rec
void 
cmdl_loader::addBlock(Block *block)
{
   if( !block )
   {
      WARN("cmdl_loader::addBlock - ignoring NULL block");
      return;
   }

   thecmdl->addBlock(block); 
}

// *********************************************************************
/// Add the block to the current mission and mark it as the main block
/// takes over ownership of rec
void 
cmdl_loader::addMainBlock(Block *block)
{
   if( !block )
   {
      WARN("cmdl_loader::addMainBlock - ignoring NULL block");
      return;
   }

   thecmdl->addMainBlock(block); 
}

/*********************************************************************/
/// Returns true if it is able to load the buffer without errors
// If ourName is set, then checks that actions are defined
bool
cmdl_loader::load(const string &ourName, const char *buffer, const int buflen)
{
   // Initialize for parsing
   cmdl_user_debug = (debug_mask & cmdl::USER_MSGS) != 0;
   sara_cmdl_debug = (debug_mask & cmdl::YACC_MSGS) != 0;
   sara_cmdl__flex_debug = (debug_mask & cmdl::LEX_MSGS) != 0;
   load_error = false;
   the_cmdl_loader = this;     // HACK since parser is not C++, give it a ptr.
   set_active_Block = false;

   // make sure the parser has the correct environment
   cmdl_root = new cmdl_Root();
   cmdl_root->setEnvironment(env);
   cmdl_root->set_ourName(ourName);

   // Setup the parse buffer
   sara_cmdl_init(buffer, buflen);

   // Process the buffer
   sara_cmdl_parse();

   // Free the buffer
   sara_cmdl_exit();

   the_cmdl_loader = NULL;		// Try to catch error references

   // If worked, then return the info
   if( !load_error )
   {
      thecmdl->setRoot( cmdl_root );

      // If the mission didn't explicitly set an active Block, copy the start Block over as default.
      if( !set_active_Block )
      {
         thecmdl->setActiveBlock( thecmdl->getMainBlock() );
      }
   }
   else
   {
      // had error.  Clean up
      delete cmdl_root;
   }

   // Done, return the NOT of had_error
   return !load_error;
}

/*********************************************************************/
}
