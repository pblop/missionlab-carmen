/**********************************************************************
 **                                                                  **
 **                  cnl_codegen.cc                                  **
 **                                                                  **
 **  Code generator targeted for AuRA Schema architecure using CNL.  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2005, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: cnl_codegen.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */


#include <libc.h>
#include <fstream>
#include <iostream>
#include <string>

#include "prototypes.h"
#include "cdl_defs.h"
#include "list.hpp"
#include "symtab.h"
#include "cdl_gram.tab.h"

#include "cfgedit_common.h"
#include "codegens.h"
#include "version.h"
#include "utilities.h"

using std::endl;
using std::string;

static enum
{
   PROCS, NODES
}
PASS;

static Symbol *root;

// DCM: This is the name given to the database variable holding the
//  list of FSAs in the mission.
char FSA_LIST[] = "FSA_LIST";

//********************************************************************

static void write_data(ofstream & out, const Symbol * robot, const Symbol * p)
{
    Symbol *rec = NULL;
    Symbol *def = NULL;
    string value;

    def = p->input_generator;

    if (def == NULL)
    {
        out << "NO_SOURCE";
        error("Missing input connection for parm:", p->name);
    }
    else if(def->symbol_type == PU_INITIALIZER)
    { 
        // Special case: if there is a name caste, then use that name instead
        if(def->name && def->data_type)
        {
            // Should really generate some type coersion code here
            rec = find_pu_data(def->name, 0);
        }
        else
        {
            // Normal case
            rec = find_pu_data(p->name, 0);
        }

        if(rec == NULL)
        {
            error("Didn't find def for pushed-up parm:",p->name);
            return;
        }

        def = rec->input_generator;

        if(def && def->name)
        {
            //DCM 02-5-00: Only dump the brackets if it really is an initializer!
            if(def->symbol_type == INITIALIZER)
            {
                out << '{';
            }

            value = def->name;

            if ((p->input_generator->pair) != NULL)
            {
                // There is a pair. Append it.
                rec = find_pu_data(p->input_generator->pair->name, 0);

                if ((rec != NULL) &&
                    ((rec->input_generator) != NULL) &&
                    ((rec->input_generator->name) != NULL))
                {
                    value = value + "," + rec->input_generator->name;
                }
            }

            out << value;

            if(def->symbol_type == INITIALIZER)
            {
                out << '}';
            }
        }
        else
        {
            out << "{MISSING_SOURCE}";
            error(
                "Missing value for input parm:", 
                p->input_generator->name ? p->input_generator->name : p->name);
        }
    }
    else if (def->symbol_type == INITIALIZER)
    {
        out << '{' << def->name << '}';
    }
    else 
    {
        if (def->robot != NULL && def->robot != robot)
        {
            out << Anonymous2External(def->robot->name) << ":";
        }

        // Binding points should refer to the name defined in the robot def
        if(def->symbol_type == AGENT_NAME && 
           (def->construction == CS_IBP || def->construction == CS_OBP))
        {
            Symbol *rsrc = real_source(def->defining_rec);
            if(rsrc)
                out << rsrc->name;
            else
            {
                out << "UNKNOWN";
                error("Unknown defining record for binding point");
            }
        }
        else
        {
            Symbol *rsrc = real_source(def);
            if(rsrc)
                out << rsrc->name;
            else
            {
                out << "UNKNOWN";
                error("Unknown source record");
            }
        }
    }
}

//********************************************************************
// returns true if success
int
write_fsa_proc(ofstream & out, Symbol * fsa, const bool local)
{
   char *output_type = fsa->defining_rec->data_type->name;

   if (!local)
      out << "external ";

   out << "nprocedure " << output_type << " ";
   out << fsa->name << "_proc with\n";

   // Write the input parm list
   Symbol *rule_list = NULL;
   Symbol *triggers = NULL;
   Symbol *members = NULL;

   void *cur;
   Symbol *p;

   if ((cur = fsa->parameter_list.first(&p)) != NULL)
   {
      do
      {
	 if (p->symbol_type == PARM_HEADER)
	 {
	    // Is a list

	    // Remember the list of rules
	    // and rules are not really parameters, they are hard coded.
	    if (strcmp(p->name,"rules") == 0)
	    {
	       rule_list = p;
	    }
	    else if (strcmp(p->name,"society") == 0)
	    {
	       members = p;
	    }
	    else if (strcmp(p->name,"triggers") == 0)
	    {
	       triggers = p;
	    }
	    else
	    {
               error("Unknown parameter in FSA");
               return false;
            }
	 }
	 else if (p->symbol_type == PU_INPUT_NAME)
	 {
	     // This FSA has push up parameters.
	 }
	 else
	 {
            error("Unknown parameter in FSA");
            return false;
	 }
      }
      while ((cur = fsa->parameter_list.next(&p, cur)) != NULL);
   }

   // Make sure found the parms
   if (rule_list == NULL)
   {
      error("Didn't define a set of rules");
      return false;
   }

   if (members == NULL)
   {
      error("Didn't define a set of members");
      return false;
   }

   if (triggers == NULL)
   {
      error("Didn't define a set of triggers");
      return false;
   }

   // Set the index value for each of the triggers
   int cnt=0;
   if ((cur = rule_list->parameter_list.first(&p)) != NULL)
   {
      do
      {
         p->input_generator->index_value = cnt++;
      } while ((cur = rule_list->parameter_list.next(&p, cur)) != NULL);
   }

   indent(out, 1);
   out << "list " << output_type << " " << members->name << ";\n";

   indent(out, 1);
   out << "list " << triggers->data_type->name << " " << triggers->name << ";\n";

   if (local)
   {
      static int fsa_num = 0;
      fsa_num++;

      out << "once\n";

      // DCM: Create a database variable keyed with the name of the FSA
      // The format is: "A SSSSSS"
      // Where   'A': is '0' if the FSA did not run this cycle or '1' if it did.
      // Where   'S': is the active state number (999,999 max)
      indent(out, 1);
      out << "put_state(\""<< fsa->name << "\", \"0 0     \");\n";
      indent(out, 1);
      out << "char *_fsaActive_ = get_state(\""<< fsa->name << "\");\n";
      indent(out, 1);
      out << "parms->activeFlag = &_fsaActive_[0];" << endl;
      indent(out, 1);
      out << "char *_fsaState_ = &_fsaActive_[2];\n";

      symbol_table < char >states(10);
      char *start_name = NULL;

      // Generate the constants defining each state value
      // and add the states into a symbol table for easy access.
      if ((cur = members->parameter_list.first(&p)) != NULL)
      {
          int val=0;
          do
          {
              if (states.exists(p->list_index->name))
              {
                  error("Two states have the same name\n");
                  return false;
              }

	    states.put(p->list_index->name, p->list_index->name);
	    p->list_index->index_value = val++;
            indent(out, 1);
	    out << "const int " << p->list_index->name << " = ";
	    out << p->list_index->index_value << ";\n";

	    if(strcasecmp(p->list_index->name,"start") == 0)
	       start_name = p->list_index->name;
         } while ((cur = members->parameter_list.next(&p, cur)) != NULL);
      }

      // DCM: Enhancement to make FSA state and task names available.
      // Write tables of state and task names to the FSA proc which is 
      // indexed by the state index number.
      // Since the index_value of each state was just assigned based on the
      // order they appear in the parameter_list, I can just dump them out
      // and the orders will match.

      // First do the state names.
      indent(out, 1);
      out << "const char *state_names[] = {";
      bool firstone = true;
      if ((cur = members->parameter_list.first(&p)) != NULL)
      {
         do
	 {
            if(firstone)
               firstone = false;
            else
	       out << ",";
	    out << "\"" << p->list_index->description << "\"";
         } while ((cur = members->parameter_list.next(&p, cur)) != NULL);
      }
      out << "};\n";

      // Now do the task names
      indent(out, 1);
      out << "const char *task_names[] = {";
      firstone = true;
      if ((cur = members->parameter_list.first(&p)) != NULL)
      {
         do
	 {
            if(firstone)
               firstone = false;
            else
	       out << ",";
	    out << "\"";

            // Have to do a lot of testing to make sure we don't dereference
            // a null pointer on some weird configuration.
            char *task_name = "unknown";
            if(p->input_generator != NULL)
            {
               // OK, we got a valid input_generator, It should only have 
               // a single child, which is the task to perform.
               if (p->input_generator->children.len() == 1)
               {
                  Symbol *child = NULL;
                  p->input_generator->children.first(&child);

                  if(child != NULL && child->name != NULL && 
                      child->name[0] != '\0')
                  {
                     // After all that, here is the task, remember its name.
                     task_name = child->name; 
                  }
                  else
                  {
                     error("Warning: child name is not valid in dump FSA proc, the state task name will not be correct\n");
                  }
               }
               else
               {
                  char buf[256];
                  sprintf(buf,"input_generator has children %d in dump FSA proc, the state task name will not be correct\n", p->children.len());
                  error(buf);
               }
            }
            else
            {
               error("NULL input_generator in dump FSA proc, the state task name will not be correct\n");
            }

            out << task_name << "\"";
         } while ((cur = members->parameter_list.next(&p, cur)) != NULL);
      }
      out << "};\n";


      // Make sure defined a start state and then set it as the initial one.
      if(start_name == NULL)
      {
	 error("One state in the FSA must be named \"start\"");
	 return false;
      }
      //indent(out, 1);
      //out << "int state = " << start_name << ";\n";

      indent(out, 1);
      out << "int fsa_num = " << fsa_num << ";\n";

      // DCM: Append the name of the FSA record to a database variable
      // so other processes can determine the names of FSAs in the mission.
      // NOTE: put_state only updates the robot's local database.
      //   If you need the values to propogate to the mlab console,
      //   change this call to exec_put_console_state, which does both.
      indent(out, 1);
      out << "char *oldval = get_state(\""<< FSA_LIST <<"\");\n";
      // If we aren't already registered, append our name to the list of FSAs.
      indent(out, 1);
      out << "if(strstr(oldval, \"" << fsa->name << "\") == NULL)\n";
      indent(out, 1);
      out << "{\n";
      indent(out, 2);
      out << "char *newval = (char *)malloc(strlen(oldval) + 256);\n";
      indent(out, 2);
      out << "strcpy(newval, oldval);\n";
      indent(out, 2);
      out << "strcat(newval, \"" << fsa->name << " \");\n";
      indent(out, 2);
      out << "put_state(\""<< FSA_LIST << "\", newval);\n";
      indent(out, 2);
      out << "free(newval);\n";
      indent(out, 1);
      out << "}\n";

      out << "header\n";
      indent(out, 1);
      out << "int state = " << start_name << ";\n";
      indent(out, 1);
      out << "int last_state = -1;\n";
      indent(out, 1);
      // See the comments "For each FSA, ..." below.
      out << "bool dummy_new_state = FALSE;\n";

      // Emit the body of the procedure
      out << "body\n";

      out << "\n";
      indent(out, 1);
      out << "current_state[fsa_num] = state;\n";
      indent(out, 1);
      out << "previous_state[fsa_num] = last_state;\n";

      // For each FSA, if the robot is still in the same state as it was in
      // the previous time cycle, it makes "in_new_state" to be TRUE.
      // However, technically, the first same state is the true new state
      // because it loads the new state during the transition from the old state.
      // dummy_new_state is used to solve this problem.
      out << "\n";
      indent(out, 1);
      out << "// For each FSA, if the robot is still in the same state as it was in\n";
      indent(out, 1);
      out << "// the previous time cycle, it makes \"in_new_state\" to be TRUE.\n";
      indent(out, 1);
      out << "// However, technically, the first same state is the true new state\n";
      indent(out, 1);
      out << "// because it loads the new state during the transition from the old state.\n";
      indent(out, 1);
      out << "// dummy_new_state is used to solve this problem.\n";
      indent(out, 1);
      out << "\n";
      indent(out, 1);
      out << "in_new_state[fsa_num] = FALSE;\n";
      indent(out, 1);
      out << "if (dummy_new_state) // the 1st same state = our new state\n";
      indent(out, 1);
      out << "{\n";
      indent(out, 2);
      out << "in_new_state[fsa_num] = TRUE;\n";
      indent(out, 2);
      out << "dummy_new_state = FALSE;\n";
      indent(out, 1);
      out << "}\n";
      indent(out, 1);
      out << "if (state != last_state)\n";
      indent(out, 1);
      out << "{\n";
      indent(out, 2);
      out << "in_new_state[fsa_num] = FALSE;\n";
      indent(out, 2);
      out << "dummy_new_state = TRUE;\n";
      indent(out, 2);
      out << "if(state < 999999)\n";
      indent(out, 3);
      out << "sprintf(_fsaState_,\"%d\", state);" << endl;
      indent(out, 2);
      out << "else\n";
      indent(out, 3);
      out << "sprintf(_fsaState_,\"XXXXXX\");" << endl;
      indent(out, 1);
      out << "}\n";

      // This is the hook for the mlab console's "Report current state"
      // function in the "Debug" menu.
      out << "\n";
      indent(out, 1);
      out << "// This is the hook for the mlab console's \"Report current state\"\n";
      indent(out, 1);
      out << "// function in the \"Debug\" menu.\n";
      indent(out, 1);
      out << "if (report_current_state)\n";
      indent(out, 1);
      out << "{\n";
      indent(out, 2);
      out << "if (state != last_state)\n";
      indent(out, 2);
      out << "{\n";
      // Report the number of the current state
      indent(out, 3);
      out << "char msg[512];\n\n";
      indent(out, 3);
      out << "sprintf(msg,\"robot(%d) FSA(%d): current state = %s [%s]\\n\",\n";
      indent(out, 4);
      out << "robot_id, fsa_num, state_names[state], task_names[state]);\n";
      indent(out, 3);
      out << "exec_report_current_state(msg);\n";
      indent(out, 3);
      out << "//fprintf(stderr,\"robot(%d) FSA(%d): current state = %s [%s]\\n\",\n";
      indent(out, 3);
      out << "//  robot_id, fsa_num, state_names[state], task_names[state]);\n";
      // Report the number of the current state
      indent(out, 2);
      out << "}\n";
      indent(out, 1);
      out << "}\n\n";

      // Add a debug print
      indent(out, 1);
      out << "if(debug)\n";
      indent(out, 1);
      out << "{\n";

      // DCM: Since we have the task name now, add it to the debug print.
      indent(out, 2);
      //out << "fprintf(stderr,\"FSA(%d): current state=%s task=%s\\n\",\n";
      out << "fprintf(stderr,\"robot(%d): current state=%s task=%s\\n\",\n";

      // DCM: We have the state and task names now so simplify the debug print.
      indent(out, 3);
      out << "robot_id, state_names[state], task_names[state]);\n\n";

      Symbol *rec;
      if ((rec = (Symbol *)fsa->table.first()) != NULL)
      {
	 do
	 {
	    if (rec->symbol_type == INDEX_NAME)
	    {
	       indent(out, 2);
               out << "fprintf(stderr,\"   %d: "
	           << rec->description
		   << " = <%.1f %.1f>\\n\",parms->society_require["
		   << rec->index_value
		   << "],society["
		   << rec->index_value
		   << "].x,society["
		   << rec->index_value
		   << "].y);\n";
	    }
	 } while ((rec = (Symbol *)fsa->table.next()) != NULL);
      }
      indent(out, 1);
      out << "{for(int i=0; i<triggers_num; i++)\n";
      indent(out, 2);
      out << "fprintf(stderr,\"   %d: triggers[%d] = %d\\n\",parms->triggers_require[i],i,triggers[i]);}\n";

      indent(out, 1);
      out << "}\n\n";

      // Write the state machine
      indent(out, 1);
      out << "if(ok_to_run)\n";
      indent(out, 1);
      out << "{\n";

      // During a state change, the new member will not be valid
      // because we loaded the old one last time instead
      indent(out, 2);
      out << "if(state == last_state)\n";
      indent(out, 3);
      out << "output = " << members->name << "[state];\n";
      
      //DCM: Post the current state and task to the database each time we 
      // start a new state.  The users can read these values and compare 
      // them to the last value they read to determine state changes.
      indent(out, 2);
      out << "else\n";
      indent(out, 2);
      out << "{\n";
      // DCM NOTE: put_state only updates the robot's local database.
      //   If you need the values to propogate to the mlab console,
      //   change this call to exec_put_console_state, which does both.
      indent(out, 3);
      out << "put_state(\""<< fsa->name << "_state\", (char *)state_names[state]);\n";
      indent(out, 3);
      out << "put_state(\""<< fsa->name << "_task\", (char *)task_names[state]);\n";
      indent(out, 2);
      out << "}\n";

      indent(out, 2);
      out << "last_state = state;\n";


      // Add a debug print
      out << '\n';
      indent(out, 2);
      out << "if(debug)\n";
      indent(out, 3);
      out << "fprintf(stderr,\"   output = <%.1f, %.1f>\\n\",";
      out << "output.x, output.y);\n";



      indent(out, 2);
      out << "switch(state)\n";
      indent(out, 2);
      out << "{\n";


      const char *state_name = states.first();

      while (state_name)
      {
	 indent(out, 3);
	 out << "case " << state_name << ":\n";

         Symbol *rule;
	 int need_else = false;

         // Write out the transition rules
	 if((cur = rule_list->parameter_list.first(&rule)))
	 {
	    do
	    {
	       if (strcmp(rule->list_index->name, state_name) == 0)
	       {
	          indent(out, 4);

	          // Use if on first and else if on the remainder
	          if (need_else)
		     out << "else ";
	          else
		     need_else = true;

	          out << "if(" << triggers->name;
                  out << '[' << rule->input_generator->index_value << "])\n";

	          indent(out, 4);
	          out << "{\n";

	          indent(out, 5);
	          out << "state = ";
	          out << rule->input_generator->list_index->name << ";\n";
   
	          indent(out, 4);
	          out << "}\n";
	       }
	    } while ((cur = rule_list->parameter_list.next(&rule, cur)) != NULL);
	 }

	 indent(out, 4);
	 out << "break;\n";
	 state_name = states.next();

	 // add some whitespace
	 if (state_name)
	    out << '\n';
      }

      indent(out, 2);
      out << "}\n";

      indent(out, 1);
      out << "}\n";

      indent(out, 1);
      out << "parms->clear_requests();\n";

      // Write the state machine to get the correct require calls
      indent(out, 1);
      out << "switch(state)\n";
      indent(out, 1);
      out << "{\n";

      state_name = states.first();

      while (state_name)
      {
	 indent(out, 2);
	 out << "case " << state_name << ":\n";

         Symbol *rule;

         // Make sure the parms are loaded
	 if((cur = rule_list->parameter_list.first(&rule)))
	 {
	    do
	    {
	       if (strcmp(rule->list_index->name, state_name) == 0)
	       {
	          indent(out, 3);
	          out << "parms->NAME2NAME_REQUIRE(" << triggers->name << ')';
	          out << '[' << rule->input_generator->index_value << "] = true;\n";
	       }
	    } while ((cur = rule_list->parameter_list.next(&rule, cur)) != NULL);
	 }

	 indent(out, 3);
	 out << "break;\n";
	 state_name = states.next();

	 // add some whitespace
	 if (state_name)
	    out << '\n';
      }

      indent(out, 1);
      out << "}\n";
      indent(out, 1);
      out << "parms->NAME2NAME_REQUIRE(" << members->name << ")[state] = true;\n";

   }
   out << "pend\n\n";

   return true;
}
//********************************************************************
// returns true if success
int
write_RL_proc (ofstream & out, Symbol * rl, const bool local)
{
  char *output_type = rl->defining_rec->data_type->name;

  if (!local)
    {
      out << "external ";
    }

  Symbol *triggers = NULL;
  Symbol *members = NULL;
  Symbol *reinforcers = NULL;

  void *cur;
  Symbol *p;

  int countCorrect;

  if ((cur = rl->parameter_list.first (&p)) != NULL)
    {
      do
        {
          if (p->symbol_type == PARM_HEADER)
            {
              // Is a list

              // Remember the list of rules
              // and rules are not really parameters, they are hard coded.
              if (strcmp (p->name, "society") == 0)
                {
                  members = p;
                }
              else if (strcmp (p->name, "triggers") == 0)
                {
                  triggers = p;
                }
              else if (strcmp (p->name, "reinforcers") == 0)
                {
                  reinforcers = p;
                }
            }
          else
            {
              countCorrect = 0;
              if (strcmp (p->name, "QfileName") != 0)
                {
                  countCorrect = 1;
                }
              else if (strcmp (p->name, "ActionTimeout") != 0)
                {
                  countCorrect = 1;
                }
              else if (strcmp (p->name, "alpha") != 0)
                {
                  countCorrect = 1;
                }
              else if (strcmp (p->name, "alphaDecay") != 0)
                {
                  countCorrect = 1;
                }
              else if (strcmp (p->name, "random") != 0)
                {
                  countCorrect = 1;
                }
              else if (strcmp (p->name, "randomDecay") != 0)
                {
                  countCorrect = 1;
                }

              if (!countCorrect)
                {
                  error ("Unknown parameter in RL");
                  error (p->name);
		  out.flush();
                  return false;
                }
            }
        }
      while ((cur = rl->parameter_list.next (&p, cur)) != NULL);
    }

  out << "nprocedure " << output_type << " ";
  out << rl->name << "_proc with\n";

  // Print the input parameter list
  // NOTE: I have not sorted the list members so they will be emitted in
  //       the order they appear in the input file (I think).
  if ((cur = rl->parameter_list.first (&p)) != NULL)
    {
      do
        {
          indent (out, 1);
          if (p->symbol_type == PARM_HEADER)
            {
              // Is a list
              out << "list ";
            }
          // Print the data type and parm name.
          out << p->data_type->name << " " << p->name << ";\n";
        }
      while ((cur = rl->parameter_list.next (&p, cur)) != NULL);
    }

  //*****************************ONCE*********************************

  out << "once\n";

  indent (out, 1);
  out << "int gotreinf;\n";
  indent (out, 1);
  out << "const char *task_names[] = {";
  bool firstone = true;
  if ((cur = members->parameter_list.first (&p)) != NULL)
    {
      do
        {
          if (firstone)
            {
              firstone = false;
            }
          else
            {
              out << ",";
            }
          out << "\"";

          // Have to do a lot of testing to make sure we don't dereference
          // a null pointer on some weird configuration.
          char *task_name = "unknown";
          if (p->input_generator != NULL)
            {
              // OK, we got a valid input_generator, It should only have 
              // a single child, which is the task to perform.
              if (p->input_generator->children.len () == 1)
                {
                  Symbol *child = NULL;
                  p->input_generator->children.first (&child);

                  if (child != NULL && child->name != NULL &&
                      child->name[0] != '\0')
                    {
                      // After all that, here is the task, remember its name.
                      task_name = child->name;
                    }
                  else
                    {
                      error
                        ("Warning: child name is not valid in dump FSA proc, the state task name will not be correct\n");
                    }
                }
              else
                {
                  char buf[256];
                  sprintf (buf,
                           "input_generator has children %d in dump FSA proc, the state task name will not be correct\n",
                           p->children.len ());
                  error (buf);
                }
            }
          else
            {
              error
                ("NULL input_generator in dump FSA proc, the state task name will not be correct\n");
            }

          out << task_name << "\"";
        }
      while ((cur = members->parameter_list.next (&p, cur)) != NULL);
    }
  out << "};\n";
  indent (out, 1);

  out <<
    "Qlearn *qlearn = new Qlearn((int)(pow(2,triggers_num)*society_num), society_num, QfileName.val,robot_id,alpha,alphaDecay,random,randomDecay);\n";
  indent (out, 1);

  //*****************************HEADER*********************************

  out << "header\n";
  indent (out, 1);
  out << "int behavioralstate = 0;\n";
  indent (out, 1);
  out << "int lastbehavioralstate = -1;\n";
  indent (out, 1);
  out << "int perceptstate = -1;\n";
  indent (out, 1);
  out << "int worldstate = -1;\n";
  indent (out, 1);
  out << "int timer = 0;\n";
  indent (out, 1);
  out << "int i;\n";
  indent (out, 1);
  out << "int lastperceptstate = -1;\n";
  indent (out, 1);
  out << "double reinf = 0;\n";
  indent (out, 1);
  out << "char* statFileName = new char[128];\n";
  indent (out, 1);
  out << "sprintf(statFileName,\"%droleInfo.out\",robot_id);\n";
  indent (out, 1);
  out << "FILE* statFile;\n";
  indent (out, 1);


  //*****************************BODY*********************************

  out << "body\n";
  indent (out, 1);

  out << "timer++;\n";
  indent (out, 1);

  out << "if (behavioralstate < 0)\n";
  indent (out, 2);
  out << "behavioralstate = 0;\n";


  indent (out, 1);
  out << "worldstate = 0;\n";
  indent (out, 1);
  out << "perceptstate = 0;\n";
  indent (out, 1);
  out << "gotreinf = 0;\n";

  // Step 1.  Calculate the World State
  indent (out, 1);
  out << "for(i = triggers_num - 1; i >= 0; i--){\n";
  indent (out, 2);
  out << "if(triggers[i]){\n";
  indent (out, 3);
  out << "worldstate += (int)pow(2, i);\n";
  indent (out, 3);
  out << "perceptstate += (int)pow(2, i);\n";
  indent (out, 2);
  out << "}\n";
  indent (out, 1);
  out << "}\n";
  indent (out, 1);
  out << "worldstate = behavioralstate+(worldstate*society_num);\n";
  indent (out, 1);

  // Step 2... Check All Reinforcers for the presence of a reward
  //    a query is performed when a reinforcer is found with the 
  //    correct worldstate.  Note that -10 is used as a '*' variable
  //    allowing any value in that state/action
  indent (out, 1);
  out << "for(i = 0; i<reinforcers_num; i++)\n";
  indent (out, 2);
  out << "{\n";
  indent (out, 2);
  out <<
    "if (((worldstate == reinforcers[i].state)||(reinforcers[i].state==-1000))&&((behavioralstate == reinforcers[i].action)||(reinforcers[i].action==-1000)))\n";
  indent (out, 3);
  out << "{\n";
  indent (out, 3);
  out << "reinf += reinforcers[i].value;\n";
  indent (out, 3);
  out <<
    "behavioralstate = qlearn->query(worldstate, reinforcers[i].value);\n";
  indent (out, 3);
  out << "statFile = fopen(statFileName,\"a+\");\n";
  indent (out, 3);
  out <<
    "fprintf(statFile,\"%d %d %d\\n\",behavioralstate,worldstate,timer);\n";
  indent (out, 3);
  out << "fclose(statFile);";
  indent (out, 3);
  out << "gotreinf = 1;\n";
  indent (out, 3);
  out << "timer = 0;\n";
  indent (out, 2);
  out << "}\n";
  indent (out, 1);
  out << "}\n";

  // Step 3... If no reward was received, but the state did change,
  //        query the table anyways with 0 reward

  indent (out, 1);
  out << "if ((perceptstate != lastperceptstate)&&(gotreinf == 0)){\n";
  indent (out, 2);
  out << "behavioralstate = qlearn->query(worldstate, 0);\n";
  indent (out, 2);

  indent (out, 2);
  out << "statFile = fopen(statFileName,\"a+\");\n";
  indent (out, 2);
  out <<
    "fprintf(statFile,\"%d %d %d\\n\",behavioralstate,worldstate,timer);\n";
  indent (out, 2);
  out << "fclose(statFile);";

  out << "timer = 0;\n";
  indent (out, 1);
  out << "}\n";

  // Step 4...  If the timer has runout since the last time a query 
  //    was performed, then query the table.  This was done to 
  //    prevent the system from becoming stuck in a local minima
  out << "if (timer>=ActionTimeout){\n";
  indent (out, 2);
  out << "behavioralstate = qlearn->query(worldstate, 0);\n";

  indent (out, 2);
  out << "statFile = fopen(statFileName,\"a+\");\n";
  indent (out, 2);
  out <<
    "fprintf(statFile,\"%d %d %d\\n\",behavioralstate,worldstate,timer);\n";
  indent (out, 2);
  out << "fclose(statFile);";

  indent (out, 2);
  out << "timer = 0;\n";
  indent (out, 1);
  out << "}\n";

  //Step 5...  Reset the parameters for the system, and
  //     set the correct behavior for execution.
  indent (out, 1);
  out << "lastperceptstate = perceptstate;\n";
  indent (out, 1);
  out << "\n";
  indent (out, 1);
  out << "parms->clear_requests();\n\n";
  indent (out, 1);
  out << "{for(i = 0; i<triggers_num; i++)\n";
  indent (out, 2);
  out << "parms->NAME2NAME_REQUIRE(" << triggers->name << ")[i] = true;\n}";

  indent (out, 1);
  out << "for(i = 0; i < reinforcers_num; i++){\n";
  indent (out, 2);
  out << "parms->NAME2NAME_REQUIRE(" << reinforcers->name << ")[i] = true;\n";
  out << "}\n";

  indent (out, 1);
  out << "if((ok_to_run)&&(behavioralstate==lastbehavioralstate))\n";
  indent (out, 2);
  out << "output = society[behavioralstate];\n";
  indent (out, 1);
  out << "parms->NAME2NAME_REQUIRE(" << members->
    name << ")[behavioralstate] = true;\n";
  indent (out, 1);
  out << "lastbehavioralstate = behavioralstate;\n";

  indent (out, 1);
  out << "qlearn->save();\n";

  out << "pend\n\n";

  out.flush();

  return true;
}

//********************************************************************
  // returns true if success
static int
write_fsa_node(ofstream & out, Symbol * robot, Symbol * node, int level)
{
    indent(out, level);
//   if (node->robot != NULL && node->robot != robot)
//   {
//      out << "remote " << "node ";
//      out << node->robot->name << ':' << node->name;
//      out << " is " << node->name << "_proc\n\n";
//   }
//   else
    {
//      if (node->robot != NULL)
//	 out << "public ";

        out << "node " << node->name << " is " << node->name << "_proc";
        if(node->description)
            out << " |" << node->description << "|";
        out << " with\n";

        // Write the input parm list
        Symbol *rule_list = NULL;
        Symbol *triggers = NULL;
        Symbol *members = NULL;

        void *cur;
        Symbol *p;

        if ((cur = node->parameter_list.first(&p)) != NULL)
        {
            do
            {
                if (p->symbol_type == PARM_HEADER)
                {
                    // Is a list
   
                    // Remember the list of rules
                    // and rules are not really parameters, they are hard coded.
                    if (strcmp(p->name,"rules") == 0)
                    {
                        rule_list = p;
                    }
                    else if (strcmp(p->name,"society") == 0)
                    {
                        members = p;
                    }
                    else if (strcmp(p->name,"triggers") == 0)
                    {
                        triggers = p;
                    }
                    else
                    {
                        error("Unknown parameter in FSA");
                        return false;
                    }
                }
                else if (p->symbol_type == PU_INPUT_NAME)
                {
                    // This FSA has push up parameters.
                }
                else
                {
                    error("Unknown parameter in FSA");
                    return false;
                }
            }
            while ((cur = node->parameter_list.next(&p, cur)) != NULL);
        }
   
        // Make sure found the parms
        if (rule_list == NULL)
        {
            error("Didn't define a set of rules");
            return false;
        }
   
        if (members == NULL)
        {
            error("Didn't define a set of members");
            return false;
        }
   
        if (triggers == NULL)
        {
            error("Didn't define a set of triggers");
            return false;
        }

        if ((cur = rule_list->parameter_list.first(&p)) != NULL)
        {
            do
            {
                indent(out, level + 1);
                out << triggers->name << " = ";
                Symbol *rsrc = real_source(p->input_generator->input_generator);
                if(rsrc)
                    out << rsrc->name;
                else
                {
                    out << "UNKNOWN";
                    error("Unconnected perceptual trigger");
                }

                out << ";\n";
            } while ((cur = rule_list->parameter_list.next(&p, cur)) != NULL);
        }

        if ((cur = members->parameter_list.first(&p)) != NULL)
        {
            do
            {
                if(p->symbol_type != PU_INPUT_NAME)
                {
                    // Write the list entry, skipping the [xxx]
                    indent(out, level + 1);
                    out << members->name << " = ";
                    write_data(out, robot, p);
                    out << ";\n";
                }
            } while ((cur = members->parameter_list.next(&p, cur)) != NULL);
        }


        indent(out, level);
        out << "nend\n\n";
    }

    return true;
}


//********************************************************************
  // returns true if success
static bool
write_node(const char *basename, ofstream & out, Symbol * robot, 
           Symbol * node, int level, const int mark_nodes)
{

    if(debug_save)
        cerr << "write_node " << node->name << " ";

    // if this node is marked, then skip it. 
    // if this is a binding point, then skip it.  It better be bound by now.
    // If this is a robot, skip it.  They are just containers
    // If this is a group name, skip it.  They are just containers
    if(node->marked || 
        (node->symbol_type == AGENT_NAME && node->bound_to) ||
        (node->symbol_type == BP_NAME && node->construction == CS_RBP) ||
        node->symbol_type == ROBOT_NAME ||
        node->symbol_type == GROUP_NAME)
    {
        if(debug_save)
            cerr << "skipping\n";

        return true;
    }

    int is_fsa = node->symbol_type == COORD_NAME &&
        node->defining_rec->operator_style == FSA_STYLE;
    int is_rl = node->symbol_type == COORD_NAME &&
        node->defining_rec->operator_style == RL_STYLE;

    // Check if this is a local or remote node
    if (mark_nodes)
    {
        // If no one has spoken for this node yet, then we'll take it.
        if (node->robot == NULL)
            node->robot = robot;
    }

    bool local = (node->robot == NULL || node->robot == robot);

    if (PASS == PROCS)
    {
        if (is_fsa)
        {
            write_fsa_proc(out, node, local);
        }
        else if (is_rl)
        {
            write_RL_proc (out, node, local);
        }
    }
    else
    {
        if (is_fsa)
        {
            write_fsa_node(out, robot, node, level);
        }
        else
        {
            // Make a local so can change it if this is a binding point
            Symbol *rec = node;

            indent(out, level);
            char *node_name = rec->name;
            out << "node " << rec->name << " is ";

            // Binding points should refer to the name defined in the robot def
            char *proc_name = "Unknown";
            if(rec->symbol_type == BP_NAME)
            {
                if(rec->bound_to == NULL)
                {
                    error("Binding point is not bound to a hardware device: ", 
                          rec->name);
                }
                else
                {
                    if(rec->bound_to->defining_rec &&
                        rec->bound_to->defining_rec->defining_rec)
                    {
                        proc_name = rec->bound_to->defining_rec->defining_rec->name;

                        // Move up to the hdw record to get the parameter list
                        rec = rec->bound_to;
                    }
                    else
                    {
                        error("Unable to find defining record for binding point: ",
                              rec->name);
                    }
                }
            }
            else
            {
                if(rec->defining_rec)
                {
                    proc_name = rec->defining_rec->name;
                }
                else
                {
                    error("Unable to find name of node");
                }
            }

            if (is_rl)
            {
                out << node_name << "_proc";
            }
            else
            {
                out << proc_name;
            }

            if(node->description)
                out << " |" << node->description << "|";
            out << " with\n";

            Symbol *p;
            void *cur;

            if ((cur = rec->parameter_list.first(&p)) != NULL)
            {
                do
                {
                    if (p->symbol_type == PARM_HEADER)
                    {
                        Symbol *lp;
                        void *cur_lp;

                        if ((cur_lp = p->parameter_list.first(&lp)) == NULL)
                        {
                            error("Empty parameter list in SELECT node");
                            return false;
                        }

                        do
                        {
                            if(lp->symbol_type != PU_INPUT_NAME)
                            {
                                // Write the list entry, skipping the [xxx]
                                indent(out, level + 1);
                                out << p->name << " = ";
                                write_data(out, robot, lp);
                                out << ";\n";
                            }
                        }
                        while ((cur_lp = p->parameter_list.next(&lp, cur_lp)) != NULL);
                    }
                    else if (p->data_type == commands_type)
                    {
                        // Commands are written to a seperate file
                        char *filename;
                        if(basename)
                            filename = strdup(basename);
                        else
                            filename = strdup("");

                        filename = strdupcat(filename, "list");
                        filename = strdupcat(filename, ".cmds");

                        FILE *file = fopen(filename, "w");
                        if (file == NULL)
                        {
                            error("Unable to create commands file");
                        }
                        else
                        {
                            char *string = p->input_generator->name;

                            if(string != NULL)
                            {
                                char *start = strchr(string,'\"');
                                char *end = strrchr(string,'\"');
                                if(start == NULL)
                                    start = string;
                                else
                                    start++;
   
                                if(end == NULL)
                                {
                                    int len = strlen(string);
                                    end = &string[len-1];
                                }
                                else
                                    end --;
			    
                                int len = (int)(end - start) + 1;
                                if(len > 0 && (int)fwrite(start,1,len,file) != len)
                                {      
                                    error("Unable to write temporary file to display commands");
                                }
                            }      
                        }
                        fclose(file);
                        cerr << "Writing command list to " << filename << "\n";
                    }
                    else
                    {
                        if(p->symbol_type != PU_INPUT_NAME)
                        {
                            indent(out, level + 1);
                            out << p->name << " = ";
                            write_data(out, robot, p);
                            out << ";\n";
                        }
                    }
                }
                while ((cur = rec->parameter_list.next(&p, cur)) != NULL);
            }

            indent(out, level);
            out << "nend\n\n";
        }
    }

    // Mark this node as generated
    node->marked = true;

    if(debug_save)
        cerr << "done\n";

    return true;
}

//********************************************************************
  // returns the filestream handle or NULL on failure
static ofstream *
create_robot_file(Symbol *robot, const char *basename)
{
    ofstream *out = NULL;
    static GTList < char *>module_names;

    if (PASS == PROCS)
    {
        char *filename;
        if(basename)
            filename = strdup(basename);
        else
            filename = strdup("");

        filename = strdupcat(filename, Anonymous2External(robot->name));
        filename = strdupcat(filename, ".cnl");


        out = new ofstream(filename);
        //cerr << "Opened file " << filename << " handle=" << (ulong)out << std::endl;
        robot->file = out;

        if(out == NULL)
        {
            error("Unable to open output file", filename);
            return NULL;
        }

        cerr << "Writing robot " << filename << '\n';

        // Write a header
        *out << "/*************************************************\n";
        *out << "*\n";
        *out << "* This file " << filename << " was created with the command\n";
        *out << "* " << command_line << '\n';
        *out << "* using the CDL compiler, version " << version_str << '\n';
        *out << "*\n";
        *out << "**************************************************/\n";

        *out << '\n';

        // include header files for all CNL libraries that were specified
        const rc_chain *files = rc_table.get(CNL_LIBRARIES);
        if(files) 
        {
            char *str;
            void *pos = files->first(&str);
            while(pos)
            {  
                char *fullname = strdupcat(strdup(str),".inc");
                *out << "#include \"" << fullname << "\"\n";

                // Get next filename
                pos = files->next(&str, pos);
            }
        }

        *out << '\n';

        // dump any initializer macros that were defined in this architecture
        SymbolTable *table = (SymbolTable *)&choosen_architecture->table;
        Symbol *data = (Symbol *)table->first();
        while(data)
        {
            if(data->symbol_type == NAMED_VALUE)
            {
                *out << "#define " << data->name << " " << data->input_generator->name << '\n';
            }
            data = (Symbol *)table->next();
        }

        *out << '\n';
        *out << "modulename = " << robot->name << ";\n\n";
        module_names.append(robot->name);

        generated_cnl_files++;
    }
    else
    {
        out = robot->file;
        //cerr << "reusing file handle=" << (ulong)out << std::endl;
        void *cur;
        char *name;

        if ((cur = module_names.first(&name)) != NULL)
        {
            do
            {
                if (strcmp(name, robot->name))
                    *out << "module " << name << ";\n";
            }
            while ((cur = module_names.next(&name, cur)) != NULL);
        }
        *out << '\n';
    }

    // Clear all the marked nodes so can generate the duplicated cases.
    root->clear_marks();

    // Clear ownership of nodes within the robot
    SymbolList queue;
    robot->enqueue_children(&queue);
    while (!queue.isempty())
    {
        Symbol *node = queue.get();

        // Add the node's children to the queue 
        node->enqueue_children(&queue);

        node->robot = NULL;
    }

    return out;
}

//********************************************************************
// closes the filestream handle 
// returns true if it really closed it.
bool close_robot_file(ofstream *out)
{
   bool rtn = false;
   if (PASS != PROCS)
   {
      out->close();
      //cerr << "Closed file " << (ulong)out << std::endl;
      rtn = true;
   }
   return rtn;
}

//********************************************************************
// Write this node and then recurse on each of its children
// Returns true on success
static bool generate_node(Symbol * node, const char *basename, ofstream *out, Symbol * robot, int level, const int mark_nodes)
{
   SymbolList queue;
   bool good = true;

   // Stack this agent
   AgentStack.insert(node);

   if (node->symbol_type == ROBOT_NAME)
   {
      cerr << "Error: Robot " << node->name << " is contained within the definition of robot " << robot->name << std::endl;
      return false;
   }
   else if(robot == NULL)
   {
      cerr << "Error: NULL Robot definition in generate_node " << node->name << std::endl;
      return false;
   }

   // Generate the node 
   good = write_node(basename, *out, robot, node, 0, true);

   // Add the node's children to the queue 
   node->enqueue_children(&queue);

   while (good && !queue.isempty())
   {
      Symbol *node = queue.get();
   
      good = generate_node(node, basename, out, robot, 0, true);
   }

   // Remove this agent
   AgentStack.get();

   return good;
}

//********************************************************************
// Walk through this node and when we find a robot, create a file and
// generate its contents.
// Returns true on success
static bool
generate_crown_node(Symbol * node, const char *basename, Symbol * robot, int level, const int mark_nodes)
{
   // Stack this agent
   AgentStack.insert(node);

   ofstream *out = NULL;
   if (node->symbol_type == ROBOT_NAME)
   {
      // found a robot, create the file to write its contents into.
      out = create_robot_file(node, basename);
      robot = node;
   }

   // Add the node's children to a queue 
   SymbolList queue;
   node->enqueue_children(&queue);

   // walk the queue expanding each.
   bool good = true;
   while (good && !queue.isempty())
   {
      Symbol *node = queue.get();
   
      if(out)
      {
	 // writing real code.
         good = generate_node(node, basename, out, robot, 0, true);
      }
      else
      {
	 // still looking for a robot
         good = generate_crown_node(node, basename, robot, 0, true);
      }
   }

   // Remove this agent
   AgentStack.get();

   // flush and close the file
   if(out)
   {
      if(close_robot_file(out))
      {
         delete out;
	 out = NULL;
      }
   }

   return good;
}

//********************************************************************
// returns true if success
int
cnl_codegen(Symbol * top, char *basename)
{
   cdl_had_error = false;

   /* Publicize the top of the tree as a global */
   root = top;
   top->parent = NULL;

   // Generate any procedure defs first, they must come before any nodes.
   PASS = PROCS;
   bool good = generate_crown_node(top, basename, NULL, 0, true);

   if(good && !cdl_had_error)
   {
      // Now generate the nodes
      PASS = NODES;
      good = generate_crown_node(top, basename, NULL, 0, true);
   }

   return good && !cdl_had_error;
}


///////////////////////////////////////////////////////////////////////
// $Log: cnl_codegen.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:14  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.2  2006/12/05 01:51:02  endo
// cdl parser now accepts {& a b} and Symbol *pair added.
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.2  2005/02/07 23:50:29  endo
// Mods from usability-2004
//
// Revision 1.1.1.1  2005/02/06 22:59:29  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.61  2003/04/06 19:40:26  endo
// *** empty log message ***
//
// Revision 1.60  2003/04/06 15:18:46  endo
// gcc 3.1.1
//
// Revision 1.59  2003/04/06 13:11:30  endo
// gcc 3.1.1
//
// Revision 1.58  2002/11/01 16:57:19  ebeowulf
// Fixed typo in previous check-in.
//
// Revision 1.57  2002/10/25 15:14:11  ebeowulf
// Added the write_RL_proc procedure to allow qlearning to control the execution of its component actions.  Also modified the naming scheme in write_node to allow for RL procedures and FSA procedures.
//
// Revision 1.56  2002/01/13 01:31:52  endo
// list -> GTList
//
// Revision 1.55  2001/12/23 20:41:57  endo
// RH 7.1 porting.
//
// Revision 1.54  2000/04/16 16:04:05  endo
// Made FSA to re-start from Start state when it was re-entered.
// Made compiler to compile properly when FSA with pushed-up parameters
// is in the CDL code.
//
// Revision 1.53  2000/03/20 00:30:01  endo
// The bug of CfgEdit failing to compile multiple robots
// was fixed.
//
// Revision 1.52  2000/03/13 18:22:41  endo
// Fixed some in_new_state stuff.
//
// Revision 1.51  2000/02/18 02:53:18  endo
// a variable in_new_state was introduced.
//
// Revision 1.50  2000/02/10 04:52:42  endo
// Checked in for Douglas C. MacKenzie.
// He modified the CDL compiler to allow it
// to correctly generate CNL code for cases where an agent
// (e.g., a DATABASE_DOUBLE) has been connected to a pushed
// up parameter.  Currently, the CDL compiler only handles
// the cases where a pushed up parameter is assigned a
// constant value.
//
// Revision 1.49  1999/12/18 09:47:07  endo
// modified, so that it can call
// exec_report_current_state(msg)
// when it's specified.
//
// Revision 1.48  1999/11/15 17:27:46  endo
// Checking in for Doug's modification.
// He added a feature to the cdl compile that, when you
// debug the robot executable, it will be able to tell
// what the name of the state it is currently in.
//
// Revision 1.47  1999/10/25 18:22:01  endo
// rolled back to this version.
//
// Revision 1.45  1996/05/07 19:53:55  doug
// fixing compile warnings
//
// Revision 1.44  1996/04/18  00:20:50  doug
// *** empty log message ***
//
// Revision 1.43  1996/04/06  23:54:56  doug
// *** empty log message ***
//
// Revision 1.42  1996/03/19  21:57:15  doug
// added better debug prints to the fSA
//
// Revision 1.41  1996/03/13  01:52:36  doug
// fixed error reporting
//
// Revision 1.40  1996/03/08  20:43:36  doug
// *** empty log message ***
//
// Revision 1.39  1996/03/08  00:46:41  doug
// *** empty log message ***
//
// Revision 1.38  1996/03/06  23:39:08  doug
// *** empty log message ***
//
// Revision 1.37  1996/02/25  01:13:39  doug
// *** empty log message ***
//
// Revision 1.36  1996/02/19  21:57:58  doug
// library components and permissions now work
//
// Revision 1.35  1996/02/18  22:44:13  doug
// handle pushup parms in cnl_codegen
//
// Revision 1.34  1996/02/14  17:15:16  doug
// added support for descriptions
//
// Revision 1.33  1996/01/17  18:46:26  doug
// *** empty log message ***
//
// Revision 1.32  1995/11/21  23:11:36  doug
// *** empty log message ***
//
// Revision 1.31  1995/11/08  16:51:38  doug
// *** empty log message ***
//
// Revision 1.30  1995/11/01  23:00:51  doug
// using tcb class
//
// Revision 1.30  1995/11/01  23:00:51  doug
// using tcb class
//
// Revision 1.29  1995/10/30  23:04:00  doug
// *** empty log message ***
//
// Revision 1.28  1995/10/30  14:51:29  doug
// remove warning
//
// Revision 1.27  1995/10/27  20:30:25  doug
// *** empty log message ***
//
// Revision 1.26  1995/10/18  18:20:33  doug
// finalizing the FSA proc code
//
// Revision 1.25  1995/10/12  20:14:14  doug
// fixing FSA procs
//
// Revision 1.24  1995/10/11  22:03:06  doug
// *** empty log message ***
//
// Revision 1.23  1995/10/10  20:43:45  doug
// *** empty log message ***
//
// Revision 1.22  1995/10/09  21:57:34  doug
// *** empty log message ***
//
// Revision 1.21  1995/09/15  22:01:03  doug
// made it write command list to a file
//
// Revision 1.20  1995/09/07  14:21:39  doug
// works
//
// Revision 1.19  1995/08/24  22:02:38  doug
// Clear the "robot" field within scope of a robot after
// generating the nodes so any other robot duping those nodes
// will create them local.
//
// Revision 1.18  1995/08/22  18:03:29  doug
// skip GROUP_NAME nodes
//
// Revision 1.17  1995/08/21  16:17:53  doug
// add support for -o flag so can set output file prefix
//
// Revision 1.16  1995/07/10  19:38:14  doug
// *** empty log message ***
//
// Revision 1.15  1995/06/29  15:02:56  jmc
// Added RCS log string.
///////////////////////////////////////////////////////////////////////
