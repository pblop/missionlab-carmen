/**********************************************************************
 **                                                                  **
 **                      bitmap_icon_displayer.h                     **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This is a header file for bitmap_icon_displayer.cc.             **
 **                                                                  **
 **  Copyright 2007 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef BITMAP_ICON_DISPLAYER_H
#define BITMAP_ICON_DISPLAYER_H

/* $Id: bitmap_icon_displayer.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <Xm/Xm.h>
#include <string.h>
#include <string>
#include <map>

#include "symbol.hpp"
#include "load_rc.h"

using std::string;
using std::map;

class BitmapIconDisplayer {

    typedef struct IconInfo_t {
        string bitmapName;
        string enforcedFgColor;
        string enforcedBgColor;
        Pixmap pixmap;
        Pixel fgPixel;
        Pixel bgPixel;
        GC fgGC;
        GC bgGC;
        Widget iconGadget;
        float labelX;
        float labelY;
        bool iconIsUp;
        bool noBackground;
    };

protected:
    Display *display_;
    XtAppContext appContext_;
    Pixel defaultFgPixel_;
    Pixel defaultBgPixel_;
    GC defaultFgGC_;
    GC defaultBgGC_;
    Widget parentWidget_;
    Widget drawingArea_;
    symbol_table<rc_chain> rcTable_;
    map<string, IconInfo_t> iconTable_;
    bool exposeDrawingAreaCallbackIsActive_;

    static const string BITMAP_DIRECTORY_;
    static const string BITMAP_EXTENSION_;
    static const string EMPTY_STRING_;
    static const int LABEL_Y_OFFSET_;

    Pixmap createPixmap_(
        string bitmapName,
        Pixel fgPixel,
        Pixel bgPixel);

public:
    BitmapIconDisplayer(
        Display *display,
        Widget parent,
        XtAppContext app,
        Widget drawingArea,
        const symbol_table<rc_chain> &rcTable);
    ~BitmapIconDisplayer(void);
    void displayIcon(
        float x,
        float y,
        string bitmapName,
        string label,
        string enforcedFgColor,
        string enforcedBgColor);
    void displayAll(void);
};

extern BitmapIconDisplayer *gBitmapIconDisplayer;

#endif
/**********************************************************************
 * $Log: bitmap_icon_displayer.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1  2007/08/30 18:33:39  endo
 * BitmapIconDisplayer class added.
 *
 **********************************************************************/
