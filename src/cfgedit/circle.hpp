/**********************************************************************
 **                                                                  **
 **                            circle.hpp                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2005, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: circle.hpp,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#ifndef CIRCLE_HPP
#define CIRCLE_HPP

#include "screen_object.hpp"

class transition;

class circle : public screen_object
{
    int x_;
    int y_;
    int r_;
    bool lifted_;
    bool is_start_;
    bool highlighted_;
    Widget name_widget_;
    Position width, height;
    bool managed_;
    Symbol *index_;	// The INDEX_NAME record defining the state
    Symbol *agent_;	// The agent hanging under the state
    Symbol *fsa_rec_;	// The agent defining the fsa
    Symbol *parm_inst_;	// The INPUT_NAME defining the member function

    // slot info
    int num_slots;
    slot_record *slot_info; // Array with num_slots entries

    // The default translations for the description widget, so we can
    // restore if not in show detail mode
    XtTranslations default_translations;

    static const int ARROW_WIDTH_;
    static const double SLOP_DISTANCE_TO_STATE_;

    void DrawTheCircle(int x, int y, GC thisGC, bool fill);

public:
    // Support required by screen_object
    void redraw_unselected();
    void redraw_selected();
    void *base_rec() {return this;} 
    void delete_tree();
    void draw(void);
    void erase();
    void xor_outline(Position x, Position y);
    void get_xy(Position *x, Position *y) const {*x = x_; *y = y_;}
    void set_xy(Position x, Position y);
    void draw_links()  {draw_or_erase_input_links(false);}
    void erase_links() {draw_or_erase_input_links(true);}
    Widget object_widget() const {return name_widget_;}
    Symbol *get_src_sym() const  {return parm_inst_;}

    // Import a read-only object into user space so they can modify it.
    void import_object();

    bool cursor_hit(Position x, Position y) const;
    int has_subtree() const {return 1;}  // Always tree under a state
    Symbol *get_subtree() const { return agent_; }
    void set_subtree(Symbol *p);
    void lift();
    void unlift();
    void handle_events();                                        
    void bypass_events();
    bool is_state()   {return true;}
    bool can_change_agent() const         { return !is_start(); }


    bool verify_can_delete() const;
    bool verify_can_copy() const;
    bool verify_can_link() const;

    screen_object *dup(bool entire_tree);
    void unhook_leaving_visible_children();

    void highlightCircle(bool highlight);
    bool isHighlighted(void);

//********************************************************************
    GTList<transition *> out_links;
    GTList<transition *> in_links;

//********************************************************************
    bool is_lifted() const {return lifted_;}
    void draw_or_erase_input_links(bool erase);

    int x() const { return x_; };
    int y() const { return y_; };
    int r() const { return r_; };
    char *name() { return index_ ? index_->name : (char *)NULL; };

//********************************************************************

    ~circle();
    circle(Symbol *fsa_rec, Symbol *index_rec, bool leave_up);

    void DrawCircle(int x, int y, GC thisGC);
    void update();
    int  radius() const { return r_; }
    char *name() const {return index_->name;}
    Symbol *index() const { return index_; }
    Symbol * create_agent();
    Symbol * fsa_rec() {return fsa_rec_;}
    bool is_start() const {return is_start_;}
};

inline void circle::highlightCircle(bool highlight)
{
    highlighted_ = highlight;
}

inline bool circle::isHighlighted(void)
{
    return highlighted_;
}

#endif


///////////////////////////////////////////////////////////////////////
// $Log: circle.hpp,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.3  2006/05/15 01:23:28  endo
// gcc-3.4 upgrade
//
// Revision 1.2  2005/02/07 22:25:27  endo
// Mods for usability-2004
//
// Revision 1.1.1.1  2005/02/06 22:59:35  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.28  2002/01/12 23:23:16  endo
// Mission Expert functionality added.
//
// Revision 1.27  2000/04/13 21:41:52  endo
// This patch extends MissionLab to allow the user to import read-only
// library code.  When the user attempts to change something which is
// read-only, cfgedit pops up a dialog box and asks if it should import
// the object so it can be edited.  If OK'd, it imports the object.
//
// This fixes the problem with FSA's (and other assemblages) not being
// editable when they are loaded from the library.
//
// Revision 1.26  1997/02/14  15:55:46  zchen
// *** empty log message ***
//
// Revision 1.25  1996/05/07  19:17:03  doug
// fixing compile warnings
//
// Revision 1.24  1996/02/27  05:01:12  doug
// *** empty log message ***
//
// Revision 1.23  1996/02/22  00:55:17  doug
// adding popups to fsa states/trans
//
// Revision 1.22  1996/02/08  19:21:56  doug
// *** empty log message ***
//
// Revision 1.21  1996/02/04  23:21:52  doug
// *** empty log message ***
//
// Revision 1.20  1996/02/02  03:00:09  doug
// getting closer
//
// Revision 1.19  1996/02/01  04:04:30  doug
// *** empty log message ***
//
// Revision 1.18  1996/01/31  03:06:53  doug
// *** empty log message ***
//
// Revision 1.17  1996/01/30  01:50:19  doug
// *** empty log message ***
//
// Revision 1.16  1996/01/29  00:07:50  doug
// *** empty log message ***
//
// Revision 1.15  1995/11/12  22:39:46  doug
// *** empty log message ***
//
// Revision 1.14  1995/06/29  17:30:25  jmc
// Added header and RCS id and log strings.
///////////////////////////////////////////////////////////////////////
