#ifndef SYMBOL_HPP
#define SYMBOL_HPP

/**********************************************************************  
 **                                                                  **  
 **                            symbol.hpp                            **  
 **                                                                  **  
 **  symbol class - the internal representation of CDL is in symbols **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 1997, 1999 - 2003 Georgia Tech Research        **
 **  Corporation Atlanta, Georgia  30332-0415                        **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **  
 **********************************************************************/ 

/* $Id: symbol.hpp,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "symbol_table.hpp"
#include "list.hpp"

using std::ostream;
using std::ofstream;
using std::string;
using std::vector;

#include "symbol_table.hpp"
#include "list.hpp"

// Additional symbol_types
const int ST_NONE = 0;
const int COORD_AGENT_NAME = 1;
const int LIBRARY_HEADER = 2;
const int LIBRARY_RECORD = 3;

const int UNDEFINED_INDEX = -1;


typedef struct Location {
   unsigned short x;
   unsigned short y;

   bool is_default()  {return (x==10 && y==10) || (x==0 && y==0);}
   void set_default() {x = 0; y = 0;}
} Location;

typedef GTList < class Symbol * >SymbolList;
typedef symbol_table < class Symbol > SymbolTable;
typedef enum CS_NAMES
{
   CS_SOCIETY, CS_CLASS, CS_SOFT, CS_IBP, CS_OBP, CS_RBP
}
CS_NAMES;

typedef enum RECORD_CLASS
{
   RC_SYSTEM, RC_LIBRARY, RC_USER
}
RECORD_CLASS;

/* symbol record */
class Symbol
{
public:
    // symbol_types are the yacc tokens as well as others defined above.
    int symbol_type;

    // The construction style of the agent.
    // Only valid when symbol_type is AGENT_NAME.
    // CS_SOCIETY: Consists of a coord and a list of agents
    //             coord points to the coord agent.
    // CS_CLASS: Is an instance of a class def
    //             defining_rec points to the class def
    // CS_SOFT: Implementation is in a seperate file
    //             source_file is the filename
    CS_NAMES construction;

    // The name of this symbol
    char *name;

    // ID of this symbol
    int id;

    // The text description of this agent for displaying to the user
    char *description;

    // Only valid when symbol_table is COORD_CLASS
    //   Values: FSA_STYLE, RL_STYLE, SELECT_STYLE, MDL_STYLE
    int operator_style;

    // When the symbol defines a page (symbol_type is AGENT_NAME, ROBOT_NAME, 
    // COORD_NAME, GROUP_NAME, or CONFIGURATION_NAME) then this is the list of 
    // of other symbols on the page.
    SymbolList children;

    // Pointer to the architecture record defining which architecture
    // this symbol is valid in.  
    // Only valid when symbol_type is CLASS_NAME.
    Symbol *arch;

    // Pointer to the symbol defining the data type of this symbol
    // When symbol_type is CLASS_NAME, this is the output type.
    // When symbol_type is PARM_NAME, this is the input type.
    Symbol *data_type;

    // Pointer to the type info record for this symbol.
    // This corresponds to the data_type field above and should replace it
    // eventually.
    class TypeInfo *TypeRecord;

    // Boolean, is this parm a list?
    // Only valid when symbol_type is PARM_NAME.
    int is_list;

    // When symbol_type is AGENT_CLASS or COORD_CLASS, this is the list 
    // of symbols which are the input parameters for this symbol.
    // When symbol_type is INPUT_NAME and is_list is TRUE, this is the 
    // list of assignments to the parm.
    SymbolList parameter_list;

    // Pointer to the coordination agent
    // Only valid when construction is CS_SOCIETY.
    Symbol *coord;

    // Pointer to the head of the related list (if not NULL)
    Symbol *parm_list_head;

    // When symbol_type is AGENT_NAME and construction is CS_CLASS, 
    //       points to the defining class.
    // When symbol_type is COORD_NAME, points to the defining coord.
    Symbol *defining_rec;

    // The filename where the agent is implemented
    // Only valid when construction is CS_SOFT
    const char *source_file;

    // Pointer to the symbol_table scoped local to this record.
    // Only valid when symbol_type is CLASS_NAME.
    SymbolTable table;

    // The text of the constant input
    // Only valid if symbol_type is INPUT_NAME or IDENTIFIER
    Symbol *input_generator;

    // For INPUT_NAME (%input_name) that has a pair symbol.
    Symbol *pair;

    // The flag to tell whether it is pair symbold or not.
    bool is_pair;

    // if symbol_type is INPUT_NAME is the symbol record defining the 
    //    named index for this list instance
    // if symbol_type is RULE_NAME is the named index for the goto dest.
    Symbol *list_index;

    // if symbol_type is ACTUATOR_NAME or SENSOR_NAME, this is the binding
    //    point that the hardware binds to.
    Symbol *binds_to;

    // if symbol_type->defining_rec->symbol_type is BP_CLASS this is the 
    // sensor or actuator that is bound to the binding point.
    Symbol *bound_to;

    // if type is INDEX_NAME, then this is the index value.
    //     unless it has the value UNDEFINED_INDEX when it is undefined.
    // if type is PARM_HEADER, then this is the next index value to assign.
    int index_value;

    // Indicates where this record came from.
    // Used in the code generators to keep from generating code
    // for builtin and library functions
    RECORD_CLASS record_class;

    // Used in print_symbols to select the indentation of the symbol
    // Two spaces are added for each level.
    int level;

    // If true, then input only accepts constants
    bool constant;

    // Used in the code generators to track the specific parent of a node
    Symbol *parent;

    // Used to track all the consumers of this node's output
    SymbolList users;

    // Used in the code generators to remember the output files
    ofstream *file;

    // Used in the code generators to mark nodes transcending
    // robots as to which robot will execute the code.
    Symbol *robot;

    // Used in the code generators to mark nodes when they are generated
    int marked;

    // Used in the gui to keep track of where on the screen the glyph
    // should be rendered.
    Location location;

    // Used in the gui to point to the glyph record which wraps this record
    void *this_screen_object;

    // Used in the code generators to keep track of if this node was generated
    // as an INSTANCE or not.  Only valid when marked is true.
    bool is_instance;

    // Used to mark that the parameter is advanced. The value will be displayed
    // only when the user presses the advanced button.
    bool is_advanced_parm;

    // Construct the record.
    Symbol(int type = ST_NONE);
    ~Symbol();
    Symbol(const Symbol & src);
    Symbol& operator= (const Symbol & src);

    // check if this node has children
    bool has_children();

    // enqueue this nodes children
    void enqueue_children(SymbolList * queue);

    // enqueue this nodes children
    void enqueue_all_subs(SymbolList * queue);

    // Clear all marked flags from this node on down the tree
    void clear_marks(void);

    // Clear all is_instance flags from this node on down the tree
    void clear_instances(void);

    // Check if a symbol is an agent (Agent, Robot, Coord, etc.) and not a const.
    bool is_agent() const;

    bool is_fsa() const;
    bool is_rl() const;
    Symbol *dup(const bool import = false, const char *newname = NULL) const;
    Symbol *dup_tree(const bool import, const char *newname = NULL) const;

    // move the node and entire tree hanging under it into the specified library
    void move_tree_into_library(const char *lib_file, const bool debug);

    // Create and return instance of an agent definition
    Symbol *inst_of(const char *name = NULL) const;

    // Return the first child of a group_name or NULL
    Symbol *first_child() const;

    // Check if a symbol is a binding point 
    bool is_bp() const;

    // Check if a binding point is bound
    bool is_bound_bp() const {return is_bp() && bound_to != NULL; }

    // Check if this is a hardware instance of a binding point
    bool is_bp_inst() const;

    // Check if a symbol is a robot record
    bool is_robot() const;

    // Check if is a robot binding point
    bool is_robot_bp() const;

    // Check if a page header contains a robot record and return it.
    bool has_robot(Symbol **robot) const;

    // Check if a symbol is a grouping operator 
    bool is_grouping_op() const;

    // Check if a symbol could be a grouping operator 
    bool could_be_grouping_op() const;

    // Check if a symbol is a hardware device bound to a binding point
    bool is_hdw() const;

    // Delete this node and any attached subtree 
    void delete_tree();

    // Skips past group names to return the real generator
    Symbol *real_source() const;

    // Copies any pushed up parms in this node up to the parent.
    void pushup_parms(Symbol *parent);

    // Resets the record class on the parms
    void ImportParmList(void);

    // Imports this symbol, then all its parents and children.
    void import_symbol();

    // This function searches the value of "data" for the "parameter".
    string findData(string parm);
    vector<string> findDataList(string parm);

    friend ostream & operator << (ostream & s, Symbol * d);
};


///////////////////////////////////////////////////////////////////////
// $Log: symbol.hpp,v $
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
// Revision 1.1.1.1  2005/02/06 23:00:07  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.41  2003/04/06 12:58:48  endo
// gcc 3.1.1
//
// Revision 1.40  2003/04/06 09:12:37  endo
// Updated for CBR Wizard Prototype II.
//
// Revision 1.39  2002/01/13 01:24:31  endo
// list ->GTList
//
// Revision 1.38  2001/09/24 21:18:20  ebeowulf
// Added RLstyle
//
// Revision 1.37  2001/02/01 20:09:11  endo
// id was added to the Symbol class.
//
// Revision 1.36  2000/04/13 22:07:24  endo
// This patch extends MissionLab to allow the user to import read-only
// library code.  When the user attempts to change something which is
// read-only, cfgedit pops up a dialog box and asks if it should import
// the object so it can be edited.  If OK'd, it imports the object.
//
// This fixes the problem with FSA's (and other assemblages) not being
// editable when they are loaded from the library.
//
// Revision 1.35  1996/10/04  21:10:59  doug
// changes to get to version 1.0c
//
// Revision 1.36  1996/10/02 21:45:53  doug
// working on pushup in states and transitions
//
// Revision 1.35  1996/10/01 13:01:32  doug
// got library writes working
//
// Revision 1.34  1996/06/13 15:23:59  doug
// fixing problems with record_class not getting correctly set
//
// Revision 1.33  1996/06/01  21:53:40  doug
// *** empty log message ***
//
// Revision 1.32  1996/05/05  21:45:27  doug
// fixing compile warnings
//
// Revision 1.31  1996/04/06  23:54:20  doug
// *** empty log message ***
//
// Revision 1.30  1996/02/28  03:57:42  doug
// fixing dup_tree
//
// Revision 1.29  1996/02/25  01:13:50  doug
// *** empty log message ***
//
// Revision 1.28  1996/02/22  00:56:01  doug
// *** empty log message ***
//
// Revision 1.27  1996/02/19  21:57:42  doug
// library components and permissions now work
//
// Revision 1.26  1996/02/15  21:00:00  doug
// added set_default to the location class
//
// Revision 1.25  1996/02/01  19:47:04  doug
// *** empty log message ***
//
// Revision 1.24  1996/01/27  00:10:14  doug
// added grouped lists support to parser
//
// Revision 1.23  1996/01/20  23:41:31  doug
// added is_hdw() boolean to check if a record is a hardware instance
// of a binding point.
//
// Revision 1.22  1996/01/17  18:46:06  doug
// *** empty log message ***
//
// Revision 1.21  1995/12/01  21:16:50  doug
// *** empty log message ***
//
// Revision 1.20  1995/11/30  23:30:28  doug
// *** empty log message ***
//
// Revision 1.19  1995/11/12  22:37:48  doug
// *** empty log message ***
//
// Revision 1.18  1995/10/31  19:23:06  doug
// add real_source call to move beyond group_name records
//
// Revision 1.17  1995/10/27  20:29:18  doug
// *** empty log message ***
//
// Revision 1.16  1995/10/06  22:12:20  doug
// is_bound_bp didn't reference is_bp as a function so didn't work
//
// Revision 1.15  1995/10/06  21:46:12  doug
// *** empty log message ***
//
// Revision 1.14  1995/09/26  21:37:32  doug
// add is_grouping_op boolean
//
// Revision 1.13  1995/09/19  15:31:53  doug
// change so all parms defined in the def record exist in any instances
//
// Revision 1.12  1995/09/07  14:23:14  doug
// works
//
// Revision 1.11  1995/07/06  19:29:15  doug
// Add a method to the location struct to check if it is the default values
//
// Revision 1.10  1995/06/29  14:44:38  jmc
// Added RCS log string.
///////////////////////////////////////////////////////////////////////

#endif
