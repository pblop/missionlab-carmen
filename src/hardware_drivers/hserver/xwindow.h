/**********************************************************************
 **                                                                  **
 **                              xwindow.h                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  X11 Interface for HServer                                       **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: xwindow.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef XWINDOW_H
#define XWINDOW_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include "module.h"

void xwindowShowWindow();
void xwindowConnect();
void xwindowPutImage(XImage *ximg,int x,int y,int w,int h);
void xwindowPutRGB(int x,int y,int w,int h, unsigned char *data);
void xwindowDrawRect(int x,int y,int w,int h,int c);
void xwindowDrawPoint(int x,int y,int c);

#define XWINDOW_MAX_COLORS 10
#define XWINDOW_NUM_COLORS 5
#define COL_BLACK    0
#define COL_WHITE    1
#define COL_RED      2
#define COL_GREEN    3
#define COL_BLUE     4

class Xwindow : public Module {

protected:
    pthread_t handler;
    Display *display;
    int screen_num;
    Window win;
    GC gc;
    unsigned int width,height;
    int x,y;
    XSizeHints *size_hints;
    int x_depth;
    int num_colors;
    unsigned int colors[XWINDOW_MAX_COLORS];

    static const int SKIP_STATUSBAR_UPDATE_;
    static const int READER_THREAD_USLEEP_;

    int loadColors();
    unsigned int RGBtoColor(int r,int g,int b);
    XImage *RGBToXImage(int w,int h,unsigned char *data);
    void create(char* name, int xp, int yp, int w, int h);
    int handleKeyboard(XKeyEvent *report);
    void drawRect(int x,int y,int w,int h,int c);
    void drawPoint(int x,int y,int c);
    void putImage(XImage *ximg,int x,int y,int w,int h);
    void updateStatusBar_(void);
  public:
    void putRGB(int x,int y,int w,int h, unsigned char *data);
    void handler_thread();
    void show();
    void control();
    Xwindow(Xwindow **a);
    ~Xwindow();
};

extern Xwindow *xwindow;

#endif

/**********************************************************************
# $Log: xwindow.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.3  2004/09/10 19:43:28  endo
# New PoseCalc integrated.
#
# Revision 1.2  2004/04/24 11:20:53  endo
# 3DM-G added.
#
# Revision 1.1.1.1  2004/02/27 22:30:51  endo
# Initial MARS 2020 revision
#
# Revision 1.3  2000/09/19 03:51:29  endo
# RCS log added.
#
#
#**********************************************************************/
