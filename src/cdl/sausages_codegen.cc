/**********************************************************************
 **                                                                  **
 **                   sausages_codegen.cc                            **
 **                                                                  **
 **  Code generator targeted for SAUSAGES UGV architecure            **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: sausages_codegen.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */


// The "v" input to the output actuator must have this datatype.
const char *MOVEMENT_TYPE_NAME = "Vector";

#include <iostream>
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
static int link_num;
static char *last_position;

const double SAUSAGES_SCALE_FACTOR = 150.0;

//***********************************************************************

// Make a version of the name that has the underscores replaced with dashes 
// The caller is responsible for freeing the new value

static char *
safe_name(const char *name)
{
   char *sname;

   if (name[0] == '$')
      sname = strdup(name + 1);
   else
      sname = strdup(name);

   char *p = sname;

   while (*p)
   {
      if (*p == '_')
	 *p = '-';
      p++;
   }

   return sname;
}

//***********************************************************************

// Make a version of the data string that has the {} brackets removed
// The caller is responsible for freeing the new value

static char *
clean_data(const char *value)
{
   char *cvalue;

   // Trim the leading bracket while making a copy of the string
   if (value[0] == '{')
      cvalue = strdup(value + 1);
   else
      cvalue = strdup(value);

   // Trim the trailing bracket
   int len = strlen(cvalue) - 1;

   if (len > 0)
   {
      if (cvalue[len] == '}')
	 cvalue[len] = '\0';
   }

   // Get rid of any commas
   char *p = cvalue;
   while (*p)
   {
      if (*p == ',')
	 *p = ' ';
      p++;
   }

   // Make sure all numbers are floats (append ".0" if not already there)
   int pos = 0;
   bool in_number = false;
   bool saw_dot = false;
   while ( cvalue[pos] )
   {
      if ( isdigit( cvalue[pos] ) )
	 in_number = true;
      else if( cvalue[pos] == '.' )
	 saw_dot = true;
      else if( in_number )
      {
	 // insert a ".0" right after the number we just finished 
	 if( !saw_dot )
	 {
	    char t = cvalue[pos];
	    cvalue[pos] = '\0';
	    char *new_str = strdup(cvalue);
	    cvalue[pos] = t;
	    new_str = strdupcat(new_str,".0", &cvalue[pos]);
	    free(cvalue);
	    cvalue = new_str;

	    // Skip past the new ".0" chars
	    pos ++;
	 }

	 in_number = false;
	 saw_dot = false;
      }

      pos++;
   }

   // string ended, was last number correct?
   if( in_number && !saw_dot )
   {
      // append a ".0"
      cvalue = strdupcat(cvalue,".0");
   }

   // Return the result.
   return cvalue;
}

//***********************************************************************

// Make a version of the data string that has the {} brackets removed
// The caller is responsible for freeing the new value

static char *
scale_data(const char *value, double scale)
{
   double x,y;
   sscanf(value,"%lf %lf)",&x,&y);

   char buf[2048];
   sprintf(buf,"%f %f",x * scale, y * scale);

   // Return the result.
   return strdup(buf);
}

//***********************************************************************

static void
write_internal_agent(ofstream & out, Symbol * node, const int level)
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
	 indent(out, level + 1);
	 char *name = (char *)(p->name == NULL ? "ERROR_NoName" : p->name);

	 // Make a version of the name that has the underscores replaced 
	 // with dashes for LISP
	 char *sname = safe_name(name);

	 out << '(' << sname << ' ';
	 write_rhs(out, p, level + 1);
	 out << ")\n";
      }
      while ((cur = node->parameter_list.next(&p, cur)) != NULL);
   }
   indent(out, level);
   out << ')';
}

//************************************************************************

static void
write_data_value(ofstream & out, char *value, Symbol * data_type)
{
   // switch on the data type
   if (data_type == UTM_type ||
       (data_type && strcmp(data_type->name, "Map_Location") == 0))
   {
      last_position = clean_data(value);

      // To get the SAUSAGE scale factor to match ours, scale the data here.
      last_position = scale_data(last_position, SAUSAGES_SCALE_FACTOR);

      out << "'(" << last_position << ")";
   }
   else if (data_type == boolean_type)
   {
      if (strcasecmp(value, "false") == 0)
	 out << "nil";
      else
	 out << 't';
   }
   else
   {
      char *cvalue = clean_data(value);

      out << cvalue;
      free(cvalue);
   }
}

//************************************************************************

// Are passing the data type in explicitly since the data type field
// isn't set for pushed-up parms.  This will need to be fixed in the parser
// at some point and then this code can be cleaned up.

static void
write_data(ofstream & out, Symbol * p, const int level,
	   Symbol * data_type)
{
   if (p->symbol_type == INITIALIZER)
   {
      if (p->name)
      {
	 write_data_value(out, p->name, data_type);
      }
      else
      {
	 out << "*MISSING-SOURCE*";
	 error("Missing data value");
      }
   }
   else if (p->symbol_type == PU_INITIALIZER)
   {
      // Special case: if there is a name caste, then use that name instead
      Symbol *rec;

      if (p->name && p->data_type)
      {
	 // Should really generate some type coersion code here      


	 rec = find_pu_data(p->name, 0);
      }
      else
      {
	 // Normal case
	 rec = find_pu_data(p->name, 0);
      }

      if (rec == NULL)
      {
	 error("Didn't find def for pushed-up parm:", p->name);
	 return;
      }

      Symbol *def = rec->input_generator;

      if (def && def->name)
      {
	 write_data_value(out, def->name, p->data_type);
      }
      else
      {
	 out << "*MISSING_SOURCE*";
	 error("Missing value for input parm:",
	     p->input_generator->name ? p->input_generator->name : p->name);
      }
   }
   else if (p->symbol_type == AGENT_NAME)
   {
      write_internal_agent(out, p, level);
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

      // Write the starting location for the move
      if (last_position)
      {
	 out << '\n';
	 indent(out, level + 1);
	 out << "'(" << last_position << ")";
      }

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

	 if (def && def->data_type == (Symbol *) PU_INITIALIZER)
	 {
	    Symbol *rec = find_pu_data(p->name, 0);

	    if (rec == NULL)
	    {
	       error("Didn't find def for pushed-up list parm:", p->name);
	       continue;
	    }
	    def = rec->input_generator;
	 }
	 write_data(out, def, level + 1, p->data_type);
      }
      while ((cur = p->parameter_list.next(&lp, cur)) != NULL);

      // Close the vector
      out << ')';
   }
   else
   {
      Symbol *def = p->input_generator;

      if (def == NULL)
      {
	 out << "NO_SOURCE";
	 error("Missing input connection for parm:", p->name);
	 return;
      }

      if (def->data_type == (Symbol *) PU_INITIALIZER)
      {
	 Symbol *rec = find_pu_data(p->name, 0);

	 if (rec == NULL)
	 {
	    error("Did not find def for pushed-up parm:", p->name);
	    return;
	 }
	 def = rec->input_generator;

	 if (def == NULL)
	 {
	    out << "NO_SOURCE";
	    error("Missing input connection for parm:", rec->name);
	    return;
	 }
      }

      // Write the starting location for the pause
      if( (p->data_type == UTM_type ||
          (p->data_type && strcmp(p->data_type->name, "Map_Location") == 0)) &&
	  last_position)
      {
	 out << '\n';
	 indent(out, level + 1);
	 out << "'(" << last_position << ")";
      }
      else
      {
         write_data(out, def, level + 1, p->data_type);
      }
   }
}


//***********************************************************************
// returns true if success

static int
emit_link(ofstream & out, Symbol * node, int level, const char *robot_name)
{
   bool good = true;

   if (node == NULL)
   {
      error("NULL node");
      good = false;
   }

   if (good)
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
		 Symbol *p;
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

		 if (member_list == NULL)
		 {
		    error("The FSA has no states!");
		    return false;
		 }

		 // Start the sequence
		 indent(out, level);
		 out << "(sequence\n";


		 // Need to generate the FSA links sequentially, since
		 // the transitions are just implied by the order in SAUSAGES.

		 // So, find the start state so we have a beginning.
		 Symbol *member = NULL;

		 if ((cur = member_list->parameter_list.first(&p)) != NULL)
		 {
		    do
		    {
		       if (strcasecmp(p->list_index->name, "start") == 0)
		       {
			  member = p;
			  break;
		       }
		    }
		    while ((cur = member_list->parameter_list.next(&p, cur)) != NULL);
		 }

		 // Make sure defined a start state and then set it as the initial one.
		 if (member == NULL)
		 {
		    error("One state in the FSA must be named \"start\"");
		    return false;
		 }

		 bool done = false;

		 while (!done)
		 {
		    // Write this link

		    // Stack this state
		    AgentStack.insert(member->list_index);

		    // Generate the link
		    if (member->list_index->name)
		    {
		       out << '\n';
		       indent(out, level + 1);
		       out << ";;" << member->list_index->name << '\n';
		    }

		    Symbol *grp = member->input_generator;
		    Symbol *agent;
		    void *cur_agent;
		    bool found_one = false;

                    // Stack this agent
                    AgentStack.insert(grp);

		    if (grp->children.len() != 1 ||
			(cur_agent = grp->children.first(&agent)) != NULL)
		    {
		       if (agent == NULL)
		       {
			  error("Internal error getting agent defining the state\n");
			  good = false;
			  break;
		       }
		       found_one = true;

		       // Start a new link
		       indent(out, level + 1);
		       out << "(link ";

		       // Go generate it
		       emit_link(out, agent, level + 2, robot_name);

		       // The link is finished
		       indent(out, level + 1);
		       out << ")\n";
		    }
		    if (good && !found_one)
		    {
		       error("Didn't find a link for robot:", robot_name);
		    }

		    // Remove the group 
		    AgentStack.get();

		    // Remove the state 
		    AgentStack.get();


		    // We're done unless we can find a next node
		    done = true;

		    // Try to figure out which is the next node   
		    if (rule_list)
		    {

		       // Search the transition rules to find one leaving this 
		       // and then the name of the new state
		       char *next_state = NULL;
		       Symbol *rule;

		       if ((cur = rule_list->parameter_list.first(&rule)))
		       {
			  do
			  {
			     if (strcmp(rule->list_index->name,
					member->list_index->name) == 0)
			     {
				next_state = rule->input_generator->list_index->name;
				break;
			     }


			  }
			  while ((cur = rule_list->parameter_list.next(&rule, cur)) != NULL);
		       }

		       // If found a new state, then look up the symbol for it.
		       if (next_state)
		       {
			  member = NULL;
			  if ((cur = member_list->parameter_list.first(&p)) != NULL)
			  {
			     do
			     {
				if (strcasecmp(p->list_index->name, next_state) == 0)
				{
				   member = p;
				   done = false;
				   break;
				}
			     }
			     while ((cur = member_list->parameter_list.next(&p, cur)) != NULL);
			  }

		       }

		    }
		 }

		 // End the sequence
		 indent(out, level);
		 out << ")\n";
		 break;
	      }

	   default:
	      error("Unknown operator style in COORD_NAME:emit_link",
		    node->defining_rec->operator_style);
	      good = false;
	   }
	  break;

       case GROUP_NAME:
	  // Skip past any groups containing a single agent 
	  Symbol * child;
	  if (node->children.len() != 1 ||
	      node->children.first(&child) == NULL)
	  {
	     error("Groups defining states must contain a single agent");
	     return false;
	  }
	  emit_link(out, child, level + 2, robot_name);
	  break;

       case AGENT_NAME:
	  {
	     if (node->defining_rec == NULL)
	     {
		error("Unable to determine type of link");
		return false;
	     }

	     // Make a version of the name that has the underscores replaced 
	     // with dashes for LISP
	     char *sname = safe_name(node->defining_rec->name);

	     out << sname << '\n';
	     free(sname);

	     indent(out, level);
	     out << "(plan-id " << link_num++ << ")\n";

	     Symbol *p;
	     void *cur;

	     if ((cur = node->parameter_list.first(&p)) == NULL)
	     {
//           error("Empty state list in AGENT_NAME node");
		//           good = false;
		break;
	     }
	     do
	     {
		indent(out, level);
		char *name = (char *)(p->name == NULL ? "ERROR_NoName" : p->name);

		// Make a version of the name that has the underscores replaced 
		// with dashes for LISP
		char *sname = safe_name(name);

		out << '(' << sname << ' ';
		free(sname);

		write_rhs(out, p, level);
		out << ")\n";
	     }
	     while ((cur = node->parameter_list.next(&p, cur)) != NULL);
	  }
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
SAUSAGES_codegen(Symbol * top, char *filename)
{
   // Start with an empty stack
   AgentStack.clear();
   link_num = 0;
   last_position = strdup("1425.0 675.0");

   ofstream out(filename);

   if (!out)
   {
      error("Unable to open output file", filename);
      return false;
   }

   Symbol *node = top;

   // Skip past any groups containing a single agent 
   while (node->symbol_type == GROUP_NAME)
   {
      if (node->children.len() != 1 ||
	  node->children.first(&node) == NULL)
      {
	 error("The top level must be a single agent");
	 return false;
      }
   }

   // We should now be at a robot binding point
   if (node->symbol_type != BP_NAME ||
       node->construction != CS_RBP)
   {
      error("Expected a single robot as the defining agent");
      return false;
   }

   // Get the robot hardware record
   node = node->bound_to;
   if (node->symbol_type != ROBOT_NAME)
   {
      error("The robot must be bound for the output operation");
      return false;
   }

   char *robot_name = node->name;

   // There should be an output binding point as the only agent in the robot
   if (node->children.len() != 1 ||
       node->children.first(&node) == NULL)
   {
      error("The robot record should contain a single output binding point");
      return false;
   }
   if (node->symbol_type != BP_NAME ||
       node->construction != CS_OBP)
   {
      error("The robot record should have an output binding point as the generating agent");
      return false;
   }

   // Get the actuator hardware record
   node = node->bound_to;
   if (node->symbol_type != AGENT_NAME ||
       node->construction != CS_OBP)
   {
      error("The actuator must be bound for the output operation");
      return false;
   }


   // Find the displacement input to the actuator
   Symbol *p;
   void *cur;
   Symbol *v_source = NULL;

   if ((cur = node->parameter_list.first(&p)) != NULL)
   {
      do
      {
	 if (p->symbol_type == INPUT_NAME)
	 {
	    if (strcmp(p->data_type->name, MOVEMENT_TYPE_NAME) == 0)
	    {
	       v_source = p;
	       break;
	    }
	 }
      }
      while ((cur = node->parameter_list.next(&p, cur)) != NULL);
   }

   if (v_source == NULL)
   {
      error("Didn't find an input for the actuator with the required type",
	    MOVEMENT_TYPE_NAME);
      return false;
   }

   node = v_source->input_generator;
   if (node == NULL)
   {
      error("The movement input for the actuator is unconnected");
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

   if (node->symbol_type != COORD_NAME)
   {
      error("SAUSAGES requires that the top level agent is an FSA");
      return false;
   }


   // Start the sequence
   out << ";; =====================================\n";
   out << ";; SAUSAGES plan robot for " << robot_name << "\n";
   out << ";; =====================================\n";

   // Set the starting location 
   out << "(setq start-loc '(" << last_position << "))\n\n";


   // Start the plan
   out << "(defplan the-plan ()\n";

   bool good = emit_link(out, node, 1, robot_name);

   // End the plan
   out << ")\n\n";

   return good;
}


///////////////////////////////////////////////////////////////////////
// $Log: sausages_codegen.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:14  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 22:59:29  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.6  2003/04/06 13:11:30  endo
// gcc 3.1.1
//
// Revision 1.5  2000/10/16 19:36:57  endo
// Modified due to the compiler upgrade.
//
// Revision 1.4  1996/06/04 22:43:51  doug
// scale data for sausages
//
// Revision 1.3  1996/06/01  21:53:31  doug
// *** empty log message ***
//
// Revision 1.2  1996/05/27  21:56:57  doug
// SAUSAGES is working
//
// Revision 1.1  1996/05/14  23:17:07  doug
// Initial revision
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
