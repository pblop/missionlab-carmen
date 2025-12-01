/**********************************************************************
 **                                                                  **
 **                             x_utils.h                            **
 **                                                                  **
 **                                                                  **
 **  Written by: Yoichiro Endo                                       **
 **                                                                  **
 **  Copyright 2003 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: x_utils.h,v 1.1.1.1 2008/07/14 16:44:25 endo Exp $ */

#ifndef X_UTILS_H
#define X_UTILS_H

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

#include <Xm/Xm.h>

GC createGC(
    Pixel fg,
    Pixel bg,
    Widget w,
    int lineWidth,
    bool setArcMode,
    bool setXor);

Pixel createPixel(Display *display, Colormap colormap, char *colorName);

Font createAndAppendFont(
    Display *display,
    char *fontName,
    char *fontTag,
    XmFontList *fontList);

#endif
/**********************************************************************
 * $Log: x_utils.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:25  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2005/02/07 19:09:12  endo
 * *** empty log message ***
 *
 **********************************************************************/
