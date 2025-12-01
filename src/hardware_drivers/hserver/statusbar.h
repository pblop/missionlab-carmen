/**********************************************************************
 **                                                                  **
 **                            statusbar.h                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  Status bar updates for HServer                                  **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: statusbar.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef STATUS_BAR_H
#define STATUS_BAR_H

#include <panel.h>

#define STATUSBAR_MAX_LINES 25

class Statusbar {

protected:
    int size;
    int index[STATUSBAR_MAX_LINES];
    int num_lines;
    char *lines[STATUSBAR_MAX_LINES];
    WINDOW *window;
    PANEL  *panel;
    char *empty_line;
public:
    void resize();
    void draw(void);
    void update(int line_idx);
    int add(char *line);
    void remove(int line_idx);
    Statusbar(int size);
};

class StatusbarSpinner {

protected:
    int counter_;

public:
    StatusbarSpinner(void);
    ~StatusbarSpinner(void);
    char getStatus(void);
};


extern Statusbar *statusbar;

#endif

/**********************************************************************
# $Log: statusbar.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.3  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2004/09/10 19:43:28  endo
# New PoseCalc integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:51  endo
# Initial MARS 2020 revision
#
# Revision 1.2  2000/09/19 03:49:46  endo
# RCS log added.
#
#
#**********************************************************************/
