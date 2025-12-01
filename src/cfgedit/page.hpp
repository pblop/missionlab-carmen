/**********************************************************************
 **                                                                  **
 **                             page.hpp                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: page.hpp,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#ifndef PAGE_HPP
#define PAGE_HPP

// Defines the page class

/************************************************************************
*			page class            				*
*************************************************************************/
class Page
{
   Symbol        *agent;		// Agent defining this page
   bool          _moving_glyph; 	// Redraw stuff needs to know when we move one
   bool          skip_update_;  	// Don't update recs when delete page
   class fsa    *fsa_node_;		// Remember the fsa, if it happens to be one.
 
   void Create_tree(Symbol * new_agent, Location *loc);

public:  //***********************************************************
   // constructor: create a new page based on the agent,
   // but, init the glyphs list first.
   Page(Symbol *agent);

   // destructor
   ~Page();

   void draw_links();
   void refresh();
   bool moving_glyph() const {return _moving_glyph;}
   void starting_to_move_glyph() {_moving_glyph = true;}
   void finished_moving_glyph() {_moving_glyph = false;}
   void erase();
   void update();
   Symbol *this_agent() const { return agent; }
   void    set_agent(Symbol *new_agent) { agent = new_agent; }
   fsa    *this_fsa() const { return fsa_node_; }

   screen_object *widget2obj(Widget w);
   screen_object *symbol2obj(Symbol *rec);
   bool is_fsa() const { return fsa_node_ != NULL; }
   void group(Symbol *node);
   void skip_update() { skip_update_ = true; }
   bool is_on_this_page(Symbol *sym);
};


#endif


///////////////////////////////////////////////////////////////////////
// $Log: page.hpp,v $
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
// Revision 1.26  1997/02/14  15:55:46  zchen
// *** empty log message ***
//
// Revision 1.25  1996/01/31  03:06:53  doug
// *** empty log message ***
//
// Revision 1.24  1996/01/30  01:50:19  doug
// *** empty log message ***
//
// Revision 1.23  1996/01/29  00:07:50  doug
// *** empty log message ***
//
// Revision 1.22  1995/11/21  23:10:50  doug
// *** empty log message ***
//
// Revision 1.21  1995/10/27  20:28:50  doug
// *** empty log message ***
//
// Revision 1.20  1995/10/06  21:45:38  doug
// *** empty log message ***
//
// Revision 1.19  1995/09/26  21:43:49  doug
// working on the group function
//
// Revision 1.18  1995/09/01  21:14:27  doug
// linking and copying glyphs works across pages
//
// Revision 1.17  1995/06/29  18:06:04  jmc
// Added header and RCS id and log strings.
///////////////////////////////////////////////////////////////////////
