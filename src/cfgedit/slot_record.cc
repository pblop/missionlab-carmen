/**********************************************************************
 **                                                                  **
 **                       slot_record.cc                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: slot_record.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <Xm/Xm.h>

#include "list.hpp"
#include "load_cdl.h"
#include "popups.h"
#include "design.h"
#include "help.h"
#include "write_cdl.h"
#include "configuration.hpp"     
#include "globals.h"
#include "screen_object.hpp" 
#include "page.hpp"
#include "slot_record.hpp"
#include "glyph.hpp" 
#include "fsa.hpp"
#include "transition.hpp" 
#include "circle.hpp"
#include "binding.h"
#include "utilities.h" 
#include "toolbar.h"
#include "callbacks.h"

//-----------------------------------------------------------------------

// Called to load a particular parm into the slot_info record.
// If this is an input or constant it will only load a single cell.
// However, a list parm will add info to 2 or more rows.
// It must be guarenteed that column 0 is called first.
// The other columns look at the index values in that column to pick which
// row to load into when they are inside of a list.

int
load_entry(int slot,
	   int col,
	   slot_record * slot_info,
	   int num_slots,
	   Symbol * p,
	   bool detail)
{
   int offset = 0;

   if (detail)
   {
      if (p->symbol_type == PARM_HEADER)
      {
	 Symbol *lp;
	 void *cur_lp;

	 // Add list header
	 slot_info[num_slots * col + slot].valid = true;
	 slot_info[num_slots * col + slot].name = p->name;
	 slot_info[num_slots * col + slot].list_header = true;
	 slot_info[num_slots * col + slot].src_symbol = p;
	 slot_info[num_slots * col + slot].parm_rec = p;
	 slot_info[num_slots * col + slot].data_type = p->data_type;
	 slot_info[num_slots * col + slot].rows_in_list = 0;
	 slot++;
	 offset++;

	 if ((cur_lp = p->parameter_list.first(&lp)) != NULL)
	 {
	    do
	    {
	       int pos = num_slots * col + slot;

	       // column 0 sets the index for the row.
	       // If we aren't column 0, then we need to lookup each
	       // index value in col 0 to find the correct slot
	       if( col != 0 )
	       {
		  bool found_one = false;
		  int cnt = slot_info[num_slots * 0 + slot-1].rows_in_list;
		  for(int i=slot; i<slot+cnt; i++)
		  {
	             if( !strcmp(*slot_info[i].index, lp->list_index->name) )
		     {
			pos = i;
			found_one = true;
			break;
		     }
		  }
		  if( !found_one )
		  {
		     warn_userf("ERROR: List header does not contain the index '%s'",lp->list_index->name);
		  }
	       }
	       slot_info[num_slots * col + pos].valid = true;
	       slot_info[num_slots * col + pos].parm_rec = lp;
	       slot_info[num_slots * col + pos].data_type = lp->data_type;
	       slot_info[num_slots * col + pos].in_list = true;
	       slot_info[num_slots * col + pos].name = lp->list_index->name;
	       slot_info[num_slots * col + pos].index = &lp->list_index->name;

	       if (lp->input_generator != (Symbol *) UP &&
		   lp->input_generator != NULL)
	       {
		  if (lp->input_generator->symbol_type == PU_INITIALIZER)
		  {
	             slot_info[num_slots * col + pos].is_pu = true;
		  }
		  if (lp->input_generator->symbol_type == INITIALIZER)
		  {
		     // Is a constant, but, is it readonly or changeable?
			  if( p->record_class != RC_USER )
			  {
			     // Readonly.
		        slot_info[num_slots * col + pos].is_readonly = true;
			  }
			  else
			  {
			     // Changable
		        slot_info[num_slots * col + pos].is_const = true;
			  }

		     slot_info[num_slots * col + pos].value = &lp->input_generator->name;
		  }
		  else
		  {
		     slot_info[num_slots * col + pos].src_symbol = lp->input_generator;
		  }
	       }
	       slot_info[num_slots * col + pos].input_generator_ = &lp->input_generator;

	       if( col == 0 )
	       {
	          slot++;
	          offset++;
	       }
	    }
	    while ((cur_lp = p->parameter_list.next(&lp, cur_lp)) != NULL);
	 }
	 if( col != 0 )
	 {
            slot += slot_info[num_slots * 0 + slot-1].rows_in_list;
            offset += slot_info[num_slots * 0 + slot-1].rows_in_list;
	 }

	 // Add a 'NEW' button to end of the list 
	 slot_info[num_slots * 0 + slot].valid = true;
	 slot_info[num_slots * 0 + slot].extra_slot = true;

	 slot++;
	 offset++;
      }
      else if (p->symbol_type != PU_INPUT_NAME)
      {
	 slot_info[num_slots * col + slot].valid = true;
	 slot_info[num_slots * col + slot].name = p->name;
	 slot_info[num_slots * col + slot].parm_rec = p;
	 slot_info[num_slots * col + slot].data_type = p->data_type;

	 if (p->input_generator != NULL)
	 {
	    if (p->input_generator == (Symbol *) UP)
	    {
	       // Nothing
	    }
	    else if (p->input_generator->symbol_type == PU_INITIALIZER)
	    {
	       // We know that this is the consumer record, since
	       // we already checked that PU_PARM_NAME isn't set.
	       slot_info[num_slots * col + slot].is_pu = true;
	    }
	    else if (p->input_generator->symbol_type == INITIALIZER)
	    {
	       if (p->data_type == commands_type)
	       {
		  // Is a commands list
		  slot_info[num_slots * col + slot].is_cmds_list = true;
	       }
	       else
	       {
		     // Is a constant, but, is it readonly or changeable?
			  if( p->record_class != RC_USER )
			  {
			     // Readonly.
		        slot_info[num_slots * col + slot].is_readonly = true;
			  }
			  else
			  {
			     // Changeable
		        slot_info[num_slots * col + slot].is_const = true;
			  }
	       }
	       slot_info[num_slots * col + slot].value = &p->input_generator->name;
	    }
	    else
	    {
	       slot_info[num_slots* col + slot].src_symbol = p->input_generator;
	    }
	 }

	 slot_info[num_slots * col + slot].input_generator_ = &p->input_generator;
	 slot++;
	 offset++;
      }
   }

   if (p->symbol_type == PU_INPUT_NAME)
   {
      slot_info[num_slots * col + slot].valid = true;
      slot_info[num_slots * col + slot].name = p->name;
      slot_info[num_slots * col + slot].parm_rec = p;
      slot_info[num_slots * col + slot].data_type = p->data_type;

      if (p->input_generator != NULL &&
	  p->input_generator != (Symbol *) UP)
      {
	 if (p->input_generator->symbol_type == PU_INITIALIZER)
	 {
	    slot_info[num_slots * col + slot].is_pu = true;
	 }
	 else if (p->input_generator->symbol_type == INITIALIZER)
	 {
		 // Is a constant, but, is it readonly or changeable?
		 if( p->record_class != RC_USER )
		 {
		    // Readonly.
		    slot_info[num_slots * col + slot].is_readonly = true;
		 }
		 else
       {
		    // Changable.
	       slot_info[num_slots * col + slot].is_const = true;
		 }

	    slot_info[num_slots * col + slot].value = &p->input_generator->name;
	 }
	 else
	 {
	    slot_info[num_slots * col + slot].src_symbol = p->input_generator;
	 }
      }
      slot_info[num_slots * col + slot].input_generator_ = &p->input_generator;
      slot++;
      offset++;
   }

   // Set the rows_in_list counter for list headers in column 0
   // This value does not count the list header slot or the extra slot
   if( col == 0 )
      slot_info[num_slots * 0 + slot-offset].rows_in_list = offset - 2;

   return offset;
}

/*-----------------------------------------------------------------------*/

slot_record *
build_slot_info(Symbol * sym, bool detail, int &level, int &num_slots, int &num_cols)
{
   // Figure out how many slots (rows) and columns we will have.
   // There are four cases:
   //   1. a constant
   //   2. an input port
   //   3. a list of 1 or 2
   //   4. a grouping list of an input port and one or more constants
   // List entries have a header and then indented names.
   //
   // Ex:
   //   >>  input port 1
   //   [some big constant]
   //   list_of_xxx

   // compute the number of slots for this node 
   num_slots = 0;
   Symbol *p;
   void *cur;

   // Clear the level counters.  We will use them to determine the
   // required number of columns.
   bool is_single = false;

   if ((cur = sym->parameter_list.first(&p)) != NULL)
   {
      do
      {
	 p->level = 0;

	 if (p->symbol_type != PARM_HEADER)
	    is_single = true;
      }
      while ((cur = sym->parameter_list.next(&p, cur)) != NULL);
   }

   if ((cur = sym->parameter_list.first(&p)) != NULL)
   {
      do
      {
	 if (detail)
	 {
	    if (p->parm_list_head != NULL)
	    {
	       // Find the correct reference in this parm set
	       Symbol *p1;
	       void *cur1;
	       Symbol *ptr = NULL;

	       if ((cur1 = sym->parameter_list.first(&p1)) != NULL)
	       {
		  do
		  {
		     if ( !strcmp(p->parm_list_head->name, p1->name) )
		     {
			ptr = p;
			break;
		     }
		  } while ((cur1 = sym->parameter_list.next(&p1, cur1)) != NULL);
	       }

               if( ptr )
	       {
	          // Is a member of a list group.
	          ptr->level++;
	          p->level = ptr->level;
	       }
	       else
	       {
		  warn_userf("Error: Didn't find the parm list header %s",
			p->name);
	       }
	    }
	    else if (p->symbol_type == PARM_HEADER)
	    {
	       // Is a list, so add one for the header
	       num_slots++;

	       Symbol *lp;
	       void *cur_lp;

	       if ((cur_lp = p->parameter_list.first(&lp)) != NULL)
	       {
		  do
		  {
		     // Add one for each child
		     num_slots++;
		  }
		  while ((cur_lp = p->parameter_list.next(&lp, cur_lp)) != NULL);
	       }

	       // Leave an extra slot for the NEW button
	       num_slots++;
	    }
	    else if (p->symbol_type != PU_INPUT_NAME)
	    {
	       // Add one for each child
	       num_slots++;
	    }
	 }

	 if (p->symbol_type == PU_INPUT_NAME)
	 {
	    // Add one for each child
	    num_slots++;
	 }
      }
      while ((cur = sym->parameter_list.next(&p, cur)) != NULL);
   }

   // compute the number of columns for this node 
   num_cols = 0;
   slot_record *slot_info = NULL;

   if (num_slots > 0)
   {
      if ((cur = sym->parameter_list.first(&p)) != NULL)
      {
	 do
	 {
	    if (p->level > num_cols)
	       num_cols = p->level;
	 }
	 while ((cur = sym->parameter_list.next(&p, cur)) != NULL);
      }
      // Make 1 based.
      num_cols++;

      // If there exists a non-list entry, then need two columns, so have a
      // place to put the parm name
      if (is_single)
	 num_cols = max(num_cols, 2);

      // Allocate a data record to hold the info.
      slot_info = new slot_record[num_slots * num_cols];

      // Now, spin the inputs again, filling in each record's info.
      int slot = 0;

      if ((cur = sym->parameter_list.first(&p)) != NULL)
      {
	 do
	 {
	    if (p->parm_list_head == NULL)
	    {
	       // Fill in column 0
	       int cnt = load_entry(slot, 0, slot_info, num_slots, p, detail);

	       // Check for any other entries in this row
	       Symbol *p1;
	       void *cur1;

	       if ((cur1 = sym->parameter_list.first(&p1)) != NULL)
	       {
		  do
		  {
		     if ( p1 != p &&
			  p1->parm_list_head &&
			  !strcmp(p1->parm_list_head->name, p->name) )
		     {
			load_entry(slot, p1->level, slot_info, num_slots, p1, detail);
		     }
		  }
		  while ((cur1 = sym->parameter_list.next(&p1, cur1)) != NULL);
	       }

	       //bump to the next slot
	       slot += cnt;
	    }
	 }
	 while ((cur = sym->parameter_list.next(&p, cur)) != NULL);
      }
   }

   return slot_info;
}



///////////////////////////////////////////////////////////////////////
// $Log: slot_record.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 22:59:34  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.14  2003/04/06 12:25:34  endo
// gcc 3.1.1
//
// Revision 1.13  2000/02/23 18:25:52  endo
// Checked in for doug.
// Code changed, so that the library agents
// are readonly in the editor. This will
// keep users from making changes to parameters
// (which will be lost) and also make the
// istinction between user space agents and
// library agents explicit.
// Readonly glyphs have a new text line added
// hich states that the object is read only
// and lists the library it comes from and
// the description and any constant fields
// are not editable.
//
// Revision 1.12  1997/02/14 16:53:35  zchen
// *** empty log message ***
//
// Revision 1.11  1996/10/04  20:58:17  doug
// changes to get to version 1.0c
//
// Revision 1.11  1996/10/01 13:00:11  doug
// went to version 1.0c
//
// Revision 1.10  1996/06/02 20:27:40  doug
// Fixed pushed up parms as weights in a list
//
// Revision 1.9  1996/05/07  19:17:03  doug
// fixing compile warnings
//
// Revision 1.8  1996/02/25  01:13:14  doug
// *** empty log message ***
//
// Revision 1.7  1996/01/31  03:06:53  doug
// *** empty log message ***
//
// Revision 1.6  1996/01/29  00:07:50  doug
// *** empty log message ***
//
// Revision 1.5  1996/01/27  00:09:24  doug
// added grouped lists of parameters
//
// Revision 1.4  1996/01/25  14:53:34  doug
// *** empty log message ***
//
// Revision 1.3  1995/11/21  23:10:50  doug
// *** empty log message ***
//
// Revision 1.2  1995/11/12  22:39:46  doug
// *** empty log message ***
//
// Revision 1.1  1995/11/12  19:58:59  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
