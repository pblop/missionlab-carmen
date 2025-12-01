/**********************************************************************
 **                                                                  **
 **  Block.cc - A parameter list                                **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: Block.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: Block.cc,v $
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
// Revision 1.9  2004/10/22 21:40:29  doug
// added goto and command exception support
//
// Revision 1.8  2004/05/11 19:34:49  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.7  2004/03/19 22:48:21  doug
// working on exceptions.
//
// Revision 1.6  2004/03/08 14:52:34  doug
// cross compiles on visual C++
//
// Revision 1.5  2004/03/01 00:47:15  doug
// cmdli runs in sara
//
// Revision 1.4  2004/02/20 14:39:35  doug
// cmdl interpreter runs a bit
//
// Revision 1.3  2003/11/13 22:29:41  doug
// adding transitions
//
// Revision 1.2  2003/11/10 22:48:30  doug
// working on cmdli
//
// Revision 1.1  2003/11/03 16:45:16  doug
// snapshot
//
///////////////////////////////////////////////////////////////////////


#include "mic.h"
#include "Block.h"
#include "Unit.h"
#include "Command.h"
#include "env_object.h"

namespace sara 
{
/***************************************************************************/
// Constructor
Block::Block() : 
   isLabeled(false)
{ /* empty */ }

/***************************************************************************/
// Destructor
Block::~Block()
{
}

// ***************************************************************************
/// adds the command to the end of this block
void 
Block::addCommand(Command *cmd)
{
   commands.push_back(cmd);

   // get the list of robots used in this command
   stringSet subs = cmd->getUnit()->getRobotNames();

   // add any new ones into our set
   stringSet::const_iterator sit;
   for(sit=subs.begin(); sit!=subs.end(); ++sit)
   {
      if( names.find( *sit ) == names.end() )
      {
         names.insert( *sit );
      }
   }
}

// ***************************************************************************
/// Get the specified command
/// Returns a pointer to the specified command or NULL if index is invalid
/// NOTE: The user must not delete the returned record!
Command *
Block::getCommand(const uint index) const
{
   Command *rtn(NULL);
   if( index < commands.size() )
   {
      rtn = commands.at(index);
   }

   return rtn;
};

// ***************************************************************************
/// adds the list of transitions to this block
void 
Block::addExceptions(const cmdl_transition::Transitions_T &exceptions)
{
   cmdl_transition::Transitions_T::const_iterator it;
   for(it=exceptions.begin(); it!=exceptions.end(); ++it)
   {
      transitions.push_back( *it );
   }
}

// ***************************************************************************
// Stream out in a form suitable for the loader
ostream & operator << (ostream & out, const Block &r)
{
   if( r.is_labeled() )
   {
      out << "DefProcedure \"" << r.get_label() << "\" " << endl;
   } 

   for(uint i=0; r.isValidCommandIndex(i); i++)
   {
      Command *cmd = r.getCommand(i);

      if( cmd->is_labeled() )
      {
         out << cmd->get_label() << ". ";
      }
      else
      {
         out << "   ";
      }

      Unit *u = cmd->getUnit();
      if( u->isNamed() )
         out << "unit " << u->getName() << " ";
       
      out << " ";

      out << r.getCommand(i)->print() << endl;
   }

   if( r.is_labeled() )
   {
      out << "EndProcedure" << endl;
   } 

   return out;
}

// ***************************************************************************
/// Is the command label valid?
bool 
Block::isValidCommandLabel(const string &label) const
{
   Commands_T::const_iterator it;
   for(it=commands.begin(); it!=commands.end(); ++it)
   {
      if( (*it)->is_labeled() && label == (*it)->get_label() )
      {
	 return true;
      }
   }

   return false;
}

// ***************************************************************************
/// get the index for the specified command label.
/// returns -1 if label not found
int 
Block::getCommandIndex(const string &label) const
{
   for(int index=0; index < (int)commands.size(); index++)
   {
      if( commands[index]->is_labeled() && commands[index]->get_label() == label )
      {
	 return index;
      }
   }

   return -1;
}

// ***************************************************************************
}
