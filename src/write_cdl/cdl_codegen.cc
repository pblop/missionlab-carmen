/**********************************************************************
 **                                                                  **
 **                          cdl_codegen.cc                          **
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

/* $Id: cdl_codegen.cc,v 1.1.1.1 2008/07/14 16:44:25 endo Exp $ */


#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <libc.h>
#include <errno.h>
#include <unistd.h>
#include <string>
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
#include "string_utils.h"

using std::ifstream;
using std::ios;
using std::endl;

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const string CDL_STRING_SOCIETY = "society";
const string CDL_STRING_RULES = "rules";
const string CDL_STRING_BOUND_TO = "bound_to";
const string CDL_STRING_BIND_ARCH = "bindArch";
const string CDL_STRING_IF = "if";
const string CDL_STRING_GOTO = "goto";
const string CDL_STRING_START = "Start";
const string CDL_STRING_WHEEL_INSTANCE = "$Wheels";
const string CDL_STRING_MAX_VELOCITY = "max_vel";
const string CDL_STRING_BASE_VELOCITY = "base_vel";
const string CDL_2SPACES = "  ";
const string CDL_4SPACES = "    ";
const string CDL_8SPACES = "        ";
const string CDL_10SPACES = "          ";
const string CDL_12SPACES = "            ";
const string CDL_14SPACES = "              ";
const string CDL_16SPACES = "                ";
const string ACDL_STRING_EMPTY = "";
const string ACDL_STRING_MOVEMENT = "movement";
const string ACDL_STRING_BIND_ARCH = "BIND_ARCH";
const string ACDL_STRING_AGENT_NAME = "AGENT_NAME";
const string ACDL_STRING_TASK_INDEX = "TASK_INDEX";
const string ACDL_STRING_TASK_DESC = "TASK_DESC";
const string ACDL_STRING_TRIGGERING_TASK_INDEX = "TRIGGERING_TASK_INDEX";
const string ACDL_STRING_PARM_NAME = "PARM_NAME";
const string ACDL_STRING_PARM_VALUE = "PARM_VALUE";
const string ACDL_STRING_IF = "IF";
const string ACDL_STRING_GOTO = "GOTO";
const string ACDL_STRING_INSTANCE = "INSTANCE";
const string ACDL_STRING_INSTANCE_INDEX = "INSTANCE_INDEX";
const string ACDL_STRING_INSTANCE_NAME = "INSTANCE_NAME";
const string ACDL_STRING_START = "START";
const string ACDL_STRING_END = "END";
const string ACDL_STRING_MAX_VELOCITY = "MAX_VELOCITY";
const string ACDL_STRING_BASE_VELOCITY = "BASE_VELOCITY";
const char ACDL_CHAR_WHITESPACE = ' ';
const char ACDL_CHAR_WHITESPACE_FILLER = '+';
const int ACDL2CDL_AUTO_TASKPLACE_STARTX = 120;
const int ACDL2CDL_AUTO_TASKPLACE_ENDX = 1020;
const int ACDL2CDL_AUTO_TASKPLACE_SPACEX = 300;
const int ACDL2CDL_AUTO_TASKPLACE_STARTY = 120;
const int ACDL2CDL_AUTO_TASKPLACE_SPACEY = 225;
const int ACDL2CDL_AUTO_TASKPLACE_NUM_COLUMN = 4;
const int ACDL2CDL_AUTO_ROBOTPLACE_STARTX = 10;
const int ACDL2CDL_AUTO_ROBOTPLACE_STARTY = 10;
const int ACDL2CDL_AUTO_ROBOTPLACE_SPACEY = 125;

//-----------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------
static bool write_config(
    Symbol * agent,
    ofstream & out,
    int level,
    bool looped,
    Symbol * index,
    const bool debug);

static void mark_instances(Symbol * agent, const bool debug);

static bool write_group(
    Symbol * node,
    ofstream & out,
    int level,
    bool embedded,
    bool in_line,
    bool use_list,
    Symbol * index,
    const bool debug);
static bool write_list(
    Symbol * node,
    ofstream & out,
    int level,
    Symbol * index,
    const bool debug);


bool write_abstracted_config(Symbol *agent, ofstream & out, bool looped, Symbol * index);

static bool write_abstracted_parms(Symbol * node, ofstream & out);

static bool write_abstracted_group(
    Symbol * node,
    ofstream & out,
    bool embedded,
    bool in_line,
    bool use_list,
    Symbol * index);

static bool write_abstracted_inst(Symbol * node, ofstream & out);

static bool write_abstracted_data(ofstream & out, Symbol * p, Symbol * index);

static GTList < Symbol * >*stack;
static RECORD_CLASS desired_class;
static const char *desired_source_file;

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

// Returns true on success
static bool
is_agent(Symbol * p)
{
   return (p != NULL &&
	   p != (Symbol *) UP &&
	   p->symbol_type != PU_INITIALIZER &&
	   p->symbol_type != INITIALIZER);
}

//********************************************************************

// Returns true on success
static bool
write_data(ofstream & out, Symbol * p, int level, Symbol * index,
	const bool debug)
{
   if (p == NULL)
   {
      return true;
   }
   else if (p == (Symbol *) UP)
   {
      out << '^';
   }
   else if (p->symbol_type == INITIALIZER)
   {
      out << '{';
      if( p->name )
         out << p->name;
      out << '}';
   }
   else if (p->symbol_type == PU_INITIALIZER)
   {
      out << "{^";

      // Spit out the name caste if it exists
      if( p->name && p->data_type && p->data_type->name )
	 out << p->data_type->name << ' ' << p->name;

      out << '}';
   }
   else if (p->symbol_type == RULE_NAME)
   {
      out << "if ";
      write_config(p->input_generator, out, level + 1, false, NULL, debug);
      out << " goto " << p->list_index->name;
   }
   else if (!p->is_instance)
   {
      return write_config(p, out, level + 1, true, index, debug);
   }
   else if( has_pu_parms(index) )
   {
      return write_list(p, out, level + 1, index, debug);
   }
   else
   {
      out << p->name;
   }

   return true;
}

//********************************************************************

static void
indent(ofstream & s, const int level)
{
   if (level > 0)
   {
      s.width(level * 2);
      s << " ";
      s.width(0);
   }
}

//********************************************************************
// returns true if writes will require a comma
static bool
write_parms(Symbol * node, ofstream & out, int level, bool pu_only,
	    bool need_comma, const bool debug)
{
   if (debug)
      cerr << "write_parms(" << node->name << ")\n";

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
	    if (need_comma)
	    {
	       out << ",\n";
	       need_comma = false;
	    }

      // DCM 02-05-00: Fix so agents connected to pushed up parms 
      // preserve their location in the CDL file.
      indent(out, level + 1);
      out << p->name;

      if (p->input_generator == NULL || is_agent(p->input_generator))
      {
         out << '<' << p->location.x << ',' << p->location.y << '>';
         if( p->description )
            out << '|' << p->description << '|';
      }
      out << " = ";

	    if (p->input_generator)
	       write_data(out, p->input_generator, level, NULL, debug);
	    need_comma = true;
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
		     if (need_comma)
		     {
			out << ",\n";
			need_comma = false;
		     }

		     indent(out, level + 1);

		     out << p->name << '[' << lp->list_index->name << ']';

		     if (lp->input_generator == NULL ||
				       is_agent(lp->input_generator))
		     {
			out << '<' << lp->list_index->location.x << ','
			   << lp->list_index->location.y << '>';
			if( lp->list_index->description )
			   out << '|' << lp->list_index->description << '|';
		     }

		     out << " = ";

		     if (lp->input_generator)
			write_data(out, lp->input_generator, level, lp->list_index, debug);
		     else if (lp->list_index && lp->list_index->input_generator)
			write_data(out, lp->list_index->input_generator, level, lp->list_index, debug);
		     need_comma = true;
		  }
		  while ((cur_lp = p->parameter_list.next(&lp, cur_lp)) != NULL);
	       }
	    }
	    else
	    {
	       // Special case, the bound_to parm is a fake one.
	       if (node->symbol_type == BP_NAME && strcmp(p->name, "bound_to") == 0)
		  continue;

	       if (need_comma)
	       {
		  out << ",\n";
		  need_comma = false;
	       }

	       indent(out, level + 1);
	       out << p->name;

	       if (p->input_generator == NULL ||
			 is_agent(p->input_generator))
	       {
		  out << '<' << p->location.x << ',' << p->location.y << '>';
		  if( p->description )
		     out << '|' << p->description << '|';
	       }

	       out << " = ";

	       if (p->input_generator)
		  write_data(out, p->input_generator, level, NULL, debug);
	       need_comma = true;
	    }
	 }
      }
      while ((cur = node->parameter_list.next(&p, cur)) != NULL);
   }

   if (is_robot && !pu_only)
   {
      if (need_comma)
      {
	 out << ",\n";
	 need_comma = false;
      }

      write_group(node, out, level + 1, true, true, false, NULL, debug);
   }

   if (node->symbol_type == BP_NAME && node->bound_to)
   {
      if (need_comma)
      {
	 out << ",\n";
	 need_comma = false;
      }
      indent(out, level + 1);
      out << "bound_to = ";

      write_data(out, node->bound_to, level + 1, NULL, debug);
      need_comma = true;
   }

   return need_comma;
}


//********************************************************************
// returns true if success
static bool
write_inst(Symbol * node, ofstream & out, int level, const bool debug)
{
   // if this node is marked, then skip it. 
   if (node->marked)
      return true;

   if (debug)
      cerr << "write_inst(" << node->name << ")\n";

   const char *cmd =
   node->symbol_type == COORD_NAME ? "instOp" :
   node->symbol_type == ROBOT_CLASS ? "instRobot" :
   node->symbol_type == BP_NAME ? "instBP" :
   node->symbol_type == AGENT_NAME ? "instAgent" :
   "instUNKNOWN";

   out << cmd << '<' << node->location.x << ',' << node->location.y << "> ";
   if( node->description )
      out << '|' << node->description << "| ";

   out << node->name << " from " << node->defining_rec->name;

   // if instance of a sensor_name or acutator_name, 
   // then need to qualify it with the class
   if( node->is_instance &&
       node->defining_rec && 
       (node->defining_rec->symbol_type == SENSOR_NAME ||
        node->defining_rec->symbol_type == ACTUATOR_NAME))
   {
      out << ':' << node->defining_rec->defining_rec->name;
   }

   out << "(\n";

   write_parms(node, out, level, false, false, debug);

   out << ");\n\n";

   // Mark this node as generated
   node->marked = true;
   node->is_instance = true;

   return true;
}


//********************************************************************
// returns TRUE if success
static bool
write_inline(Symbol * node, ofstream & out, int level, const bool debug)
{
   // if this node is marked, then skip it. 
   if (node->marked)
      return true;

   if (debug)
      cerr << "write_inline(" << node->name << ")\n";

   if (node->name && strlen(node->name) > 0 && node->name[0] != '$')
      out << node->name << ':';

   char *def_name;

   // Determine the definition name
   if (node->defining_rec == NULL)
   {
      def_name = "NULL_NAME";
   }
   else if (node->defining_rec->defining_rec &&
	    (node->defining_rec->symbol_type == ACTUATOR_NAME ||
	     node->defining_rec->symbol_type == SENSOR_NAME))
   {
      out << node->defining_rec->name << ':';
      def_name = node->defining_rec->defining_rec->name;
   }
   else
   {
      def_name = node->defining_rec->name;
   }

   if (debug)
      cerr << "write_inline(" << node->name << ") as " << def_name << "\n";

   // Write the def name
   out << def_name << "(\n";

   write_parms(node, out, level + 1, false, false, debug);

   out << ")<" << node->location.x << ',' << node->location.y << '>';
   if( node->description )
      out << '|' << node->description << "|";
   out << '\n';

   // Mark this node as generated
   node->marked = true;

   return true;
}

//********************************************************************
// returns TRUE if need_comma
static bool
write_list(Symbol * node, ofstream & out, int level, Symbol * index, 
	const bool debug)
{
   bool need_comma = false;

   // if this node is marked, then skip it. 
//   if (node->marked)
//      return true;

   // Write a list if have pushed up parms.
   if(index && has_pu_parms(index))
   {
      out << "(\n";
      need_comma = write_parms(index, out, level + 1, true, need_comma, debug);
   }

   if (need_comma)
   {
      out << ",\n";
      need_comma = false;
   }

   indent(out, level + 1);
   write_data(out, node, level + 1, NULL, debug);

   // Write a list if have pushed up parms.
   if (index && has_pu_parms(index))
      out << ")\n";

   return need_comma;
}

//********************************************************************
// returns TRUE if success
static bool
write_group(Symbol * node, ofstream & out, int level, bool embedded,
	    bool in_line, bool use_list, Symbol * index, const bool debug)
{
   // if this node is marked, then skip it. 
   if (!embedded && node->marked)
      return true;

   if (debug)
      cerr << "write_group(" << node->name << ")\n";

   bool need_comma = false;

   int len = node->children.len();

   if (len == 0 && !use_list)
      return true;

   bool use_group = len > 1 || embedded || in_line;

   if (!in_line)
   {
      out << "instGroup ";
      if( node->description )
         out << '|' << node->description << "| ";
		out << node->name << " from ";
      use_group = true;
   }

   if (use_group)
   {
#if 0
      // Write a list if have pushed up parms.
      if (use_list && index && has_pu_parms(index))
      {
	 out << "(\n";
	 need_comma = write_parms(index, out, level + 1, true, need_comma, debug);
      }
      if (need_comma)
      {
	 out << ",\n";
	 need_comma = false;
      }
#endif

      if (in_line && node->symbol_type != BP_NAME &&
	  node->name && strlen(node->name) > 0 && node->name[0] != '$')
      {
         out << node->name << ':';
      }

      out << "[\n";

      // Write any pushed up parms.
      if (has_pu_parms(node))
      {
         need_comma = write_parms(node, out, level + 1, true, need_comma, debug);
         out << "\n";
      }
   }


   Symbol *p;
   void *cur;

   if ((cur = node->children.first(&p)) != NULL)
   {
      do
      {
	 if (need_comma)
	 {
	    out << ",\n";
	    need_comma = false;
	 }

	 if (debug)
	    cerr << "Writing child " << p->name << "\n";

	 if (write_config(p, out, level + 1, true, NULL, debug))
	    need_comma = true;
      }
      while ((cur = node->children.next(&p, cur)) != NULL);
   }

   // Write any pushed up parms.
   //   need_comma = write_parms(node, out, level + 1, true, need_comma, debug);

   if (use_group)
      out << "]";
   if (use_group && !embedded && in_line)
   {
      out << '<' << node->location.x << ',' << node->location.y << '>';
      if( node->description )
         out << '|' << node->description << '|';

#if 0
      // Write a list if have pushed up parms.
      if (use_list && index && has_pu_parms(index))
	 out << ")\n";
#endif
   }
   if (!in_line)
      out << ';';

   if (use_group)
      out << "\n";

   // Mark this node as generated
   if (!embedded)
      node->marked = true;

   if( level == 0 )
      out << "\n";

   return true;
}


//********************************************************************
// returns true if success
static bool
write_parm_defs(Symbol * node, ofstream & out, int level, const bool debug)
{
   if (debug)
      cerr << "write_parm_defs(" << node->name << ")\n";

   Symbol *p;
   void *cur;
   bool need_comma = false;

   if ((cur = node->parameter_list.first(&p)) != NULL)
   {
      do
      {
	 // the "bound_to" parm is special.  Ignore it in defs.
	 if( strcmp(p->name,"bound_to") == 0 )
	    continue;

	 if (need_comma)
	 {
	    out << ",\n";
	    need_comma = false;
	 }

	 indent(out, level + 1);
	 if (p->is_list)
	    out << "list ";
	 out << p->data_type->name << ' ' << p->name;
	 need_comma = true;
      }
      while ((cur = node->parameter_list.next(&p, cur)) != NULL);
   }
   return true;
}


//********************************************************************
// returns true if success
static bool
write_def(Symbol * node, ofstream & out, int level, const bool debug)
{
   if (debug)
      cerr << "write_def(" << node->name << ")\n";

   // if this node is marked or not a user record, then skip it. 
   if (node->marked || node == NULL || node->record_class != desired_class)
   {
      return true;
   }

   if( node->record_class == RC_LIBRARY &&
       strcmp(node->source_file, desired_source_file) != 0 )
   {
      return true;
   }


// dcm:: Hack to avoid extra defsensor, defactuator commands for
//       autoinstanced SorA's
   if( node->symbol_type == SENSOR_NAME || node->symbol_type == ACTUATOR_NAME)
      return true;


   const char *cmd = node->symbol_type == AGENT_CLASS ? "defAgent" :
   node->symbol_type == COORD_CLASS ? "defOp" :
   node->symbol_type == SENSOR_NAME ? "defSensor" :
   node->symbol_type == ACTUATOR_NAME ? "defActuator" :
   node->symbol_type == ROBOT_CLASS ? "defRobot" :
   node->symbol_type == BP_CLASS ? "defBP" :
   "UNKNOWN_SYMBOL_TYPE";

   out << cmd;
   if (node->arch != NULL && node->arch != free_arch)
      out << "[" << node->arch->name << "]";

   if (node->symbol_type == SENSOR_NAME ||
       node->symbol_type == ACTUATOR_NAME)
   {
      if( node->binds_to && node->binds_to->name )
         out << " binds " << node->binds_to->name;
      else
         out << " binds " << "UNKNOWN";
   }

   if (node->symbol_type != ROBOT_CLASS)
   {
      if( node->data_type && node->data_type->name )
         out << ' ' << node->data_type->name;
      else
         out << ' ' << "UNKNOWN_TYPE";
   }

   out << ' ' << node->name;

   if (node->symbol_type == COORD_CLASS)
   {
      const char *name = node->operator_style == FSA_STYLE ? "FSAstyle" :
      node->operator_style == SELECT_STYLE ? "SELECTstyle" :
      "UNKNOWN_STYLE";

      out << ' ' << name;
   }

   out << "(\n";

   write_parm_defs(node, out, level + 1, debug);

   out << ");\n\n";

   // Mark this node as generated
   node->marked = true;

   return true;
}

//********************************************************************
// returns true if success
static bool
write_typedef(Symbol * agent, ofstream & out, int level)
{
   // if this agent is marked, then skip it. 
   if (agent->marked)
      return true;

   if( agent->record_class != desired_class ||
       (agent->record_class == RC_LIBRARY &&
        strcmp(agent->source_file, desired_source_file) != 0) )
   {
      return true;
   }

   out << "defType";
   if (agent->arch != free_arch)
      out << "[" << agent->arch->name << "]";
   out << " " << agent->name << ";\n\n";

   // Mark this agent as generated
   agent->marked = true;

   return true;
}

//********************************************************************
// returns TRUE if it wrote something
static bool
write_config(Symbol * agent, ofstream & out, int level, bool looped,
	     Symbol * index, const bool debug)
{
    if (agent == NULL) return false;

    if (debug) cerr << "write_config(" << agent->name << ")\n";

    if( agent->record_class != desired_class )
    {
	if (debug) cerr << "wrong class\n";

	if( agent->defining_rec && agent->is_agent() && isanonymous(agent->name) )
	{
	    // Write an anoymous reference to the record.
	    write_inline(agent, out, level, debug);
	}
	else
	{
	    // emit the reference to the library component
	    indent(out, level);
	    out << agent->name;
	}
	return true;
    }

    bool wrote_something = false;

    switch (agent->symbol_type) {

    case AGENT_CLASS:
    case COORD_CLASS:
    case SENSOR_NAME:
    case BP_CLASS:
    case ROBOT_CLASS:
    case ACTUATOR_NAME:
	wrote_something = write_def(agent, out, level, debug);
	return wrote_something;

    case TYPE_NAME:
	wrote_something = write_typedef(agent, out, level);
	return wrote_something;
    }

    // Does this agent have more than 1 user?
    if (agent->is_instance)
    {
	// If have hit an instance, and isn't this subtree's root, then just
	// emit the name reference.
	if (looped)
	{
	    indent(out, level);
	    out << agent->name;
	    wrote_something = true;
	}
	else
	{
	    switch (agent->symbol_type) {
	    case GROUP_NAME:
	    case CONFIGURATION_NAME:
		wrote_something = write_group(agent, out, level, false, false, true, index, debug);
		break;
	    default:
		wrote_something = write_inst(agent, out, level, debug);
		break;
	    }
	}
	return wrote_something;
    }

    // No, so generate it in-line
    switch (agent->symbol_type) {
    case GROUP_NAME:
    case CONFIGURATION_NAME:
	wrote_something = write_group(agent, out, level, false, true, true, index, debug);
	break;
    default:
	wrote_something = write_inline(agent, out, level, debug);
	break;
    }

    return wrote_something;
}

//********************************************************************

static void
mark_data(Symbol * p, const bool debug)
{
   // These are not really data values, just flags 
   if (p == (Symbol *) UP)
      return;

   // there is nothing to do with constants
   if ( p->symbol_type == INITIALIZER ||
        p->symbol_type == PU_INITIALIZER )
   {
      return;
   }

   // get the perceptual trigger attached to the rule
   if (p->symbol_type == RULE_NAME)
   {
      mark_instances(p->input_generator, debug);
      return;
   }

   // Mark everything else
   mark_instances(p, debug);
}

//********************************************************************

static void
mark_type(Symbol * p, const bool debug)
{
   if (debug)
      cerr << "mark_type(" << p->name << ")\n";

   if (p->record_class != desired_class )
   {
      if (debug)
         cerr << "wrong class\n";
      return;
   }

   // Gen the data type
   stack->insert(p);

   if (debug)
      cerr << "\tStacked type " << p->name << "\n";
}

//********************************************************************
static void
mark_parms(Symbol * node, const bool debug)
{
   if (debug)
      cerr << "mark_parms(" << node->name << ")\n";

   Symbol *p;
   void *cur;

   if ((cur = node->parameter_list.first(&p)) != NULL)
   {
      do
      {
	 if (p->data_type)
	    mark_type(p->data_type, debug);

	 if (p->symbol_type == PARM_HEADER)
	 {
	    Symbol *lp;
	    void *cur_lp;

	    if ((cur_lp = p->parameter_list.first(&lp)) != NULL)
	    {
	       do
	       {
		  if (lp->input_generator)
		     mark_data(lp->input_generator, debug);
		  else if (lp->list_index && lp->list_index->input_generator)
		     mark_data(lp->list_index->input_generator, debug);
	       }
	       while ((cur_lp = p->parameter_list.next(&lp, cur_lp)) != NULL);
	    }
	 }
	 else
	 {
	    if (p->input_generator)
	       mark_data(p->input_generator, debug);
	 }
      }
      while ((cur = node->parameter_list.next(&p, cur)) != NULL);
   }

   // Check for any children
   if ((cur = node->children.first(&p)) != NULL)
   {
      do
      {
	 mark_instances(p, debug);
      }
      while ((cur = node->children.next(&p, cur)) != NULL);
   }

   if (node->symbol_type == BP_NAME && node->bound_to)
   {
      mark_instances(node->bound_to, debug);
   }

}


//********************************************************************
// returns TRUE if success
static void
mark_instances(Symbol * agent, const bool debug)
{
   if( agent == NULL )
      return;

   if (debug)
      cerr << "mark_instances(" << agent->name << ")\n";

   if( agent->record_class != desired_class )
   {
      if (debug)
         cerr << "wrong class\n";

      return;
   }

   // Is this a class definition?
   if (agent->symbol_type == AGENT_CLASS ||
           agent->symbol_type == COORD_CLASS ||
           agent->symbol_type == SENSOR_NAME ||
           agent->symbol_type == BP_CLASS ||
           agent->symbol_type == ROBOT_CLASS ||
           agent->symbol_type == ACTUATOR_NAME)
   {
          // Some sort of def, so stack it
          stack->insert(agent);

          if (debug)
             cerr << "\tStacked def " << agent->name << "\n";

          // Make sure the return type is defined
          if (agent->data_type)
	     mark_type(agent->data_type, debug);

          if (agent->symbol_type == SENSOR_NAME ||
	      agent->symbol_type == ACTUATOR_NAME)
          {
	     // Process the binding point 
	     mark_instances(agent->binds_to, debug);
          }
   }
   else
   {
          // Mark that needs to be an instance if has more than 1 user
          if( !agent->is_instance )
          {
	     if( agent->users.len() > 1 ||
		 agent->symbol_type == BP_NAME )
	     {
	        // Mark it and stack it
	        agent->is_instance = true;
	     }
          }

          // Add (or just raise up) the instance in the stack
          if( agent->is_instance )
	  {
	     stack->insert(agent);

             if (debug)
                cerr << "\tStacked instance " << agent->name << "\n";
	  }

	  // If agent is an instance of a sensor_name
	  if( agent->is_instance &&
	      agent->defining_rec && 
	      (agent->defining_rec->symbol_type == SENSOR_NAME ||
	       agent->defining_rec->symbol_type == ACTUATOR_NAME))
          {
	     // Don't generate the sensor, will do it inline.
	  }
	  else
	  {
             // Mark the defining record
             mark_instances(agent->defining_rec, debug);
	  }
    
          // mark the children.
          mark_parms(agent, debug);
   }
}

//**************************************************************
// Returns true if success
bool
save_workspace(Symbol * agent, const char *filename, bool debug,
               bool backup_files)
{
    if (debug)
        cerr << "save_workspace(" << filename << ")\n";

    char *out_name = strdup(filename);

    // Only write records at the user level.
    desired_class = RC_USER;

    // Make sure we get any changes from the currently displayed page
    config->this_page()->update();

    // If want backups, check if the destination already exists.
    // If not, no need to make a backup.
    if( backup_files )
    {
        FILE *test = fopen(filename,"r");
        if( test != NULL )
            fclose(test);
        else
            backup_files = false;
    }

    // write the new file to a temp name.
    if( backup_files )
    {
        char buf[256];
        sprintf(buf,"cfgedit%d.cdl",getpid());
        out_name = strdup(buf);
    }

    ofstream out(out_name);

    if (!out)
    {
        cerr << "Unable to open output file " << out_name;
        return false;
    }

    if (debug)
        cerr << "Writing output file " << out_name;

    out << "/*************************************************\n";
    out << "*\n";
    out << "* This CDL file " << filename << " was created with cfgedit\n";
    out << "* version " << version_str << '\n';
    out << "*\n";
    out << "**************************************************/\n\n";

    // Define the architecture
    out << "bindArch " << choosen_architecture->name << ";\n\n";

    // Clear all the marked nodes
    agent->clear_marks();
    agent->clear_instances();

    // Allocate a stack to keep track entry points into the tree to generate
    // subtrees.  These are rooted at nodes requiring generation as instances.
    stack = new GTList < Symbol * >;

    // Stack the configuration root
    stack->append(agent);

    // mark any nodes which need to be written as instances
    mark_instances(agent, debug);

    // Reset the marks.
    agent->clear_marks();

    // if are writing a subtree, then ensure the top level is written as
    // an instance
    if( agent->symbol_type != CONFIGURATION_NAME )
        agent->is_instance = true;

    // Write each of the subconfigurations in a depth first fashion.
    while (!stack->isempty())
        write_config(stack->get(), out, 0, false, NULL, debug);

    delete stack;

    if( agent->symbol_type != CONFIGURATION_NAME )
    {
        out << "[" << agent->name << "]";
    }

    out.close();

    if( backup_files )
    {
        // rename the original file to the same name with "~" on the end.

        char buf[1024];
        char extension[1024];
        extension[0] = '\0';
        bool done = false;
        do
        {
            strcat(extension,"~");
            sprintf(buf,"%s%s",filename,extension);
            FILE *test = fopen(buf,"r");
            if( test != NULL )
            {
                fclose(test);
            }
            else
            {
                done = true;
            }
        } while( !done );

        if( rename(filename, buf) )
        {
            /*
            if (errno < sys_nerr)
            {
                warn_userf(
                    "Unable to backup file %s as %s: %s",
                    filename,
                    buf,
                    sys_errlist[errno]);
            }
            else
            {
                warn_userf(
                    "Unable to backup file %s as %s: Unknown error number: %d",
                    filename,
                    buf,
                    errno);
            }
            */
            warn_userf(
                "Unable to backup file %s as %s",
                filename,
                buf);
            unlink(filename);
        }

        if( rename(out_name, filename) )
        {
            /*
            if (errno < sys_nerr)
            {
                warn_userf(
                    "Unable to move temporary file %s to %s: %s",
                    out_name,
                    filename,
                    sys_errlist[errno]);
            }
            else
            {
                warn_userf(
                    "Unable to move temporary file %s to %s: Unknown error number: %d",
                    out_name,
                    filename,
                    errno);
            }
            */
            warn_userf(
                "Unable to move temporary file %s to %s",
                out_name,
                filename);
        }
    }

    return true;
}

//**************************************************************
// Returns true if success
bool
write_library(SymbolList * librecs, const char *filename, bool debug,
               bool backup_files, const char *source_file)
{
//debug = true;
    if (debug)
        cerr << "write_library(" << filename << ")\n";

    char *out_name = strdup(filename);

    // write records at the library level.
    desired_class = RC_LIBRARY;
    desired_source_file = source_file;

    // Make sure we get any changes from the currently displayed page
    config->this_page()->update();

    // If want backups, check if the destination already exists.
    // If not, no need to make a backup.
    if( backup_files )
    {
        FILE *test = fopen(filename,"r");
        if( test != NULL )
            fclose(test);
        else
            backup_files = false;
    }

    // write the new file to a temp name.
    if( backup_files )
    {
        char buf[256];
        sprintf(buf,"cfgedit%d.cdl",getpid());
        out_name = strdup(buf);
    }

    ofstream out(out_name);

    if (!out)
    {
        cerr << "Unable to open output file " << out_name;
        return false;
    }

    if (debug)
        cerr << "Writing output file " << out_name;

    out << "/*************************************************\n";
    out << "*\n";
    out << "* This library file " << filename << " was created with cfgedit\n";
    out << "* version " << version_str << '\n';
    out << "*\n";
    out << "**************************************************/\n\n";

    // Define the architecture
    out << "bindArch " << choosen_architecture->name << ";\n\n";

    // Allocate a stack to keep track entry points into the tree to generate
    // subtrees.  These are rooted at nodes requiring generation as instances.
    stack = new GTList < Symbol * >;

    // Loop through the records and eliminate any
    // records from "librecs" that appear as a node
    // in a different tree.
    void *cur;
    Symbol *agent;
    bool pruned;
    do
    {
        pruned = false;
        cur = librecs->first(&agent);
        while( cur )
        {
            agent->clear_marks();
   
            if( prune_list(librecs, agent, RC_LIBRARY, true) )
            {
                pruned = true;
                break;
            }

            cur = librecs->next(&agent,cur);
        }
    } while(pruned);

    // Clear all the marked nodes
    cur = librecs->first(&agent);
    while( cur )
    {
        agent->clear_marks();
        agent->clear_instances();

        // Stack the configuration root
        stack->append(agent);

        if (debug)
            cerr << "\tStacked root " << agent->name << "\n";

        cur = librecs->next(&agent,cur);
    }

    cur = librecs->first(&agent);
    while( cur )
    {
        // mark any nodes which need to be written as instances
        mark_instances(agent, debug);

        cur = librecs->next(&agent,cur);
    }

    cur = librecs->first(&agent);
    while( cur )
    {
        // Reset the marks.
        agent->clear_marks();

        cur = librecs->next(&agent,cur);
    }

    // Write each of the subconfigurations in a depth first fashion.
    while (!stack->isempty())
    {
        Symbol *root = stack->get();

        if (debug && root)
            cerr << "\tWrite subtree rooted at " << root->name << "\n";

        write_config(root, out, 0, false, NULL, debug);
    }

    delete stack;

    out << "//************************************************\n";
    out << "// Need to switch back to the generic namespace\n";
    out << "bindArch free;\n";
    out << "\n";

    if( backup_files )
    {
        // rename the original file to the same name with "~" on the end.
        out.close();

        char buf[1024];
        char extension[1024];
        extension[0] = '\0';
        bool done = false;
        do
        {
            strcat(extension,"~");
            sprintf(buf,"%s%s",filename,extension);
            FILE *test = fopen(buf,"r");
            if( test != NULL )
            {
                fclose(test);
            }
            else
            {
                done = true;
            }
        } while( !done );

        if( rename(filename, buf) )
        {
            /*
            if (errno < sys_nerr)
            {
                warn_userf(
                    "Unable to backup file %s as %s: %s",
                    filename,
                    buf,
                    sys_errlist[errno]);
            }
            else
            {
                warn_userf(
                    "Unable to backup file %s as %s: Unknown error number: %d",
                    filename,
                    buf,
                    errno);
            }
            */
            warn_userf(
                "Unable to backup file %s as %s",
                filename,
                buf);

            unlink(filename);
        }

        if( rename(out_name, filename) )
        {
            /*
            if (errno < sys_nerr)
            {
                warn_userf(
                    "Unable to move temporary file %s to %s: %s",
                    out_name,
                    filename,
                    sys_errlist[errno]);
            }
            else
            {
                warn_userf(
                    "Unable to move temporary file %s to %s: Unknown error number: %d",
                    out_name,
                    filename,
                    errno);
            }
            */
            warn_userf(
                "Unable to move temporary file %s to %s",
                out_name,
                filename);
        }
    }

    return true;
}

//-----------------------------------------------------------------------
// This function saves an abstracted representation of the work space
// in a file.
//-----------------------------------------------------------------------
bool save_abstracted_workspace(Symbol * agent, const char *filename)
{
    string output_filename = strdup(filename);

    // Only write records at the user level.
    desired_class = RC_USER;

    // Make sure we get any changes from the currently displayed page
    config->this_page()->update();

    ofstream out(output_filename.c_str());

    if (!out)
    {
        cerr << "Unable to open output file " << output_filename.c_str();
        return false;
    }

    // Define the architecture
    out << ACDL_STRING_BIND_ARCH << " " << choosen_architecture->name << "\n\n";

    // Specify the max velocity.
    out << ACDL_STRING_MAX_VELOCITY << " " << config->root()->findData(CDL_STRING_MAX_VELOCITY) << "\n\n";

    // Specify the base velocity.
    out << ACDL_STRING_BASE_VELOCITY << " " << (config->root()->findDataList(CDL_STRING_BASE_VELOCITY)).back() << "\n\n";

    // Clear all the marked nodes
    agent->clear_marks();
    agent->clear_instances();

    // Allocate a stack to keep track entry points into the tree to generate
    // subtrees.  These are rooted at nodes requiring generation as instances.
    stack = new GTList <Symbol *>;

    // Stack the configuration root
    stack->append(agent);

    // mark any nodes which need to be written as instances
    mark_instances(agent, false);

    // Reset the marks.
    agent->clear_marks();

    // if are writing a subtree, then ensure the top level is written as
    // an instance
    if( agent->symbol_type != CONFIGURATION_NAME )
    {
        agent->is_instance = true;
    }

    // Write each of the subconfigurations in a depth first fashion.
    while (!stack->isempty())
    {
        write_abstracted_config(stack->get(), out, false, NULL);
    }

    delete stack;

    out.close();

    return true;
}

//-----------------------------------------------------------------------
// This function writes an abstracted representation of the configuration
// in a file.
//-----------------------------------------------------------------------
bool write_abstracted_config(Symbol *agent, ofstream & out, bool looped, Symbol * index)
{
    if (agent == NULL)
    {
        return false;
    }

    if( agent->record_class != desired_class )
    {
        if( agent->defining_rec && agent->is_agent() && isanonymous(agent->name) )
        {
            // Write an anoymous reference to the record.
            if (!(agent->marked))
            {
                write_abstracted_parms(agent, out);
                agent->marked = true;
            }
        }
        else
        {
            // Emit the reference to the library component
            out << ACDL_STRING_AGENT_NAME << " " << agent->name << endl;
        }
        return true;
    }

    // Does this agent have more than 1 user?
    if (agent->is_instance)
    {
        // If have hit an instance, and isn't this subtree's root, then just
        // emit the name reference.
        if (!looped)
        {
            switch (agent->symbol_type) {
            case GROUP_NAME:
            case CONFIGURATION_NAME:
                write_abstracted_group(agent, out, false, false, true, index);
                break;
            default:
                write_abstracted_inst(agent, out);
                break;
            }
        }
        return true;
    }

    // No, so generate it in-line
    switch (agent->symbol_type) {
    case GROUP_NAME:
    case CONFIGURATION_NAME:
        write_abstracted_group(agent, out, false, true, true, index);
        break;
    default:
        if (!(agent->marked))
        {
            write_abstracted_parms(agent, out);
            agent->marked = true;
        }
        break;
    }

    return true;
}

//-----------------------------------------------------------------------
// This function writes an abstracted representation of the parameters
// (gains, goal location, etc.) in a file.
//-----------------------------------------------------------------------
static bool write_abstracted_parms(Symbol * node, ofstream & out)
{
    Symbol *p, *lp;
    void *cur, *cur_lp;
    bool is_robot = false;
    string bufString;

    if ((node->symbol_type == ROBOT_NAME) ||
        ((node->symbol_type == BP_NAME) && 
         (node->construction == CS_RBP) &&
         (node->bound_to == NULL)))
    {
        is_robot = true;
    }

    if ((cur = node->parameter_list.first(&p)) != NULL)
    {
        do
        {
            if (p->symbol_type == PU_INPUT_NAME)
            {
                if (p->input_generator)
                {
                    write_abstracted_data(out, p->input_generator, NULL);
                    out << ACDL_STRING_PARM_NAME << " " << p->name << endl;
                    out << ACDL_STRING_PARM_VALUE << " " << "{";

                    // The value usually contains white spaces.
                    // Replace it with a filler ('+'),
                    bufString = p->input_generator->name;
                    bufString = replaceCharInString(
                        bufString,
                        ACDL_CHAR_WHITESPACE,
                        ACDL_CHAR_WHITESPACE_FILLER);
                    out << bufString << "}" << endl;
                }
            }
            else if (!is_robot)
            {
                if (p->symbol_type == PARM_HEADER)
                {
                    if ((cur_lp = p->parameter_list.first(&lp)) != NULL)
                    {
                        do
                        {
                            bufString = p->name;

                            if (bufString == CDL_STRING_SOCIETY)
                            {
                                out << ACDL_STRING_TASK_INDEX << " ";
                                out << lp->list_index->name << endl;

                                out << ACDL_STRING_TASK_DESC << " ";
                                out << lp->list_index->description << endl;
                            }
                            else if (bufString == CDL_STRING_RULES)
                            {
                                out << ACDL_STRING_TRIGGERING_TASK_INDEX;
                                out << " " << lp->list_index->name << endl;
                            }

                            if (lp->input_generator)
                            {
                                write_abstracted_data(out, lp->input_generator, lp->list_index);
                            }
                        }
                        while ((cur_lp = p->parameter_list.next(&lp, cur_lp)) != NULL);
                    }
                }
                else
                {
                    // Special case, the bound_to parm is a fake one.
                    bufString = p->name;
                    if ((node->symbol_type == BP_NAME) && (bufString == CDL_STRING_BOUND_TO))
                    {
                        continue;
                    }

                    if (p->input_generator)
                    {
                        // Data for the instance.
                        write_abstracted_data(out, p->input_generator, NULL);
                    }
                }
            }
        }
        while ((cur = node->parameter_list.next(&p, cur)) != NULL);
    }

    if (node->symbol_type == BP_NAME && node->bound_to)
    {
        write_abstracted_data(out, node->bound_to, NULL);
    }

    return true;
}

//-----------------------------------------------------------------------
// This function writes an abstracted representation of the group in a
// file.
//-----------------------------------------------------------------------
static bool write_abstracted_group(
    Symbol * node,
    ofstream & out,
    bool embedded,
    bool in_line,
    bool use_list,
    Symbol * index)
{
    Symbol *p;
    void *cur;
    int len;
    bool use_group;

    // if this node is marked, then skip it. 
    if (!embedded && node->marked)
    {
	return true;
    }

    len = node->children.len();

    if (len == 0 && !use_list)
    {
	return true;
    }

    use_group = len > 1 || embedded || in_line;

    if (!in_line)
    {
	use_group = true;
    }

    if (use_group)
    {
	// Write any pushed up parms.
	if (has_pu_parms(node))
	{
	    write_abstracted_parms(node, out);
	}
    }

    if ((cur = node->children.first(&p)) != NULL)
    {
	do
	{
	    write_abstracted_config(p, out, true, NULL);
	}
	while ((cur = node->children.next(&p, cur)) != NULL);
    }

    // Mark this node as generated
    if (!embedded)
    {
	node->marked = true;
    }

    return true;
}

//-----------------------------------------------------------------------
// This function writes an abstracted representation of the instance in a
// file.
//-----------------------------------------------------------------------
static bool write_abstracted_inst(Symbol * node, ofstream & out)
{
   // if this node is marked, then skip it. 
   if (node->marked)
   {
      return true;
   }

   //out << ACDL_STRING_INSTANCE_INDEX << " " << node->name << endl;
   out << ACDL_STRING_INSTANCE_NAME << " " << node->defining_rec->name << endl;

   out << endl;
   out << ACDL_STRING_INSTANCE << " " << ACDL_STRING_START << endl;
   out << endl;

   write_abstracted_parms(node, out);

   out << endl;
   out << ACDL_STRING_INSTANCE << " " << ACDL_STRING_END << endl;
   out << endl;

   // Mark this node as generated
   node->marked = true;
   node->is_instance = true;

   return true;
}

//-----------------------------------------------------------------------
// This function writes an abstracted representation of the data in a
// file.
//-----------------------------------------------------------------------
static bool write_abstracted_data(ofstream & out, Symbol * p, Symbol * index)
{
    if ((p == NULL) ||
        (p == (Symbol *) UP) ||
        (p->symbol_type == INITIALIZER) ||
        (p->symbol_type == PU_INITIALIZER))
    {
        return true;
    }
    else if (p->symbol_type == RULE_NAME)
    {
        out << ACDL_STRING_IF << " " << ACDL_STRING_START << endl;
        write_abstracted_config(p->input_generator, out, false, NULL);
        out << ACDL_STRING_GOTO << " " << p->list_index->name << endl;
        out << ACDL_STRING_IF << " " << ACDL_STRING_END << endl;
    }
    else if (!p->is_instance)
    {
        return write_abstracted_config(p, out, true, index);
    }

    return true;
}

//-----------------------------------------------------------------------
// This function converts the abscracted CDL code into the regular CDL
// foramt.
//-----------------------------------------------------------------------
char *acdl2cdl(
    const char *acdlCode,
    const string defaultRobotName,
    int *status)
{
    FILE *tmpACDLFile = NULL;
    char tmpACDLFilename[256], tmpCDLFilename[256];
    char *cdlCode = NULL;
    char buf1[1024], buf2[1024], buf3[1024], buf4[1024], buf5[1024], buf6[1024];
    char numBuf[1024], stateBuf[1024], cautiousVelBuf[1024];
    string bufString1, bufString2, bufString3, bufString4, bufString5, bufString6;
    string robotName, anonymousRobotName, anonymousRobotNameBP;
    string taskIndexString;
    string maxVelValue, baseVelValue, cautiousVelValue;
    vector<string> anonymousRobotNameBPList;
    double baseVel, cautiousVel;
    bool firstTaskPlaced = false;
    int fd;
    int i, cdlCodeSize;
    int taskPosX, taskPosY;
    int taskStartX = ACDL2CDL_AUTO_TASKPLACE_STARTX;
    int taskStartY = ACDL2CDL_AUTO_TASKPLACE_STARTY;
    int taskSpaceX = ACDL2CDL_AUTO_TASKPLACE_SPACEX;
    int taskSpaceY = ACDL2CDL_AUTO_TASKPLACE_SPACEY;
    int taskEndX = ACDL2CDL_AUTO_TASKPLACE_ENDX;
    int taskNumColumn = ACDL2CDL_AUTO_TASKPLACE_NUM_COLUMN;
    int taskCount = 0, numRobots = 0, stateNum;
    int robotX = ACDL2CDL_AUTO_ROBOTPLACE_STARTX;
    int robotY = ACDL2CDL_AUTO_ROBOTPLACE_STARTY;
    const double DEFAULT_BASE_TO_CAUTIOUS_VEL_MULTIPLIER = 0.2;
    
    // Dump the ACDL code in a temporary file first.
    sprintf(tmpACDLFilename, "/tmp/%s-cfgedit-acdl2cdl-acdl-XXXXXX", getenv("USER"));
    fd = mkstemp(tmpACDLFilename);
    unlink(tmpACDLFilename);

    // Dump the data.
    tmpACDLFile = fopen(tmpACDLFilename, "w");
    fprintf(tmpACDLFile, "%s", acdlCode);
    fclose(tmpACDLFile);

    // Now, open up the file, again, for reading.
    tmpACDLFile = fopen(tmpACDLFilename, "r");

    // To construct the CDL code, use another temorary file.
    sprintf(tmpCDLFilename, "/tmp/%s-cfgedit-acdl2cdl-cdl-XXXXXX", getenv("USER"));
    fd = mkstemp(tmpCDLFilename);
    unlink(tmpCDLFilename);
    ofstream tmpCDLWriteFile(tmpCDLFilename);

    if (!tmpCDLWriteFile)
    {
        fprintf(stderr, "Warning: acdl2cdl() failed to open a temporary file.\n");
        return NULL;
    }

    // Create a header.
    tmpCDLWriteFile << "/*************************************************\n";
    tmpCDLWriteFile << "*\n";
    tmpCDLWriteFile << "* This CDL file was converted from an abstracted\n";
    tmpCDLWriteFile << "* CDL (ACDL) file.\n";
    tmpCDLWriteFile << "*\n";
    tmpCDLWriteFile << "**************************************************/\n\n";
    
    // Process the data.
    while (fscanf(tmpACDLFile, "%s %s", buf1, buf2) != EOF)
    {
        bufString1 = buf1;
        bufString2 = buf2;

        if (bufString1 == ACDL_STRING_BIND_ARCH)
        {
            // The architecture name is found.
            tmpCDLWriteFile << CDL_STRING_BIND_ARCH << " " << bufString2 << ";\n\n";

        }
        else if (bufString1 == ACDL_STRING_MAX_VELOCITY)
        {
            maxVelValue = bufString2;	    
        }
        else if (bufString1 == ACDL_STRING_BASE_VELOCITY)
        {
            baseVelValue = bufString2;	    

            // Compute the cautious velocity.
            baseVel = atof(baseVelValue.c_str());
            cautiousVel = DEFAULT_BASE_TO_CAUTIOUS_VEL_MULTIPLIER * baseVel; 
            sprintf(cautiousVelBuf, "%.2f", cautiousVel);
            cautiousVelValue = cautiousVelBuf;
        }
        else if ((bufString1 == ACDL_STRING_INSTANCE_NAME) &&
                 (bufString2 == ACDL_STRING_MOVEMENT))
        {
            firstTaskPlaced = false;
            taskCount = 0;
            taskIndexString = ACDL_STRING_EMPTY;

            // Create a robot name.
            numRobots++;
            sprintf(numBuf, "%d", numRobots);
            robotName = defaultRobotName;
            robotName += numBuf;
            anonymousRobotName = "$";
            anonymousRobotName += robotName;

            tmpCDLWriteFile << "instBP<222,15> |The Wheels Binding Point| ";
            tmpCDLWriteFile << anonymousRobotName << " from movement(\n";
            //tmpCDLWriteFile << "  v<0,0> = ,\n";
            //tmpCDLWriteFile << "  " << CDL_STRING_BASE_VELOCITY << " = {0.1},\n";
            tmpCDLWriteFile << "  " << CDL_STRING_BOUND_TO << " = base:DRIVE_W_SPIN(\n";
            tmpCDLWriteFile << CDL_8SPACES << "v<12,15> = FSA(\n";

            // Now, read about FSA.
            while (fscanf(tmpACDLFile, "%s %s", buf3, buf4) != EOF)
            {
                bufString3 = buf3;
                bufString4 = buf4;

                if (bufString3 == ACDL_STRING_TASK_INDEX)
                {
                    // Task index $AN_xx (or Start) is found.
                    taskIndexString = bufString4;
                }
                else if (bufString3 == ACDL_STRING_TASK_DESC)
                {
                    if (taskIndexString == ACDL_STRING_EMPTY)
                    {
                        fprintf(
                            stderr,
                            "Error(cfgedit): acdl2cdl() [%s: line%d]: taskIndexString is empty.\n",
                            __FILE__,
                            __LINE__);
                        continue;
                    }

                    if (taskCount > 0)
                    {
                        tmpCDLWriteFile << ",\n";
                    }

                    if (firstTaskPlaced)
                    {
                        stateNum = taskCount;
                        sprintf(stateBuf, "State%d", stateNum);

                    }
                    else
                    {
                        if (bufString4 == CDL_STRING_START)
                        {
                            // Special case. Start task.
                            stateNum = 0;
                            sprintf(stateBuf, CDL_STRING_START.c_str());
                            firstTaskPlaced = true;
                        }
                        else
                        {
                            stateNum = taskCount + 1;
                            sprintf(stateBuf, "State%d", stateNum);
                        }
                    }

                    // Computer the horizontal position of the task to be placed.
                    if (((stateNum / taskNumColumn) % 2) == 0)
                    {
                        // Even rows run left to right.
                        taskPosX = (taskSpaceX * (stateNum % taskNumColumn)) + taskStartX;
                    }
                    else
                    {
                        // Odd rows place right to left.
                        taskPosX = taskEndX - (taskSpaceX * (stateNum % taskNumColumn));
                    }

                    // Computer the vertical position of the task to be placed.
                    taskPosY = (taskSpaceY * (stateNum / taskNumColumn)) + taskStartY;

                    tmpCDLWriteFile << CDL_14SPACES << CDL_STRING_SOCIETY;
                    tmpCDLWriteFile << "[" << taskIndexString << "]";
                    tmpCDLWriteFile << "<" << taskPosX << "," << taskPosY << ">";
                    tmpCDLWriteFile << "|" << stateBuf << "| = [\n";

                    taskCount++;
                    taskIndexString = ACDL_STRING_EMPTY;
                }
                else if (bufString3 == ACDL_STRING_PARM_NAME)
                {
                    // Write a parameter name.
                    tmpCDLWriteFile << CDL_16SPACES << bufString4 << " = ";
                }
                else if (bufString3 == ACDL_STRING_PARM_VALUE)
                {
                    // Write a parameter value (after fixing the white spaces).
                    bufString4 = replaceCharInString(
                        bufString4,
                        ACDL_CHAR_WHITESPACE_FILLER,
                        ACDL_CHAR_WHITESPACE);
                    tmpCDLWriteFile << bufString4 << ",\n";
                }
                else if (bufString3 == ACDL_STRING_AGENT_NAME)
                {
                    // Task name is found. Save in as a segement of the plan.
                    tmpCDLWriteFile << CDL_14SPACES << bufString4 << "]<10,10>\n";
                }
                else if (bufString3 == ACDL_STRING_TRIGGERING_TASK_INDEX)
                {
                    tmpCDLWriteFile << ",\n";
                    tmpCDLWriteFile << CDL_12SPACES << CDL_STRING_RULES << "[";
                    tmpCDLWriteFile << bufString4 << "]<50,50> = " << CDL_STRING_IF << " [\n";

                    while (fscanf(tmpACDLFile, "%s %s", buf5, buf6) != EOF)
                    {
                        bufString5 = buf5;
                        bufString6 = buf6;

                        if (bufString5 == ACDL_STRING_PARM_NAME)
                        {
                            // Write a parameter name.
                            tmpCDLWriteFile << CDL_16SPACES << bufString6 << " = ";
                        }
                        else if (bufString5 == ACDL_STRING_PARM_VALUE)
                        {
                            // Write a parameter value (after fixing the white spaces).
                            bufString6 = replaceCharInString(
                                bufString6,
                                ACDL_CHAR_WHITESPACE_FILLER,
                                ACDL_CHAR_WHITESPACE);
                            tmpCDLWriteFile << bufString6 << ",\n";
                        }
                        else if (bufString5 == ACDL_STRING_AGENT_NAME)
                        {
                            tmpCDLWriteFile << CDL_14SPACES << bufString6 << "]<10,10>\n";

                        }
                        else if (bufString5 == ACDL_STRING_GOTO)
                        {
                            tmpCDLWriteFile << " " << CDL_STRING_GOTO << " " << bufString6;
                        }
                        else if ((bufString5 == ACDL_STRING_IF) &&
                                 (bufString6 == ACDL_STRING_END))
                        {
                            break;
                        }
                    }
                }
                else if ((bufString3 == ACDL_STRING_INSTANCE) &&
                         (bufString4 == ACDL_STRING_END))
                {
                    break;
                }
            }

            tmpCDLWriteFile << ")<292,156>|The State Machine|\n";
            tmpCDLWriteFile << ",\n";
            tmpCDLWriteFile << CDL_8SPACES << CDL_STRING_MAX_VELOCITY << " = {" << maxVelValue << "},\n";
            tmpCDLWriteFile << CDL_8SPACES << CDL_STRING_BASE_VELOCITY << " = {" << baseVelValue << "},\n";
            //tmpCDLWriteFile << CDL_8SPACES << "cautious_vel = {0.05},\n";
            tmpCDLWriteFile << CDL_8SPACES << "cautious_vel = {" << cautiousVelValue << "},\n";
            tmpCDLWriteFile << CDL_8SPACES << "cautious_mode = {true})<222,15>|The Wheels Actuator|\n";
            tmpCDLWriteFile << ");\n\n";

            anonymousRobotNameBP = anonymousRobotName;
            anonymousRobotNameBP += "BP";
            anonymousRobotNameBPList.push_back(anonymousRobotNameBP);

            tmpCDLWriteFile << "instBP<0,0> " << anonymousRobotNameBP;
            tmpCDLWriteFile << " from vehicle(\n";
            tmpCDLWriteFile << CDL_2SPACES << CDL_STRING_BOUND_TO;
            tmpCDLWriteFile << " = " << robotName << ":DEFAULT_ROBOT(\n";
            tmpCDLWriteFile << robotName << ":[\n";
            tmpCDLWriteFile << CDL_10SPACES << anonymousRobotName <<  "]\n";
            tmpCDLWriteFile << ")<" << robotX << "," << robotY << ">|Individual Robot|\n";
            robotY += ACDL2CDL_AUTO_ROBOTPLACE_SPACEY;
            tmpCDLWriteFile << ");\n\n";
        }
    }

    tmpCDLWriteFile << "[\n";
    tmpCDLWriteFile << "[\n";
    tmpCDLWriteFile << CDL_4SPACES << anonymousRobotNameBPList[0];

    for (i = 1; i < (int)(anonymousRobotNameBPList.size()); i++)
    {
        tmpCDLWriteFile << ",\n";
        tmpCDLWriteFile << CDL_4SPACES << anonymousRobotNameBPList[i];
    }
    
    tmpCDLWriteFile << "]<10,10>|Group of Robots|\n";
    tmpCDLWriteFile << "]<10,10>\n";

    tmpCDLWriteFile.close();
    
    // Next, read the saved code and return it.
    ifstream tmpCDLReadFile(tmpCDLFilename);

    if (tmpCDLReadFile.bad())
    {
        fprintf(stderr, "Warning: acdl2cdl() failed to open a temporary file.\n");
        return NULL;
    }

    // Find the data size.
    tmpCDLReadFile.seekg (0, ios::end);
    cdlCodeSize = tmpCDLReadFile.tellg();
    tmpCDLReadFile.seekg (0, ios::beg);

    // To append NULL at the end.
    cdlCodeSize++; 

    // Read in the data.
    cdlCode = new char[cdlCodeSize];
    tmpCDLReadFile.read (cdlCode, cdlCodeSize-1);
    
    // Append a null, to make it a char string.
    cdlCode[cdlCodeSize-1] = '\0';

    
    tmpCDLReadFile.close();
    unlink(tmpCDLFilename);

    fclose(tmpACDLFile);
    unlink(tmpACDLFilename);

    if (cdlCode == NULL)
    {
        *status = ACDL2CDL_STATUS_FAILURE;
    }
    else if (taskCount == 0)
    {
        *status = ACDL2CDL_STATUS_EMPTYMISSION;
    }
    else
    {
        *status = ACDL2CDL_STATUS_SUCCESS;
    }

    return cdlCode;
}


///////////////////////////////////////////////////////////////////////
// $Log: cdl_codegen.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:25  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.2  2007/08/09 19:22:58  endo
// Fix for numbering a state in acdl2cdl().
//
// Revision 1.1.1.1  2006/07/20 17:17:52  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:38:01  endo
// MissionLab 7.0
//
// Revision 1.4  2006/02/19 17:58:31  endo
// Experiment related modifications
//
// Revision 1.3  2006/01/10 06:08:24  endo
// AO-FNC Type-I check-in.
//
// Revision 1.2  2005/02/07 23:39:02  endo
// Mods from usability-2004
//
// Revision 1.1.1.1  2005/02/06 23:00:22  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.9  2003/04/21 03:26:13  endo
// *** empty log message ***
//
// Revision 1.8  2003/04/06 15:12:39  endo
// gcc 3.1.1
//
// Revision 1.7  2003/04/06 09:10:03  endo
// Updated for CBR Wizard Prototype II.
//
// Revision 1.6  2002/01/13 01:12:15  endo
// list -> GTList
//
// Revision 1.5  2000/04/20 03:26:39  endo
// Checked in for Doug.
// This patch fixes some problems with the cfgedit import code which
// caused it to core dump.  It also corrects a problem where the imported
// assemblage would still have the same name as the library assemblage,
// causing conflicts.  It now moves the name to the description field
// and assigns a unique name to the imported assemblage.  A small correction
// to the write_cdl code was neccessary to get it to actually write the
// descriptions for group records.
//
// Revision 1.4  2000/02/10 05:10:48  endo
// Checked in for Douglas C. MacKenzie.
// He modified the CDL code generator to allow it
// to correctly write the screen location of agents
// (e.g., a DATABASE_DOUBLE) connected to a pushed up parameter.
// Currently, the location is lost and a default used.
// It also fixes the remaining compiler warnings in write_cdl
// by adding an include file and removing some library prototypes
// which are no longer needed in RedHat 6.0.
//
// Revision 1.3  1999/12/16 22:57:23  endo
// rh-6.0 porting.
//
// Revision 1.2  1996/10/01 13:00:51  doug
// got library writes working
//
// Revision 1.1  1996/09/25 19:12:07  doug
// Initial revision
//
// Revision 1.57  1996/05/07  19:17:03  doug
// fixing compile warnings
//
// Revision 1.56  1996/02/25  01:13:14  doug
// *** empty log message ***
//
// Revision 1.55  1996/02/19  21:57:05  doug
// library components and permissions now work
//
// Revision 1.54  1996/02/18  00:03:13  doug
// binding is working better
//
// Revision 1.53  1996/02/16  00:07:18  doug
// *** empty log message ***
//
// Revision 1.52  1996/02/01  04:04:30  doug
// *** empty log message ***
//
// Revision 1.51  1996/01/31  03:06:53  doug
// *** empty log message ***
//
// Revision 1.50  1996/01/29  00:07:50  doug
// *** empty log message ***
//
// Revision 1.49  1996/01/27  00:09:24  doug
// added grouped lists of parameters
//
// Revision 1.48  1995/12/14  21:53:29  doug
// *** empty log message ***
//
// Revision 1.47  1995/11/30  23:31:07  doug
// *** empty log message ***
//
// Revision 1.46  1995/11/29  23:13:28  doug
// *** empty log message ***
//
// Revision 1.45  1995/11/21  23:10:08  doug
// *** empty log message ***
//
// Revision 1.44  1995/09/26  21:42:55  doug
// write the name on groups if not anonymous
//
// Revision 1.43  1995/09/15  22:02:20  doug
// added support for the command list in the executive
//
// Revision 1.42  1995/09/07  14:22:26  doug
// works
//
// Revision 1.41  1995/09/01  16:14:56  doug
// Able to run double wander config
//
// Revision 1.40  1995/07/10  17:05:45  doug
// *** empty log message ***
//
// Revision 1.39  1995/07/07  18:17:46  doug
// added a couple more tests for NULL to stop core dumps
//
// Revision 1.38  1995/06/29  17:09:54  jmc
// Added header and RCS id and log strings.
///////////////////////////////////////////////////////////////////////
