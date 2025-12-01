/**********************************************************************
 **                                                                  **
 **                     screen_object.hpp                            **
 **                                                                  **
 **  base class for glyphs, states, and transitions                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: screen_object.hpp,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#ifndef SCREEN_OBJECT_HPP
#define SCREEN_OBJECT_HPP

#include "slot_record.hpp"

/************************************************************************
*		screen_object class            				*
*************************************************************************/

class screen_object
{
public:  //***********************************************************

           screen_object() 				{};
   virtual ~screen_object() 				{};
   virtual void *base_rec()	 			= 0;
   virtual void redraw_unselected() 			= 0;
   virtual void redraw_selected() 			= 0;
   virtual void delete_tree() 				= 0;

   // If the object is not lifted, draw it in its final form
   virtual void draw() 					= 0;

   // Erase the object from the screen
   virtual void erase() 				= 0;

   // lift an object so it doesn't get drawn on expose events
   virtual void lift() 					= 0;

   // replace an object so it gets drawn on expose events
   virtual void unlift() 				= 0;

   // XOR an outline of the object at the specified spot
   virtual void xor_outline(Position x, Position y) 	= 0;

   // report the screen location of the object
   virtual void get_xy(Position *x, Position *y) const	= 0;

   // report the screen location of the object's output
   virtual void get_output_xy(Position *x, Position *y) const { get_xy(x,y); }

   // set the screen location of the object
   virtual void set_xy(Position x, Position y) 		= 0;

   // Draw input connections to this object
   virtual void draw_links()				= 0;

   // Erase input connections to this object
   virtual void erase_links()				= 0;

   // convert a widget to an object.
   virtual Widget object_widget() const			= 0;

   // return the symbol defining this object
   virtual Symbol *get_src_sym() const			= 0;

   // check if the cursor is near enough to this object to report a hit.
   virtual bool cursor_hit(Position x, Position y)const = 0;

   // check if there are hidden children to show
   // Returns 1 if there is a subtree
   //         2 if there is an implementation file
   //         0 if there is nothing to show.
   virtual int has_subtree() const 			= 0;

   // return the child to move into
   virtual Symbol *get_subtree() const 			= 0;

	// Set a new child
   virtual void set_subtree(Symbol *p)                  = 0;

   // Import a read-only object into user space so they can modify it.
   virtual void import_object()           = 0;

   virtual void handle_events()				= 0;
   virtual void bypass_events()				= 0;

   virtual bool verify_can_delete() const		{return true;}
   virtual bool verify_can_copy() const			{return true;}
   virtual bool verify_can_link() const			{return true;}

   virtual screen_object *dup(bool entire_tree)		= 0;
   virtual void unhook_leaving_visible_children()	= 0;

   virtual slot_record get_slot_info(int /*slot*/)			 
   {
      abort();
   }
   virtual void set_slot_info(int /*slot*/, slot_record /*new_info*/)	 
   { 
      abort();
   }
   virtual bool add_connection(screen_object * /*dest*/, int /*slot_num*/)
   { 
      abort();
   }
   virtual void remove_input_connection(int /*slot_num*/)
   { 
      abort();
   }
   virtual bool add_input_slot(int /*slot_num*/)
   { 
      abort();
   }
   virtual void convert_link_to_constant(int /*slot_num*/)
   { 
      abort();
   }
   virtual void convert_constant_to_link(int /*slot_num*/)
   { 
      abort();
   }
   virtual bool is_state()                   		         {return false;}
   virtual bool can_change_agent() const       		         {return false;}
};

#endif

///////////////////////////////////////////////////////////////////////
// $Log: screen_object.hpp,v $
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
// Revision 1.16  2000/04/13 21:48:52  endo
// This patch extends MissionLab to allow the user to import read-only
// library code.  When the user attempts to change something which is
// read-only, cfgedit pops up a dialog box and asks if it should import
// the object so it can be edited.  If OK'd, it imports the object.
//
// This fixes the problem with FSA's (and other assemblages) not being
// editable when they are loaded from the library.
//
// Revision 1.15  2000/01/20 03:27:17  endo
// Code checked in for doug. A feature for
// cfgedit to allow toggling constants to
// input allows was added.
//
// Revision 1.14  1997/02/14  15:55:46  zchen
// *** empty log message ***
//
// Revision 1.13  1996/05/07  19:17:03  doug
// fixing compile warnings
//
// Revision 1.12  1996/02/28  03:56:24  doug
// *** empty log message ***
//
// Revision 1.11  1996/02/27  05:01:12  doug
// *** empty log message ***
//
// Revision 1.10  1996/02/22  00:55:17  doug
// adding popups to fsa states/trans
//
// Revision 1.9  1996/02/16  00:07:18  doug
// *** empty log message ***
//
// Revision 1.8  1996/02/08  19:21:56  doug
// *** empty log message ***
//
// Revision 1.7  1996/02/06  18:16:42  doug
// *** empty log message ***
//
// Revision 1.6  1996/02/04  23:21:52  doug
// *** empty log message ***
//
// Revision 1.5  1996/02/02  03:00:09  doug
// getting closer
//
// Revision 1.4  1996/02/01  04:04:30  doug
// *** empty log message ***
//
// Revision 1.3  1996/01/31  03:06:53  doug
// *** empty log message ***
//
// Revision 1.2  1996/01/30  01:50:19  doug
// *** empty log message ***
//
// Revision 1.1  1996/01/29  00:07:50  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
