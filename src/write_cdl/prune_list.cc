/**********************************************************************
 **                                                                  **
 **                          prune_list.cc                          **
 **                                                                  **
 **                                                                  **
 **  Code generator targeted for the Schema architecure using CNL.   **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: prune_list.cc,v 1.1.1.1 2008/07/14 16:44:25 endo Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <libc.h>

#include <Xm/Xm.h>

#include "list.hpp"
#include "load_cdl.h"
#include "configuration.hpp"
#include "globals.h"
#include "screen_object.hpp"
#include "page.hpp"
#include "version.h"
#include "prototypes.h"
#include "write_cdl.h"
#include "popups.h"

static bool
write_group(SymbolList *symlist, Symbol * node, bool embedded,
	bool in_line, bool use_list, const bool is_root);

static bool
write_list(SymbolList *symlist, Symbol * node, Symbol * index, 
	const bool is_root);

static RECORD_CLASS desired_class;

//******************************************************************** 
// Returns true if pruned entry
static bool
prune_entry(SymbolList *symlist, Symbol *node)
{
   bool pruned = false;

   if( symlist && node && node->name )
   { 
      Symbol *p;
      void *cur;
      cur = symlist->first(&p);
      while( cur != NULL )
      {
         if( strcmp(p->name, node->name) == 0 )
         {
            symlist->remove(p);
	    pruned = true;
            break;
         }

         cur = symlist->next(&p,cur);
      }
   }

   return pruned;
}

//********************************************************************
// returns true if there are pushed-up parms in this node
static bool
has_pu_parms(Symbol * node)
{
   if( node )
   {
      Symbol *p;
      void *cur;

      if ((cur = node->parameter_list.first(&p)) != NULL)
      {
         do
         {
	    if (p->symbol_type == PU_INPUT_NAME)
	    {
	       return true;
	    }
         }
         while ((cur = node->parameter_list.next(&p, cur)) != NULL);
      }
   }

   return false;
}


//********************************************************************

// Returns true if pruned entry
static bool
write_data(SymbolList *symlist, Symbol * p, Symbol * index, const bool is_root)
{
   if (p == NULL || p == (Symbol *) UP)
   {
      return false;
   }

   if (p->symbol_type == RULE_NAME)
   {
      return prune_list(symlist, p->input_generator, desired_class, false);
   }

   if (!p->is_instance)
   {
      if( !is_root )
         return prune_list(symlist,p, desired_class, is_root);
      return false;
   }

   if( has_pu_parms(index) )
   {
      return write_list(symlist, p, index, is_root);
   }

   return false;
}

//********************************************************************
// returns true if pruned entry
static bool
write_parms(SymbolList *symlist, Symbol * node, bool pu_only,const bool is_root)
{
   Symbol *p;
   void *cur;
   bool is_robot = node->symbol_type == ROBOT_NAME ||
	   		(node->symbol_type == BP_NAME && 
	    		 node->construction == CS_RBP &&
	    		 node->bound_to == NULL);

   if ((cur = node->parameter_list.first(&p)) != NULL)
   {
      do
      {
	 if (p->symbol_type == PU_INPUT_NAME)
	 {
	    if (p->input_generator)
	    {
	       if( write_data(symlist, p->input_generator, NULL, false) )
		  return true;
	    }
	 }
	 else if (!is_robot)
	 {
	    if (p->symbol_type == PARM_HEADER)
	    {
	       Symbol *lp;
	       void *cur_lp;

	       if ((cur_lp = p->parameter_list.first(&lp)) != NULL)
	       {
		  do
		  {
		     if (lp->input_generator)
		     {
			if( write_data(symlist, lp->input_generator, lp->list_index, false) )
			   return true;
		     }
		     else if (lp->list_index && lp->list_index->input_generator)
		     {
			if( write_data(symlist, lp->list_index->input_generator, lp->list_index, false) )
			   return true;
		     }
		  }
		  while ((cur_lp = p->parameter_list.next(&lp, cur_lp)) != NULL);
	       }
	    }
	    else
	    {
	       // Special case, the bound_to parm is a fake one.
	       if (node->symbol_type == BP_NAME && strcmp(p->name, "bound_to") == 0)
		  continue;

	       if (p->input_generator)
	       {
		  if( write_data(symlist, p->input_generator, NULL, false) )
		     return true;
	       }
	    }
	 }
      }
      while ((cur = node->parameter_list.next(&p, cur)) != NULL);
   }

   if (is_robot && !pu_only)
   {
      if( write_group(symlist, node, true, true, false, is_root) )
	 return true;
   }

   if (node->symbol_type == BP_NAME && node->bound_to)
   {
      if( write_data(symlist, node->bound_to, NULL, false) )
	 return true;
   }

   return false;
}


//********************************************************************
// returns TRUE if pruned entry
static bool
write_inline(SymbolList *symlist, Symbol * node, const bool is_root)
{
   // if this node is marked, then skip it. 
   if (node->marked)
      return false;

   if( !is_root )
   {
      if( prune_entry(symlist, node) )
         return true;
   }

   // Mark this node as generated
   node->marked = true;

   return write_parms(symlist, node, false, is_root);
}

//********************************************************************
// returns TRUE if pruned entry
static bool
write_list(SymbolList *symlist, Symbol * node, Symbol * index, const bool is_root)
{
   // Write a list if have pushed up parms.
   if(index && has_pu_parms(index))
   {
      if( write_parms(symlist, index, true, false) )
	 return true;
   }

   return write_data(symlist, node, NULL, is_root);
}

//********************************************************************
// returns TRUE if pruned entry
static bool
write_group(SymbolList *symlist, Symbol * node, bool embedded,
	    bool in_line, bool use_list, const bool is_root)
{
   // if this node is marked, then skip it. 
   if (!embedded && node->marked)
      return false;

   int len = node->children.len();

   if (len == 0 && !use_list)
      return false;

   bool use_group = len > 1 || embedded || in_line;

   if (!in_line)
   {
      use_group = true;
   }

   if (use_group)
   {
      // Write any pushed up parms.
      if (has_pu_parms(node))
      {
         if( write_parms(symlist, node, true, is_root) )
	    return true;
      }
   }


   Symbol *p;
   void *cur;
   if ((cur = node->children.first(&p)) != NULL)
   {
      do
      {
	 if( prune_list(symlist,p, desired_class, false) )
	    return true;
      }
      while ((cur = node->children.next(&p, cur)) != NULL);
   }

   // Mark this node as generated
   if (!embedded)
   {
      node->marked = true;

      if( !is_root )
      {
         if( prune_entry(symlist, node) )
	    return true;
      }
   }

   return false;
}


//********************************************************************
// returns true if pruned entry
static bool
write_def(SymbolList *symlist, Symbol * node, const bool is_root)
{
   // if this node is marked or not a user record, then skip it. 
   if (node->marked || node == NULL || node->record_class != desired_class)
   {
      return false;
   }

// dcm:: Hack to avoid extra defsensor, defactuator commands for
//       autoinstanced SorA's
   if( node->symbol_type == SENSOR_NAME || node->symbol_type == ACTUATOR_NAME)
      return false;

   // Mark this node as generated
   node->marked = true;

   if( !is_root )
      return prune_entry(symlist, node);

   return false;
}

//********************************************************************
// returns true if pruned entry
static bool
write_typedef(SymbolList *symlist, Symbol * node, const bool is_root)
{
   // if this node is marked, then skip it. 
   if (node->marked)
      return false;

   // Mark this node as generated
   node->marked = true;

   if( !is_root )
      return prune_entry(symlist, node);

   return false;
}

//********************************************************************
// returns TRUE if it wrote something
bool
prune_list(SymbolList *symlist, Symbol * agent, RECORD_CLASS _desired_class,
	bool is_root)
{
   desired_class = _desired_class;

   // if this node isn't at the user level, then skip it.
   if (agent == NULL)
      return false;

   if( agent->record_class != desired_class )
   {
      return false;
   }

   bool pruned = false;

   // Is this a class definition?
   if (agent->symbol_type == AGENT_CLASS ||
       agent->symbol_type == COORD_CLASS ||
       agent->symbol_type == SENSOR_NAME ||
       agent->symbol_type == BP_CLASS ||
       agent->symbol_type == ROBOT_CLASS ||
       agent->symbol_type == ACTUATOR_NAME)
   {
      pruned = write_def(symlist,agent,is_root);
   }
   // Is this a type definition?
   else if (agent->symbol_type == TYPE_NAME)
   {
      pruned = write_typedef(symlist,agent,is_root);
   }
   else if (agent->symbol_type == GROUP_NAME ||
	  agent->symbol_type == CONFIGURATION_NAME)
   {
      pruned = write_group(symlist, agent, false, true, true,is_root);
   }
   else
   {
      pruned = write_inline(symlist,agent,is_root);
   }

   return pruned;
}



///////////////////////////////////////////////////////////////////////
// $Log: prune_list.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:25  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:52  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:38:01  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 23:00:22  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.2  1996/10/01  13:00:51  doug
// got library writes working
//
// Revision 1.2  1996/10/01 13:00:51  doug
// got library writes working
//
// Revision 1.1  1996/09/25 23:04:09  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
