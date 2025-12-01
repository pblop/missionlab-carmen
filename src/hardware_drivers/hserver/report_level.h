/**********************************************************************
 **                                                                  **
 **                            report_level.h                        **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: report_level.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef REPORT_LEVEL_H_
#define REPORT_LEVEL_H_

#include <panel.h>

#define REPORTLEVEL_WIN_COL                   31
#define REPORTLEVEL_WIN_ROW                   17
#define REPORTLEVEL_WIN_X                     8
#define REPORTLEVEL_WIN_Y                     5        

#define REPORT_LEVEL_MAX_LINES 20

class ReportLevel
{
  private:
    // int size; Ernest -variable not used
    char index[REPORT_LEVEL_MAX_LINES];
    int num_lines,curSelect;
    int *lines[REPORT_LEVEL_MAX_LINES];
    int maxNameLen;
    char *names[REPORT_LEVEL_MAX_LINES];
    int panX,panY,panW,panH;
    WINDOW *window;
    PANEL *panel;
    char *levStr(int report_level);
    int drawWindow();
    int runKey(int c);
    void capInt(int *i,int l,int u);
    void movePanel(int xdif,int ydif);
    void movePanelKey(int c);
    void adjust(int adj, int set = -1);

  public:
    void show();
    int add(char *name,int *line);
    void remove(int line_idx);
    ReportLevel();
    ~ReportLevel();

};

extern ReportLevel *reportlevel;

#endif

/**********************************************************************
# $Log: report_level.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.1.1.1  2005/02/06 22:59:42  endo
# AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
#
# Revision 1.4  2002/01/16 22:23:28  ernest
# Added destructor; remove unused "size" variable
#
# Revision 1.3  2000/09/19 03:47:14  endo
# RCS log added.
#
#
#**********************************************************************/
