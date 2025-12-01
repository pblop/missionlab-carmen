/**********************************************************************
 **                                                                  **
 **                        configuration.hpp                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 1997, 1999 - 2003 Georgia Tech Research        **
 **  Corporation Atlanta, Georgia  30332-0415                        **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: configuration.hpp,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <Xm/Xm.h>
#include <vector>
#include <string>

#include "symtab.h"
#include "symbol.hpp"

class circle;

typedef struct config_bahavior_name_t
{
    string name;
    string description;
};

typedef struct config_fsa_lists_t
{
    vector<config_bahavior_name_t> taskList;
    vector<config_bahavior_name_t> triggerList;
};

// Defines the configuration class

/************************************************************************
*			configuration class            				*
*************************************************************************/
class configuration
{
   class Page   *cur_page;	// pointer to the currently visible page
   char         *configname;
   char         *filename;
   bool          needs_write;
   GTList<Symbol *> traversal_links;
   Symbol	*root_;
   Symbol	*robot_;	// If are inside a robot, the ptr to record
   Symbol	*arch_;		// Current architecture (&free_arch, etc).
   XmString     *behavior_names_[NUM_GROUPS];
   int           num_behaviors_[NUM_GROUPS];
   XmString     *arch_names_;
   int           num_arches_;

   static const string EMPTY_STRING_;   

   // Create the behavior lists
   GTList < char *>list_of_names[NUM_GROUPS];
   vector<config_bahavior_name_t> get_name_list_(int kind);

   void        	*parent_page_loc_;

   void delete_configuration();
 
   string findStateInNode_(
       string fsaName,
       string stateName,
       SymbolList *pageStack,
       Symbol *node,
       Symbol **location);

   string findStateInPage_(
       string fsaName,
       string stateName,
       SymbolList *pageStack,
       Symbol *pageDef,
       Symbol *index,
       Symbol **location);

   string findStateInFSA_(
       string fsaName,
       string stateName,
       SymbolList *pageStack,
       Symbol *node,
       Symbol **location);

public:  //***********************************************************

   // constructor: create objects
   configuration();

   // destructor
   ~configuration();

   void new_design();
   void move_up();
   void move_down(Symbol *rec, bool open_sub_page, Symbol *prestuff = NULL);
   void goto_page(SymbolList *page_stack, Symbol *rec);
   bool load(const char *name, const bool reset_filename = true);
   bool save(const char *name);
   const char *configuration_name() const {return configname;}
   const char *configuration_filename() const {return filename;}
   bool needs_save() const {return needs_write;}
   bool is_empty() const {return cur_page == NULL;}
   void did_save();
   void made_change();
   Page *this_page() {return cur_page;}
   Symbol *root() {return root_;}
   XmString *behavior_names(int kind) const { return behavior_names_[kind]; }
   Symbol *behavior_by_pos(int kind, int pos) const;
   int num_behaviors(int kind) const { return num_behaviors_[kind]; }
   Symbol *find_agent_by_name(char *name) const;
   vector<config_bahavior_name_t> get_task_list(void);
   vector<config_bahavior_name_t> get_trigger_list(void);
   int pos_by_name(int kind, char *name) const;
   char *kind2name(int kind) const { return (char *)(kind==PG_PRIMS?"Primitives":
						     kind==PG_AGENTS?"Assemblages":
						     "Items");}
   void make_empty_config();
   void create_name_lists();
   void create_arch_lists();
   bool bind_arch(char *archname);
   bool unbind_arch();
   XmString *arch_names() const {return arch_names_;}
   int num_arches() const {return num_arches_;}
   void regen_page(Symbol *new_page_def = NULL);
   bool at_top() const {return traversal_links.isempty();}
   bool arch_is_bound() const {return choosen_architecture != free_arch;}
   bool arch_is_AuRA() const {return choosen_architecture == AuRA_arch;}
   bool arch_is_AuRA_urban() const {return choosen_architecture == AuRA_urban_arch;}
   bool arch_is_AuRA_naval() const {return choosen_architecture == AuRA_naval_arch;}
   bool arch_is_UGV() const {return choosen_architecture == UGV_arch;}
   Symbol *this_robot() const { return robot_; }
   bool are_in_robot() const { return robot_ != NULL; }
   void update_menu_bar();
   void reset_slider_bars(void);
   void replace_traversal_page(Symbol *old_rec, Symbol *new_rec);

   Symbol *parent_page();
   Symbol *next_parent_page();

   // Add a new toplevel page into the traversal stack.
   void add_new_top_page(Symbol * new_page_rec);

   // Sets a new name for the configuration
   void rename(const char *name);

   // Start over new configuration
   void start_over(void);

   // Quit the program.
   void quit(void);

   // This page (level) is read-only
   bool is_read_only_page(void);

   string highlightState(string fsaName, string stateName);
   void clearHighlightStates(void);

   bool currentPageIsFSA(void);
};

inline vector<config_bahavior_name_t> configuration::get_task_list(void)
{
    return get_name_list_(PG_AGENTS);
}

inline vector<config_bahavior_name_t> configuration::get_trigger_list(void)
{
    return get_name_list_(PG_TRIGS);
}

extern configuration *config;

/************************************************************************/

#endif


///////////////////////////////////////////////////////////////////////
// $Log: configuration.hpp,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.4  2006/05/15 01:23:28  endo
// gcc-3.4 upgrade
//
// Revision 1.3  2005/05/18 21:14:43  endo
// AuRA.naval added.
//
// Revision 1.2  2005/02/07 22:25:27  endo
// Mods for usability-2004
//
// Revision 1.1.1.1  2005/02/06 22:59:32  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.35  2003/04/06 09:00:07  endo
// Updated for CBR Wizard Prototype II.
//
// Revision 1.34  2002/01/12 23:23:16  endo
// Mission Expert functionality added.
//
// Revision 1.33  2000/10/16 19:33:40  endo
// Modified due to the compiler upgrade.
//
// Revision 1.32  2000/03/13 23:59:40  endo
// The "Start Over" button was added to CfgEdit.
//
// Revision 1.31  1999/09/03 19:46:19  endo
// arch_is_AuRA_urban stuff added.
//
// Revision 1.30  1997/02/14 15:55:46  zchen
// *** empty log message ***
//
// Revision 1.29  1996/03/06  23:39:17  doug
// *** empty log message ***
//
// Revision 1.28  1996/02/22  00:55:17  doug
// adding popups to fsa states/trans
//
// Revision 1.27  1996/02/16  00:07:18  doug
// *** empty log message ***
//
// Revision 1.26  1996/01/30  01:50:19  doug
// *** empty log message ***
//
// Revision 1.26  1996/01/30  01:50:19  doug
// *** empty log message ***
//
// Revision 1.25  1996/01/29  00:07:50  doug
// *** empty log message ***
//
// Revision 1.24  1996/01/19  20:54:42  doug
// *** empty log message ***
//
// Revision 1.23  1996/01/17  18:47:01  doug
// *** empty log message ***
//
// Revision 1.22  1995/11/30  23:31:07  doug
// *** empty log message ***
//
// Revision 1.21  1995/11/21  23:10:08  doug
// *** empty log message ***
//
// Revision 1.20  1995/11/07  14:30:36  doug
// *** empty log message ***
//
// Revision 1.19  1995/11/04  23:46:50  doug
// *** empty log message ***
//
// Revision 1.18  1995/09/07  14:22:26  doug
// works
//
// Revision 1.17  1995/09/01  21:14:27  doug
// linking and copying glyphs works across pages
//
// Revision 1.16  1995/06/29  17:33:03  jmc
// Added header and RCS id and log strings.
///////////////////////////////////////////////////////////////////////
