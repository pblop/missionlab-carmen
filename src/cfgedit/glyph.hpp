/**********************************************************************
 **                                                                  **
 **                             glyph.hpp                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: glyph.hpp,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#ifndef GLYPH_HPP
#define GLYPH_HPP

const bool ERASE = true;
const bool DRAW = false;

/************************************************************************
*			glyph class            				*
*************************************************************************/
class glyph: public screen_object
{
   Symbol      *sym_rec;	  // Pointer to symbol record implementing
				  // this glyph.
   Location    *location_;        // Pointer to the struct holding this
				  // glyph's screen x,y
   Widget       _glyph_widget;	  // widget for this glyph
   Widget      *body_widget_;	  // widgets for the column forms
   Widget       name_widget_;	  // name text widget for lookups
   Widget       output_widget;	  // name text widget for lookups
   bool         is_detailed;	  // Is this a detailed view?
   bool         has_down_button_; // Does glyph have the down button?
   Widget       down_button_;	  // widget for the down button (if not NULL).
   bool         managed_;	  // Is glyph managed?
   bool         lifted_;	  // OK to draw glyph?

   // slot info
   int num_slots;
   int num_cols;
   slot_record *slot_info; // Array with num_slots entries

   // Array of default translations for the widgets, so we can restore if 
   // not handling events
   XtTranslations *default_translations_;
   int             num_translations_;

public:  //***********************************************************

   // Support required by screen_object
   void redraw_unselected();
   void redraw_selected();
   void *base_rec() {return this;}
   void delete_tree();
   void draw();
   void erase();
   void xor_outline(Position x, Position y);
   void get_xy(Position *x, Position *y) const;
   void get_output_xy(Position *x, Position *y) const;
   void set_xy(Position x, Position y);
   void draw_links()  {draw_links(false);}
   void erase_links() {draw_links(true);}
   Widget object_widget() const {return _glyph_widget;}
   Symbol *get_src_sym() const { return sym_rec; }
   // Go ahead and say no, won't get called anyway.
   bool cursor_hit(Position x, Position y) const;
   int has_subtree() const;
   Symbol *get_subtree() const { return sym_rec; } 
   void set_subtree(Symbol *p);

   // Import a read-only object into user space so they can modify it.
   void import_object();

   void draw_links(bool erase);
   void lift() {lifted_ = true;}
   void unlift() {lifted_ = false;}
   bool is_lifted() { return lifted_; }
   void handle_events();
   void bypass_events();
   screen_object *dup(bool entire_tree);
   void unhook_leaving_visible_children();

   bool verify_can_delete() const;
   bool verify_can_copy() const;
   bool verify_can_link() const;


   // Glyph methods
   int num_columns() const {return num_cols;}

   // constructor: create objects
   glyph(Symbol *sym, bool detail, Location *loc);
   glyph(const glyph& a);
   glyph& operator=(const glyph& a);
   ~glyph();

   // attach_glyph: called to place a new glyph and pop it up
   void attach_glyph();

   slot_record get_slot_info(int slot)
   {
      assert(slot >= 0 && slot < num_slots);
      return slot_info[slot];
   }

   void set_slot_info(int slot,slot_record new_info)
   {
      assert(slot >= 0 && slot < num_slots);
      slot_info[slot] = new_info;
   }

   // updates the symbol tree from the screen glyph.
   void update();

   Widget glyph_widget() const {return _glyph_widget;}
   Widget body_widget(int col) const {return body_widget_[col];}
   Widget down_button() const 
	{ if( has_down_button_ )
	     return down_button_;
	  return NULL;
	}

   bool has_detail() const { return !is_detailed && sym_rec->has_children();}
   bool is_detail() const { return is_detailed;}
   void set_sym_rec(Symbol *rec) { sym_rec = rec; }

   glyph *src_glyph(const int slot) const 
   {
      // Not a connection.
      if (slot_info[num_slots * 0 + slot].list_header || 
	  slot_info[num_slots * 0 + slot].is_const ||
	  slot_info[num_slots * 0 + slot].is_cmds_list)
      {
	 return NULL;
      }

      // Defining record
      Symbol *rec = slot_info[num_slots * 0 + slot].src_symbol;

      // no src
      if( rec == NULL )
	 return NULL;

      // The glyph is hanging off the hardware record
      if( rec->symbol_type == BP_NAME && rec->bound_to )
	 rec = rec->bound_to;

      // source not instantiated.
      if( rec->this_screen_object == NULL )
	 return NULL;

      // The source glyph
      return (glyph *)(rec->this_screen_object);
   }

   Widget find_dest(glyph *src);

   void size(Dimension *height, Dimension *width) const;

   int arrow2slot(Widget src);
   bool is_output_arrow(Widget src) const { return src == output_widget; }
   bool has_output_arrow() const { return output_widget != NULL; }

   // add a connection between this glyph's output and the dest glyph's input
   // slot arrow.
   // Returns true if need to regenerate after hooking up the extra list slot
   bool add_connection(screen_object *dest, int slot_num);

   // Convert an input arrow to a constant
   void convert_link_to_constant(int slot_num);    

   // Convert a constant to an input arrow 
   void convert_constant_to_link(int slot_num);    

   Symbol *input_parm(int slot) const;

   // return the slot number where the src glyph is connected into this glyph
   // or return -1 if no connection.
   int find_input_slot(glyph *src);

   // Unhook an input connection to a glyph.
   // Deletes the graphic
   void remove_input_connection(int slot_num);

   bool has_sub_tree() const {return has_down_button_; }

   void replace_input_connection(int slot_num, Symbol *new_sym);

   bool add_input_slot(int slot_num);
};


glyph *add_new_prim(Symbol *def, bool detail);
void select_glyph_cb(Widget w, XButtonEvent * event, String * args, int *num_args);


#endif


///////////////////////////////////////////////////////////////////////
// $Log: glyph.hpp,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 22:59:33  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.42  2000/04/13 21:44:06  endo
// This patch extends MissionLab to allow the user to import read-only
// library code.  When the user attempts to change something which is
// read-only, cfgedit pops up a dialog box and asks if it should import
// the object so it can be edited.  If OK'd, it imports the object.
//
// This fixes the problem with FSA's (and other assemblages) not being
// editable when they are loaded from the library.
//
// Revision 1.41  2000/01/20 03:23:05  endo
// Code checked in for doug. A feature for
// cfgedit to allow toggling constants to
// input allows was added.
//
// Revision 1.40  1997/02/14  15:55:46  zchen
// *** empty log message ***
//
// Revision 1.39  1996/05/07  19:17:03  doug
// fixing compile warnings
//
// Revision 1.38  1996/02/28  03:56:24  doug
// *** empty log message ***
//
// Revision 1.37  1996/02/22  00:55:17  doug
// adding popups to fsa states/trans
//
// Revision 1.37  1996/02/22  00:55:17  doug
// adding popups to fsa states/trans
//
// Revision 1.36  1996/02/17  17:05:53  doug
// *** empty log message ***
//
// Revision 1.35  1996/02/16  00:07:18  doug
// *** empty log message ***
//
// Revision 1.34  1996/02/08  19:21:56  doug
// *** empty log message ***
//
// Revision 1.33  1996/02/06  18:16:42  doug
// *** empty log message ***
//
// Revision 1.32  1996/02/04  23:21:52  doug
// *** empty log message ***
//
// Revision 1.31  1996/02/02  03:00:09  doug
// getting closer
//
// Revision 1.30  1996/02/01  04:04:30  doug
// *** empty log message ***
//
// Revision 1.29  1996/01/31  03:06:53  doug
// *** empty log message ***
//
// Revision 1.28  1996/01/30  01:50:19  doug
// *** empty log message ***
//
// Revision 1.27  1996/01/29  00:07:50  doug
// *** empty log message ***
//
// Revision 1.26  1996/01/27  00:09:24  doug
// added grouped lists of parameters
//
// Revision 1.25  1996/01/10  19:34:39  doug
// *** empty log message ***
//
// Revision 1.24  1995/11/21  23:10:50  doug
// *** empty log message ***
//
// Revision 1.23  1995/11/04  23:46:50  doug
// *** empty log message ***
//
// Revision 1.22  1995/10/26  14:59:55  doug
// type checking is working
//
// Revision 1.21  1995/09/19  15:33:19  doug
// The executive module is now working
//
// Revision 1.20  1995/09/01  21:14:27  doug
// linking and copying glyphs works across pages
//
// Revision 1.19  1995/06/29  17:57:04  jmc
// Added header and RCS id and log strings.
///////////////////////////////////////////////////////////////////////
