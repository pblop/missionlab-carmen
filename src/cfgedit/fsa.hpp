/**********************************************************************
 **                                                                  **
 **                             fsa.hpp                              **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: fsa.hpp,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#ifndef FSA_HPP
#define FSA_HPP

#include <string>

using std::string;

class circle;
class transition;
class screen_object;

class fsa {
    bool dragging_;
    int cur_cursor_x, cur_cursor_y;
    int offset_x, offset_y;
    circle *drag_state;
    symbol_table<circle> states;
    Symbol *src_symbol_;
    Symbol *rule_list_;
    Symbol *member_list_;
    bool adding_transition_;
    circle *trans_start_;

public:
    void erase();
    void draw(bool erase = false);
    void update();
    ~fsa();
    fsa(Symbol *node);
    int getNumberOfStatesInFSA(void);
    bool closest_state (int x, int y, circle **state);
    bool are_dragging() const { return dragging_; }
    void start_moving(int x, int y);
    void done_moving(int x, int y);
    void include_state(circle *p);
    circle *add_state(Symbol *old_state = NULL);
    circle *dup_state(Symbol *old_state);
    void adding_transition() {adding_transition_ = true; }
    bool are_adding_trans() { return adding_transition_; }
    void clicked(int x, int y);
    Symbol *members() const { return member_list_; }
    Symbol *rules() const { return rule_list_; }
    Symbol *fsa_node() const { return src_symbol_; }
    bool unhook_transition(transition *trans);
    bool delete_transition(transition *trans);
    bool delete_state(circle *the_circle);
    bool unhook_state(circle *the_circle);
    screen_object *add_transition(screen_object *src, screen_object *des);
    bool ok_to_add_outlink(screen_object *s);
    bool is_start_state(screen_object *s);
    circle *getState(string stateName);
    void clearHighlightStates(void);
};

#endif


/**********************************************************************
 * $Log: fsa.hpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:54  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2005/02/07 22:25:27  endo
 * Mods for usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 22:59:33  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.23  2000/03/31 17:07:53  endo
 * getNumberOfStatesInFSA(void) added.
 *
 * Revision 1.22  1997/02/14 15:55:46  zchen
 * *** empty log message ***
 *
 * Revision 1.21  1996/10/04  20:58:17  doug
 * changes to get to version 1.0c
 *
 * Revision 1.21  1996/10/02 21:45:20  doug
 * working on pushup in states and transitions
 *
 * Revision 1.20  1996/03/13  01:55:23  doug
 * *** empty log message ***
 *
 * Revision 1.19  1996/02/16  00:07:18  doug
 * *** empty log message ***
 *
 * Revision 1.18  1996/02/04  23:21:52  doug
 * *** empty log message ***
 *
 * Revision 1.17  1996/02/02  03:00:09  doug
 * getting closer
 *
 * Revision 1.16  1996/02/01  04:04:30  doug
 * *** empty log message ***
 *
 * Revision 1.15  1996/01/31  03:06:53  doug
 * *** empty log message ***
 *
 * Revision 1.14  1996/01/29  00:07:50  doug
 * *** empty log message ***
 *
 * Revision 1.13  1995/11/12  22:39:46  doug
 * *** empty log message ***
 *
 * Revision 1.12  1995/06/29  17:48:13  jmc
 * Added header and RCS id and log strings.
 **********************************************************************/
