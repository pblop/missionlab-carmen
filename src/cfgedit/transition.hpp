/**********************************************************************
 **                                                                  **
 **                          transition.hpp                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: transition.hpp,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#ifndef TRANSITION_HPP
#define TRANSITION_HPP

class circle;

class transition : public screen_object
{
    circle *src_;
    circle *dest_;
    Symbol *inst_;
    Widget  rule_widget_;
    Position width_, height_;	// Size of glyph
    bool moved_;			// Is location non-default?
    bool managed_;
    bool lifted_;

    // cache the location of several screen points
    Position last_src_x_, last_src_y_;
    Position last_des_x_, last_des_y_;
    Position last_p1_x_,  last_p1_y_;
    Position last_p2_x_,  last_p2_y_;
    Position x_, y_;			// Location of glyph
    Position last_bubble_x_, last_bubble_y_;
    short    last_bubble_t1_, last_bubble_t2_;

    static const int BUBBLE_RADIUS_;
    static const int ARROW_LENGTH_;    

    // Update the above location caches
    void compute_locations();

    // The default translations for the description widget, so we can
    // restore if not in show detail mode
    XtTranslations default_translations;

public:
    // Support required by screen_object
    void redraw_unselected();
    void redraw_selected();
    void *base_rec() {return this;} 
    void delete_tree();
    void draw();
    void erase();
    void xor_outline(Position x, Position y);
    void get_xy(Position *x, Position *y) const 		{*x = x_; *y = y_;}
    void set_xy(Position x, Position y);
    void draw_links()  				{}
    void erase_links() 				{}
    Widget object_widget() const 			{return rule_widget_;}
    Symbol *get_src_sym() const  				{return inst_;}

    // Import a read-only object into user space so they can modify it.
    void import_object();

    bool cursor_hit(Position x, Position y) const;
    int has_subtree() const 	{ return (get_subtree() != NULL) ? 1 : 0;}
    Symbol *get_subtree() const;
    void set_subtree(Symbol *p);
    void lift() 						{lifted_ = true;} 
    void unlift() 					{lifted_ = false;}
    bool is_lifted() const 				{return lifted_;}
    void handle_events();
    void bypass_events();
    screen_object *dup(bool entire_tree);        
    void unhook_leaving_visible_children();
    void set_default_xy();
    bool can_change_agent() const;


//********************************************************************

    ~transition();
    transition(circle *src, circle *dest, Symbol *inst);
    void update();
    Symbol *the_agent() const 
        { 
            if( inst_ && inst_->input_generator )
                return inst_->input_generator->input_generator;
            else
                return NULL;
        }
    Symbol * create_agent();
    Widget rule_widget() const { return rule_widget_; }
    circle *dest() const {return dest_;}
    Symbol *inst() const { return inst_; }
};

#endif


///////////////////////////////////////////////////////////////////////
// $Log: transition.hpp,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.2  2005/02/07 22:25:27  endo
// Mods for usability-2004
//
// Revision 1.1.1.1  2005/02/06 22:59:34  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.19  2000/04/13 21:50:10  endo
// Checked in for Doug.
// This patch extends MissionLab to allow the user to import read-only
// library code.  When the user attempts to change something which is
// read-only, cfgedit pops up a dialog box and asks if it should import
// the object so it can be edited.  If OK'd, it imports the object.
//
// This fixes the problem with FSA's (and other assemblages) not being
// editable when they are loaded from the library.
//
// Revision 1.18  2000/02/10 04:44:18  endo
// Checked in for Douglas C. MacKenzie.
// He got rid of warning signs upon compilation.
//
// Revision 1.17  1997/02/14 15:55:46  zchen
// *** empty log message ***
//
// Revision 1.16  1996/02/28  03:56:24  doug
// *** empty log message ***
//
// Revision 1.15  1996/02/27  05:01:12  doug
// *** empty log message ***
//
// Revision 1.14  1996/02/22  00:55:17  doug
// adding popups to fsa states/trans
//
// Revision 1.13  1996/02/16  00:07:18  doug
// *** empty log message ***
//
// Revision 1.12  1996/02/08  19:21:56  doug
// *** empty log message ***
//
// Revision 1.11  1996/02/04  23:21:52  doug
// *** empty log message ***
//
// Revision 1.10  1996/02/02  03:00:09  doug
// getting closer
//
// Revision 1.9  1996/02/01  04:04:30  doug
// *** empty log message ***
//
// Revision 1.8  1996/01/31  03:06:53  doug
// *** empty log message ***
//
// Revision 1.7  1996/01/29  00:07:50  doug
// *** empty log message ***
//
// Revision 1.6  1995/07/06  20:38:14  doug
// *** empty log message ***
//
// Revision 1.5  1995/06/29  18:15:28  jmc
// Added header and RCS id and log strings.
///////////////////////////////////////////////////////////////////////
