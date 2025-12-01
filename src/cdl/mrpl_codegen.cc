/**********************************************************************
 **                                                                  **
 **                       main.cc                                    **
 **                                                                  **
 **  Code generator targeted for MRPL UGV architecure                **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: mrpl_codegen.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */


#include <fstream.h>
#include <libc.h>

#include "prototypes.h"
#include "cdl_defs.h"
#include "list.hpp"
#include "symtab.h"
#include "cdl_gram.tab.h"
#include "version.h"
#include "utilities.h"

#include "codegens.h"

static void write_rhs(ofstream & out, Symbol * p, int level);

//***********************************************************************

static void
write_internal_agent(ofstream & out, Symbol *node, const int level)
{
   out << '\n';
   indent(out, level);
   out << "'(\n";

   Symbol *p;
   void *cur;
   if ((cur = node->parameter_list.first(&p)) != NULL)
   {
       do
       {
	  indent(out, level+1);
	  char *name = p->name == NULL ? "ERROR_NoName" : p->name;

	  out << '(' << name << ' ';
	  write_rhs(out, p, level+1);
	  out << ")\n";
       } while ((cur = node->parameter_list.next(&p, cur)) != NULL);
   }
   indent(out, level);
   out << ')';
}

//************************************************************************

// Are passing the data type in explicitly since the data type field
// isn't set for pushed-up parms.  This will need to be fixed in the parser
// at some point and then this code can be cleaned up.

static void
write_data(ofstream & out, Symbol * p, const int level, 
           Symbol *data_type)
{
   if( p->symbol_type == AGENT_NAME )
   {
      write_internal_agent(out, p, level);
   }
   else if (p->symbol_type == INITIALIZER)
   {
      // Now switch on the data type
      if (data_type == UTM_type)
      {
         out << "'(" << p->name << ')';
      }
      else if (data_type == boolean_type)
      {
         if (strcasecmp(p->name, "false") == 0)
	    out << "nil";
         else
	    out << 't';
      }
      else
      {
         out << p->name;
      }
   }
   else
   {
      error("write_data with unknown symbol_type:", p->symbol_type);
      return;
   }
}

//************************************************************************

static void
write_rhs(ofstream & out, Symbol * p, int level)
{

   if (p->symbol_type == PARM_HEADER)
   {
      out << "(vector";

      Symbol *lp;
      void *cur;

      if ((cur = p->parameter_list.first(&lp)) == NULL)
      {
	 error("Empty parameter list in AGENT_NAME node");
	 return;
      }

      do
      {
	 out << '\n';
	 indent(out, level + 1);

	 Symbol *def = lp->input_generator;
	 if( def && def->data_type == (Symbol *) PU_INITIALIZER )
	 {
	    Symbol *rec = find_pu_data(p->name, 0);
	    if( rec == NULL )
	    {
	       error("Didn't find def for pushed-up list parm:",p->name);
	       continue;
	    }
	    def = rec->input_generator;
	 }
	 write_data(out, def, level+1, lp->data_type);
      }
      while ((cur = p->parameter_list.next(&lp, cur)) != NULL);

      // Close the vector
      out << ')';
   }
   else
   {
      Symbol *def = p->input_generator;
      if( def && def->data_type == (Symbol *) PU_INITIALIZER )
      {
         Symbol *rec = find_pu_data(p->name, 0);
         if( rec == NULL )
         {
	    error("Didn't find def for pushed-up parm:",p->name);
            return;
         }
	 def = rec->input_generator;
      }
      write_data(out, def, level+1, p->data_type);
   }
}


//***********************************************************************
// returns true if success

static int
emit_link(ofstream & out, Symbol * node, int level, char *robot_name)
{
   bool good = true;
   if (node == NULL)
   {
      error("NULL node");
      good = false;
   }

   if( good )
   {
   // Stack this agent
   AgentStack.insert(node);

   switch (node->symbol_type)
    {
    case COORD_NAME:
       switch (node->defining_rec->operator_style)
	{
	case FSA_STYLE:
           {
	   // Find the rules
	   Symbol * p;
	   void *cur;

	   Symbol *rule_list = NULL;
	   Symbol *member_list = NULL;

	   if ((cur = node->parameter_list.first(&p)) != NULL)
	   {
	      do
	      {
		 if (p->symbol_type == PARM_HEADER)
		 {
		    // Is a list

		    // Remember the list of rules
		    if (p->data_type == expression_type)
		       rule_list = p;
		    else if (p->data_type == member_type)
		       member_list = p;
		 }
	      }
	      while ((cur = node->parameter_list.next(&p, cur)) != NULL);
	   }

	   // Start the sequence
	   indent(out, level);
	   out << "(sequence\n";

	   // Write the sequence
	   if (member_list != NULL)
	   {
	      // Create the links in the fsa
	      Symbol *member;

	      if ((cur = member_list->parameter_list.first(&member)) != NULL)
	      {
		 do
		 {
                    // Stack this agent
                    AgentStack.insert(member->list_index);

		    // Generate the link
		    if (member->list_index->name)
		    {
		       out << '\n';
		       indent(out, level + 1);
		       out << ";;" << member->list_index->name << '\n';
		    }

		    indent(out, level + 1);
		    Symbol *grp = member->input_generator;
		    Symbol *bp;
		    void *cur_bp;
		    bool found_one = false;

		    if ((cur_bp = grp->children.first(&bp)) != NULL)
		    {
		       do
		       {
			  Symbol *robot = bp->bound_to;

			  if (strcmp(robot->name, robot_name) == 0)
			  {
			     // Generate the link for this one
			     if (robot->children.len() != 1)
			     {
				error("Expected one agent defining the robot\n");
				good = false;
				break;
			     }
			     Symbol *link;

			     if (robot->children.first(&link) == NULL)
			     {
				error("Internal error getting agent defining the robot\n");
				good = false;
				break;
			     }
			     found_one = true;

                             // Stack this robot
                             AgentStack.insert(robot);

			     out <<"(link " << link->defining_rec->name << '\n';
			     emit_link(out, link, level + 2, robot_name);
			     indent(out, level + 1);
			     out << ")\n";

                             // Remove the robot
                             AgentStack.get();
			  }
		       } while ((cur_bp = grp->children.next(&bp, cur_bp)) != NULL);
		    }
		    if( good && !found_one )
		    {
		       error("Didn't find a link for robot:",robot_name);
		    }

                    // Remove the link 
                    AgentStack.get();
		 } while (good && (cur = member_list->parameter_list.next(&member, cur)));
	      }
	   }

	   // End the sequence
	   indent(out, level);
	   out << ")\n";
	   break;
	   }

	default:
	   error("Unknown operator style in COORD_NAME:emit_link", node->defining_rec->operator_style);
	   good = false;
	}
       break;

    case AGENT_NAME:
       indent(out, level);
       out << "(plan-id, (gen-plan-id))\n";

       Symbol *p;
       void *cur;
       if ((cur = node->parameter_list.first(&p)) == NULL)
       {
	  error("Empty state list in AGENT_NAME node");
	  good = false;
	  break;
       }
       do
       {
	  indent(out, level);
	  char *name = p->name == NULL ? "ERROR_NoName" : p->name;

	  out << '(' << name << ' ';
	  write_rhs(out, p, level);
	  out << ")\n";
       }
       while ((cur = node->parameter_list.next(&p, cur)) != NULL);

       break;

    default:
       error("Unknown symbol type in emit_link", node->symbol_type);
       good = false;
    }

   // Remove this agent
   AgentStack.get();
   }

   return good;
}

//***********************************************************************
// returns true if success
int
emit_plans(ofstream & out, Symbol * fsa)
{
   // This gets tricky.
   // MRPL inverts the fsa and distributes it within the robots.

   // Find the group of robots in the first state.
   // This will serve as the list of robots we iterate over

   // Find the rules
   Symbol * p;
   void *cur;
   Symbol *member_list = NULL;

   if ((cur = fsa->parameter_list.first(&p)) != NULL)
   {
      do
      {
         if (p->symbol_type == PARM_HEADER)
	 {
            if (p->data_type == member_type)
	    {
	       member_list = p;
	       break;
	    }
         }
      } while ((cur = fsa->parameter_list.next(&p, cur)) != NULL);
   }

   if (member_list == NULL)
   {
	 error("Didn't define any states?\n");
	 return false;
   }

   // Create the links in the fsa
   Symbol *state1;

   if (member_list->parameter_list.first(&state1) == 0 )
   {
	 error("Didn't define any states?\n");
	 return false;
   }

   Symbol *grp = state1->input_generator;
   Symbol *bp;
   void *cur_bp;

   bool good = true;
   // Step through each of the robots in turn.
   if ((cur_bp = grp->children.first(&bp)) != NULL)
   {
      do
      {
         char *robot_name = bp->bound_to->name;

         // Start the sequence
         out << ";; =====================================\n";
	 out << ";; MRPL plan robot for " << robot_name << "\n";
         out << ";; =====================================\n";
         out << "(defplan " << robot_name << "plan ()\n";

	 good &= emit_link(out, fsa, 1, robot_name);

         out << ")\n\n";

      } while ((cur_bp = grp->children.next(&bp, cur_bp)) != NULL);
   }

   return good;
}

//***********************************************************************
// returns true if success
int
mrpl_codegen(Symbol * top, char *filename)
{
   // Start with an empty stack
   AgentStack.clear();

   ofstream out(filename);

   if (!out)
   {
      error("Unable to open output file", filename);
      return false;
   }

   if (top->symbol_type != GROUP_NAME ||
       top->children.len() != 1)
   {
      error("Expected a single agent at the top level");
      return false;
   }

   // Write a header
   out << ";;/*************************************************\n";
   out << ";;*\n";
   out << ";;* This file " << filename << " was created with the command\n";
   out << ";;* " << command_line << '\n';
   out << ";;* using the CDL compiler, version " << version_str << '\n';
   out << ";;*\n";
   out << ";;**************************************************/\n";
   out << '\n';

   Symbol *fsa;

   if (top->children.first(&fsa) == NULL)
   {
      error("Expected an FSA as the top level agent");
      return false;
   }

   return emit_plans(out, fsa);
}


///////////////////////////////////////////////////////////////////////
// $Log: mrpl_codegen.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:14  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 22:59:28  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.15  1996/05/07  19:53:55  doug
// fixing compile warnings
//
// Revision 1.14  1996/05/06  03:11:39  doug
// fixing compiler warnings
//
// Revision 1.13  1996/03/08  20:43:36  doug
// *** empty log message ***
//
// Revision 1.12  1996/02/25  01:13:39  doug
// *** empty log message ***
//
// Revision 1.11  1996/02/18  22:44:13  doug
// handle pushup parms in cnl_codegen
//
// Revision 1.10  1995/10/30  23:04:00  doug
// *** empty log message ***
//
// Revision 1.9  1995/06/29  15:02:04  jmc
// Added RCS log string.
///////////////////////////////////////////////////////////////////////
