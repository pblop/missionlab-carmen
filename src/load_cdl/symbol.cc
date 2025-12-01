/**********************************************************************
 **                                                                  **
 **                       symbol.cc                                  **  
 **                                                                  **
 **  methods for the symbol class                                    **  
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: symbol.cc,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#include <assert.h>
#include <string>
#include <vector>

#include "symbol_table.hpp"
#include "list.hpp"
#include "symtab.h"
#include "symbol.hpp"
#include "cdl_gram.tab.h"
#include "prototypes.h"

using std::string;
using std::vector;

const string EMPTY_STRING = "";

// Construct the record.
Symbol::Symbol(int type)
{
   memset(this,0,sizeof(Symbol));

   symbol_type = type;
   record_class = current_class;
   source_file = current_source_file;
}

// Copy Constructor
Symbol::Symbol(const Symbol & src)
{
   memcpy(this,&src,sizeof(Symbol));

   parameter_list = src.parameter_list;
   table = src.table;
   name = src.name ? strdup(src.name) : (char *)NULL;
   description = src.description ? strdup(src.description) : (char *)NULL;
   id = src.id;

   // the source filename string isn't duplicated.
   source_file = src.source_file;

   input_generator = src.input_generator;
   pair = src.pair;
   parm_list_head = src.parm_list_head;
}

// assignment: cleanup and copy
Symbol & Symbol::operator = (const Symbol & src)
{
    assert(&src != NULL);
    assert(this);

    if (this != &src)
    {
        // Delete existing data
        if (name)
            delete name;

        if (description)
            delete description;

        // Copy record
        symbol_type = src.symbol_type;
        arch = src.arch;
        data_type = src.data_type;
        parameter_list = src.parameter_list;
        defining_rec = src.defining_rec;
        table = src.table;
        name = src.name ? strdup(src.name) : (char *)NULL;
        id = src.id;
      
        // the source filename string isn't duplicated.
        source_file = src.source_file;

        input_generator = src.input_generator;
        pair = src.pair;
        list_index = src.list_index;
        level = src.level;
        is_list = src.is_list;
        index_value = src.index_value;
        robot = src.robot;
        marked = src.marked;
        constant = src.constant;
        is_advanced_parm = src.is_advanced_parm;
        is_pair = src.is_pair;
        record_class = src.record_class;
        bound_to = src.bound_to;
        binds_to = src.binds_to;
        this_screen_object = src.this_screen_object;
        location = src.location;
        description = src.description ? strdup(src.description) : (char *)NULL;
        parm_list_head = src.parm_list_head;
    }

    return *this;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=

// Destroy the record.
Symbol::~Symbol()
{
   if (name)
   {
      Symbol *p = LookupLocalName(name);

      if(p == this)
      {
         // Remove our name from the symbol table
         RemoveName(this);
      }

      // Free the allocated string
      delete[]name;
   }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=

// check if this node has children
bool
Symbol::has_children()
{
    Symbol *p;
    void *cur;

    if (symbol_type == is_fsa())
        return true;

    if ((cur = parameter_list.first(&p)) != NULL)
    {
        do
        {
            if (p->symbol_type == PARM_HEADER)
            {
                Symbol *lp;
                void *cur_lp;

                if ((cur_lp = p->parameter_list.first(&lp)) != NULL)
                {
                    do
                    {
                        // If child is a node, then spit it out
                        if (lp->input_generator != (Symbol *) UP &&
                            (lp->input_generator == NULL ||
                             lp->input_generator->is_agent()))
                        {
                            return true;
                        }
                    }
                    while ((cur_lp = p->parameter_list.next(&lp, cur_lp)) != NULL);
                }
                else
                    return true;
            }
            else if (p->input_generator != (Symbol *) UP &&
                     (p->input_generator == NULL ||
                      p->input_generator->is_agent()))
            {
                // If child is a node, then spit it out
                return true;
            }
        }
        while ((cur = parameter_list.next(&p, cur)) != NULL);
    }
    return false;
}

// enqueue this nodes children
void Symbol::enqueue_children(SymbolList * queue)
{
    Symbol *p;
    void *cur;

    if (is_bp() && bound_to != NULL)
        queue->merge(bound_to);

    if ((cur = parameter_list.first(&p)) != NULL)
    {
        do
        {
            if (p->symbol_type == PARM_HEADER)
            {
                Symbol *lp;
                void *cur_lp;

                //dcm
                bool is_rule = p->data_type == expression_type;

                if ((cur_lp = p->parameter_list.first(&lp)) != NULL)
                {
                    do
                    {
                        // If child is a node, then spit it out
                        if (lp->input_generator && lp->input_generator->is_agent())
                        {
                            lp->input_generator->parent = this;
                            queue->merge(lp->input_generator);
                        }
                        //dcm
                        if (is_rule &&
                            lp->input_generator &&
                            lp->input_generator->input_generator &&
                            lp->input_generator->input_generator->is_agent())
                        {
                            lp->input_generator->input_generator->parent = this;
                            queue->merge(lp->input_generator->input_generator);
                        }
                    }
                    while ((cur_lp = p->parameter_list.next(&lp, cur_lp)) != NULL);
                }
            }
            else if (p->input_generator &&
                     p->input_generator != (Symbol *) UP &&
                     p->input_generator->is_agent())
            {
                // If child is a node, then spit it out
                p->input_generator->parent = this;
                queue->merge(p->input_generator);
            }
        }
        while ((cur = parameter_list.next(&p, cur)) != NULL);
    }

    // Add any unattached children in the "children" list
    if ((cur = children.first(&p)) != NULL)
    {
        do
        {
            if (p)
                queue->merge(p);
        }
        while ((cur = children.next(&p, cur)) != NULL);
    }
}

// enqueue this nodes children
void Symbol::enqueue_all_subs(SymbolList * queue)
{
    Symbol *p;
    void *cur;

    // Add the binding point
    if (symbol_type == SENSOR_NAME || symbol_type == ACTUATOR_NAME)
    {
        if (binds_to)
            queue->merge(binds_to);
    }
    if (is_bp())
    {
        if (bound_to)
            queue->merge(bound_to);
    }

    if (symbol_type == RULE_NAME)
    {
        if (input_generator)
            queue->merge(input_generator);
    }
    else if (symbol_type != INITIALIZER &&
             (cur = parameter_list.first(&p)) != NULL)
    {
        do
        {
            if (p->data_type)
                queue->merge(p->data_type);
            if (p->symbol_type == PARM_HEADER)
            {
                Symbol *lp;
                void *cur_lp;

                if ((cur_lp = p->parameter_list.first(&lp)) != NULL)
                {
                    do
                    {
                        // If child is a node, then spit it out
                        if (lp->input_generator != NULL &&
                            lp->input_generator != (Symbol *) UP &&
                            lp->input_generator->symbol_type != INITIALIZER)
                        {
                            lp->input_generator->parent = this;

                            if (lp->input_generator)
                                queue->merge(lp->input_generator);

                            if (lp->input_generator->defining_rec)
                            {
                                queue->merge(lp->input_generator->defining_rec);
                                if (lp->input_generator->defining_rec->data_type)
                                    queue->merge(lp->input_generator->defining_rec->data_type);
                            }
                        }
                        if (lp->list_index)
                        {
                            queue->merge(lp->list_index);
                        }
                    }
                    while ((cur_lp = p->parameter_list.next(&lp, cur_lp)) != NULL);
                }
            }
            else if (p->input_generator != NULL &&
                     p->input_generator != (Symbol *) UP &&
                     p->input_generator->symbol_type != INITIALIZER)
            {
                // If child is a node, then spit it out
                p->input_generator->parent = this;

                if (p->input_generator)
                    queue->merge(p->input_generator);

                if (p->input_generator->defining_rec)
                {
                    queue->merge(p->input_generator->defining_rec);
                    if (p->input_generator->defining_rec->data_type)
                        queue->merge(p->input_generator->defining_rec->data_type);
                }
            }
        }
        while ((cur = parameter_list.next(&p, cur)) != NULL);
    }

    // Add any unattached children in the "children" list
    if ((cur = children.first(&p)) != NULL)
    {
        do
        {
            if (p)
                queue->merge(p);
        }
        while ((cur = children.next(&p, cur)) != NULL);
    }
}

// Clear all marked flags from this node on down the tree
void Symbol::clear_marks(void)
{
    SymbolList queue;

    //cerr << "Clearing marks beginning at " << name << "\n";

    marked = false;
    enqueue_all_subs(&queue);

    while (!queue.isempty())
    {
        Symbol *cur = queue.get();

        //cerr << "Clearing mark in " << cur->name << "\n";

        cur->marked = false;
        cur->enqueue_all_subs(&queue);
    }
}

// Clear all is_instance flags from this node on down the tree
void Symbol::clear_instances(void)
{
    SymbolList queue;

    marked = false;
    enqueue_all_subs(&queue);

    while (!queue.isempty())
    {
        Symbol *cur = queue.get();

        cur->is_instance = false;
        cur->enqueue_all_subs(&queue);
    }
}

// Check if a symbol is an agent (Agent, Robot, Coord, etc.) and not a const.
bool Symbol::is_agent() const
{
    return this != NULL &&
        this != (Symbol *) UP &&
        (symbol_type == COORD_NAME ||
         symbol_type == ACTUATOR_NAME ||
         symbol_type == SENSOR_NAME ||
         symbol_type == AGENT_NAME ||
         symbol_type == GROUP_NAME ||
         symbol_type == ROBOT_NAME ||
         symbol_type == BP_NAME);
}

bool Symbol::is_fsa() const
{
    return (symbol_type == COORD_CLASS && operator_style == FSA_STYLE) ||
        (symbol_type == COORD_NAME && defining_rec != NULL && 
         defining_rec->operator_style == FSA_STYLE);
}

/* DCM 01-13-00: Added to support Reinforcement Learner */
bool Symbol::is_rl() const
{
    return (symbol_type == COORD_CLASS && operator_style == RL_STYLE) ||
        (symbol_type == COORD_NAME && defining_rec != NULL &&
         defining_rec->operator_style == RL_STYLE);
}

// Duplicate an agent
Symbol *Symbol::dup(const bool import, const char *newname) const
{
	Symbol *p = new Symbol;

    p->description = description ? strdup(description) : (char *)NULL;
    p->symbol_type = symbol_type;
    p->construction = construction;
    p->operator_style = operator_style;
    p->is_list = is_list;
    p->index_value = index_value;
    p->level = level;
    //   p->file = file;
	//   p->marked = marked;
    p->location = location;

    if(import)
    {
        // Force to user class
        p->record_class = RC_USER;

        // User records don't have a file
        p->source_file = NULL;
    }
    else
    {
        // Leave library class if it was.
        p->record_class = record_class;

        // the source filename string isn't duplicated.
        p->source_file = source_file;
    }


    // Duping doesn't inherit the screen widget
    p->this_screen_object = NULL;

    p->is_instance = is_instance;
    p->is_advanced_parm = is_advanced_parm;
    p->is_pair = is_pair;
    p->constant = constant;

	// Symbol pointers
    p->arch = arch;
    p->data_type = data_type;
    p->coord = coord;
    p->defining_rec = defining_rec;
    p->input_generator = input_generator;
    p->pair = pair;
    p->list_index = list_index;
    p->binds_to = binds_to;
    p->parm_list_head = parm_list_head;

	// tables
    assert(p->table.tablesize() == 0);
    //   p->table = table;

	// Lists

	// Dup the children list, then add this new user for each child
    p->children = children.dup();
    UseChildren(p);

	// Don't dup the users list, to difficult to figure out if that is good
	//      p->users = users.dup();

	// Duplicating the parameter_list requires care since it is multilevel
	Symbol *rec;
	void *cur;

	if ((cur = parameter_list.first(&rec)) != NULL)
	{
        do
        {
            Symbol *new_rec = rec->dup(import);

            // Parms should keep the same name and ID
            new_rec->name = rec->name;
            new_rec->id = rec->id;

            if (new_rec->symbol_type == PARM_HEADER)
            {
                // Dump the old list, it points to the old parms.
                new_rec->parameter_list.clear();

                Symbol *lp;
                void *pos;
                if ((pos = rec->parameter_list.first(&lp)) != NULL)
                {
                    do
                    {
                        Symbol *parm = lp->dup(import);
                        if (parm->input_generator != (Symbol *) UP &&
                            parm->input_generator != NULL &&
                            parm->input_generator->symbol_type == INITIALIZER)
                        {
                            // Constants should be dup'ed so can change them.
                            parm->input_generator = parm->input_generator->dup(import);
                        }

                        new_rec->parameter_list.append(parm);
                    }
                    while ((pos = rec->parameter_list.next(&lp, pos)) != NULL);
                }
            }
            else if (new_rec->input_generator != (Symbol *) UP &&
                     new_rec->input_generator != NULL &&
                     new_rec->input_generator->symbol_type == INITIALIZER)
            {
                // Constants should be dup'ed so can change them.
                new_rec->input_generator = new_rec->input_generator->dup(import);
            }
            p->parameter_list.append(new_rec);
        }
        while ((cur = parameter_list.next(&rec, cur)) != NULL);

        // Add this new user to each parm
        AddUser(&p->parameter_list, p);
	}

    p->parm_list_head = parm_list_head;

	// Constants should keep the same value (i.e., name)
	if (newname != NULL)
	{
        p->name = strdup(newname);
	}
	else if (p->symbol_type == INITIALIZER)
	{
        if(name)
            p->name = strdup(name);
        else
            p->name = NULL;
	}
	else
	{
        p->name = AnonymousName();
	}

	p->id = id;

	// If is a binding point, and is bound, dup the agent instance too.
	if (bound_to && symbol_type == BP_NAME && is_bound_bp())
	{
        // Make a new instance agent to bind to this new binding point
        p->bound_to = bound_to->dup(import);

        // Add the link back to this new bp from the instance
        p->bound_to->bound_to = p;
	}
	else
    {
        p->bound_to = NULL;
    }

	return p;
}


// Create and return an instance of an agent definition
// Does NOT define the new name in the symbol table
Symbol *Symbol::inst_of(const char *name) const
{
    // Define the new instance with an anonymous name
    // We duplicate to get the parameter definitions.
    Symbol *p = dup();

    switch (symbol_type) {

    case AGENT_CLASS:
	    p->symbol_type = AGENT_NAME;
	    p->construction = CS_CLASS;
	    break;

    case ACTUATOR_NAME:
	    p->symbol_type = AGENT_NAME;
	    p->construction = CS_OBP;
	    break;

    case SENSOR_NAME:
	    p->symbol_type = AGENT_NAME;
	    p->construction = CS_IBP;
	    break;

    case SENSOR_CLASS:
	    p->symbol_type = SENSOR_NAME;
	    p->construction = CS_IBP;
	    break;

    case ACTUATOR_CLASS:
	    p->symbol_type = ACTUATOR_NAME;
	    p->construction = CS_OBP;
	    break;

    case COORD_CLASS:
	    p->symbol_type = COORD_NAME;
	    if (p->operator_style == FSA_STYLE)
	    {
            // Need to define the rule_list_ pointer
            //               cerr << "Warning: inst_of with type FSA_STYLE: Not finished coding\n";
	    }
	    break;

    case ROBOT_CLASS:
	    p->symbol_type = ROBOT_NAME;
	    p->construction = CS_CLASS;
	    break;

    case BP_CLASS:
	    // the input name "bound_to" indicates bindings and is only
	    // used in file IO.  Delete this parm from the list so doesn't
	    // display and look stupid.
	    Symbol * parm;
	    void *cur;

	    if ((cur = p->parameter_list.first(&parm)) != NULL)
	    {
            do
            {
                if (strcmp(parm->name, "bound_to") == 0)
                {
                    // Remove it
                    p->parameter_list.remove(parm);

                    // Only will occur once.
                    break;
                }
            }
            while ((cur = p->parameter_list.next(&parm, cur)) != NULL);
	    }
	    p->symbol_type = BP_NAME;
	    break;

    default:
	    cerr << "Error: Incorrect definition to generate inst_of\n";
	    abort();
    }

	p->defining_rec = (Symbol *) this;

	if (name)
        p->name = (char *) name;
	else
        p->name = AnonymousName();

	// Mark as in this (possible different) class
	p->record_class = current_class;

	// Instantiate any constants
	Symbol *rec;
	void *cur;

	if ((cur = p->parameter_list.first(&rec)) != NULL)
	{
        do
        {
            // Mark as in this (possible different) class
            rec->record_class = current_class;

            // Promote parameters to inputs.
            if(rec->symbol_type == PARM_NAME)
                rec->symbol_type = INPUT_NAME;

            // Hang a blank initializer off any constants.
            if (rec->input_generator == NULL && rec->constant)
            {
                Symbol *inst = new Symbol(INITIALIZER);

                inst->name = strdup("");
                rec->input_generator = inst;
            }
        }
        while ((cur = p->parameter_list.next(&rec, cur)) != NULL);
	}

	return p;
}


// ---------------------------------------------------------------
Symbol *Symbol::first_child() const
{
   Symbol *p;
   if (children.first(&p))
      return p;

   return NULL;
}


// ---------------------------------------------------------------

// Check if a symbol is a binding point 
bool Symbol::is_bp() const
{
   return this && symbol_type == BP_NAME;
}

// ---------------------------------------------------------------

// Check if this is a hardware instance of a binding point 
bool
Symbol::is_bp_inst() const
{
	return bound_to &&
        (symbol_type == ROBOT_NAME ||
         (symbol_type == AGENT_NAME &&
          (construction == CS_IBP || construction == CS_OBP)));
}

// ---------------------------------------------------------------
// Check if is a robot
bool Symbol::is_robot() const
{
	return this && symbol_type == ROBOT_NAME;
}

// ---------------------------------------------------------------
// Check if is a robot binding point
bool Symbol::is_robot_bp() const
{
	return this && symbol_type == BP_NAME && construction == CS_RBP;
}

// ---------------------------------------------------------------
// If is a robot or contains one, return the robot
bool Symbol::has_robot(Symbol ** robot) const
{
	if (is_robot())
	{
        *robot = (Symbol *) this;
        return true;
	}

	// Check if one of the children is a robot
	Symbol *rec;
	void *cur;

	if ((cur = children.first(&rec)) != NULL)
	{
        do
        {
            if (rec->symbol_type == BP_NAME && rec->construction == CS_RBP &&
                rec->bound_to->is_robot())
            {
                *robot = rec->bound_to;
                return true;
            }
        }
        while ((cur = children.next(&rec, cur)) != NULL);
	}

	return false;
}

// ---------------------------------------------------------------

// Check if a symbol is a grouping operator
bool Symbol::is_grouping_op(void) const
{
	return symbol_type == GROUP_NAME ||
        symbol_type == ROBOT_NAME ||
        symbol_type == CONFIGURATION_NAME ||
        (symbol_type == BP_NAME && construction == CS_RBP) ||
        is_fsa();
}

// ---------------------------------------------------------------

// Check if a symbol could be a grouping operator
bool Symbol:: could_be_grouping_op(void) const
{
	if (this == (Symbol *) UP ||
	    is_grouping_op() ||
	    symbol_type == is_fsa())
	{
        return false;
	}

	if (symbol_type != COORD_NAME &&
	    symbol_type != AGENT_NAME)
	{
        return false;
	}

	Symbol *p;
	void *cur;

	if ((cur = parameter_list.first(&p)) != NULL)
	{
        do
        {
            if (p->symbol_type == PARM_HEADER)
            {
                Symbol *lp;
                void *cur_lp;

                if ((cur_lp = p->parameter_list.first(&lp)) != NULL)
                {
                    do
                    {
                        // If there is a non-constant input, then done
                        if (lp->input_generator == NULL ||
                            lp->input_generator->is_agent())
                        {
                            return true;
                        }
                    }
                    while ((cur_lp = p->parameter_list.next(&lp, cur_lp)) != NULL);
                }
                else
                    return true;
            }
            else if (p->input_generator != (Symbol *) UP &&
                     (p->input_generator == NULL ||
                      p->input_generator->is_agent()))
            {
                // If child is a node, then spit it out
                return true;
            }
        }
        while ((cur = parameter_list.next(&p, cur)) != NULL);
	}
	return false;
}


//********************************************************************

Symbol *Symbol::real_source(void) const
{
    // If this is a group name, skip it.  They are just containers
    if (this != NULL && symbol_type == GROUP_NAME)
    {
        Symbol *p = NULL;
        children.first(&p);

        return p->real_source();
    }
    // If this is a bound binding point, return the hardware record
    else if(is_bound_bp())
        return bound_to;

    return (Symbol *) this;
}


// ---------------------------------------------------------------

// Check if a symbol is a hardware device bound to a binding point
bool Symbol::is_hdw(void) const
{
    // DCM 01-10-00: Old test didn't work.
    return bound_to != NULL && bound_to->symbol_type == BP_NAME;
}


// ---------------------------------------------------------------


// delete the tree rooted at this agent using a depth first approach
// Also be careful to remove the names from the symbol tables
void Symbol::delete_tree(void)
{
    // Someone is still using us, so don't delete.
    if (users.len())
    {
        Symbol *rec;
        void *cur = users.first(&rec);

        while (cur)
            cur = users.next(&rec, cur);

        return;
    }

    // Don't delete library components.
    if(record_class == RC_LIBRARY)
        return;

    // Delete any children
    Symbol *rec;
    void *cur = children.first(&rec);

    while (cur)
    {
        // Remove us as a user so will kill the node if we are the only user.
        rec->users.remove(this);

        // try and delete it.
        rec->delete_tree();

        // Move to the next one.
        cur = children.next(&rec, cur);
    }

    // Delete any input parameters
    cur = parameter_list.first(&rec);
    while (cur)
    {
        if (rec->symbol_type == PARM_HEADER)
        {
            Symbol *lp;
            void *pos = rec->parameter_list.first(&lp);

            while (pos)
            {
                if (lp->input_generator != (Symbol *) UP &&
                    lp->input_generator != NULL)
                {
                    if (lp->input_generator->is_agent())
                    {
                        // Remove us as user so will kill node if we are only user.
                        lp->input_generator->users.remove(this);

                        // try and delete it.
                        lp->input_generator->delete_tree();
                    }
                    else if (lp->input_generator->symbol_type == RULE_NAME)
                    {
                        Symbol *group = lp->input_generator->input_generator;

                        // delete the rule record.
                        delete lp->input_generator;

                        if (group)
                        {
                            // Remove us as user so will kill node if we are only user.
                            group->users.remove(this);

                            // try and delete it.
                            group->delete_tree();
                        }
                    }
                }
                pos = rec->parameter_list.next(&lp, pos);
            }
        }
        else if (rec->input_generator != (Symbol *) UP &&
                 rec->input_generator != NULL &&
                 rec->input_generator->is_agent())
        {
            // Remove us as a user so will kill the node if we are only user.
            rec->input_generator->users.remove(this);

            // try and delete it.
            rec->input_generator->delete_tree();
        }

        // Check next parm
        cur = parameter_list.next(&rec, cur);
    }

    delete this;
}

// ---------------------------------------------------------------


// Duplicate the node and the entire tree hanging under it
Symbol *Symbol::dup_tree(const bool import, const char *newname) const
{
    // If we are not importing, then don't dup non-user components
    if(record_class != RC_USER && !import)
        return (Symbol *)this;

    Symbol *p = new Symbol;

    p->description = description ? strdup(description) : (char *)NULL;
    p->symbol_type = symbol_type;
    p->construction = construction;
    p->operator_style = operator_style;
    p->is_list = is_list;
    p->index_value = index_value;

    if(import)
    {
        // Force to user class
        p->record_class = RC_USER;

        // User records don't have a file
        p->source_file = NULL;
    }
    else
    {
        // Leave library class if it was.
        p->record_class = record_class;

        // the source filename string isn't duplicated.
        p->source_file = source_file;
    }

    p->level = level;
    p->location = location;

    // Duping doesn't inherit the screen widget
    p->this_screen_object = NULL;

    p->is_instance = is_instance;
    p->is_advanced_parm = is_advanced_parm;
    p->is_pair = is_pair;
    p->constant = constant;

    // Symbol pointers
    p->arch = arch;
    p->data_type = data_type;
    p->coord = coord;
    p->defining_rec = defining_rec;
    p->input_generator = input_generator ? input_generator->dup_tree(import) : (Symbol *)NULL;
    p->pair = pair;
    p->list_index = list_index ? list_index->dup_tree(import, list_index->name) : (Symbol *)NULL;
    p->binds_to = binds_to;
    p->parm_list_head = parm_list_head;

    // tables
    assert(p->table.tablesize() == 0);
    //   p->table = table;

	// Lists
    p->children.clear();   
              
    // recurse on each of our old unconnected children
    Symbol *rec;     
    void *cur;       
    if ((cur = children.first(&rec)) != NULL)
    {
        do      
        {             
            Symbol *new_child = rec->dup_tree(import);
            new_child->users.append(p);
            p->children.append(new_child);
        } while ((cur = children.next(&rec, cur)) != NULL);
    }             


    // Duplicating the parameter_list is pretty easy.
    // We will recurse on PARM_HEADER's and get the second level for free.
    // The input generators are duped above, so just build the parm list here.
    if ((cur = parameter_list.first(&rec)) != NULL)
    {
        do
        {
            // This will build a duplicate of the entire tree hanging off here.
            Symbol *new_rec = rec->dup_tree(import);

            // Parms should keep the same name
            new_rec->name = rec->name;

            // Stick it in the list
            p->parameter_list.append(new_rec);
        } while ((cur = parameter_list.next(&rec, cur)) != NULL);

        // Add this new user to each parm
        AddUser(&p->parameter_list, p);
    }


    // library components and Constants should keep their same value
    if(p->record_class == RC_LIBRARY || 
	    p->symbol_type == PU_INITIALIZER ||
	    p->symbol_type == INITIALIZER)
    {
        if(name)
        {
            p->name = strdup(name);
        }
        else
        {
            p->name = NULL;
        }
    }
    else if(newname)
    {
        p->name = strdup(newname);
    }
    else
    {
        p->name = AnonymousName();
    }

    // If is a binding point, and is bound, dup the agent instance too.
    if (bound_to && symbol_type == BP_NAME && is_bound_bp())
    {
        // Make a new instance agent to bind to this new binding point
        p->bound_to = bound_to->dup_tree(import);

        // Add the link back to this new bp from the instance
        p->bound_to->bound_to = p;
    }
    else
        p->bound_to = NULL;

    return p;
}

// ---------------------------------------------------------------


// move the node and the entire tree hanging under it into the specified library
void Symbol::move_tree_into_library(const char *lib_file, const bool debug)
{
    if(debug)
        cerr << "move_tree_into_library(" << name << ")\n";

    // Only move user level components
    if(record_class != RC_USER)
    {
        if(debug)
            cerr << "\twrong class\n";
        return;
    }

    // Force to library class
    record_class = RC_LIBRARY;

    // Update the library file
    source_file = lib_file;

    // recurse on each of our old unconnected children
    Symbol *rec;     
    void *cur;       
    if ((cur = children.first(&rec)) != NULL)
    {
        do      
        {             
            rec->move_tree_into_library(lib_file, debug);
        } while ((cur = children.next(&rec, cur)) != NULL);
    }             


    // moving the parameter_list is pretty easy.
    // We will recurse on PARM_HEADER's and get the second level for free.
    // The input generators are handled above, so just do the parm list here.
    if ((cur = parameter_list.first(&rec)) != NULL)
    {
        do
        {
            // This will move the entire tree hanging off here.
            rec->move_tree_into_library(lib_file, debug);
        } while ((cur = parameter_list.next(&rec, cur)) != NULL);
    }

    // We may be a parameter record, so check for input generator links
    if(input_generator && input_generator->is_agent())
    {
        input_generator->move_tree_into_library(lib_file, debug);
    }

    // If is a binding point, and is bound, dup the agent instance too.
    if (bound_to && symbol_type == BP_NAME && is_bound_bp())
    {
        // Make a new instance agent to bind to this new binding point
        bound_to->move_tree_into_library(lib_file, debug);
    }
}

// ---------------------------------------------------------------


// Add any parms that are defined as pushed up in the parent
void Symbol::pushup_parms(Symbol *parent)
{
    void *cur;
    Symbol *p;
    if((cur = parameter_list.first(&p)) != NULL)
    {
        do
        {
            if(p->name && p->name[0] == '%')
            {
                if(p->symbol_type == PARM_HEADER)
                {
                    Symbol *new_ph = p->dup(false,p->name);
                    parent->parameter_list.append(new_ph);

                    Symbol *lp;
                    void *cur_lp;
                    if ((cur_lp = new_ph->parameter_list.first(&lp)) != NULL)
                    {
                        do
                        {
                            // If is a pushed up parm
                            if (lp->symbol_type == PU_INPUT_NAME)
                            {
                                Symbol *inst;
      
                                if (lp->input_generator &&
                                    lp->input_generator->symbol_type == INITIALIZER)
                                {
                                    // Dup the default value
                                    inst = lp->input_generator->dup();
                                }
                                else
                                {
                                    // Build a data record for the value 
                                    inst = new Symbol(INITIALIZER);
                                    inst->name = NULL;
                                    inst->data_type = NULL;
                                }
                                lp->input_generator = inst;
                            }
                        }
                        while ((cur_lp = new_ph->parameter_list.next(&lp, cur_lp)) != NULL);
                    }
                }
                else
                {
                    // If is a pushed up parm
                    if (p->symbol_type == PU_INPUT_NAME)
                    {
                        Symbol *new_p = p->dup(false,p->name);
                        parent->parameter_list.append(new_p);

                        Symbol *inst;

                        if (p->input_generator &&
                            p->input_generator->symbol_type == INITIALIZER)
                        {
                            // Dup the default value
                            inst = p->input_generator->dup();
                        }
                        else
                        {
                            // Build a data record for the value 
                            inst = new Symbol(INITIALIZER);
                            inst->name = NULL;
                            inst->data_type = NULL;
                        }
                        new_p->input_generator = inst;
                    }
                }
            }
        } while((cur = parameter_list.next(&p,cur)) != NULL);
    }
}



// Reset the record class for the parms
void Symbol::ImportParmList(void)
{
   // Spin through the parameter list and reset the record class
   // for the INPUT_NAME and PARM_HEADER records to the current one.
   Symbol *rec;
   void *cur;

   if ((cur = parameter_list.first(&rec)) != NULL)
   {
      do
      {
         if (rec->symbol_type == PARM_HEADER)
	 {
            Symbol *lp;
	    void *pos;
	    if ((pos = rec->parameter_list.first(&lp)) != NULL)
	    {
	        do
		{
                   lp->record_class = current_class;
		} while ((pos = rec->parameter_list.next(&lp, pos)) != NULL);
            }
         }
         rec->record_class = current_class;
      } while ((cur = parameter_list.next(&rec, cur)) != NULL);

   }
}

//*******************************************************************
  // Imports this symbol, then all its parents and children. 
void 
Symbol::import_symbol()
{
    if(record_class == RC_USER)
        return;

    // Import us and our parameters.
    record_class = RC_USER;
    ImportParmList();

	// If we have a regular name, then we will now collide with the library.
	// So, change it to an anonymous one.
    if(name && name[0] != '$')
	{
        // Move our real name to the description field.
		// That is the one the user sees anyway.
        if(description)
            free(description);
        description = name;

        // Give us a new anonymous name.
        name = AnonymousName();
	}

    // Make sure our parents are also in user space.
	bool found_one;
    Symbol *parent;
    void *cur;
	do
	{
        found_one = false;
        cur = users.first(&parent);
        while (cur)
        {
            if(parent->record_class != RC_USER)
			{
                parent->import_symbol();
				found_one = true;

				// Need to restart, since changed the list.
				break;
			}

            cur = users.next(&parent, cur);
        }
	} while(found_one);

    // Make sure that any of our children and direct sources that we 
	// can't reference by name in the library are also in user space.

    // Import any direct connections.
    if(input_generator && 
       input_generator->name && 
       input_generator->name[0] == '$' && 
       input_generator->record_class != RC_USER)
    {
        input_generator->import_symbol();
    }
   
    // Import any children with anonymous names.
    Symbol *child;
	do
	{
        found_one = false;
        cur = children.first(&child);
        while (cur)
        {
            if(child->name &&
               child->name[0] == '$' && 
               child->record_class != RC_USER)
			{
                child->import_symbol();
				found_one = true;

				// Need to restart, since changed the list.
				break;
			}

            cur = children.next(&child, cur);
        }
	} while(found_one);

    // Import any sources with anonymous names.
	do
	{
        found_one = false;
        Symbol *p;
        if ((cur = parameter_list.first(&p)) != NULL)
        {
            do
            {
                if (p->symbol_type == PARM_HEADER)
                {
                    Symbol *lp;
                    void *cur_lp;

                    if ((cur_lp = p->parameter_list.first(&lp)) != NULL)
                    {
                        do
                        {
                            if(lp->input_generator &&
                               lp->input_generator->name && 
                               lp->input_generator->name[0] == '$' && 
						       lp->input_generator->record_class != RC_USER)
                            {
                                lp->input_generator->import_symbol();
                                found_one = true;
                                // Need to restart, since changed the list.
                                break;
                            }

                        } while ((cur_lp = p->parameter_list.next(&lp, cur_lp)) != NULL);
                    }
                }
                else
                {
                    if(p->input_generator &&
                       p->input_generator->name &&
                       p->input_generator->name[0] == '$' && 
                       p->input_generator->record_class != RC_USER)
                    {
                        p->input_generator->import_symbol();
                        found_one = true;

                        // Need to restart, since changed the list.
                        break;
                    }
                }
            } while(!found_one && ((cur=parameter_list.next(&p,cur)) != NULL));
        }
    } while(found_one);
}

//-----------------------------------------------------------------------
// This function searches the value of "data" for the "parameter".
//
// Note: some parameters such as the ones used inside state (e.g. gain)
//       may not be found.
//-----------------------------------------------------------------------
string Symbol::findData(string parm)
{
    Symbol *node, *p;
    void *cur;
    string name, desc = EMPTY_STRING, data = EMPTY_STRING;

    node = this;

    if ((cur = node->parameter_list.first(&p)) != NULL)
    {
        do
        {
            if (p == NULL)
            {
                continue;
            }

            name = p->name;

            if (name == parm)
            {
                data = p->input_generator->name;
                return data;
            }
        }
        while ((cur = node->parameter_list.next(&p, cur)) != NULL);
    }	    

    if ((cur = node->children.first(&p)) != NULL)
    {
        do
        {
            if (p == NULL)
            {
                continue;
            }

            data = p->findData(parm);

            if (data != EMPTY_STRING)
            {
                return data;
            }

            if (p->data_type != NULL)
            {
                data = p->data_type->findData(parm);

                if (data != EMPTY_STRING)
                {
                    return data;
                }
            }

            if (p->input_generator != NULL)
            {
                data = p->input_generator->findData(parm);

                if (data != EMPTY_STRING)
                {
                    return data;
                }
            }

            if (p->list_index != NULL)
            {
                data = p->list_index->findData(parm);

                if (data != EMPTY_STRING)
                {
                    return data;
                }
            }

            if (p->binds_to != NULL)
            {
                data = p->binds_to->findData(parm);

                if (data != EMPTY_STRING)
                {
                    return data;
                }
            }

            if (p->bound_to != NULL)
            {
                data = p->bound_to->findData(parm);

                if (data != EMPTY_STRING)
                {
                    return data;
                }
            }
        }
        while ((cur = node->children.next(&p, cur)) != NULL);
    }	    

    return data;
}

//-----------------------------------------------------------------------
// This function searches the value of "data" for the "parameter".
// 
// Note: some parameters such as the ones used inside state (e.g. gain)
//       may not be found.
//-----------------------------------------------------------------------
vector<string> Symbol::findDataList(string parm)
{
    Symbol *node, *p;
    void *cur;
    string name, desc = EMPTY_STRING, data = EMPTY_STRING;
    vector<string> dataList, tmpDataList;
    int i;

    node = this;

    if ((cur = node->parameter_list.first(&p)) != NULL)
    {
        do
        {
            if (p == NULL)
            {
                continue;
            }

            name = p->name;

            if (name == parm)
            {
                data = p->input_generator->name;
                dataList.push_back(data);
            }
        }
        while ((cur = node->parameter_list.next(&p, cur)) != NULL);
    }	    

    if ((cur = node->children.first(&p)) != NULL)
    {
        do
        {
            if (p == NULL)
            {
                continue;
            }

            tmpDataList = p->findDataList(parm);

            for (i = 0; i < (int)(tmpDataList.size()); i++)
            {
                dataList.push_back(tmpDataList[i]);
            }

            if (p->data_type != NULL)
            {
                tmpDataList = p->data_type->findDataList(parm);

                for (i = 0; i < (int)(tmpDataList.size()); i++)
                {
                    dataList.push_back(tmpDataList[i]);
                }
            }

            if (p->input_generator != NULL)
            {
                tmpDataList = p->input_generator->findDataList(parm);

                for (i = 0; i < (int)(tmpDataList.size()); i++)
                {
                    dataList.push_back(tmpDataList[i]);
                }
            }

            if (p->list_index != NULL)
            {
                tmpDataList = p->list_index->findDataList(parm);

                for (i = 0; i < (int)(tmpDataList.size()); i++)
                {
                    dataList.push_back(tmpDataList[i]);
                }
            }

            if (p->binds_to != NULL)
            {
                tmpDataList = p->binds_to->findDataList(parm);

                for (i = 0; i < (int)(tmpDataList.size()); i++)
                {
                    dataList.push_back(tmpDataList[i]);
                }
            }

            if (p->bound_to != NULL)
            {
                tmpDataList = p->bound_to->findDataList(parm);

                for (i = 0; i < (int)(tmpDataList.size()); i++)
                {
                    dataList.push_back(tmpDataList[i]);
                }
            }
        }
        while ((cur = node->children.next(&p, cur)) != NULL);
    }	    

    return dataList;
}

///////////////////////////////////////////////////////////////////////
// $Log: symbol.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:22  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.3  2006/12/05 01:47:02  endo
// cdl parser now accepts {& a b} and Symbol *pair added.
//
// Revision 1.2  2006/08/29 15:13:57  endo
// Advanced parameter flag added to symbol.
//
// Revision 1.1.1.1  2006/07/20 17:17:50  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:59  endo
// MissionLab 7.0
//
// Revision 1.3  2006/05/14 07:38:31  endo
// gcc-3.4 upgrade
//
// Revision 1.2  2005/02/07 23:56:26  endo
// Mods from usability-2004
//
// Revision 1.1.1.1  2005/02/06 23:00:07  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.69  2003/04/06 09:12:37  endo
// Updated for CBR Wizard Prototype II.
//
// Revision 1.68  2001/09/24 21:18:20  ebeowulf
// Added RLstyle
//
// Revision 1.67  2001/02/01 20:09:11  endo
// id was added to the Symbol class.
//
// Revision 1.66  2000/04/20 03:25:06  endo
// Checked in for Doug.
// This patch fixes some problems with the cfgedit import code which
// caused it to core dump.  It also corrects a problem where the imported
// assemblage would still have the same name as the library assemblage,
// causing conflicts.  It now moves the name to the description field
// and assigns a unique name to the imported assemblage.
//
// Revision 1.65  2000/04/13 22:06:12  endo
// Checked in for Doug.
// This patch extends MissionLab to allow the user to import read-only
// library code.  When the user attempts to change something which is
// read-only, cfgedit pops up a dialog box and asks if it should import
// the object so it can be edited.  If OK'd, it imports the object.
//
// This fixes the problem with FSA's (and other assemblages) not being
// editable when they are loaded from the library.
//
// Revision 1.64  2000/01/22 20:56:16  endo
// Code checked in for doug. libload_cdl.a was modified
// so that a bound object can be delted (in CfgEdit)
// by using "Cut". Note that some objects, such as
// actuators, still cannot be deleted even with this
// modification.
//
// Revision 1.63  1999/12/16 22:29:19  endo
// rh-6.0 porting.
//
// Revision 1.62  1996/10/04 21:10:59  doug
// changes to get to version 1.0c
//
// Revision 1.63  1996/10/02 21:45:53  doug
// working on pushup in states and transitions
//
// Revision 1.62  1996/10/01 13:01:32  doug
// got library writes working
//
// Revision 1.61  1996/06/17 22:58:49  doug
// *** empty log message ***
//
// Revision 1.60  1996/06/13  15:23:59  doug
// fixing problems with record_class not getting correctly set
//
// Revision 1.59  1996/06/02  20:28:16  doug
// Changed dup_tree so it no longer duplicates nodes that are
// not at the user level unless the import flag is set.
// This vastly speeds up the duplication process, but may
// cause problems if people edit library components.  In effect,
// they are all linked now.  Need to dup components as people edit them.
//
// Revision 1.58  1996/06/01  21:53:40  doug
// *** empty log message ***
//
// Revision 1.57  1996/05/05  21:45:27  doug
// fixing compile warnings
//
// Revision 1.56  1996/05/02  22:58:45  doug
// *** empty log message ***
//
// Revision 1.55  1996/02/28  03:57:42  doug
// fixing dup_tree
//
// Revision 1.54  1996/02/26  05:02:34  doug
// *** empty log message ***
//
// Revision 1.53  1996/02/25  01:13:50  doug
// *** empty log message ***
//
// Revision 1.52  1996/02/22  00:56:01  doug
// *** empty log message ***
//
// Revision 1.51  1996/02/19  21:57:42  doug
// library components and permissions now work
//
// Revision 1.50  1996/02/18  23:56:36  doug
// allow default initializers in inst_of
//
// Revision 1.49  1996/02/18  23:31:42  doug
// *** empty log message ***
//
// Revision 1.48  1996/02/18  00:03:53  doug
// *** empty log message ***
//
// Revision 1.47  1996/02/15  19:30:32  doug
// fixed pushed up parms in groups
//
// Revision 1.46  1996/02/08  19:22:31  doug
// *** empty log message ***
//
// Revision 1.45  1996/02/07  17:51:35  doug
// *** empty log message ***
//
// Revision 1.44  1996/02/04  23:22:25  doug
// *** empty log message ***
//
// Revision 1.43  1996/02/02  03:00:50  doug
// *** empty log message ***
//
// Revision 1.42  1996/02/01  19:47:04  doug
// *** empty log message ***
//
// Revision 1.41  1996/01/27  00:10:14  doug
// added grouped lists support to parser
//
// Revision 1.40  1996/01/21  18:49:57  doug
// remove a debug print in delete_Tree
//
// Revision 1.39  1996/01/20  23:41:31  doug
// added is_hdw() boolean to check if a record is a hardware instance
// of a binding point.
//
// Revision 1.38  1996/01/17  18:46:06  doug
// *** empty log message ***
//
// Revision 1.37  1995/12/14  21:54:03  doug
// *** empty log message ***
//
// Revision 1.36  1995/12/05  17:25:55  doug
// *** empty log message ***
//
// Revision 1.35  1995/12/01  21:16:50  doug
// *** empty log message ***
//
// Revision 1.34  1995/11/30  23:30:28  doug
// *** empty log message ***
//
// Revision 1.33  1995/11/29  23:12:30  doug
// *** empty log message ***
//
// Revision 1.32  1995/11/21  23:09:22  doug
// *** empty log message ***
//
// Revision 1.31  1995/11/12  22:37:48  doug
// *** empty log message ***
//
// Revision 1.30  1995/11/07  14:29:58  doug
// *** empty log message ***
//
// Revision 1.29  1995/10/31  19:23:06  doug
// add real_source call to move beyond group_name records
//
// Revision 1.28  1995/10/27  20:29:18  doug
// *** empty log message ***
//
// Revision 1.27  1995/10/10  20:41:12  doug
// *** empty log message ***
//
// Revision 1.26  1995/10/06  21:46:12  doug
// added unbound binding points as an agent
//
// Revision 1.25  1995/09/26  21:37:32  doug
// fix assignment of symbols
//
// Revision 1.24  1995/09/19  15:31:53  doug
// change so all parms defined in the def record exist in any instances
//
// Revision 1.23  1995/09/07  14:23:14  doug
// works
//
// Revision 1.22  1995/06/29  14:50:24  jmc
// Added RCS log string.
///////////////////////////////////////////////////////////////////////
